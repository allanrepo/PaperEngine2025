//#pragma once
//#include <Algorithm/AutoTileResolver.h>
//#include <Win32/Window.h>
//#include <Core/Event.h>
//#include <Utilities/Logger.h>
//#include <Graphics/Core/ICanvas.h>
//#include <Graphics/Core/Canvas.h>
//#include <Graphics/Core/DX11CanvasImpl.h>
//#include <Graphics/Renderer/IRenderer.h>
//#include <Graphics/Renderer/DX11RendererBatchImpl.h>
//#include <Graphics/Renderer/DX11RendererImmediateImpl.h>
//#include <Graphics/Renderer/Renderer.h>
//#include <Graphics/Resource/ISpriteAtlas.h>
//#include <Engine/Factory/SpriteAtlasFactory.h>
//#include <Graphics/Renderable/Sprite.h>
//#include <Core/Input.h>
//#include <Graphics/Resource/IFontAtlas.h>
//#include <Graphics/Resource/FontAtlas.h>
//#include <Graphics/Resource/SpriteAtlas.h>
//#include <Cache/Registry.h>
//#include <Graphics/Animation/Animation.h>
//#include <Timer/StopWatch.h>
//#include <Algorithm/Pathfinding.h>
//#include <Engine/Loader/AsyncLoader.h>
//#include <Graphics/Core/Primitives.h>
//#include <Engine/Graphics/Draw.h>
//#include <Engine/Factory/AnimationFactory.h>
//#include "Actor.h"
//
//namespace TestEditMap
//{
//	using namespace std;
//	using namespace engine;
//	using namespace engine::graphics;
//	using namespace engine::graphics::renderable;
//	using namespace engine::graphics::animation;
//	using namespace engine::win32;
//	using namespace engine::graphics::renderer;
//	using namespace engine::timer;
//	using namespace engine::input;
//	using namespace engine::event;
//	using namespace engine::graphics::dx11::renderer;
//	using namespace engine::graphics::dx11;
//	using namespace engine::component::tile;
//	using namespace engine::graphics::factory;
//	using namespace engine::graphics::resource;
//	using namespace engine::container;
//	using namespace engine::loader::tile;
//	using namespace engine::spatial;
//	using namespace engine::math;
//	using namespace engine::math::geometry;
//	using namespace engine::component;
//	using namespace engine::graphics::tile;
//	using namespace engine::navigation::tile;
//	using namespace engine::graphics::navigation;
//
//	template<typename T>
//	using Registry = engine::cache::Registry<T>;
//
//	template<typename T>
//	using Animation = engine::graphics::animation::Animation<T>;
//	
//
//
//	// this is a tile definition class, not exactly tile class. tile class is Tile<T> and this is what is assigned to T
//	class AnimatedTile
//	{
//	private:
//		engine::graphics::animation::Animator<engine::graphics::renderable::Sprite> m_animator;
//		std::unordered_map<std::string, engine::graphics::animation::Animation<engine::graphics::renderable::Sprite>> m_animations;
//		bool m_walkable;
//		int m_index;
//
//	public:
//		AnimatedTile(bool walkable, const std::string& name, const engine::graphics::animation::Animation<engine::graphics::renderable::Sprite>& anim, int index) :
//			m_walkable(walkable),
//			m_index(index)
//
//		{
//			// copy the animation into our container
//			m_animations[name] = anim;
//
//			// assign the animation from our container into animator (don't assign the passed animation. that is reference to animation outside which is not safe
//			m_animator.Play(m_animations[name]);
//		}
//
//		bool IsRunning() const
//		{
//			return m_animator.IsRunning();
//		}
//
//		const engine::graphics::renderable::Sprite& GetSprite() const
//		{
//			return m_animator.GetCurrent();
//		}
//
//		void Update(double delta)
//		{
//			m_animator.Update(delta);
//		}
//
//		int GetIndex() const
//		{
//			return m_index;
//		}
//	};
//
//	class RenderableTile
//	{
//	private:
//		Sprite m_sprite;
//		bool m_walkable;
//		int m_index;
//
//	public:
//		RenderableTile(const Sprite& sprite, bool walkable, int index) :
//			m_sprite(sprite),
//			m_walkable(walkable),
//			m_index(index)
//		{
//		}
//		
//		int GetIndex() const
//		{
//			return m_index;
//		}
//
//		const Sprite& GetSprite() const
//		{
//			return m_sprite;
//		}
//
//		bool IsWalkable() const
//		{
//			return m_walkable;
//		}
//	};
//	
//
//	class Test
//	{
//	public:
//
//
//	private:
//		std::unique_ptr<Window> m_window;
//		std::unique_ptr<ICanvas> m_canvas;
//		std::unique_ptr<IRenderer> m_renderer;
//
//		StopWatch m_stopwatch;
//		double m_elapsed;
//
//		Input m_input;
//
//		PositionF m_mousePos;
//
//	public:
//		Test() 
//		{
//			Window::OnInitialize += Handler(this, &Test::OnInitialize);
//			Window::OnExit += Handler(this, &Test::OnExit);
//			Window::OnIdle += Handler(this, &Test::OnIdle);
//			Window::Run();
//		}
//
//		// function that will be called just before we enter into message loop
//		void OnInitialize()
//		{
//			// create our window here
//			m_window = make_unique<Window>();
//			m_window->OnClose += Handler(this, &Test::OnWindowClose);
//			m_window->OnCreate += Handler(this, &Test::OnWindowCreate);
//			m_window->OnSize += Handler(this, &Test::OnWindowSize);
//			m_window->Create(L"TestEditMap", 1400, 900);
//			m_window->OnWindowMessage += Handler(&m_input, &Input::ProcessWin32Message);
//
//			m_input.KeyDownEvent += Handler(this, &Test::OnKeyDown);
//			m_input.MouseDownEvent += Handler(this, &Test::OnMouseDown);
//			m_input.MouseMoveEvent += Handler(this, &Test::OnMouseMove);
//		}
//
//		// when window is created. we can now safely create resources dependent on window
//		void OnWindowCreate(void* hWnd)
//		{
//			LOG("Window created...");
//
//			// create dx11 canvas
//			m_canvas = make_unique<Canvas>(make_unique<DX11CanvasImpl>());
//			m_canvas->Initialize(hWnd);
//			m_canvas->SetViewPort();
//			LOG("Canvas (DX11) created...");
//
//			// create dx11 renderer batched
//			m_renderer = make_unique<Renderer>(make_unique<DX11RendererBatchImpl>());
//			m_renderer->Initialize();
//			LOG("Renderer Batch (DX11) created...");
//
//			// set map parameters
//			{
//				Registry<SizeF>::Instance().Register("tile_size", make_unique<SizeF>(64.0f, 64.0f));
//				Registry<PositionF>::Instance().Register("map_position", make_unique<PositionF>(50.0f, 50.0f));
//				Registry<Size<size_t>>::Instance().Register("map_size", make_unique<Size<size_t>>(20, 12));
//			}
//
//			// setup water tilemap
//			{
//				// create sprite atlas to be used by tilemap
//				SpriteAtlasFactory::Create("1x1_64x64_water_background", L"../Assets/1x1_64x64_water_background.png", 1, 1);
//				ISpriteAtlas& atlas = Registry<ISpriteAtlas>::Instance().Get("1x1_64x64_water_background");
//
//				// create our tileset
//				Registry<Tileset<RenderableTile>>::Instance().Register("1x1_64x64_water_background", std::make_unique<Tileset<RenderableTile>>());
//				Tileset<RenderableTile>& tileset = Registry<Tileset<RenderableTile>>::Instance().Get("1x1_64x64_water_background");
//
//				tileset.Register(0, std::make_unique<RenderableTile>(atlas.MakeSprite(0), false, 0)); // water so not walkable. doesn't matter. this is background map
//
//				// create tile region
//				Registry<TileRegion<RenderableTile>>::Instance().Register("1x1_64x64_water_background", make_unique<TileRegion<RenderableTile>>());
//				TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("1x1_64x64_water_background");
//
//				// load tile region by filling it with all '0' tile
//				Table<string> map({ 20, 12 }, "0");
//				AsyncTileRegionLoader<RenderableTile, int> tileRegionLoader;
//				tileRegionLoader.LoadImmediate(region, map, [&tileset](const int& cell) -> Tile<RenderableTile> { return tileset.MakeTile(cell); });
//			}
//
//			// setup first level map
//			{
//				// create sprite atlas to be used by tilemap
//				SpriteAtlasFactory::Create("576x384px_6x9tile_TileMap", L"../Assets/576x384px_6x9tile_TileMap.png", 6, 9);
//				ISpriteAtlas& atlas = Registry<ISpriteAtlas>::Instance().Get("576x384px_6x9tile_TileMap");
//
//				// create our tileset
//				Registry<Tileset<RenderableTile>>::Instance().Register("576x384px_6x9tile_TileMap", std::make_unique<Tileset<RenderableTile>>());
//				Tileset<RenderableTile>& tileset = Registry<Tileset<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");
//
//				// each sprite from atlas is a static tile (single frame), so we create tile from each sprite
//				for (int i = 0; i < atlas.GetUVRectCount(); i++)
//				{
//					tileset.Register(i, std::make_unique<RenderableTile>(atlas.MakeSprite(i), true, i)); // make it all walkable for now
//				}
//
//				// create tile region
//				Registry<TileRegion<RenderableTile>>::Instance().Register("576x384px_6x9tile_TileMap", make_unique<TileRegion<RenderableTile>>());
//				TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");
//
//				// load tile region by filling it with all '0' tile
//				Table<string> map({ 20, 12 }, "4");
//				AsyncTileRegionLoader<RenderableTile, int> tileRegionLoader;
//				tileRegionLoader.LoadImmediate(region, map, [&tileset](const int& cell) -> Tile<RenderableTile> { return tileset.MakeTile(cell); });
//				region.Set(2, 2, tileset.MakeTile(30));
//			}
//
//			// configure land auto-tile mapping
//			{			
//				// index → variant mapping
//				m_autoTileMapLand.Register(4, engine::tile::TileVariant::Empty);
//				m_autoTileMapLand.Register(30, engine::tile::TileVariant::Island);
//				m_autoTileMapLand.Register(10, engine::tile::TileVariant::Full);
//
//				m_autoTileMapLand.Register(21, engine::tile::TileVariant::NorthEdge);
//				m_autoTileMapLand.Register(3, engine::tile::TileVariant::SouthEdge);
//				m_autoTileMapLand.Register(29, engine::tile::TileVariant::EastEdge);
//				m_autoTileMapLand.Register(27, engine::tile::TileVariant::WestEdge);
//
//				m_autoTileMapLand.Register(0, engine::tile::TileVariant::NECorner);
//				m_autoTileMapLand.Register(2, engine::tile::TileVariant::NWCorner);
//				m_autoTileMapLand.Register(18, engine::tile::TileVariant::SECorner);
//				m_autoTileMapLand.Register(20, engine::tile::TileVariant::SWCorner);
//
//				m_autoTileMapLand.Register(12, engine::tile::TileVariant::Vertical);
//				m_autoTileMapLand.Register(28, engine::tile::TileVariant::Horizontal);
//
//				m_autoTileMapLand.Register(1, engine::tile::TileVariant::TNorth);
//				m_autoTileMapLand.Register(19, engine::tile::TileVariant::TSouth);
//				m_autoTileMapLand.Register(9, engine::tile::TileVariant::TEast);
//				m_autoTileMapLand.Register(11, engine::tile::TileVariant::TWest);
//			}
//
//			// create splash map
//			{
//				// create sprite atlas for the water splash animation
//				engine::graphics::factory::SpriteAtlasFactory::Create("water_splash", L"../Assets/3072x192px_1x17tile_waterfoam.png", 1, 16);
//				engine::graphics::resource::ISpriteAtlas& atlas = engine::cache::Registry<engine::graphics::resource::ISpriteAtlas>::Instance().Get("water_splash");
//
//				// create animation and load all of our sprite atlas' sprite into it and store in registry
//				//engine::graphics::factory::AnimationFactory::Create("water_splash", atlas, { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 }, 100.0f, true);
//				engine::graphics::factory::AnimationFactory::Create("water_splash", atlas, 100.0f, true);
//				engine::graphics::animation::Animation<engine::graphics::renderable::Sprite>& anim = Registry<engine::graphics::animation::Animation<engine::graphics::renderable::Sprite>>::Instance().Get("water_splash");
//
//				// create our tileset
//				Registry<Tileset<AnimatedTile>>::Instance().Register("water_splash", std::make_unique<Tileset<AnimatedTile>>());
//				Tileset<AnimatedTile>& tileset = Registry<Tileset<AnimatedTile>>::Instance().Get("water_splash");
//
//				// load tile definition into tileset. since this is animated tile, we create AnimatedTile definition which holds the animation and animator for this tile.
//				// we only have one - water splash tile, so we just register it at index 0. 
//				tileset.Register(0, std::make_unique<AnimatedTile>(false, "water_splash", anim, 0)); 
//
//				// create tile region
//				Registry<TileRegion<AnimatedTile>>::Instance().Register("water_splash", make_unique<TileRegion<AnimatedTile>>());
//				TileRegion<AnimatedTile>& region = Registry<TileRegion<AnimatedTile>>::Instance().Get("water_splash");
//
//				// load tile region by filling it with all '0' tile
//				Table<string> map({ 20, 12 }, "1");
//				AsyncTileRegionLoader<AnimatedTile, int> tileRegionLoader;
//				tileRegionLoader.LoadImmediate(region, map, [&tileset](const int& cell) -> Tile<AnimatedTile> { return tileset.MakeTile(cell); });
//				//region.Set(9, 9, tileset.MakeTile(0));
//				//region.Set(0, 0, tileset.MakeTile(0));
//			}
//
//			// configure land auto-tile mapping
//			{
//				// index → variant mapping
//				m_autoTileMapSplash.Register(1, engine::tile::TileVariant::Empty);
//				m_autoTileMapSplash.Register(0, engine::tile::TileVariant::Island);
//				m_autoTileMapSplash.Register(0, engine::tile::TileVariant::Full);
//
//				m_autoTileMapSplash.Register(0, engine::tile::TileVariant::NorthEdge);
//				m_autoTileMapSplash.Register(0, engine::tile::TileVariant::SouthEdge);
//				m_autoTileMapSplash.Register(0, engine::tile::TileVariant::EastEdge);
//				m_autoTileMapSplash.Register(0, engine::tile::TileVariant::WestEdge);
//
//				m_autoTileMapSplash.Register(0, engine::tile::TileVariant::NECorner);
//				m_autoTileMapSplash.Register(0, engine::tile::TileVariant::NWCorner);
//				m_autoTileMapSplash.Register(0, engine::tile::TileVariant::SECorner);
//				m_autoTileMapSplash.Register(0, engine::tile::TileVariant::SWCorner);
//
//				m_autoTileMapSplash.Register(0, engine::tile::TileVariant::Vertical);
//				m_autoTileMapSplash.Register(0, engine::tile::TileVariant::Horizontal);
//
//				m_autoTileMapSplash.Register(0, engine::tile::TileVariant::TNorth);
//				m_autoTileMapSplash.Register(0, engine::tile::TileVariant::TSouth);
//				m_autoTileMapSplash.Register(0, engine::tile::TileVariant::TEast);
//				m_autoTileMapSplash.Register(0, engine::tile::TileVariant::TWest);
//			}
//
//			// setup stopwatch to manage timing and start it
//			m_stopwatch.OnLap += Handler(this, &Test::OnLap);
//			m_stopwatch.Start();
//		}
//
//		void OnKeyDown(int key)
//		{
//			switch (key)
//			{
//			case 27: // escape
//			{
//				// clear map of land
//				{
//					Tileset<RenderableTile>& tileset = Registry<Tileset<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");
//					TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");
//					region.Set(tileset.MakeTile(4));
//				}
//
//				// clear water splash map
//				{
//					Tileset<AnimatedTile>& tileset = Registry<Tileset<AnimatedTile>>::Instance().Get("water_splash");
//					TileRegion<AnimatedTile>& region = Registry<TileRegion<AnimatedTile>>::Instance().Get("water_splash");
//					region.Set(tileset.MakeTile(4));	
//				}
//
//				break;
//			}
//			case 32: // space
//				break;
//			case 49: // 1
//			{
//				{
//					TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");
//					Tileset<RenderableTile>& tileset = Registry<Tileset<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");
//					engine::spatial::Coord coord = PositionToMapCoord(m_mousePos);
//					m_autoTileMapLand.Set(region, tileset, coord);
//				}
//
//				{
//					TileRegion<AnimatedTile>& region = Registry<TileRegion<AnimatedTile>>::Instance().Get("water_splash");
//					Tileset<AnimatedTile>& tileset = Registry<Tileset<AnimatedTile>>::Instance().Get("water_splash");
//					engine::spatial::Coord coord = PositionToMapCoord(m_mousePos);
//					m_autoTileMapSplash.Set(region, tileset, coord);
//				}
//
//				break;
//			}
//			case 50: // 2
//			{
//				{
//					TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");
//					Tileset<RenderableTile>& tileset = Registry<Tileset<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");
//					engine::spatial::Coord coord = PositionToMapCoord(m_mousePos);
//					m_autoTileMapLand.Remove(region, tileset, coord);
//				}
//
//				{
//					TileRegion<AnimatedTile>& region = Registry<TileRegion<AnimatedTile>>::Instance().Get("water_splash");
//					Tileset<AnimatedTile>& tileset = Registry<Tileset<AnimatedTile>>::Instance().Get("water_splash");
//					engine::spatial::Coord coord = PositionToMapCoord(m_mousePos);
//					m_autoTileMapSplash.Remove(region, tileset, coord);
//				}
//				break;
//			}
//			case 51: // 3
//			{
//				break;
//			}
//
//			default:
//				break;
//			}
//		}
//
//		void OnMouseDown(int btn, int x, int y)
//		{
//		}
//
//		void OnMouseMove(int x, int y)
//		{
//			m_mousePos = PositionF((float)x, (float)y);
//		}
//
//		// this method is fired up whenever the OnLap event is triggered from stopwatch
//		void OnLap(double time)
//		{
//			// update our animated tile's animator  
//			Tileset<AnimatedTile>& tileset = Registry<Tileset<AnimatedTile>>::Instance().Get("water_splash");
//			for(auto& [id, tile] : tileset)
//			{
//				if (tile->IsRunning())
//				{
//					tile->Update(time);
//				}
//			}
//		}
//
//		// fun stuff. this is called on each loop of the message loop. this is where we draw!
//		void OnIdle()
//		{
//			// call lap to get elapsed time and trigger OnLap event
//			m_stopwatch.Lap<milliseconds>();
//
//			m_input.Update();
//
//			m_canvas->Clear({ 0.2f, 0.2f, 1.0f, 1.0f });
//
//			// start the canvas. we can draw from here
//			m_canvas->Begin();
//			{
//				m_renderer->Begin();
//
//				// draw water background map
//				{
//					TileRegion<RenderableTile> region = Registry<TileRegion<RenderableTile>>::Instance().Get("1x1_64x64_water_background");
//					TileMap<RenderableTile> tilemap = region.MakeTileMap();
//
//					// get tilemap parameters
//					PositionF pos = Registry<PositionF>::Instance().Get("map_position");
//					SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");
//
//					DrawTileMap(*m_renderer, tilemap, tilesize, pos, { 1,1,1,1 });
//				}
//
//				// draw water splash map
//				{
//					TileRegion<AnimatedTile> region = Registry<TileRegion<AnimatedTile>>::Instance().Get("water_splash");
//					TileMap<AnimatedTile> tilemap = region.MakeTileMap();
//
//					// get tilemap parameters
//					PositionF pos = Registry<PositionF>::Instance().Get("map_position");
//					SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");
//
//					DrawTileMap(*m_renderer, tilemap, tilesize, pos, { 1,1,1,1 }, { -64, -68 }, { 3,3 }, 1.0f);
//				}
//
//				// draw first level map
//				{
//					TileRegion<RenderableTile> region = Registry<TileRegion<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");
//					TileMap<RenderableTile> tilemap = region.MakeTileMap();
//
//					// get tilemap parameters
//					PositionF pos = Registry<PositionF>::Instance().Get("map_position");
//					SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");
//
//					DrawTileMap(*m_renderer, tilemap, tilesize, pos, { 1,1,1,1 });
//				}
//				m_renderer->End();
//			}
//			m_canvas->End();
//		}
//
//		void OnExit()
//		{
//
//		}
//
//		void OnWindowClose()
//		{
//		}
//
//		void OnWindowSize(size_t nWidth, size_t nHeight)
//		{
//			LOG("Window resized to: " + to_string(nWidth) + ", " + to_string(nHeight));
//			m_canvas->Resize({ static_cast<unsigned int>(nWidth), static_cast<unsigned int>(nHeight) });
//			m_canvas->SetViewPort();
//		}
//
//		Coord PositionToMapCoord(const PositionF& pos)
//		{
//			// get parameters of tilemap
//			PositionF mapPos = Registry<PositionF>::Instance().Get("map_position");
//			SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");
//
//			// calculate the coordinate of tile that intersect wih mouse click position
//			Coord coord;
//			coord.col = (int)((pos.x - mapPos.x) / tilesize.width);
//			coord.row = (int)((pos.y - mapPos.y) / tilesize.height);
//
//			return coord;
//		}
//
//
//
//		enum class TileVariant : unsigned int 
//		{
//			// Base tiles
//			Empty = 4,
//
//			Island = 30,   // single land tile surrounded by water
//			Full = 10,   // land surrounded on all sides
//
//			// Single-edge tiles (land on one side, water on three)
//			NorthEdge = 21,
//			SouthEdge = 3,
//			EastEdge = 29,
//			WestEdge = 27,
//
//			// Corner tiles (land on two adjacent sides)
//			NECorner = 0,
//			NWCorner = 2,
//			SECorner = 18,
//			SWCorner = 20,
//
//			// Strips (land on opposite sides)
//			Vertical = 12,  // land up/down, water left/right
//			Horizontal = 28,  // land left/right, water up/down
//
//			// Junctions
//			//Cross = 40,  // land in all four cardinal directions
//			TNorth = 1,  // land south+east+west, water north
//			TSouth = 19,  // land north+east+west, water south
//			TEast = 9,  // land north+south+west, water east
//			TWest = 11   // land north+south+east, water west
//		};
//
//		bool SetLandTile(const Coord& coord)
//		{
//			TileRegion<RenderableTile> region = Registry<TileRegion<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");
//			if (!region.IsInBounds(coord)) return false;
//
//			unsigned int mask = ComputeMask(coord.row, coord.col);
//
//			TileVariant tv = ResolveTileVariant(mask);
//
//			PlaceTile(coord.row, coord.col, tv);
//
//			return true;
//		}
//
//		unsigned int ComputeMask(int row, int col)
//		{
//			TileRegion<RenderableTile> region = Registry<TileRegion<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");
//
//			// we define our mask for all neighbor tiles. application don't need to know about it. this is internal logic of our AutoTileResolver class.
//
//
//			// check cardinal neighbors if they are land tiles already.  
//			// so if coord is surrounded by land on all side, mask = 0b1111 = 15.
//			int mask = 0;
//			if (region.IsInBounds(row - 1, col) && region.Get(row - 1, col)->IsWalkable()) mask |= 8; // N
//			if (region.IsInBounds(row + 1, col) && region.Get(row + 1, col)->IsWalkable()) mask |= 2; // S
//			if (region.IsInBounds(row, col + 1) && region.Get(row, col + 1)->IsWalkable()) mask |= 4; // E
//			if (region.IsInBounds(row, col - 1) && region.Get(row, col - 1)->IsWalkable()) mask |= 1; // W
//			return mask;
//		}
//
//		TileVariant ResolveTileVariant(int mask) 
//		{
//			switch (mask) 
//			{
//			case 0:   return TileVariant::Island;   // surrounded by nothing
//			case 15:  return TileVariant::Full;   // surrounded by same tile type on all 4 sides
//			
//			case 8:   return TileVariant::NorthEdge;  // same tile type on north only. nothing on south, east, west
//			case 2:   return TileVariant::SouthEdge;  // same tile type on south only. nothing on north, east, west
//			case 1:   return TileVariant::EastEdge;   // same tile type on east only. nothing on north, south, west
//			case 4:   return TileVariant::WestEdge;   // same tile type on west only. nothing on north, south, east
//
//			case 10:  return TileVariant::Vertical;   // same tile type on north+south. nothing on east, west
//			case 5:   return TileVariant::Horizontal; // same tile type on east+west. nothing on north, south
//
//			case 7:   return TileVariant::TNorth;     // same tile type on south+east+west. nothing on north
//			case 13:  return TileVariant::TSouth;     // same tile type on north+east+west. nothing on south
//			case 14:  return TileVariant::TEast;      // same tile type on north+south+west. nothing on east	
//			case 11:  return TileVariant::TWest;      // same tile type on north+south+east. nothing on west
//
//			// Corners 
//			case 6: return TileVariant::NECorner; // same tile type on north+east. nothing on south, west 
//			case 3: return TileVariant::NWCorner; // same tile type on north+west. nothing on south, east
//			case 12: return TileVariant::SECorner; // same tile type on south+east. nothing on north, west
//			case 9: return TileVariant::SWCorner; // same tile type on south+west. nothing on north, east
//
//			default:  return TileVariant::Empty; // default to empty tile if mask configuration not found. this should not happen if we cover all cases.
//			}
//		}
//
//		void PlaceTile(int row, int col, TileVariant type)
//		{
//			TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");
//			Tileset<RenderableTile>& tileset = Registry<Tileset<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");
//
//			// Set the selected tile
//			region.Set(row, col, tileset.MakeTile(static_cast<int>(type)));
//
//			// Update self + 4 neighbors (N, S, E, W)
//			for (auto [dr, dc] : std::array<std::pair<int, int>, 5>
//				{
//				{{0,0}, {-1,0}, {1,0}, {0,-1}, {0,1}}
//				}) {
//				int nr = row + dr, nc = col + dc;
//				if (region.IsInBounds(nr, nc))
//				{
//					if (!region.Get(nr, nc)->IsWalkable()) continue;
//
//					// Use 4-neighbor mask computation
//					int mask = ComputeMask(nr, nc);
//					TileVariant variant = ResolveTileVariant(mask);
//
//					region.Set(nr, nc, tileset.MakeTile(static_cast<int>(variant)));
//				}
//			}
//		}
//
//	};
//
//}