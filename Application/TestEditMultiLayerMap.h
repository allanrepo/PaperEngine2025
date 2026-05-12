#pragma once
#include <Algorithm/AutoTileResolver.h>
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
#include <Engine/Factory/SpriteAnimationFactory.h>
#include "Actor.h"

namespace TestEditMultiLayerMap
{
	using namespace std;
	using namespace engine;
	using namespace engine::graphics;
	using namespace engine::graphics::renderable;
	using namespace engine::graphics::animation;
	using namespace engine::win32;
	using namespace engine::graphics::renderer;
	using namespace engine::timer;
	using namespace engine::input;
	using namespace engine::event;
	using namespace engine::graphics::dx11::renderer;
	using namespace engine::graphics::dx11;
	using namespace engine::component::tile1;
	using namespace engine::graphics::factory;
	using namespace engine::graphics::resource;
	using namespace engine::container;
	using namespace engine::loader::tile;
	using namespace engine::spatial;
	using namespace engine::math;
	using namespace engine::math::geometry;
	using namespace engine::component;
	using namespace engine::graphics::tile;
	using namespace engine::navigation::tile;
	using namespace engine::graphics::navigation;

	template<typename T>
	using Registry = engine::cache::Registry<T>;

	template<typename T>
	using Animation = engine::graphics::animation::Animation<T>;


	// this is a tile definition class, not exactly tile class. tile class is Tile<T> and this is what is assigned to T
	class AnimatedTile
	{
	private:
		engine::graphics::animation::Animator<engine::graphics::Sprite> m_animator;
		std::unordered_map<std::string, engine::graphics::animation::Animation<engine::graphics::Sprite>> m_animations;
		bool m_walkable;
		int m_index;

	public:
		AnimatedTile(bool walkable, const std::string& name, const engine::graphics::animation::Animation<engine::graphics::Sprite>& anim, int index) :
			m_walkable(walkable),
			m_index(index)

		{
			// copy the animation into our container
			m_animations[name] = anim;

			// assign the animation from our container into animator (don't assign the passed animation. that is reference to animation outside which is not safe
			m_animator.Play(m_animations[name]);
		}

		bool IsRunning() const
		{
			return m_animator.IsRunning();
		}

		const engine::graphics::Sprite& GetSprite() const
		{
			return m_animator.GetCurrent();
		}

		void Update(double delta)
		{
			m_animator.Update(delta);
		}

		int GetIndex() const
		{
			return m_index;
		}
	};

	class RenderableTile
	{
	private:
		engine::graphics::Sprite m_sprite;
		bool m_walkable;
		int m_index;

	public:
		RenderableTile(const engine::graphics::Sprite& sprite, bool walkable, int index) :
			m_sprite(sprite),
			m_walkable(walkable),
			m_index(index)
		{
		}

		int GetIndex() const
		{
			return m_index;
		}

		const engine::graphics::Sprite& GetSprite() const
		{
			return m_sprite;
		}

		bool IsWalkable() const
		{
			return m_walkable;
		}
	};


	class Test
	{
	public:


	private:
		std::unique_ptr<Window> m_window;
		std::unique_ptr<ICanvas> m_canvas;
		std::unique_ptr<IRenderer> m_renderer;

		StopWatch m_stopwatch;
		double m_elapsed;

		Input m_input;

		PositionF m_mousePos;

		bool m_toggle;
	public:
		Test():
			m_toggle(true)
		{
			Window::OnInitialize += Handler(this, &Test::OnInitialize);
			Window::OnExit += Handler(this, &Test::OnExit);
			Window::OnIdle += Handler(this, &Test::OnIdle);
			Window::Run();
		}

		// function that will be called just before we enter into message loop
		void OnInitialize()
		{
			// create our window here
			m_window = make_unique<Window>();
			m_window->OnClose += Handler(this, &Test::OnWindowClose);
			m_window->OnCreate += Handler(this, &Test::OnWindowCreate);
			m_window->OnSize += Handler(this, &Test::OnWindowSize);
			m_window->Create(L"TestEditMultiLayerMap", 1400, 900);
			m_window->OnWindowMessage += Handler(&m_input, &Input::ProcessWin32Message);

			m_input.KeyDownEvent += Handler(this, &Test::OnKeyDown);
			m_input.MouseDownEvent += Handler(this, &Test::OnMouseDown);
			m_input.MouseMoveEvent += Handler(this, &Test::OnMouseMove);
		}

		// when window is created. we can now safely create resources dependent on window
		void OnWindowCreate(void* hWnd)
		{
			LOG("Window created...");

			// create dx11 canvas
			m_canvas = make_unique<Canvas>(make_unique<DX11CanvasImpl>());
			m_canvas->Initialize(hWnd);
			m_canvas->SetViewPort();
			LOG("Canvas (DX11) created...");

			// create dx11 renderer batched
			m_renderer = make_unique<Renderer>(make_unique<DX11RendererBatchImpl>());
			m_renderer->Initialize();
			LOG("Renderer Batch (DX11) created...");

			// set map parameters
			{
				Registry<SizeF>::Instance().Register("tile_size", make_unique<SizeF>(64.0f, 64.0f));
				Registry<PositionF>::Instance().Register("map_position", make_unique<PositionF>(50.0f, 50.0f));
				Registry<Size<size_t>>::Instance().Register("map_size", make_unique<Size<size_t>>(20, 12));
				Registry<PositionF>::Instance().Register("layer_height", make_unique<PositionF>(0.0f, 64.0f));
			}

			// setup water tilemap
			{
				// create sprite atlas to be used by tilemap
				SpriteAtlasFactory::Create("1x1_64x64_water_background", L"../Assets/1x1_64x64_water_background.png", 1, 1);
				ISpriteAtlas& atlas = Registry<ISpriteAtlas>::Instance().Get("1x1_64x64_water_background");

				// create our tileset
				Registry<Tileset<RenderableTile>>::Instance().Register("1x1_64x64_water_background", std::make_unique<Tileset<RenderableTile>>());
				Tileset<RenderableTile>& tileset = Registry<Tileset<RenderableTile>>::Instance().Get("1x1_64x64_water_background");

				tileset.Register(0, std::make_unique<RenderableTile>(atlas.MakeSprite(0), false, 0)); // water so not walkable. doesn't matter. this is background map

				// create tile region
				Registry<TileRegion<RenderableTile>>::Instance().Register("1x1_64x64_water_background", make_unique<TileRegion<RenderableTile>>());
				TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("1x1_64x64_water_background");

				// load tile region by filling it with all '0' tile
				Table<string> map({ 20, 12 }, "0");
				AsyncTileRegionLoader<RenderableTile, int> tileRegionLoader;
				tileRegionLoader.LoadImmediate(region, map, [&tileset](const int& cell) -> Tile<RenderableTile> { return tileset.MakeTile(cell); });
			}

			// setup land map
			{
				// create sprite atlas to be used by tilemap
				SpriteAtlasFactory::Create("576x384px_6x9tile_TileMap", L"../Assets/576x384px_6x9tile_TileMap.png", 6, 9);
				ISpriteAtlas& atlas = Registry<ISpriteAtlas>::Instance().Get("576x384px_6x9tile_TileMap");

				// create our tileset
				Registry<Tileset<RenderableTile>>::Instance().Register("576x384px_6x9tile_TileMap", std::make_unique<Tileset<RenderableTile>>());
				Tileset<RenderableTile>& tileset = Registry<Tileset<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");

				// each sprite from atlas is a static tile (single frame), so we create tile from each sprite
				for (int i = 0; i < atlas.GetUVRectCount(); i++)
				{
					tileset.Register(i, std::make_unique<RenderableTile>(atlas.MakeSprite(i), true, i)); // make it all walkable for now
				}

				// create tile region
				Registry<TileRegion<RenderableTile>>::Instance().Register("576x384px_6x9tile_TileMap", make_unique<TileRegion<RenderableTile>>());
				TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");

				// load tile region by filling it with all '4' tile
				Size<size_t> mapsize = Registry<Size<size_t>>::Instance().Get("map_size");
				Table<string> map(mapsize, "4");
				AsyncTileRegionLoader<RenderableTile, int> tileRegionLoader;
				tileRegionLoader.LoadImmediate(region, map, [&tileset](const int& cell) -> Tile<RenderableTile> { return tileset.MakeTile(cell); });

				// create lookup tile resolver for land map. this will be used to determine tile variant based on surrounding tiles
				Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Register("land_map", make_unique<engine::tile::AutoTileResolver<RenderableTile>>(region, tileset));
				engine::tile::AutoTileResolver<RenderableTile>& resolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("land_map");

				// configure land map auto-tile mapping
				resolver.Register(4, engine::tile::TileVariant::Empty);
				resolver.Register(30, engine::tile::TileVariant::Island);
				resolver.Register(10, engine::tile::TileVariant::Full);

				resolver.Register(21, engine::tile::TileVariant::NorthEdge);
				resolver.Register(3, engine::tile::TileVariant::SouthEdge);
				resolver.Register(29, engine::tile::TileVariant::EastEdge);
				resolver.Register(27, engine::tile::TileVariant::WestEdge);

				resolver.Register(0, engine::tile::TileVariant::NECorner);
				resolver.Register(2, engine::tile::TileVariant::NWCorner);
				resolver.Register(18, engine::tile::TileVariant::SECorner);
				resolver.Register(20, engine::tile::TileVariant::SWCorner);

				resolver.Register(12, engine::tile::TileVariant::Vertical);
				resolver.Register(28, engine::tile::TileVariant::Horizontal);

				resolver.Register(1, engine::tile::TileVariant::TNorth);
				resolver.Register(19, engine::tile::TileVariant::TSouth);
				resolver.Register(9, engine::tile::TileVariant::TEast);
				resolver.Register(11, engine::tile::TileVariant::TWest);
			}

			// create hill map
			{
				Tileset<RenderableTile>& tileset = Registry<Tileset<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");

				// create tile region	
				Registry<TileRegion<RenderableTile>>::Instance().Register("hill_map", make_unique<TileRegion<RenderableTile>>());
				TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("hill_map");

				// load tile region by filling it with all '4' tile
				Size<size_t> mapsize = Registry<Size<size_t>>::Instance().Get("map_size");
				Table<string> map(mapsize, "4");
				AsyncTileRegionLoader<RenderableTile, int> tileRegionLoader;
				tileRegionLoader.LoadImmediate(region, map, [&tileset](const int& cell) -> Tile<RenderableTile> { return tileset.MakeTile(cell); });

				// create lookup tile resolver for land map. this will be used to determine tile variant based on surrounding tiles
				Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Register("hill_map", make_unique<engine::tile::AutoTileResolver<RenderableTile>>(region, tileset));
				engine::tile::AutoTileResolver<RenderableTile>& resolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("hill_map");

				// configure hill map auto-tile mapping
				resolver.Register(4, engine::tile::TileVariant::Empty);
				resolver.Register(35, engine::tile::TileVariant::Island);
				resolver.Register(15, engine::tile::TileVariant::Full);

				resolver.Register(26, engine::tile::TileVariant::NorthEdge);
				resolver.Register(8, engine::tile::TileVariant::SouthEdge);
				resolver.Register(34, engine::tile::TileVariant::EastEdge);
				resolver.Register(32, engine::tile::TileVariant::WestEdge);

				resolver.Register(5, engine::tile::TileVariant::NECorner);
				resolver.Register(7, engine::tile::TileVariant::NWCorner);
				resolver.Register(23, engine::tile::TileVariant::SECorner);
				resolver.Register(25, engine::tile::TileVariant::SWCorner);

				resolver.Register(17, engine::tile::TileVariant::Vertical);
				resolver.Register(33, engine::tile::TileVariant::Horizontal);

				resolver.Register(6, engine::tile::TileVariant::TNorth);
				resolver.Register(24, engine::tile::TileVariant::TSouth);
				resolver.Register(14, engine::tile::TileVariant::TEast);
				resolver.Register(16, engine::tile::TileVariant::TWest);
			}

			// create splash map
			{
				// create sprite atlas for the water splash animation
				engine::graphics::factory::SpriteAtlasFactory::Create("water_splash", L"../Assets/3072x192px_1x17tile_waterfoam.png", 1, 16);
				engine::graphics::resource::ISpriteAtlas& atlas = engine::cache::Registry<engine::graphics::resource::ISpriteAtlas>::Instance().Get("water_splash");

				// create animation and load all of our sprite atlas' sprite into it and store in registry
				//engine::graphics::factory::SpriteAnimationFactory::Create("water_splash", atlas, { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 }, 100.0f, true);
				engine::graphics::factory::SpriteAnimationFactory::Create("water_splash", atlas, 100.0f, true);
				engine::graphics::animation::Animation<engine::graphics::Sprite>& anim = Registry<engine::graphics::animation::Animation<engine::graphics::Sprite>>::Instance().Get("water_splash");

				// create our tileset
				Registry<Tileset<AnimatedTile>>::Instance().Register("water_splash", std::make_unique<Tileset<AnimatedTile>>());
				Tileset<AnimatedTile>& tileset = Registry<Tileset<AnimatedTile>>::Instance().Get("water_splash");

				// load tile definition into tileset. since this is animated tile, we create AnimatedTile definition which holds the animation and animator for this tile.
				// we only have one - water splash tile, so we just register it at index 0. 
				tileset.Register(0, std::make_unique<AnimatedTile>(false, "water_splash", anim, 0));

				// create tile region
				Registry<TileRegion<AnimatedTile>>::Instance().Register("water_splash", make_unique<TileRegion<AnimatedTile>>());
				TileRegion<AnimatedTile>& region = Registry<TileRegion<AnimatedTile>>::Instance().Get("water_splash");

				// load tile region by filling it with all '0' tile
				Table<string> map({ 20, 12 }, "1");
				AsyncTileRegionLoader<AnimatedTile, int> tileRegionLoader;
				tileRegionLoader.LoadImmediate(region, map, [&tileset](const int& cell) -> Tile<AnimatedTile> { return tileset.MakeTile(cell); });

				// create lookup tile resolver for splash map. this will be used to determine tile variant based on surrounding tiles
				Registry<engine::tile::LookupTileResolver<AnimatedTile>>::Instance().Register("splash_map", make_unique<engine::tile::LookupTileResolver<AnimatedTile>>(region, tileset));
				engine::tile::LookupTileResolver<AnimatedTile>& resolver = Registry<engine::tile::LookupTileResolver<AnimatedTile>>::Instance().Get("splash_map");

				// map tile index to tile variant for splash map. this will be used by auto-tile resolver to determine tile variant based on surrounding tiles
				resolver.Register(engine::tile::TileVariant::Empty, 1);
				resolver.Register(engine::tile::TileVariant::Island, 0);
				resolver.Register(engine::tile::TileVariant::Full, 1);

				resolver.Register(engine::tile::TileVariant::NorthEdge, 0);
				resolver.Register(engine::tile::TileVariant::SouthEdge, 0);
				resolver.Register(engine::tile::TileVariant::EastEdge, 0);
				resolver.Register(engine::tile::TileVariant::WestEdge, 0);

				resolver.Register(engine::tile::TileVariant::NECorner, 0);
				resolver.Register(engine::tile::TileVariant::NWCorner, 0);
				resolver.Register(engine::tile::TileVariant::SECorner, 0);
				resolver.Register(engine::tile::TileVariant::SWCorner, 0);

				resolver.Register(engine::tile::TileVariant::Vertical, 0);
				resolver.Register(engine::tile::TileVariant::Horizontal, 0);

				resolver.Register(engine::tile::TileVariant::TNorth, 0);
				resolver.Register(engine::tile::TileVariant::TSouth, 0);
				resolver.Register(engine::tile::TileVariant::TEast, 0);
				resolver.Register(engine::tile::TileVariant::TWest, 0);

				// let splash tile resolver subscribe to land auto-tile map so that it can update splash tile variants when land tiles change
				engine::tile::AutoTileResolver<RenderableTile>& landTileResolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("land_map");
				landTileResolver.TileVariantChangedEvent += Handler(&resolver, &engine::tile::LookupTileResolver<AnimatedTile>::Set);
			}

			// setup wall map
			{
				// get our sprite atlas for wall tilemap. we use the same sprite atlas as land map since we are using the same tile for wall. 
				ISpriteAtlas& atlas = Registry<ISpriteAtlas>::Instance().Get("1x1_64x64_water_background");

				// use the same tileset as our first level land map since we are using the same tile for wall.
				Tileset<RenderableTile>& tileset = Registry<Tileset<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");

				// create tile region	
				Registry<TileRegion<RenderableTile>>::Instance().Register("wall_map", make_unique<TileRegion<RenderableTile>>());
				TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("wall_map");

				// load tile region by filling it with all '4' tile
				Size<size_t> mapsize = Registry<Size<size_t>>::Instance().Get("map_size");
				Table<string> map(mapsize, "4");
				AsyncTileRegionLoader<RenderableTile, int> tileRegionLoader;
				tileRegionLoader.LoadImmediate(region, map, [&tileset](const int& cell) -> Tile<RenderableTile> { return tileset.MakeTile(cell); });

				// create lookup tile resolver for wall map. this will be used to determine tile variant based on surrounding tiles
				Registry<engine::tile::LookupTileResolver<RenderableTile>>::Instance().Register("wall_map", make_unique<engine::tile::LookupTileResolver<RenderableTile>>(region, tileset));
				engine::tile::LookupTileResolver<RenderableTile>& resolver = Registry<engine::tile::LookupTileResolver<RenderableTile>>::Instance().Get("wall_map");	

				// map tile index to tile variant for wall map. this will be used by auto-tile resolver to determine tile variant based on surrounding tiles
				resolver.Register(engine::tile::TileVariant::Empty, 4);
				resolver.Register(engine::tile::TileVariant::Island, 44);
				resolver.Register(engine::tile::TileVariant::Full, 4);

				resolver.Register(engine::tile::TileVariant::NorthEdge, 44);
				resolver.Register(engine::tile::TileVariant::SouthEdge, 4);
				resolver.Register(engine::tile::TileVariant::EastEdge, 43);
				resolver.Register(engine::tile::TileVariant::WestEdge, 41);

				resolver.Register(engine::tile::TileVariant::NECorner, 4);
				resolver.Register(engine::tile::TileVariant::NWCorner, 4);
				resolver.Register(engine::tile::TileVariant::SECorner, 41);
				resolver.Register(engine::tile::TileVariant::SWCorner, 43);

				resolver.Register(engine::tile::TileVariant::Vertical, 4);
				resolver.Register(engine::tile::TileVariant::Horizontal, 42);

				resolver.Register(engine::tile::TileVariant::TNorth, 4);
				resolver.Register(engine::tile::TileVariant::TSouth, 42);
				resolver.Register(engine::tile::TileVariant::TEast, 4);
				resolver.Register(engine::tile::TileVariant::TWest, 4);

				// let wall tile resolver subscribe to hill auto-tile map so that it can update wall tile variants when hill tiles change
				engine::tile::AutoTileResolver<RenderableTile>& hillTileResolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("hill_map");
				hillTileResolver.TileVariantChangedEvent += Handler(&resolver, &engine::tile::LookupTileResolver<RenderableTile>::Set);
			}

			// setup stopwatch to manage timing and start it
			m_stopwatch.OnLap += Handler(this, &Test::OnLap);
			m_stopwatch.Start();
		}

		void OnKeyDown(int key)
		{
			// all maps have the same size and position. so they share the same position to coord conversion. calculate coord based on mouse position and map position and tile size
			PositionF mapPos = Registry<PositionF>::Instance().Get("map_position");
			SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");
			engine::spatial::Coord coord = engine::spatial::PositionToCoord(m_mousePos - mapPos, tilesize);

			switch (key)
			{
			case 27: // escape
			{
				// clear map of land
				{
					Tileset<RenderableTile>& tileset = Registry<Tileset<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");
					TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");
					region.Fill(tileset.MakeTile(4));
				}

				// clear water splash map
				{
					Tileset<AnimatedTile>& tileset = Registry<Tileset<AnimatedTile>>::Instance().Get("water_splash");
					TileRegion<AnimatedTile>& region = Registry<TileRegion<AnimatedTile>>::Instance().Get("water_splash");
					region.Fill(tileset.MakeTile(4));
				}
				// clear wall map
				{
					Tileset<RenderableTile>& tileset = Registry<Tileset<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");
					TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("wall_map");
					region.Fill(tileset.MakeTile(4));
				}
				// clear hill map
				{
					Tileset<RenderableTile>& tileset = Registry<Tileset<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");
					TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("hill_map");
					region.Fill(tileset.MakeTile(4));
				}

				break;
			}
			case 32: // space
				break;
			case 49: // 1
			{
				{
					engine::tile::AutoTileResolver<RenderableTile>& resolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("land_map");
					resolver.Set(coord);
				}

				break;
			}
			case 50: // 2
			{
				// remove land tile at coord and update surrounding tiles
				{
					engine::tile::AutoTileResolver<RenderableTile>& resolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("land_map");
					resolver.Remove(coord);
				}
				// remove hill tile at coord and update surrounding tiles
				{
					engine::tile::AutoTileResolver<RenderableTile>& resolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("hill_map");
					resolver.Remove(coord);
				}
				break;
			}
			case 51: // 3
			{
				{
					engine::tile::AutoTileResolver<RenderableTile>& resolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("land_map");
					resolver.Set(coord);
				}
				// update hill map
				{
					engine::tile::AutoTileResolver<RenderableTile>& resolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("hill_map");
					resolver.Set(coord);
				}
				break;
			}
			case 54: // 6
			{
				m_toggle = !m_toggle;
				break;
			}

			default:
				break;
			}
		}

		void OnMouseDown(int btn, int x, int y)
		{
		}

		void OnMouseMove(int x, int y)
		{
			m_mousePos = PositionF((float)x, (float)y);
		}

		// this method is fired up whenever the OnLap event is triggered from stopwatch
		void OnLap(double time)
		{
			// update our animated tile's animator  
			Tileset<AnimatedTile>& tileset = Registry<Tileset<AnimatedTile>>::Instance().Get("water_splash");
			for (auto& [id, tile] : tileset)
			{
				if (tile->IsRunning())
				{
					tile->Update(time);
				}
			}
		}

		// fun stuff. this is called on each loop of the message loop. this is where we draw!
		void OnIdle()
		{
			// call lap to get elapsed time and trigger OnLap event
			m_stopwatch.Lap<milliseconds>();

			m_input.Update();

			m_canvas->Clear({ 0.2f, 0.2f, 1.0f, 1.0f });

			// start the canvas. we can draw from here
			m_canvas->Begin();
			{
				m_renderer->Begin();

				// draw water background map
				{
					TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("1x1_64x64_water_background");
					TileMap<RenderableTile> tilemap = region.MakeTileMap();

					// get tilemap parameters
					PositionF pos = Registry<PositionF>::Instance().Get("map_position");
					SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");

					DrawTileMap(*m_renderer, tilemap, tilesize, pos, { 1,1,1,1 });
				}

				// draw water splash map
				{
					TileRegion<AnimatedTile>& region = Registry<TileRegion<AnimatedTile>>::Instance().Get("water_splash");
					TileMap<AnimatedTile> tilemap = region.MakeTileMap();

					// get tilemap parameters
					PositionF pos = Registry<PositionF>::Instance().Get("map_position");
					SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");

					DrawTileMap(*m_renderer, tilemap, tilesize, pos, { 1,1,1,1 }, { -64, -68 }, { 3,3 }, 1.0f);
				}

				// draw first level map
				{
					TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");
					TileMap<RenderableTile> tilemap = region.MakeTileMap();

					// get tilemap parameters
					PositionF pos = Registry<PositionF>::Instance().Get("map_position");
					SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");

					DrawTileMap(*m_renderer, tilemap, tilesize, pos, { 1,1,1,1 });
				}

				// draw wall map
				{
					TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("wall_map");
					TileMap<RenderableTile> tilemap = region.MakeTileMap();

					// get tilemap parameters
					PositionF pos = Registry<PositionF>::Instance().Get("map_position");
					SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");

					DrawTileMap(*m_renderer, tilemap, tilesize, pos, { 1,1,1,1 });
				}

				// draw hill map
				{
					TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("hill_map");
					TileMap<RenderableTile> tilemap = region.MakeTileMap();

					// get tilemap parameters
					PositionF pos = Registry<PositionF>::Instance().Get("map_position");
					SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");
					PositionF layerheight = Registry<PositionF>::Instance().Get("layer_height");

					if(m_toggle)DrawTileMap(*m_renderer, tilemap, tilesize, pos - layerheight, { 1,1,1,1 });

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
			LOG("Window resized to: " + to_string(nWidth) + ", " + to_string(nHeight));
			m_canvas->Resize({ static_cast<unsigned int>(nWidth), static_cast<unsigned int>(nHeight) });
			m_canvas->SetViewPort();
		}
	};

}