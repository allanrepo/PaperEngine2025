#pragma once

#include <Spatial/ObjectGrid.h>
#include <Win32/Window.h>
#include <Core/Event.h>
#include <Utilities/Logger.h>
#include <Graphics/Core/ICanvas.h>
#include <Graphics/Core/Canvas.h>
#include <Graphics/Core/DX11CanvasImpl.h>
#include <Graphics/Renderer/IRenderer.h>
#include <Graphics/Renderer/DX11RendererBatchImpl.h>
#include <Graphics/Renderer/DX11RendererImmediateImpl.h>
#include <Graphics/Renderer/Renderer.h>
#include <Graphics/Resource/ISpriteAtlas.h>
#include <Engine/Factory/SpriteAtlasFactory.h>
#include <Graphics/Core/Sprite.h>
#include <Core/Input.h>
#include <Graphics/Resource/IFontAtlas.h>
#include <Graphics/Resource/FontAtlas.h>
#include <Graphics/Resource/SpriteAtlas.h>
#include <Cache/Registry.h>
#include <Graphics/Animation/Animation.h>
#include <Timer/StopWatch.h>
#include <Algorithm/Pathfinding.h>
#include <Engine/Loader/AsyncLoader.h>
#include <Graphics/Core/Primitives.h>
#include <Engine/Graphics/Draw.h>
#include <Spatial/Coord.h>
#include <Engine/Factory/FontFactory.h>
#include <Algorithm/Resolvers.h>
#include <Algorithm/AutoTileResolver.h>
#include <bitset>
#include <Graphics/Core/IRenderable.h>
#include <Graphics/Core/IAnimated.h>
#include <Graphics/Core/Renderable.h>
#include <Graphics/Core/Animated.h>
#include <Components/Tile.h>


namespace TestProp
{
#pragma region // namespaces
	using FontFactory = engine::graphics::factory::FontFactory;
	using IFontAtlas = engine::graphics::resource::IFontAtlas;
	using ISpriteAtlas = engine::graphics::resource::ISpriteAtlas;
	using Sprite = engine::graphics::Sprite;
	using SpriteAtlasFactory = engine::graphics::factory::SpriteAtlasFactory;
	using AnimationFactory = engine::graphics::factory::AnimationFactory;
	using TileConstraint = engine::navigation::tile::TileConstraint;
	using Animator = engine::graphics::animation::Animator<Sprite>;
	using AnimationSet = engine::graphics::animation::AnimationSet<Sprite>;
	using AnimationSystemCache = engine::graphics::animation::AnimationSystemCache<Sprite>;
	using Coord = engine::spatial::Coord;
	using PathFinder = engine::navigation::tile::PathFinder;
	using TileNavigationResolver = engine::navigation::tile::TileNavigationResolver;
	using ITileNavigationResolver = engine::navigation::tile::ITileNavigationResolver;
	using IRenderable = engine::graphics::IRenderable;
	using Animated = engine::graphics::Animated;
	using Renderable = engine::graphics::Renderable;
	using ConstraintGrid = engine::navigation::tile::ConstraintGrid;

	template <typename K, typename T>
	using ObjectGrid = engine::spatial::ObjectGrid<K, T>;

	template<typename T>
	using TileGrid = engine::tile::TileGrid<T>;

	template<typename T>
	using Tile = engine::tile::Tile<T>;

	template<typename T>
	using Tileset = engine::tile::Tileset<T>;

	template<typename T>
	using Rect = engine::math::geometry::Rect<T>;

	template<typename T>
	using Size = engine::spatial::Size<T>;

	using AutoTileResolver = engine::tile1::AutoTileResolver;
	using TileVariant = engine::tile1::TileVariant;

	template<typename T, typename K, typename V>
	using LookupResolver = engine::algorithm::LookupResolver<T, K, V>;

	template<typename Owner>
	using AnimationController = engine::graphics::animation::AnimationController<Sprite, Owner>;

	template<typename T>
	using Registry = engine::cache::Registry<T>;

#pragma endregion

	struct DrawCommand
	{
		engine::graphics::Sprite sprite;              // what to draw
		engine::spatial::PositionF pos;    // world position
		engine::spatial::SizeF size;       // size on screen
		engine::graphics::ColorF tint;     // color modulation
		float rotation;                    // rotation angle
		float depth;
	};


	class DrawQueue
	{
	private:
		std::vector<DrawCommand> m_commands;

	public:
		void Reserve(size_t capacity)
		{
			m_commands.reserve(capacity);
		}

		void Add(const DrawCommand& cmd)
		{
			m_commands.push_back(cmd);
		}

		void Sort()
		{
			std::sort(m_commands.begin(), m_commands.end(),
				[](const DrawCommand& a, const DrawCommand& b)
				{
					// if depth is not same, e.g. lower and higher tile, higher tile (b) has higher depth than lower tile (a). draw lower tile first
					if (a.depth != b.depth) return a.depth < b.depth;

					// if same depth, whichever is farthest from screen(a) gets drawn first. nearest from screen (b) is drawn last
					return a.pos.y < b.pos.y; // depth by Y
				});
		}

		void Execute(engine::graphics::renderer::IRenderer& renderer)
		{
			for (auto& cmd : m_commands)
			{
				renderer.Draw(cmd.sprite, cmd.pos, cmd.size, cmd.tint, cmd.rotation);
			}
		}

		void Clear()
		{
			m_commands.clear();
		}
	};


	class Test
	{
	private:
		std::unique_ptr<win32::Window> m_window;
		std::unique_ptr<ICanvas> m_canvas;
		std::unique_ptr<renderer::IRenderer> m_renderer;
		timer::StopWatch m_stopwatch;
		DrawQueue m_drawQueue;
		PositionF m_mousePos;

		Coord m_startTile;
		Coord m_endTile;
		std::vector<Coord> m_path;

		enum class ActionState
		{
			TilePlacement,
			TreePlacement,
			BuildingPlacement,
			Pathfinding,
		};

		ActionState m_actionState;

	public:
		Test()
		{
			win32::Window::OnInitialize += event::Handler(this, &Test::OnInitialize);
			win32::Window::OnExit += event::Handler(this, &Test::OnExit);
			win32::Window::OnIdle += event::Handler(this, &Test::OnIdle);
			win32::Window::Run();
		}

		// function that will be called just before we enter into message loop
		void OnInitialize()
		{
			// create our window here
			m_window = std::make_unique<win32::Window>();
			m_window->OnClose += event::Handler(this, &Test::OnWindowClose);
			m_window->OnCreate += event::Handler(this, &Test::OnWindowCreate);
			m_window->OnSize += event::Handler(this, &Test::OnWindowSize);
			m_window->Create(L"Test Renderable", 1400, 900);
			m_window->OnWindowMessage += event::Handler(&input::Input::Instance(), &input::Input::ProcessWin32Message);

			input::Input::Instance().KeyDownEvent += event::Handler(this, &Test::OnKeyDown);
			input::Input::Instance().MouseDownEvent += event::Handler(this, &Test::OnMouseDown);
			input::Input::Instance().MouseMoveEvent += event::Handler(this, &Test::OnMouseMove);
		}

		// when window is created. we can now safely create resources dependent on window
		void OnWindowCreate(void* hWnd)
		{
			LOG("Window created...");

			// create dx11 canvas
			m_canvas = std::make_unique<Canvas>(std::make_unique<dx11::DX11CanvasImpl>());
			m_canvas->Initialize(hWnd);
			m_canvas->SetViewPort();
			LOG("Canvas (DX11) created...");

			// create dx11 renderer batched
			m_renderer = std::make_unique<renderer::Renderer>(std::make_unique<dx11::renderer::DX11RendererBatchImpl>());
			m_renderer->Initialize();
			LOG("Renderer Batch (DX11) created...");

			// create font and store in cache
			FontFactory::Create("font", "Terminal", 12);
			LOG("Font atlas (terminal, 12) created...");

			// create sprite atlases and store in cache
			{
				SpriteAtlasFactory::Create("tree", L"../Assets/tree_1x8_1536x192.png", 1, 8); // tree
				SpriteAtlasFactory::Create("pine_tree", L"../Assets/tree_1x8_1536x256.png", 1, 8); // pine tree
			}

			// setup resources for tree item
			{
				// create animation set and store in cache
				Registry<AnimationSet>::Instance().Register("tree", make_unique<AnimationSet>());
				AnimationSet& animset = Registry<AnimationSet>::Instance().Get("tree");

				// get tree atlas
				ISpriteAtlas& atlas = Registry<ISpriteAtlas>::Instance().Get("tree");

				// create animation for trees and store in animation set
				animset.Register("storm", AnimationFactory::Create(atlas, std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7 }, 25.0f, true, PositionF{ 0.5f, 0.85f }));
				animset.Register("idle", AnimationFactory::Create(atlas, std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7 }, 200.0f, true, PositionF{ 0.5f, 0.85f }));
				animset.Register("frozen", AnimationFactory::Create(atlas, std::vector<int>{ 0 }, 1000.0f, true, PositionF{ 0.5f, 0.85f }));
			}

			// set map parameters
			{
				Registry<SizeF>::Instance().Register("tile_size", make_unique<SizeF>(64.0f, 64.0f));
				Registry<PositionF>::Instance().Register("map_position", make_unique<PositionF>(50.0f, 50.0f));
				Registry<Size<size_t>>::Instance().Register("map_size", make_unique<Size<size_t>>(20, 12));
				Registry<PositionF>::Instance().Register("depth", make_unique<PositionF>(0.0f, 64.0f));
			}

			// terrain tile map to demonstrate sprite (static) tiles
			{
				// load sprite atlas for tile
				SpriteAtlasFactory::Create("tile", L"../Assets/576x384px_6x9tile_TileMap.png", 6, 9); // tile

				// create our tileset
				Registry<Tileset<IRenderable>>::Instance().Register("tile", std::make_unique<Tileset<IRenderable>>());
				Tileset<IRenderable>& tileset = Registry<Tileset<IRenderable>>::Instance().Get("tile");

				// each sprite from atlas is a static tile (single frame), so we create tile from each sprite
				ISpriteAtlas& atlas = Registry<ISpriteAtlas>::Instance().Get("tile");
				for (int i = 0; i < atlas.GetUVRectCount(); i++) tileset.Register(i, std::make_unique<Renderable>(atlas.MakeSprite(i)));

				// create tile grid and load with default tiles
				Registry<TileGrid<IRenderable>>::Instance().Register("tile", make_unique<TileGrid<IRenderable>>());
				TileGrid<IRenderable>& tilegrid = Registry<TileGrid<IRenderable>>::Instance().Get("tile");
				Size<size_t>& mapsize = Registry<Size<size_t>>::Instance().Get("map_size");
				tilegrid.Initialize(mapsize, tileset.MakeTile(4));

				// create auto tile config for ceiling layer
				Registry<AutoTileResolver::AutoTileConfig>::Instance().Register("tile", make_unique<AutoTileResolver::AutoTileConfig>());
				AutoTileResolver::AutoTileConfig& config = Registry<AutoTileResolver::AutoTileConfig>::Instance().Get("tile");

				// configure land map auto-tile mapping
				config.Register(4, TileVariant::Empty);
				config.Register(30, TileVariant::Island);
				config.Register(10, TileVariant::Full);
				config.Register(21, TileVariant::NorthEdge);
				config.Register(3, TileVariant::SouthEdge);
				config.Register(29, TileVariant::EastEdge);
				config.Register(27, TileVariant::WestEdge);
				config.Register(0, TileVariant::NECorner);
				config.Register(2, TileVariant::NWCorner);
				config.Register(18, TileVariant::SECorner);
				config.Register(20, TileVariant::SWCorner);
				config.Register(12, TileVariant::Vertical);
				config.Register(28, TileVariant::Horizontal);
				config.Register(1, TileVariant::TNorth);
				config.Register(19, TileVariant::TSouth);
				config.Register(9, TileVariant::TEast);
				config.Register(11, TileVariant::TWest);

				// create lookup tile resolver for land map. this will be used to determine tile variant based on surrounding tiles
				Registry<AutoTileResolver>::Instance().Register("tile", make_unique<AutoTileResolver>(
					[&tilegrid](const Coord& coord) -> bool { return tilegrid.IsInBounds(coord);  },
					[&tilegrid](const Coord& coord) -> int { return tilegrid.Get(coord).GetIndex();  },
					[&tilegrid, &tileset](const Coord& coord, int index) { tilegrid.Set(coord, tileset.MakeTile(index)); },
					config
				));
			}

			// water splash tile map to demonstrate animated tiles
			{
				// create sprite atlas for the water splash animation
				engine::graphics::factory::SpriteAtlasFactory::Create("splash", L"../Assets/3072x192px_1x17tile_waterfoam.png", 1, 16);
				engine::graphics::resource::ISpriteAtlas& atlas = engine::cache::Registry<engine::graphics::resource::ISpriteAtlas>::Instance().Get("splash");

				// create animation set to store our splash animation 
				Registry<AnimationSet>::Instance().Register("splash", make_unique<AnimationSet>());
				AnimationSet& animset = Registry<AnimationSet>::Instance().Get("splash");

				// create animation for splash and store in animation set
				animset.Register("splash", AnimationFactory::Create(atlas, 100.0f, true, { .33f, .355f }));

				// create our tileset
				Registry<Tileset<IRenderable>>::Instance().Register("splash", std::make_unique<Tileset<IRenderable>>());
				Tileset<IRenderable>& tileset = Registry<Tileset<IRenderable>>::Instance().Get("splash");

				// register our lone "splash" animation as Animated object in tileset
				tileset.Register(0, std::make_unique<Animated>(animset, "splash"));

				// create tile grid and load with default tiles
				Registry<TileGrid<IRenderable>>::Instance().Register("splash", make_unique<TileGrid<IRenderable>>());
				TileGrid<IRenderable>& tilegrid = Registry<TileGrid<IRenderable>>::Instance().Get("splash");
				Size<size_t>& mapsize = Registry<Size<size_t>>::Instance().Get("map_size");
				tilegrid.Initialize(mapsize, tileset.MakeTile(-1));

				// create resolver. this will map our splash tiles into floor edge tiles
				Registry<LookupResolver<const Coord&, TileVariant, int>>::Instance().Register("splash", std::make_unique<LookupResolver<const Coord&, TileVariant, int >>());
				LookupResolver<const Coord&, TileVariant, int>& splashresolver = Registry<LookupResolver<const Coord&, TileVariant, int>>::Instance().Get("splash");

				// map tile index to tile variant for splash map. this will be used by auto-tile resolver to determine tile variant based on surrounding tiles
				splashresolver.Register(TileVariant::Empty, -1);
				splashresolver.Register(TileVariant::Island, 0);
				splashresolver.Register(TileVariant::Full, -1);
				splashresolver.Register(TileVariant::NorthEdge, 0);
				splashresolver.Register(TileVariant::SouthEdge, 0);
				splashresolver.Register(TileVariant::EastEdge, 0);
				splashresolver.Register(TileVariant::WestEdge, 0);
				splashresolver.Register(TileVariant::NECorner, 0);
				splashresolver.Register(TileVariant::NWCorner, 0);
				splashresolver.Register(TileVariant::SECorner, 0);
				splashresolver.Register(TileVariant::SWCorner, 0);
				splashresolver.Register(TileVariant::Vertical, 0);
				splashresolver.Register(TileVariant::Horizontal, 0);
				splashresolver.Register(TileVariant::TNorth, 0);
				splashresolver.Register(TileVariant::TSouth, 0);
				splashresolver.Register(TileVariant::TEast, 0);
				splashresolver.Register(TileVariant::TWest, 0);

				// setup event handler to handle tile update on splash map when tile map changes
				splashresolver.LookupEvent += engine::event::Handler(std::function<void(const Coord&, int)>(
					[&tilegrid, &tileset](const Coord& c, int i)
					{
						tilegrid.Set(c, tileset.MakeTile(i));
					})
				);

				// let splash tile resolver subscribe to land auto-tile map so that it can update splash tile variants when land tiles change
				AutoTileResolver& resolver = Registry<AutoTileResolver>::Instance().Get("tile");
				resolver.TileVariantChangedEvent += engine::event::Handler(&splashresolver, &LookupResolver<const Coord&, TileVariant, int>::Resolve);
			}

			// setup props map (ObjectGrid) to demonstrate prop placement with tile constraints
			{
				// create a tree animation set and store in cache
				Registry<AnimationSet>::Instance().Register("tree", make_unique<AnimationSet>());
				AnimationSet& animset = Registry<AnimationSet>::Instance().Get("tree");

				// get tree atlas
				ISpriteAtlas& atlas = Registry<ISpriteAtlas>::Instance().Get("tree");

				// create animation for trees and store in animation set
				animset.Register("storm", AnimationFactory::Create(atlas, std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7 }, 25.0f, true, PositionF{ 0.5f, 0.85f }));
				animset.Register("idle", AnimationFactory::Create(atlas, std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7 }, 200.0f, true, PositionF{ 0.5f, 0.85f }));
				animset.Register("frozen", AnimationFactory::Create(atlas, std::vector<int>{ 0 }, 1000.0f, true, PositionF{ 0.5f, 0.85f }));

				// create object grid named props
				Registry<ObjectGrid<TileConstraint, IRenderable>>::Instance().Register("props", make_unique<ObjectGrid<TileConstraint, IRenderable>>());
				ObjectGrid<TileConstraint, IRenderable>& props = Registry<ObjectGrid<TileConstraint, IRenderable>>::Instance().Get("props");

				Size<size_t>& mapsize = Registry<Size<size_t>>::Instance().Get("map_size");
				props.Initialize(mapsize);
			}

			// setup constraint grid
			{
				// create constraint map
				Registry<ConstraintGrid>::Instance().Register("constraints", make_unique<ConstraintGrid>());
				ConstraintGrid& constraintmap = Registry<ConstraintGrid>::Instance().Get("constraints");

				// set its size and fill with NONE
				Size<size_t>& mapsize = Registry<Size<size_t>>::Instance().Get("map_size");
				constraintmap.Initialize(mapsize, TileConstraint::NONE);

				// set constraint based on floor tile variant
				TileGrid<IRenderable>& floor = Registry<TileGrid<IRenderable>>::Instance().Get("tile");
				AutoTileResolver& floorResolver = Registry<AutoTileResolver>::Instance().Get("tile");
				for (int row = 0; row < (int)mapsize.height; row++)
				{
					for (int col = 0; col < (int)mapsize.width; col++)
					{
						Tile<IRenderable> tile = floor.Get(row, col);
						int index = tile.GetIndex();
						constraintmap.Set(row, col, floorResolver.Get(index) == TileVariant::Empty ? TileConstraint::BLOCKED : TileConstraint::NONE);
					}
				}

				m_startTile = { 0,0 };
				m_endTile = { 0,0 };

			}

			// set initial state of the world
			{
				AutoTileResolver& tileresolver = Registry<AutoTileResolver>::Instance().Get("tile");
				ConstraintGrid& constraintmap = Registry<ConstraintGrid>::Instance().Get("constraints");
				Size<size_t>& mapsize = Registry<Size<size_t>>::Instance().Get("map_size");
				tileresolver.Set(mapsize);
				constraintmap.Fill(TileConstraint::NONE); // nothing is walkable since we remove all floor tile

				// default action state is tile placement. we can place/remove floor tiles and the path will update accordingly
				m_actionState = ActionState::TilePlacement;

				// set and store size of building
				Registry<SizeF>::Instance().Register("building_size", make_unique<SizeF>(SizeF{160, 80}));

				Registry<std::vector<Coord>>::Instance().Register("building_coords", make_unique<std::vector<Coord>>());

				Registry<Dictionary<Coord, std::vector<Coord>>>::Instance().Register("tiles_with_subtiles", make_unique<Dictionary<Coord, std::vector<Coord>>>());
			}

			// setup stopwatch to manage timing and start it
			m_stopwatch.OnLap += event::Handler(this, &Test::OnLap);
			m_stopwatch.Start();
		}

		void OnKeyDown(int key)
		{
			PositionF mapPos = Registry<PositionF>::Instance().Get("map_position");
			SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");
			AutoTileResolver& tileresolver = Registry<AutoTileResolver>::Instance().Get("tile");
			engine::spatial::Coord coord = engine::spatial::PositionToCoord(m_mousePos - mapPos, tilesize);
			ObjectGrid<TileConstraint, IRenderable>& props = Registry<ObjectGrid<TileConstraint, IRenderable>>::Instance().Get("props");
			AnimationSet& animset = Registry<AnimationSet>::Instance().Get("tree");
			ConstraintGrid& constraintmap = Registry<ConstraintGrid>::Instance().Get("constraints");
			Size<size_t>& mapsize = Registry<Size<size_t>>::Instance().Get("map_size");
			TileGrid<IRenderable>& tilegrid = Registry<TileGrid<IRenderable>>::Instance().Get("tile");

			if(tilegrid.IsInBounds(coord) == false)
			{
				// if out of bounds, ignore input
				return;
			}

			switch (key)
			{
			case 27: // ESC
				props.Clear();
				tileresolver.Remove(mapsize);
				constraintmap.Fill(TileConstraint::BLOCKED); // nothing is walkable since we remove all floor tile
				constraintmap.FindPath(m_startTile, m_endTile, m_path);
				break;
			case 32: // SPACE
				props.Clear();
				tileresolver.Set(mapsize);
				constraintmap.Fill(TileConstraint::NONE); // nothing is walkable since we remove all floor tile
				break;
			case 49: // 1
				m_actionState = ActionState::TilePlacement;
				break;
			case 50: // 2
				m_actionState = ActionState::TreePlacement;
				break;
			case 51: // 3 
				m_actionState = ActionState::BuildingPlacement;
				break;
			case 52: // 4
				m_actionState = ActionState::Pathfinding;
				break;
			case 53: // 5
				break;
			case 54: // 6
				break;
			case 55: // 7
				break;
			case 56: // 8
				break;
			default:
				break;
			}
		}

		void HandleBuildingPlacement(PositionF pos)
		{
			// get information
			PositionF mapPos = Registry<PositionF>::Instance().Get("map_position");
			SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");

			// shift position such that the building will be placed with its center at the given position
			SizeF buildingSize = Registry<SizeF>::Instance().Get("building_size");
			pos -= PositionF(buildingSize.width / 2.0f, buildingSize.height / 2.0f);

			// get the tile coordinates where the 4 corners of the building footprint intersects
			Coord topLeft = engine::spatial::PositionToCoord(pos - mapPos, tilesize);
			Coord bottomRight = engine::spatial::PositionToCoord(pos + PositionF(buildingSize.width, buildingSize.height) - mapPos, tilesize);

			// check if all 4 corners are on valid floor tiles. if not, we cannot place building here
			ConstraintGrid& constraintmap = Registry<ConstraintGrid>::Instance().Get("constraints");
			if (!constraintmap.IsInBounds(topLeft) ||
				!constraintmap.IsInBounds(bottomRight))
			{
				// out of bounds. cannot place building
				return;
			}

			// get our building coords  list and clear it first 
			std::vector<Coord>& buildingCoords = Registry<std::vector<Coord>>::Instance().Get("building_coords");
			buildingCoords.clear();

			for (int col = topLeft.col; col <= bottomRight.col; col++)
			{
				for (int row = topLeft.row; row <= bottomRight.row; row++)
				{
					buildingCoords.push_back({ row, col });
				}
			}

			SizeF subTileSize = tilesize / 3.0f;
			Coord subTopLeft = engine::spatial::PositionToCoord(pos - mapPos, subTileSize);
			Coord subBottomRight = engine::spatial::PositionToCoord(pos + PositionF(buildingSize.width, buildingSize.height) - mapPos, subTileSize);

			Dictionary<Coord, std::vector<Coord>>& tilesWithSubTiles = Registry<Dictionary<Coord, std::vector<Coord>>>::Instance().Get("tiles_with_subtiles");
			tilesWithSubTiles.Clear();
			for (int col = subTopLeft.col; col <= subBottomRight.col; col++)
			{
				for (int row = subTopLeft.row; row <= subBottomRight.row; row++)
				{
					Coord parentTileCoord = { row / 3, col / 3 };

					// get coord of this sub-tile relative to its parent tile 
					Coord relativeSubTileCoord = {  row % 3, col % 3 };

					tilesWithSubTiles[parentTileCoord].push_back(relativeSubTileCoord);
				}
			}




		}

		void OnMouseDown(int btn, int x, int y)
		{
			// get coordinate of the tile we clicked on
			PositionF mapPos = Registry<PositionF>::Instance().Get("map_position");
			SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");
			engine::spatial::Coord coord = engine::spatial::PositionToCoord(m_mousePos - mapPos, tilesize);


			// if click is out of bounds, ignore
			ConstraintGrid& constraintmap = Registry<ConstraintGrid>::Instance().Get("constraints");
			if (!constraintmap.IsInBounds(coord)) return;

			// what did we click? left button? right button?
			switch (m_actionState)
			{
				case ActionState::TilePlacement:
				{
					AutoTileResolver& tileresolver = Registry<AutoTileResolver>::Instance().Get("tile");
					ObjectGrid<TileConstraint, IRenderable>& props = Registry<ObjectGrid<TileConstraint, IRenderable>>::Instance().Get("props");

					// left click to place tile
					if (btn == 1)
					{
						props.Clear(coord);
						tileresolver.Set(coord);
						constraintmap.Set(coord, TileConstraint::NONE);
					}
					// right click to remove tile
					else if (btn == 2)
					{
						props.Clear(coord);
						tileresolver.Remove(coord);
						constraintmap.Set(coord, TileConstraint::BLOCKED);
					}
					constraintmap.FindPath(m_startTile, m_endTile, m_path);

					break;
				}
				case ActionState::Pathfinding:
				{
					if (btn == 1)
					{
						m_startTile = coord;
					}
					else if (btn == 2)
					{
						m_endTile = coord;
					}
					constraintmap.FindPath(m_startTile, m_endTile, m_path);
					break;
				}
				case ActionState::TreePlacement:
				{
					if (btn == 1)
					{
						TileConstraint constraint = GetConstraintFromPosition(m_mousePos - mapPos, tilesize);

						// make sure there is a floor tile
						AutoTileResolver& tileresolver = Registry<AutoTileResolver>::Instance().Get("tile");
						tileresolver.Set(coord);

						// place the tree prop at the center of the tile
						constraintmap.AddFlag(coord, constraint);
					}
					else if (btn == 2)
					{
						TileConstraint constraint = GetConstraintFromPosition(m_mousePos - mapPos, tilesize);

						constraintmap.RemoveFlag(coord, constraint);
					}
					constraintmap.FindPath(m_startTile, m_endTile, m_path);
					break;
				}
				case ActionState::BuildingPlacement:
				{
					if (btn == 1)
					{
						SizeF buildingSize = Registry<SizeF>::Instance().Get("building_size");
						PositionF pos = m_mousePos;
						pos -= PositionF(buildingSize.width / 2.0f, buildingSize.height / 2.0f);

						// get the tile coordinates where the 4 corners of the building footprint intersects
						Coord topLeft = engine::spatial::PositionToCoord(pos - mapPos, tilesize);
						Coord topRight = engine::spatial::PositionToCoord(pos + PositionF(buildingSize.width, 0) - mapPos, tilesize);
						Coord bottomLeft = engine::spatial::PositionToCoord(pos + PositionF(0, buildingSize.height) - mapPos, tilesize);
						Coord bottomRight = engine::spatial::PositionToCoord(pos + PositionF(buildingSize.width, buildingSize.height) - mapPos, tilesize);

						// check if all 4 corners are on valid floor tiles. if not, we cannot place building here
						if (!constraintmap.IsInBounds(topLeft) ||
							!constraintmap.IsInBounds(topRight) ||
							!constraintmap.IsInBounds(bottomLeft) ||
							!constraintmap.IsInBounds(bottomRight))
						{
							// out of bounds. cannot place building
							break;
						}

						// get our building coords  list and clear it first 
						std::vector<Coord>& buildingCoords = Registry<std::vector<Coord>>::Instance().Get("building_coords");
						buildingCoords.clear();

						for(int col = topLeft.col; col <= bottomRight.col; col++)
						{
							for (int row = topLeft.row; row <= bottomRight.row; row++)
							{
								buildingCoords.push_back({ col, row });
							}
						}

					}
					else if (btn == 2)
					{

					}
					break;
				}
			}
			return;
		}

		TileConstraint GetConstraintFromPosition(const PositionF& pos, const SizeF& tilesize)
		{
			// get coordinate of the tile at this position
			engine::spatial::Coord coord = engine::spatial::PositionToCoord(pos, tilesize);

			// get world position (top-left) of this coordinate 
			engine::spatial::PositionF worldPosThisTile = engine::spatial::CoordToPosition(coord, tilesize);

			// translate the actual world position into this tile's local coordinate (0,0 top-left of the tile, 1,1 bottom-right of the tile)
			PositionF PosInThisTile = (pos - worldPosThisTile);

			// determine tile constraint based on where we are clicking on the tile and return it
			SizeF subTile = tilesize / 3.0f;
			engine::spatial::Coord coordInThisTile = PositionToCoord(PosInThisTile, subTile);

			if(coordInThisTile.col == 0 && coordInThisTile.row == 0)
			{
				return TileConstraint::NW;
			}
			else if(coordInThisTile.col == 1 && coordInThisTile.row == 0)
			{
				return TileConstraint::N;
			}
			else if (coordInThisTile.col == 2 && coordInThisTile.row == 0)
			{
				return TileConstraint::NE;
			}
			else if (coordInThisTile.col == 0 && coordInThisTile.row == 1)
			{
				return TileConstraint::W;
			}
			else if (coordInThisTile.col == 1 && coordInThisTile.row == 1)
			{
				return TileConstraint::CENTER;
			}
			else if (coordInThisTile.col == 2 && coordInThisTile.row == 1)
			{
				return TileConstraint::E;
			}
			else if (coordInThisTile.col == 0 && coordInThisTile.row == 2)
			{
				return TileConstraint::SW;
			}
			else if (coordInThisTile.col == 1 && coordInThisTile.row == 2)
			{
				return TileConstraint::S;
			}
			else if (coordInThisTile.col == 2 && coordInThisTile.row == 2)
			{
				return TileConstraint::SE;
			}
			else
			{
				return TileConstraint::NONE;
			}

		}

		void OnMouseMove(int x, int y)
		{
			m_mousePos = PositionF((float)x, (float)y);

			if(m_actionState == ActionState::BuildingPlacement)
			{
				HandleBuildingPlacement(m_mousePos);
			}

		}

		// this method is fired up whenever the OnLap event is triggered from stopwatch
		void OnLap(double time)
		{
			AnimationSystemCache::Instance().Update(time);
		}

		// fun stuff. this is called on each loop of the message loop. this is where we draw!
		void OnIdle()
		{
			// call lap to get elapsed time and trigger OnLap event
			m_stopwatch.Lap<engine::timer::milliseconds>();

			engine::input::Input::Instance().Update();

			m_canvas->Clear({ 0.2f, 0.2f, 1.0f, 1.0f });

			// start the canvas. we can draw from here
			m_canvas->Begin();
			{
				m_renderer->Begin();

				PositionF pos = Registry<PositionF>::Instance().Get("map_position");
				SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");
				Size<size_t> mapsize = Registry<Size<size_t>>::Instance().Get("map_size");
				PositionF depth = Registry<PositionF>::Instance().Get("depth");
				TileGrid<IRenderable>& tilegrid = Registry<TileGrid<IRenderable>>::Instance().Get("tile");
				TileGrid<IRenderable>& splashgrid = Registry<TileGrid<IRenderable>>::Instance().Get("splash");
				ObjectGrid<TileConstraint, IRenderable>& props = Registry<ObjectGrid<TileConstraint, IRenderable>>::Instance().Get("props");
				ConstraintGrid& constraintmap = Registry<ConstraintGrid>::Instance().Get("constraints");

				// draw tiles in order of their depth (Y) so that tiles with higher Y (lower on the screen) are drawn after 
				// tiles with lower Y (higher on the screen) to create proper overlapping. props will be drawn in between floor 
				// and edge tiles based on their tile constraint, so we draw all floor and edge tiles first, then props, 
				// then debug constraint indicators
				for (int row = 0; row < (int)mapsize.height; row++)
				{
					m_drawQueue.Clear();

					for (int col = 0; col < (int)mapsize.width; col++)
					{
						QueueDrawCommand(splashgrid, m_drawQueue, row, col, tilesize, pos, 1.0f, { 1,1,1,1 }, { 3, 3 });
					}

					m_drawQueue.Sort();
					m_drawQueue.Execute(*m_renderer);
				}

				// draw props but for now this does not do anything
				for (int row = 0; row < (int)mapsize.height; row++)
				{
					m_drawQueue.Clear();

					for (int col = 0; col < (int)mapsize.width; col++)
					{
						QueueDrawCommand(tilegrid, m_drawQueue, row, col, tilesize, pos, 1.0f, { 1,1,1,1 }, { 1, 1 });

						props.ForEach(row, col, TileConstraint::CENTER, [&pos, &tilesize, &row, &col, &depth, this](IRenderable* renderable)
							{
								Sprite sprite = renderable->GetSprite();
								PositionF translated = pos;

								// translate position so that the prop's anchor is at the center of the tile									
								translated.x += tilesize.width / 2.0f;
								translated.y += tilesize.height / 2.0f;

								// get the top-left position of this tile in world (tilemap) coordinate.
								translated.x += col * tilesize.width;
								translated.y += row * tilesize.height;

								m_drawQueue.Add({
									sprite,
									translated,   // world (tilemap)
									sprite.GetSize(),
									{ 1,1,1,1 },
									0.0f,
									depth.y
									});
							});

						props.ForEach(row, col, TileConstraint::SW, [&pos, &tilesize, &row, &col, &depth, this](IRenderable* renderable)
							{
								Sprite sprite = renderable->GetSprite();
								PositionF translated = pos;

								// translate position so that prop's anchor is at south-west corner of the tile
								translated.y += tilesize.height;

								// get the top-left position of this tile in world (tilemap) coordinate.
								translated.x += col * tilesize.width;
								translated.y += row * tilesize.height;

								m_drawQueue.Add({
									sprite,
									translated,   // world (tilemap)
									sprite.GetSize(),
									{ 1,1,1,1 },
									0.0f,
									depth.y
									});
							});
					}

					m_drawQueue.Sort();
					m_drawQueue.Execute(*m_renderer);
				}


				// draw constraint markers
				SizeF subTileSize = tilesize / 3.0f; 
				for (int row = 0; row < (int)mapsize.height; row++)
				{
					for (int col = 0; col < (int)mapsize.width; col++)
					{
						TileConstraint constraint = constraintmap.Get(row, col);

						if((int)(constraint & TileConstraint::CENTER))
						{
							// get the top-left position of this tile in world (tilemap) coordinate.
							PositionF translated = pos;
							translated.x += col * tilesize.width;
							translated.y += row * tilesize.height;

							// translate position so that the prop's anchor is at the center of the tile									
							translated.x += tilesize.width / 2.0f - subTileSize.width / 2.0f;
							translated.y += tilesize.height / 2.0f - subTileSize.height / 2.0f;

							// get the top-left position of this tile in world (tilemap) coordinate.

							m_renderer->Draw(translated, subTileSize, { 1,0,0,1 }, 0.0f); // debug: draw a red dot at the anchor point of the prop

						}

						if ((int)(constraint & TileConstraint::SW))
						{
							// get the top-left position of this tile in world (tilemap) coordinate.
							PositionF translated = pos;
							translated.x += col * tilesize.width;
							translated.y += row * tilesize.height;

							// translate position so that the prop's anchor is at the SW of the tile									
							translated.y += tilesize.height - subTileSize.height;

							m_renderer->Draw(translated, subTileSize, { 1,1,0,1 }, 0.0f); // debug: draw a red dot at the anchor point of the prop
						}

						if ((int)(constraint & TileConstraint::NE))
						{
							// get the top-left position of this tile in world (tilemap) coordinate.
							PositionF translated = pos;
							translated.x += col * tilesize.width;
							translated.y += row * tilesize.height;

							// translate position so that the prop's anchor is at the NE of the tile									
							translated.x += tilesize.width - subTileSize.width;

							m_renderer->Draw(translated, subTileSize, { 1,0,1,1 }, 0.0f); // debug: draw a red dot at the anchor point of the prop
						}

						if ((int)(constraint & TileConstraint::SE))
						{
							// get the top-left position of this tile in world (tilemap) coordinate.
							PositionF translated = pos;
							translated.x += col * tilesize.width;
							translated.y += row * tilesize.height;

							// translate position so that the prop's anchor is at the SE of the tile									
							translated.x += tilesize.width - subTileSize.width;
							translated.y += tilesize.height - subTileSize.height;

							m_renderer->Draw(translated, subTileSize, { 0.5f,.5f,1,1 }, 0.0f); // debug: draw a red dot at the anchor point of the prop
						}


						if ((int)(constraint & TileConstraint::NW))
						{
							// get the top-left position of this tile in world (tilemap) coordinate.
							PositionF translated = pos;
							translated.x += col * tilesize.width;
							translated.y += row * tilesize.height;

							m_renderer->Draw(translated, subTileSize, { 0.5f,0.5f,1,1 }, 0.0f); // debug: draw a red dot at the anchor point of the prop
						}

						if ((int)(constraint & TileConstraint::N))
						{
							// get the top-left position of this tile in world (tilemap) coordinate.
							PositionF translated = pos;
							translated.x += col * tilesize.width;
							translated.y += row * tilesize.height;

							// translate position so that the prop's anchor is at the north of the tile									
							translated.x += tilesize.width / 2.0f - subTileSize.width / 2.0f;

							m_renderer->Draw(translated, subTileSize, { 0.5f,1,0.5f,1 }, 0.0f); // debug: draw a red dot at the anchor point of the prop
						}
						if((int)(constraint & TileConstraint::S))
						{
							// get the top-left position of this tile in world (tilemap) coordinate.
							PositionF translated = pos;
							translated.x += col * tilesize.width;
							translated.y += row * tilesize.height;
							// translate position so that the prop's anchor is at the south of the tile									
							translated.x += tilesize.width / 2.0f - subTileSize.width / 2.0f;
							translated.y += tilesize.height - subTileSize.height;

							m_renderer->Draw(translated, subTileSize, { 0.5f,1,0.5f,1 }, 0.0f); // debug: draw a red dot at the anchor point of the prop
						}
						if((int)(constraint & TileConstraint::E))
						{
							// get the top-left position of this tile in world (tilemap) coordinate.
							PositionF translated = pos;
							translated.x += col * tilesize.width;
							translated.y += row * tilesize.height;
							// translate position so that the prop's anchor is at the east of the tile									
							translated.x += tilesize.width - subTileSize.width;
							translated.y += tilesize.height / 2.0f - subTileSize.height / 2.0f;
							m_renderer->Draw(translated, subTileSize, { 0.5f,1,0.5f,1 }, 0.0f); // debug: draw a red dot at the anchor point of the prop
						}
						if((int)(constraint & TileConstraint::W))
						{
							// get the top-left position of this tile in world (tilemap) coordinate.
							PositionF translated = pos;
							translated.x += col * tilesize.width;
							translated.y += row * tilesize.height;
							// translate position so that the prop's anchor is at the west of the tile									
							translated.y += tilesize.height / 2.0f - subTileSize.height / 2.0f;
							m_renderer->Draw(translated, subTileSize, { 0.5f,1,0.5f,1 }, 0.0f); // debug: draw a red dot at the anchor point of the prop
						}
					}
				}

				// draw path if there is one
				std::vector<engine::spatial::Coord> wp = engine::navigation::tile::GetWayPoints(m_path);
				engine::graphics::navigation::DrawWaypoints(*m_renderer, wp, tilesize, pos, { 1,1,1,0.5F }, 6.0f);

				// if we are placing buildings, draw a preview of the building at the mouse position
				if (m_actionState == ActionState::BuildingPlacement)
				{
					Dictionary<Coord, std::vector<Coord>>& tilesWithSubTiles = Registry<Dictionary<Coord, std::vector<Coord>>>::Instance().Get("tiles_with_subtiles");
					for (auto& kv : tilesWithSubTiles)    
					{
						const Coord& key = kv.first;
						std::vector<Coord>& vec = kv.second;

						PositionF translated = pos;
						translated.x += key.col * tilesize.width;
						translated.y += key.row * tilesize.height;
						m_renderer->Draw(translated, tilesize, { 1,1,0,0.5f }, 0.0f); // debug: draw a red dot at the anchor point of the prop

						for (Coord& subTile: vec)
						{
							PositionF subTranslated = translated;
							subTranslated.x += subTile.col * subTileSize.width;
							subTranslated.y += subTile.row * subTileSize.height;
							subTranslated.x += subTileSize.width / 2.0f;
							subTranslated.y += subTileSize.height / 2.0f;

							SizeF halfSubTileSize = subTileSize / 2.0f;

							m_renderer->Draw(subTranslated, halfSubTileSize, { 1,0,0,0.3f }, 0.0f); // debug: draw a red dot at the anchor point of the prop
						}
					}

					// let's draw the building footprint as well
					SizeF buildingSize = Registry<SizeF>::Instance().Get("building_size");
					PositionF pos = m_mousePos;
					pos -= PositionF(buildingSize.width / 2.0f, buildingSize.height / 2.0f);
					m_renderer->Draw(pos, buildingSize, { 1,1,1,0.5f }, 0.0f); // debug: draw a red dot at the anchor point of the prop
				}

				std::string msg = "Animators (Cache): " + std::to_string(AnimationSystemCache::Instance().Size());
				m_renderer->Draw(Registry<IFontAtlas>::Instance().Get("font"), msg, { 600, 5 }, { 1,1,1,1 });

				engine::spatial::Coord coord = engine::spatial::PositionToCoord(m_mousePos - pos, tilesize);
				if (constraintmap.IsInBounds(coord))
				{
					TileConstraint tc = constraintmap.Get(coord);
					msg.clear();
					msg = "Constraint: " + std::bitset<16>((unsigned int)tc).to_string();
					m_renderer->Draw(Registry<IFontAtlas>::Instance().Get("font"), msg, { 600, 35 }, { 1,1,1,1 });
				}

				m_renderer->End();
			}
			m_canvas->End();
		}

		void OnExit()
		{

		}

		void OnWindowClose()
		{
		}

		void OnWindowSize(size_t nWidth, size_t nHeight)
		{
			LOG("Window resized to: " + std::to_string(nWidth) + ", " + std::to_string(nHeight));
			m_canvas->Resize({ static_cast<unsigned int>(nWidth), static_cast<unsigned int>(nHeight) });
			m_canvas->SetViewPort();
		}

		PositionF GetPos(TileConstraint constraint, SizeF size, PositionF origin)
		{
			Dictionary<TileConstraint, PositionF>& positionLookup = Registry<Dictionary<TileConstraint, PositionF>>::Instance().Get("tree");
			PositionF factor = positionLookup.Get(constraint);

			factor.x *= size.width;
			factor.y *= size.height;

			factor += origin;

			return factor;
		}

		template<typename T>
		void QueueDrawCommand(
			TileGrid<T>& map,
			DrawQueue& queue,
			int row, int col,
			const engine::spatial::SizeF& tilesize,
			const engine::spatial::PositionF& pos,
			float depth,
			const engine::graphics::ColorF& tint = { 1,1,1,1 },
			engine::math::VecF scale = { 1,1 }
		)
		{
			if (!map.IsInBounds(row, col))
			{
				return;
			}

			const Tile<T>& tile = map.Get(row, col);
			if (tile.IsValid())
			{
				engine::spatial::PositionF origin =
				{
					col * tilesize.width,
					row * tilesize.height
				};

				engine::spatial::SizeF scaledtilesize
				{
					tilesize.width * scale.x,
					tilesize.height * scale.y
				};

				queue.Add({
					tile->GetSprite(),
					pos + origin,   // world (tilemap)
					scaledtilesize,
					tint,
					0.0f,
					depth
					});
			}
		}


	};
}