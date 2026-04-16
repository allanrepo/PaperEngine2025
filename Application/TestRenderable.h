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


using namespace engine;
using namespace engine::graphics;

namespace TestRenderable
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

#pragma region // map layer

	class MapLayer
	{
	private:
		TileGrid<IRenderable> m_tilemap;                       // your tile layer
		ObjectGrid<TileConstraint, IRenderable> m_objectmap;      // objects that sit on top
		ConstraintGrid m_constraintmap;                        // movement/pathfinding
		Tileset<IRenderable>* m_tileset;                      // reference for tile data and auto tile resolver
		AutoTileResolver m_autoTileResolver;                      // resolves tile index based on surrounding tiles for auto tiling

	public:
		MapLayer(AutoTileResolver::AutoTileConfig& config) :
			m_autoTileResolver(
				[this](const Coord& coord) -> bool { return m_tilemap.IsInBounds(coord);  },
				[this](const Coord& coord) -> int { return m_tilemap.Get(coord).GetIndex();  },
				[this](const Coord& coord, int index) { m_tilemap.Set(coord, m_tileset->MakeTile(index)); },
				config
				)
		{
		}

#pragma region // bound checks. references to tilemap for bound checks since tilemap is the base layer and all layers should have the same dimension
		bool IsInBounds(int row, int col) const
		{
			return m_tilemap.IsInBounds(row, col);
		}

		bool IsInBounds(const engine::spatial::Coord& coord) const
		{
			return m_tilemap.IsInBounds(coord);
		}
#pragma endregion

#pragma region // size query. references to tilemap for size query since tilemap is the base layer and all layers should have the same dimension
		// returns grid width
		size_t GetWidth() const
		{
			return m_tilemap.GetWidth();
		}

		// returns grid height. includes last row even if it is incomplete
		size_t GetHeight() const
		{
			return m_tilemap.GetHeight();
		}

		spatial::Size<size_t> GetSize() const
		{
			return m_tilemap.GetSize();
		}

		bool IsEmpty() const
		{
			return m_tilemap.IsEmpty();
		}
#pragma endregion

		void Initialize(size_t width, size_t height, const Tile<IRenderable>& data)
		{
			m_tilemap.Initialize(width, height, data);  
			m_objectmap.Initialize(width, height);
			m_constraintmap.Initialize(width, height, TileConstraint::NONE);
		}

		// remove any existing object in the key and set the new object. then update constraint based on the new object
		void SetObject(const Coord& coord, const TileConstraint& key, std::unique_ptr<IRenderable> obj)
		{
			m_objectmap.Add(coord, key, std::move(obj));
			m_constraintmap.Set(coord, key);
		}

#pragma region // accessors
		Tile<IRenderable>& GetTile(int row, int col)
		{
			return m_tilemap.Get(row, col);
		}

		const Tile<IRenderable>& GetTile(int row, int col) const
		{
			return m_tilemap.Get(row, col);
		}

		// retrieves the data at Coord
		Tile<IRenderable>& GetTile(const engine::spatial::Coord& coord)
		{
			return m_tilemap.Get(coord.row, coord.col);
		}

		// retrieves the data at Coord
		const Tile<IRenderable>& GetTile(const engine::spatial::Coord& coord) const
		{
			return m_tilemap.Get(coord.row, coord.col);
		}
#pragma endregion


		//TileGrid<IRenderable>& TileMap() { return m_tilemap; }
		//ObjectGrid<std::string, IRenderable>& ObjectMap() { return m_objectmap; }
		//ConstraintGrid& ConstraintMap() { return m_constraintmap; }
	};
#pragma endregion

#pragma region // map resolver
	template<typename T>
	class AutoTileResolver1
	{
	public:
		struct AutoTileConfig
		{
			engine::container::Dictionary<int, TileVariant> indexToVariant;
			engine::container::Dictionary<TileVariant, int> variantToIndex;

			void Register(int index, TileVariant variant)
			{
				indexToVariant[index] = variant;
				variantToIndex[variant] = index;
			}

			bool HasIndex(int index) const
			{
				return indexToVariant.Has(index);
			}

			bool HasVariant(TileVariant variant) const
			{
				return variantToIndex.Has(variant);
			}

			TileVariant ToVariant(int index) const
			{
				return indexToVariant[index];
			}

			int ToIndex(TileVariant variant) const
			{
				return variantToIndex[variant];
			}
		};

	private:
		AutoTileConfig* m_autoTileConfig;

		TileVariant ResolveTileVariant(int mask)
		{
			switch (mask)
			{
			case 0:   return TileVariant::Island;		// surrounded by nothing
			case 15:  return TileVariant::Full;			// surrounded by same tile type on all 4 sides

			case 8:   return TileVariant::NorthEdge;	// same tile type on north only. nothing on south, east, west
			case 2:   return TileVariant::SouthEdge;	// same tile type on south only. nothing on north, east, west
			case 1:   return TileVariant::EastEdge;		// same tile type on east only. nothing on north, south, west
			case 4:   return TileVariant::WestEdge;		// same tile type on west only. nothing on north, south, east

			case 10:  return TileVariant::Vertical;		// same tile type on north+south. nothing on east, west
			case 5:   return TileVariant::Horizontal;	// same tile type on east+west. nothing on north, south

			case 7:   return TileVariant::TNorth;		// same tile type on south+east+west. nothing on north
			case 13:  return TileVariant::TSouth;		// same tile type on north+east+west. nothing on south
			case 14:  return TileVariant::TEast;		// same tile type on north+south+west. nothing on east	
			case 11:  return TileVariant::TWest;		// same tile type on north+south+east. nothing on west

			case 6: return TileVariant::NECorner;		// same tile type on north+east. nothing on south, west 
			case 3: return TileVariant::NWCorner;		// same tile type on north+west. nothing on south, east
			case 12: return TileVariant::SECorner;		// same tile type on south+east. nothing on north, west
			case 9: return TileVariant::SWCorner;		// same tile type on south+west. nothing on north, east

			default:  return TileVariant::Empty;		// default to empty tile if mask configuration not found. this should not happen if we cover all cases.
			}
		}

		void UpdateMask(const engine::spatial::Coord& coord, unsigned int& mask, unsigned int bit, int index)
		{
			if (m_autoTileConfig->indexToVariant.Has(index) && m_autoTileConfig->indexToVariant[index] != TileVariant::Empty)
			{
				mask |= bit;
			}
		}

		unsigned int ComputeMask(const engine::spatial::Coord& coord, int index)
		{
			unsigned int mask = 0;

			UpdateMask({ coord.row - 1, coord.col }, mask, 8, index);	// N
			UpdateMask({ coord.row + 1, coord.col }, mask, 2, index);	// S
			UpdateMask({ coord.row, coord.col + 1 }, mask, 4, index);	// E
			UpdateMask({ coord.row, coord.col - 1 }, mask, 1, index);	// W

			return mask;
		}

		void PlaceTile(const engine::spatial::Coord& coord, TileGrid<T>& grid, const Tileset<T>& set, const TileVariant type, int index)
		{
			// Set the selected tile
			grid.Set(coord, set.MakeTile(m_autoTileConfig->variantToIndex[type]));

			// Notify listeners about the tile variant change
			TileVariantChangedEvent(coord, type, index);
		}

		void ResolveNeighbors(const engine::spatial::Coord& coord, TileGrid<T>& grid, const Tileset<T>& set)
		{
			// Update self + 4 neighbors (skip diagonals)
			for (int dr = -1; dr <= 1; ++dr)
			{
				for (int dc = -1; dc <= 1; ++dc)
				{
					// Skip corners (diagonals)
					if (std::abs(dr) + std::abs(dc) > 1) continue;

					// neighbor tile coords
					engine::spatial::Coord neighborCoord = { coord.row + dr, coord.col + dc };

					// if neighbor tile is out of bounds or not walkable, skip it.
					if (!grid.IsInBounds(neighborCoord)) continue;

					// if this is the tile we just placed, so we already know its new variant. skip it since we don't need to recompute it.
					if (dr == 0 && dc == 0) continue;

					// if tile exists but is empty tile, skip it since empty tile is like "air" and doesn't affect autotiling of neighbors
					int index = grid.Get(neighborCoord).GetIndex();
					if (m_autoTileConfig->indexToVariant.Has(index) && m_autoTileConfig->indexToVariant[index] == TileVariant::Empty) continue;

					// evaluate this neighbor if this it of same tile type as the one we just placed. if not, skip it since its tile variant won't be affected by the new tile.
					if (!m_autoTileConfig->indexToVariant.Has(index)) continue;

					unsigned int mask = ComputeMask(neighborCoord);
					TileVariant variant = ResolveTileVariant(mask);

					TileVariant currVariant = m_autoTileConfig->indexToVariant[index];
					if (currVariant != variant)
					{
						// Set the selected tile
						PlaceTile(neighborCoord, variant, index);
					}

					//PlaceTile(neighborCoord, variant);
				}
			}
		}

	public:
		AutoTileResolver1(
			AutoTileConfig& autoTileConfig
		) :
			m_autoTileConfig(&autoTileConfig)
		{
		}

		AutoTileResolver1(const AutoTileResolver1&) = delete;
		AutoTileResolver1& operator=(const AutoTileResolver1&) = delete;
		AutoTileResolver1(AutoTileResolver1&&) = delete;
		AutoTileResolver1& operator=(AutoTileResolver1&&) = delete;

		int Get(TileVariant variant)
		{
			return m_autoTileConfig->variantToIndex[variant];
		}

		TileVariant Get(int index)
		{
			return m_autoTileConfig->indexToVariant[index];
		}

		bool Has(TileVariant variant)
		{
			return m_autoTileConfig->variantToIndex.Has(variant);
		}

		bool Has(int index)
		{
			return m_autoTileConfig->indexToVariant.Has(index);
		}

		void Register(int index, TileVariant variant)
		{
			m_autoTileConfig->indexToVariant[index] = variant;
			m_autoTileConfig->variantToIndex[variant] = index;
		}

		virtual ~AutoTileResolver1()
		{
			TileVariantChangedEvent.Clear();
		}

		void Set(const engine::spatial::Coord& coord, TileGrid<T>& grid, const Tileset<T>& set)
		{
			// quick check if coord is within bounds
			if (!grid.IsInBounds(coord)) return;

			// get the variant of the current tile in the coord
			int currIndex = grid.Get(coord).GetIndex();

			// evaluates cardinal neighboors of this coord to check if they are set as tiles
			unsigned int mask = ComputeMask(coord, currIndex);

			// get the variant of the current tile in the coord
			int currIndex = grid.Get(coord).GetIndex();
			if (!m_autoTileConfig->indexToVariant.Has(currIndex)) return;
			TileVariant currVariant = m_autoTileConfig->indexToVariant[currIndex];

			// decide which variant of tile this coord is going to be
			TileVariant variant = ResolveTileVariant(mask);

			// if current variant is same from replacement variant, don't bother setting it.
			if (currVariant != variant)
			{
				// Set the selected tile
				PlaceTile(coord, variant, currIndex);
			}

			// (TODO: always, but find out why) update neighbors to ensure seamless transitions
			ResolveNeighbors(coord, grid, set);
		}

		void Remove(const engine::spatial::Coord& coord, TileGrid<T>& grid, const Tileset<T>& set)
		{
			// quick check if coord is within bounds
			if (!grid.IsInBounds(coord)) return;

			// if we don't have an empty tile registered, we can't remove. just return early.
			if (!m_autoTileConfig->variantToIndex.Has(TileVariant::Empty)) return;

			// get the variant of the current tile in the coord
			int currIndex = grid.Get(coord).GetIndex();
			if (!m_autoTileConfig->indexToVariant.Has(currIndex)) return;
			TileVariant currVariant = m_autoTileConfig->indexToVariant[currIndex];

			// if tile is already empty, no need to remove anymore.
			if (currVariant != TileVariant::Empty)
			{
				// remove the selected tile
				PlaceTile(coord, TileVariant::Empty, currIndex);
			}

			// update neighbors to ensure seamless transitions
			ResolveNeighbors(coord);
		}

		void Set(engine::spatial::Size<size_t> size, TileGrid<T>& grid, const Tileset<T>& set)
		{
			for (int row = 0; row < size.height; row++)
			{
				for (int col = 0; col < size.width; col++)
				{
					Set(engine::spatial::Coord(row, col), grid, set);
				}
			}
		}

		void Remove(engine::spatial::Size<size_t> size, TileGrid<T>& grid, const Tileset<T>& set)
		{
			for (int row = 0; row < size.height; row++)
			{
				for (int col = 0; col < size.width; col++)
				{
					Remove(engine::spatial::Coord(row, col), grid, set);
				}
			}
		}

		engine::event::Event<const engine::spatial::Coord&, TileVariant, int> TileVariantChangedEvent;
	};
#pragma endregion

#pragma region // tile map
	class TileMap
	{
	private:
		std::vector<MapLayer> m_layers;
		engine::spatial::Size<size_t> m_size;

	public:

	};
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

			// setup ceiling map
			{
				// create ceiling grid and load with default tiles
				Registry<TileGrid<IRenderable>>::Instance().Register("ceiling", make_unique<TileGrid<IRenderable>>());
				TileGrid<IRenderable>& tilegrid = Registry<TileGrid<IRenderable>>::Instance().Get("ceiling");

				// initialize with empty tiles
				Size<size_t>& mapsize = Registry<Size<size_t>>::Instance().Get("map_size");
				Tileset<IRenderable>& tileset = Registry<Tileset<IRenderable>>::Instance().Get("tile");
				tilegrid.Initialize(mapsize, tileset.MakeTile(4));

				// create auto tile config for ceiling map and configure it. this will be used by ceiling auto tile resolver to determine tile variant based on surrounding tiles
				Registry<AutoTileResolver::AutoTileConfig>::Instance().Register("ceiling", make_unique<AutoTileResolver::AutoTileConfig>());
				AutoTileResolver::AutoTileConfig& config = Registry<AutoTileResolver::AutoTileConfig>::Instance().Get("ceiling");

				// configure ceiling map auto-tile mapping
				config.Register(4, TileVariant::Empty);
				config.Register(35, TileVariant::Island);
				config.Register(15, TileVariant::Full);
				config.Register(26, TileVariant::NorthEdge);
				config.Register(8, TileVariant::SouthEdge);
				config.Register(34, TileVariant::EastEdge);
				config.Register(32, TileVariant::WestEdge);
				config.Register(5, TileVariant::NECorner);
				config.Register(7, TileVariant::NWCorner);
				config.Register(23, TileVariant::SECorner);
				config.Register(25, TileVariant::SWCorner);
				config.Register(17, TileVariant::Vertical);
				config.Register(33, TileVariant::Horizontal);
				config.Register(6, TileVariant::TNorth);
				config.Register(24, TileVariant::TSouth);
				config.Register(14, TileVariant::TEast);
				config.Register(16, TileVariant::TWest);

				// create lookup tile resolver for land map. this will be used to determine tile variant based on surrounding tiles
				Registry<AutoTileResolver>::Instance().Register("ceiling", make_unique<AutoTileResolver>(
					[&tilegrid](const Coord& coord) -> bool { return tilegrid.IsInBounds(coord);  },
					[&tilegrid](const Coord& coord) -> int { return tilegrid.Get(coord).GetIndex();  },
					[&tilegrid, &tileset](const Coord& coord, int index) { tilegrid.Set(coord, tileset.MakeTile(index)); },
					config
				));
			}

			// chain the tile's auto resolver with ceiling's auto resolver. when ceiling is set to a tile, the floor must also set to tile
			{
				// get references to auto tile resolvers
				AutoTileResolver& ceilingautoresolver = Registry<AutoTileResolver>::Instance().Get("ceiling");
				AutoTileResolver& tileautoresolver = Registry<AutoTileResolver>::Instance().Get("tile");

				// tile's autoresolver subscribes to ceiling's and set its tile too when ceiling sets its tile
				ceilingautoresolver.TileVariantChangedEvent += engine::event::Handler(std::function<void(const Coord&, TileVariant)>(
					[&tileautoresolver](const Coord& c, TileVariant tv)
					{
						if (tv != TileVariant::Empty)
						{
							tileautoresolver.Set(c);
						}
					}));
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

			// setup wall prop placement based on ceiling tile variant. when there is a ceiling tile, we want to place a wall prop. when there is no ceiling tile, we want to remove the wall prop
			{
				ISpriteAtlas& atlas = Registry<ISpriteAtlas>::Instance().Get("tile");
				ObjectGrid<TileConstraint, IRenderable>& props = Registry<ObjectGrid<TileConstraint, IRenderable>>::Instance().Get("props");

				// create lookup resolver for our wall props. we want wall to automatically be placed when a ceiling tile is placed
				// we also want resolver to decide which type of wall is placed e.g. center, edge, island.
				Registry<LookupResolver<const Coord&, TileVariant, int>>::Instance().Register("wall", std::make_unique<LookupResolver<const Coord&, TileVariant, int>>());
				LookupResolver<const Coord&, TileVariant, int>& resolver = Registry<LookupResolver<const Coord&, TileVariant, int>>::Instance().Get("wall");

				resolver.Register(TileVariant::Empty, -1);
				resolver.Register(TileVariant::Island, 44);
				resolver.Register(TileVariant::Full, -1);
				resolver.Register(TileVariant::NorthEdge, 44);
				resolver.Register(TileVariant::SouthEdge, -1);
				resolver.Register(TileVariant::EastEdge, 43);
				resolver.Register(TileVariant::WestEdge, 41);
				resolver.Register(TileVariant::NECorner, -1);
				resolver.Register(TileVariant::NWCorner, -1);
				resolver.Register(TileVariant::SECorner, 41);
				resolver.Register(TileVariant::SWCorner, 43);
				resolver.Register(TileVariant::Vertical, -1);
				resolver.Register(TileVariant::Horizontal, 42);
				resolver.Register(TileVariant::TNorth, -1);
				resolver.Register(TileVariant::TSouth, 42);
				resolver.Register(TileVariant::TEast, -1);
				resolver.Register(TileVariant::TWest, -1);

				AutoTileResolver& ceilingautoresolver = Registry<AutoTileResolver>::Instance().Get("ceiling");
				ceilingautoresolver.TileVariantChangedEvent += engine::event::Handler(&resolver, &LookupResolver<const Coord&, TileVariant, int>::Resolve);

				resolver.LookupEvent += engine::event::Handler(std::function<void(const Coord&, int)>(
					[&props, &atlas](const Coord& c, int i)
					{
						// note that this event handler will only be invoked if ceiling tile really change. e.g. if ceiling does not exist
						// and is to be removed, then there is nothing to remove, and this event handler does not get invoked. if placing 
						
						// it is ok to just blindly clear this cell of any props. if there is a ceiling, it means there is no other props other than wall.
						// if there is no ceiling, it means we are placing one. that means we need to remove whatever props is in it.
						
						// a ceiling tile, clear cell of any props
						props.Clear(c);

						// if index is invalid, cieling is likely being removed
						if (i < 0 || i >= atlas.GetUVRectCount())
						{
							// bail out. no need to set anything on this cell
							return;
						}

						// then place the ceiling
						props.Add(c, TileConstraint::SW, std::make_unique<Renderable>(atlas.MakeSprite(i, PositionF{ 0.0f, 1.0f })));
					})
				);
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
						constraintmap.Set(row, col, floorResolver.Get(index) == TileVariant::Empty? TileConstraint::BLOCKED: TileConstraint::NONE);
					}
				}

				// also append constraint based on ceiling 
				TileGrid<IRenderable>& ceiling = Registry<TileGrid<IRenderable>>::Instance().Get("ceiling");
				for (int row = 0; row < (int)mapsize.height; row++)
				{
					for (int col = 0; col < (int)mapsize.width; col++)
					{
						Tile<IRenderable> tile = floor.Get(row, col);
						int index = tile.GetIndex();
						constraintmap.AddFlag(row, col, floorResolver.Get(index) == TileVariant::Empty ? TileConstraint::BLOCKED : TileConstraint::NONE);
					}
				}

				m_startTile = { 0,0 };
				m_endTile = { 0,0 };

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
			AutoTileResolver& ceilresolver = Registry<AutoTileResolver>::Instance().Get("ceiling");
			engine::spatial::Coord coord = engine::spatial::PositionToCoord(m_mousePos - mapPos, tilesize);
			ObjectGrid<TileConstraint, IRenderable>& props = Registry<ObjectGrid<TileConstraint, IRenderable>>::Instance().Get("props");
			AnimationSet& animset = Registry<AnimationSet>::Instance().Get("tree");
			ConstraintGrid& constraintmap = Registry<ConstraintGrid>::Instance().Get("constraints");
			Size<size_t>& mapsize = Registry<Size<size_t>>::Instance().Get("map_size");
			TileGrid<IRenderable>& tilegrid = Registry<TileGrid<IRenderable>>::Instance().Get("tile");

			if (!tilegrid.IsInBounds(coord))
			{
				// if out of bounds, ignore input
				return;
			}

			switch (key)
			{
			case 27: // ESC
				props.Clear();
				ceilresolver.Remove(mapsize);
				tileresolver.Remove(mapsize);
				constraintmap.Fill(TileConstraint::BLOCKED); // nothing is walkable since we remove all floor tile
				constraintmap.FindPath(m_startTile, m_endTile, m_path);
				break;
			case 32: // SPACE
				props.Clear();
				ceilresolver.Remove(mapsize);
				tileresolver.Set(mapsize);
				constraintmap.Fill(TileConstraint::NONE); // nothing is walkable since we remove all floor tile
				break;
			case 49: // 1
				// remove anything on this tile
				ceilresolver.Remove(coord);
				props.Clear(coord);

				// set floor tile and this is fully passable
				tileresolver.Set(coord);
				constraintmap.Set(coord, TileConstraint::NONE);

				// update path
				constraintmap.FindPath(m_startTile, m_endTile, m_path);
				break;

			case 50: // 2
				// set ceiling. the resolver will not place it if it already is placed before
				ceilresolver.Set(coord);

				// with ceiling, this tile is fully blocked
				constraintmap.Set(coord, TileConstraint::BLOCKED);

				// update path
				constraintmap.FindPath(m_startTile, m_endTile, m_path);

				break;
			case 51: // 3 
				// remove ceiling if any then add tree at center of tile
				tileresolver.Set(coord);
				ceilresolver.Remove(coord);
				props.Set(coord, TileConstraint::CENTER, std::make_unique<Animated>(animset, "idle"));

				constraintmap.Set(coord, TileConstraint::CENTER);
				constraintmap.FindPath(m_startTile, m_endTile, m_path);

				break;
			case 52: // 4
				break;
			case 53: // 5
				break;
			case 54: // 6
				// remove everything in the tile
				tileresolver.Remove(coord);
				ceilresolver.Remove(coord);
				props.Clear(coord);

				// water tile now so not passable
				constraintmap.Set(coord, TileConstraint::BLOCKED);
				constraintmap.FindPath(m_startTile, m_endTile, m_path);

				break;
			case 55: // 7
				// just remove ceiling
				ceilresolver.Remove(coord);

				// since we still have floor tile, it should be fully passable
				constraintmap.Set(coord, TileConstraint::NONE);
				constraintmap.FindPath(m_startTile, m_endTile, m_path);

				break;
			case 56: // 8
				// just remove prop excluding wall
				props.Clear(coord, TileConstraint::CENTER);

				// since we still have floor tile, it should be fully passable
				constraintmap.Set(coord, TileConstraint::NONE);
				constraintmap.FindPath(m_startTile, m_endTile, m_path);
				break;
			default:
				break;
			}
		}

		void OnMouseDown(int btn, int x, int y)
		{
			PositionF mapPos = Registry<PositionF>::Instance().Get("map_position");
			SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");
			engine::spatial::Coord coord = engine::spatial::PositionToCoord(m_mousePos - mapPos, tilesize);

			// if click is out of bounds, ignore
			ConstraintGrid& constraintmap = Registry<ConstraintGrid>::Instance().Get("constraints");
			if (!constraintmap.IsInBounds(coord)) return;

			if (btn == 1)
			{
				m_startTile = coord;
				constraintmap.FindPath(m_startTile, m_endTile, m_path);
				return;
			}
			else if (btn == 2)
			{
				m_endTile = coord;
				constraintmap.FindPath(m_startTile, m_endTile, m_path);
				return;
			}

			return;
		}

		void OnMouseMove(int x, int y)
		{
			m_mousePos = PositionF((float)x, (float)y);

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
				TileGrid<IRenderable>& ceilgrid = Registry<TileGrid<IRenderable>>::Instance().Get("ceiling");
				TileGrid<IRenderable>& splashgrid = Registry<TileGrid<IRenderable>>::Instance().Get("splash");
				ObjectGrid<TileConstraint, IRenderable>& props = Registry<ObjectGrid<TileConstraint, IRenderable>>::Instance().Get("props");


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


				for (int row = 0; row < (int)mapsize.height; row++)
				{
					m_drawQueue.Clear();

					for (int col = 0; col < (int)mapsize.width; col++)
					{
						QueueDrawCommand(tilegrid, m_drawQueue, row, col, tilesize, pos, 1.0f, { 1,1,1,1 }, { 1, 1 });
						// TODO: should the depth be substracted like this? we already pass another "depth" which valued now as 2. isn't this confusing???
						QueueDrawCommand(ceilgrid, m_drawQueue, row, col, tilesize, pos - depth, 2.0f, { 1,1,1,1 }, { 1, 1 });

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


						//props.ForEach(row, col, [&pos, &tilesize, &row, &col, &depth, this](IRenderable* renderable, TileConstraint key)
						//	{
						//		Sprite sprite = renderable->GetSprite();
						//		PositionF translated = pos;

						//		if (key == TileConstraint::CENTER)
						//		{
						//			// translate position so that the prop's anchor is at the center of the tile									
						//			translated.x += tilesize.width / 2.0f;
						//			translated.y += tilesize.height / 2.0f;

						//			// get the top-left position of this tile in world (tilemap) coordinate.
						//			translated.x += col * tilesize.width;
						//			translated.y += row * tilesize.height;
						//		}
						//		else if (key == TileConstraint::SW)
						//		{
						//			// translate position so that prop's anchor is at south-west corner of the tile
						//			translated.y += tilesize.height;

						//			// get the top-left position of this tile in world (tilemap) coordinate.
						//			translated.x += col * tilesize.width;
						//			translated.y += row * tilesize.height;
						//		}
						//		else
						//		{
						//			return;
						//		}
						//		m_drawQueue.Add({
						//			sprite,
						//			translated,   // world (tilemap)
						//			sprite.GetSize(),
						//			{ 1,1,1,1 },
						//			0.0f,
						//			depth.y
						//			});
						//	});

					}

					m_drawQueue.Sort();
					m_drawQueue.Execute(*m_renderer);
				}

				std::vector<engine::spatial::Coord> wp = engine::navigation::tile::GetWayPoints(m_path);
				engine::graphics::navigation::DrawWaypoints(*m_renderer, wp, tilesize, pos, { 1,1,1,1 }, 6.0f);


				std::string msg = "Animators (Cache): " + std::to_string(AnimationSystemCache::Instance().Size());
				m_renderer->Draw(Registry<IFontAtlas>::Instance().Get("font"), msg, { 600, 5 }, { 1,1,1,1 });

				engine::spatial::Coord coord = engine::spatial::PositionToCoord(m_mousePos - pos, tilesize);
				ConstraintGrid& constraintmap = Registry<ConstraintGrid>::Instance().Get("constraints");
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
					tilesize.width* scale.x,
					tilesize.height* scale.y
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