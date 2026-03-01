#pragma once

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
#include <Graphics/Renderable/Sprite.h>
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
#include <Engine/Factory/AnimationFactory.h>
#include "Actor.h"

namespace TestEditMap
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
	using namespace engine::component::tile;
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

	class RenderableTile
	{
	private:
		Sprite m_sprite;
		bool m_walkable;

	public:
		RenderableTile(const Sprite& sprite, bool walkable) :
			m_sprite(sprite),
			m_walkable(walkable)
		{
		}

		const Sprite& GetSprite() const
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

	public:
		Test() 
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
			m_window->Create(L"TestActorNavigation", 1400, 900);
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
			}

			// setup water tilemap
			{
				// create sprite atlas to be used by tilemap
				SpriteAtlasFactory::Create("1x1_64x64_water_background", L"../Assets/1x1_64x64_water_background.png", 1, 1);
				ISpriteAtlas& atlas = Registry<ISpriteAtlas>::Instance().Get("1x1_64x64_water_background");

				// create our tileset
				Registry<Tileset<RenderableTile>>::Instance().Register("1x1_64x64_water_background", std::make_unique<Tileset<RenderableTile>>());
				Tileset<RenderableTile>& tileset = Registry<Tileset<RenderableTile>>::Instance().Get("1x1_64x64_water_background");

				tileset.Register(0, std::make_unique<RenderableTile>(atlas.MakeSprite(0), false)); // water so not walkable. doesn't matter. this is background map

				// create tile region
				Registry<TileRegion<RenderableTile>>::Instance().Register("1x1_64x64_water_background", make_unique<TileRegion<RenderableTile>>());
				TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("1x1_64x64_water_background");

				// load tile region by filling it with all '0' tile
				Table<string> map({ 20, 12 }, "0");
				AsyncTileRegionLoader<RenderableTile, int> tileRegionLoader;
				tileRegionLoader.LoadImmediate(region, map, [&tileset](const int& cell) -> Tile<RenderableTile> { return tileset.MakeTile(cell); });
			}

			// setup first level map
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
					bool walkable = false;
					walkable |= (i == 0); // 
					walkable |= (i == 1);
					walkable |= (i == 2);
					walkable |= (i == 3);
					walkable |= (i == 9);
					walkable |= (i == 10);
					walkable |= (i == 11);
					walkable |= (i == 12);
					walkable |= (i == 18);
					walkable |= (i == 19);
					walkable |= (i == 20);
					walkable |= (i == 21);
					walkable |= (i == 27);
					walkable |= (i == 28);
					walkable |= (i == 29);
					walkable |= (i == 30);

					tileset.Register(i, std::make_unique<RenderableTile>(atlas.MakeSprite(i), walkable)); // make it all walkable for now
				}

				// create tile region
				Registry<TileRegion<RenderableTile>>::Instance().Register("576x384px_6x9tile_TileMap", make_unique<TileRegion<RenderableTile>>());
				TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");

				// load tile region by filling it with all '0' tile
				Table<string> map({ 20, 12 }, "4");
				AsyncTileRegionLoader<RenderableTile, int> tileRegionLoader;
				tileRegionLoader.LoadImmediate(region, map, [&tileset](const int& cell) -> Tile<RenderableTile> { return tileset.MakeTile(cell); });
				region.Set(2, 2, tileset.MakeTile(30));
			}

			// setup stopwatch to manage timing and start it
			m_stopwatch.OnLap += Handler(this, &Test::OnLap);
			m_stopwatch.Start();
		}

		void OnKeyDown(int key)
		{
			switch (key)
			{
			case 27: // escape
			{
				// clear map of land
				Tileset<RenderableTile>& tileset = Registry<Tileset<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");
				TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");
				region.Set(tileset.MakeTile(4));

				break;
			}
			case 32: // space
				break;
			case 49: // 1
			{
				//Tileset<RenderableTile>& tileset = Registry<Tileset<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");
				//TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");
				//// add a land tile
				//Coord c = PositionToMapCoord(m_mousePos);
				//region.Set(c.row, c.col, tileset.MakeTile(30));

				SetLandTile(PositionToMapCoord(m_mousePos));
				break;
			}
			case 50: // 2
			{
				break;
			}
			case 51: // 3
			{
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
					TileRegion<RenderableTile> region = Registry<TileRegion<RenderableTile>>::Instance().Get("1x1_64x64_water_background");
					TileMap<RenderableTile> tilemap = region.MakeTileMap();

					// get tilemap parameters
					PositionF pos = Registry<PositionF>::Instance().Get("map_position");
					SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");

					DrawTileMap(*m_renderer, tilemap, tilesize, pos, { 1,1,1,1 });
				}

				// draw first level map
				{
					TileRegion<RenderableTile> region = Registry<TileRegion<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");
					TileMap<RenderableTile> tilemap = region.MakeTileMap();

					// get tilemap parameters
					PositionF pos = Registry<PositionF>::Instance().Get("map_position");
					SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");

					DrawTileMap(*m_renderer, tilemap, tilesize, pos, { 1,1,1,1 });
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

		Coord PositionToMapCoord(const PositionF& pos)
		{
			// get parameters of tilemap
			PositionF mapPos = Registry<PositionF>::Instance().Get("map_position");
			SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");

			// calculate the coordinate of tile that intersect wih mouse click position
			Coord coord;
			coord.col = (int)((pos.x - mapPos.x) / tilesize.width);
			coord.row = (int)((pos.y - mapPos.y) / tilesize.height);

			return coord;
		}

		enum class TileVariant : int {
			// Base tiles
			Empty = 4,

			Island = 30,   // single land tile surrounded by water
			Full = 10,   // land surrounded on all sides

			// Single-edge tiles (land on one side, water on three)
			NorthEdge = 21,
			SouthEdge = 3,
			EastEdge = 29,
			WestEdge = 27,

			// Corner tiles (land on two adjacent sides)
			NECorner = 0,
			NWCorner = 2,
			SECorner = 18,
			SWCorner = 20,

			// Strips (land on opposite sides)
			Vertical = 12,  // land up/down, water left/right
			Horizontal = 28,  // land left/right, water up/down

			// Junctions
			//Cross = 40,  // land in all four cardinal directions
			TNorth = 1,  // land south+east+west, water north
			TSouth = 19,  // land north+east+west, water south
			TEast = 9,  // land north+south+west, water east
			TWest = 11   // land north+south+east, water west
		};


		bool SetLandTile(const Coord& coord)
		{
			TileRegion<RenderableTile> region = Registry<TileRegion<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");
			if (!region.IsInBounds(coord)) return false;

			int mask = ComputeMask(coord.row, coord.col);

			TileVariant tv = ResolveTileVariant(mask);

			PlaceTile(coord.row, coord.col, tv);

			return true;
		}

		int ComputeMask(int row, int col) 
		{
			TileRegion<RenderableTile> region = Registry<TileRegion<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");

			int mask = 0;
			if (region.IsInBounds(row - 1, col) && region.Get(row - 1, col)->IsWalkable()) mask |= 8; // N
			if (region.IsInBounds(row + 1, col) && region.Get(row + 1, col)->IsWalkable()) mask |= 2; // S
			if (region.IsInBounds(row, col + 1) && region.Get(row, col + 1)->IsWalkable()) mask |= 4; // E
			if (region.IsInBounds(row, col - 1) && region.Get(row, col - 1)->IsWalkable()) mask |= 1; // W
			return mask;
		}

		TileVariant ResolveTileVariant(int mask) 
		{
			switch (mask) {
			case 0:   return TileVariant::Island;   // surrounded by water
			case 15:  return TileVariant::Full;   // surrounded by land

			case 8:   return TileVariant::NorthEdge;  // land north only
			case 2:   return TileVariant::SouthEdge;  // land south only
			case 1:   return TileVariant::EastEdge;   // land east only
			case 4:   return TileVariant::WestEdge;   // land west only

			case 10:  return TileVariant::Vertical;   // land north+south
			case 5:   return TileVariant::Horizontal; // land east+west

			case 7:   return TileVariant::TNorth;     // land S+E+W
			case 13:  return TileVariant::TSouth;     // land N+E+W
			case 14:  return TileVariant::TEast;      // land N+S+W
			case 11:  return TileVariant::TWest;      // land N+S+E

			// Corners 
			case 6: return TileVariant::NECorner; // 0 
			case 3: return TileVariant::NWCorner; // 2 
			case 12: return TileVariant::SECorner; // 18 
			case 9: return TileVariant::SWCorner; // 20

			default:  return TileVariant::Empty;       // fallback
			}
		}

		void PlaceTile(int row, int col, TileVariant type)
		{
			TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");
			Tileset<RenderableTile>& tileset = Registry<Tileset<RenderableTile>>::Instance().Get("576x384px_6x9tile_TileMap");

			// Set the selected tile
			region.Set(row, col, tileset.MakeTile(static_cast<int>(type)));

			// Update self + 4 neighbors (N, S, E, W)
			for (auto [dr, dc] : std::array<std::pair<int, int>, 5>
				{
				{{0,0}, {-1,0}, {1,0}, {0,-1}, {0,1}}
				}) {
				int nr = row + dr, nc = col + dc;
				if (region.IsInBounds(nr, nc))
				{
					if (!region.Get(nr, nc)->IsWalkable()) continue;

					// Use 4-neighbor mask computation
					int mask = ComputeMask(nr, nc);
					TileVariant variant = ResolveTileVariant(mask);

					region.Set(nr, nc, tileset.MakeTile(static_cast<int>(variant)));
				}
			}
		}

	};

}