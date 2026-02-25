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
#include <Engine/Loader/SpriteAtlasLoader.h>
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

using namespace engine;
using namespace engine::graphics;

namespace TestPathFinding
{
	// tile descriptor
	enum class TileDesc : int
	{						// NWSE
		CLEAR  = 0b0000,	// 0000 -> clear
		BLOCKED = 0b1111,	// 1111 -> blocked
		NW = 0b1100,		// 1100 -> NW blocked half diagonal
		SE = 0b0011,		// 0011 -> SE blocked half diagonal
		NE = 0b1001,		// 1001 -> NE blocked half diagonal
		SW = 0b0110			// 0110 -> SW blocked half diagonal
	};


	class RenderableTile
	{
	private:
		engine::graphics::renderable::Sprite m_sprite;
		TileDesc m_mask;

	public:
		RenderableTile(const engine::graphics::renderable::Sprite& sprite, TileDesc mask = TileDesc::CLEAR) :
			m_sprite(sprite),
			m_mask(mask)
		{
		}

		const engine::graphics::renderable::Sprite& GetSprite() const
		{
			return m_sprite;
		}

		bool IsWalkable() const
		{
			return !(int)m_mask;
		}

		TileDesc GetMask() const
		{
			return m_mask;
		}
	};




	class Test
	{
	private:
		std::unique_ptr<win32::Window> m_window;
		std::unique_ptr<ICanvas> m_canvas;
		std::unique_ptr<renderer::IRenderer> m_rendererBatch;

		input::Input m_input;
		std::unique_ptr<resource::IFontAtlas> m_FontAtlas;

		timer::StopWatch m_stopwatch;
		double m_elapsed;

		engine::spatial::PositionF m_pos;
		engine::spatial::SizeF m_tilesize;

		
		std::unique_ptr<engine::component::tile::TileRegion<RenderableTile>> m_region;
		engine::navigation::tile::PathFinder m_pathFinder;
		engine::component::tile::Coord m_startTile;
		engine::component::tile::Coord m_goalTile;

		std::vector<component::tile::Coord> m_path;


	public:

		std::function<bool(const engine::component::tile::Coord&, const engine::component::tile::Coord&)> CanMoveDiagonally = 
			[this](const engine::component::tile::Coord& curr, const engine::component::tile::Coord& next) -> bool
			{
				enum Direction
				{
					NE,
					SE,
					NW,
					SW,
					NONE
				};

				// get direction 
				engine::math::Vec<int> dir;
				dir.y = next.row - curr.row;
				dir.x = next.col - curr.col;

				Direction direction;
				direction =
					dir.y > 0 && dir.x > 0 ? SE :
					dir.y < 0 && dir.x < 0 ? NW :
					dir.y < 0 && dir.x > 0 ? NE :
					dir.y > 0 && dir.x < 0 ? SW :
					NONE;

				// if not diagonal, just do a regular compare if block or not
				if (dir.y == 0 || dir.x == 0)
				{
					return m_region->Get(next)->IsWalkable();
				}

				// get adjacent tiles bitmask
				TileDesc CRNC = m_region->Get(curr.row, next.col)->GetMask();
				TileDesc NRCC = m_region->Get(next.row, curr.col)->GetMask();

				//// tile descriptor
				//enum class TileDesc: int
				//{						// NWSE
				//	BLOCKED	= 0b0000,	// 0000 -> clear
				//	CLEAR	= 0b1111,	// 1111 -> blocked
				//	NW		= 0b1100,	// 1100 -> NW blocked half diagonal
				//	SE		= 0b0011,	// 0011 -> SE blocked half diagonal
				//	NE		= 0b1001,	// 1001 -> NE blocked half diagonal
				//	SW		= 0b0110	// 0110 -> SW blocked half diagonal
				//};

				if (CRNC == TileDesc::CLEAR && NRCC == TileDesc::CLEAR)
				{
					return true;
				}
				if (direction == Direction::SE && CRNC == TileDesc::NE && NRCC == TileDesc::SW)
				{
					return true;
				}
				else if (direction == Direction::NW && CRNC == TileDesc::SW && NRCC == TileDesc::NE)
				{
					return true;
				}
				else if (direction == Direction::NE && CRNC == TileDesc::SE && NRCC == TileDesc::NW)
				{
					return true;
				}
				else if (direction == Direction::SW && CRNC == TileDesc::NW && NRCC == TileDesc::SE)
				{
					return true;
				}
				else 
				{
					return false;
				}
			};

		Test():
			m_pos(50, 50),
			m_tilesize(32, 32),
			m_pathFinder(
				[this](int row, int col) -> bool
				{
					return m_region->Get(row, col)->IsWalkable();
				},
				nullptr,
				true,
				false
			),
			m_startTile({ 0,0 }),
			m_goalTile({ 0,0 })
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
			m_window->Create(L"Test Font", 1400, 900);
			m_window->OnWindowMessage += event::Handler(&m_input, &input::Input::ProcessWin32Message);

			m_input.KeyDownEvent += event::Handler(this, &Test::OnKeyDown);
			m_input.MouseDownEvent += event::Handler(this, &Test::OnMouseDown);
			m_input.MouseMoveEvent += event::Handler(this, &Test::OnMouseMove);
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
			m_rendererBatch = std::make_unique<renderer::Renderer>(std::make_unique<dx11::renderer::DX11RendererBatchImpl>());
			m_rendererBatch->Initialize();
			LOG("Renderer Batch (DX11) created...");

			// create font
			m_FontAtlas = std::make_unique<resource::FontAtlas>(std::make_unique<resource::SpriteAtlas>(std::make_unique<dx11::resource::DX11TextureImpl>()));
			m_FontAtlas->Initialize("Terminal", 12);

			{
				// create sprite atlas to be used by tilemap
				engine::graphics::factory::SpriteAtlasFactory::Create("12x1_384x32_tile", L"../Assets/12x1_384x32_tile.png", 1, 12);
				engine::graphics::resource::ISpriteAtlas& atlas = engine::cache::Registry<engine::graphics::resource::ISpriteAtlas>::Instance().Get("12x1_384x32_tile");

				engine::cache::Registry<engine::component::tile::Tileset<RenderableTile>>::Instance().Register("12x1_384x32_tile", std::make_unique<engine::component::tile::Tileset<RenderableTile>>());
				engine::component::tile::Tileset<RenderableTile>& tileset = engine::cache::Registry<engine::component::tile::Tileset<RenderableTile>>::Instance().Get("12x1_384x32_tile");

				tileset.Register(0, std::make_unique<RenderableTile>(atlas.MakeSprite(0), TileDesc::CLEAR)); // walkable
				tileset.Register(1, std::make_unique<RenderableTile>(atlas.MakeSprite(1), TileDesc::CLEAR)); // obstacle
				tileset.Register(2, std::make_unique<RenderableTile>(atlas.MakeSprite(2), TileDesc::CLEAR)); // obstacle
				tileset.Register(3, std::make_unique<RenderableTile>(atlas.MakeSprite(3), TileDesc::CLEAR)); // obstacle
				tileset.Register(4, std::make_unique<RenderableTile>(atlas.MakeSprite(4), TileDesc::BLOCKED)); // walkable
				tileset.Register(5, std::make_unique<RenderableTile>(atlas.MakeSprite(5), TileDesc::CLEAR)); // obstacle
				tileset.Register(6, std::make_unique<RenderableTile>(atlas.MakeSprite(6), TileDesc::CLEAR)); // obstacle
				tileset.Register(7, std::make_unique<RenderableTile>(atlas.MakeSprite(7), TileDesc::CLEAR)); // obstacle
				tileset.Register(8, std::make_unique<RenderableTile>(atlas.MakeSprite(8), TileDesc::SE)); // obstacle
				tileset.Register(9, std::make_unique<RenderableTile>(atlas.MakeSprite(9), TileDesc::SW)); // obstacle
				tileset.Register(10, std::make_unique<RenderableTile>(atlas.MakeSprite(10), TileDesc::NE)); // obstacle
				tileset.Register(11, std::make_unique<RenderableTile>(atlas.MakeSprite(11), TileDesc::NW)); // obstacle

				engine::container::Table<std::string> map({ 24, 16 }, "0");

				m_region = std::make_unique<engine::component::tile::TileRegion<RenderableTile>>();

				engine::loader::tile::AsyncTileRegionLoader<RenderableTile, int> tileRegionLoader;
				tileRegionLoader.LoadImmediate(*m_region, map,
					[this, &tileset](const int& cell) -> engine::component::tile::Tile<RenderableTile>
					{
						return tileset.MakeTile(cell);
					});	

				m_goalTile.row = (int)m_region->GetHeight() - 1;
				m_goalTile.col = (int)m_region->GetWidth() - 1;

				m_region->Set(m_startTile.row, m_startTile.col, tileset.MakeTile(7));
				m_region->Set(m_goalTile.row, m_goalTile.col, tileset.MakeTile(6));

				//m_region->Set(10, 5, tileset.MakeTile(8));
				m_region->Set(8, 4, tileset.MakeTile(8));
				//m_region->Set(9, 4, tileset.MakeTile(11));
				m_region->Set(7, 3, tileset.MakeTile(11));

				m_region->Set(5, 15, tileset.MakeTile(11));


				m_region->Set(11, 12, tileset.MakeTile(9));
				m_region->Set(10, 13, tileset.MakeTile(10));
				m_region->Set(10, 11, tileset.MakeTile(9));
				m_region->Set( 9, 12, tileset.MakeTile(10));
				m_region->Set( 9, 10, tileset.MakeTile(9));
				m_region->Set( 8, 11, tileset.MakeTile(10));

				m_pathFinder.SetCanMoveDiagonallyFunc(CanMoveDiagonally);

			}

			// setup stopwatch to manage timing and start it
			m_stopwatch.OnLap += event::Handler(this, &Test::OnLap);
			m_stopwatch.Start();
		}

		void OnKeyDown(int key)
		{
			engine::component::tile::Tileset<RenderableTile>& tileset = engine::cache::Registry<engine::component::tile::Tileset<RenderableTile>>::Instance().Get("12x1_384x32_tile");

			switch (key)
			{
			case 49: // 1
				// remove all obstacles
				for (int row = 0; row < m_region->GetHeight(); row++)
				{
					for (int col = 0; col < m_region->GetWidth(); col++)
					{
						if (!m_region->Get(row, col)->IsWalkable())
						{
							m_region->Set(row, col, tileset.MakeTile(0));
						}
					}
				}
				break;
			default:
				break;
			}
		}

		void OnMouseDown(int btn, int x, int y)
		{
			// get selected tile row and col based on mouse position
			int col = (int)((x - m_pos.x) / m_tilesize.width);
			int row = (int)((y - m_pos.y) / m_tilesize.height);

			// get map size
			engine::spatial::Size<size_t> size = m_region->GetSize();

			// if row and col is out of bounds, bail out
			if (col >= size.width || row >= size.height) return;

			// skip if this tile is block tile
			if (!m_region->Get(row, col)->IsWalkable()) return;

			// get tileset
			engine::component::tile::Tileset<RenderableTile>& tileset = engine::cache::Registry<engine::component::tile::Tileset<RenderableTile>>::Instance().Get("12x1_384x32_tile");

			for (int i = 1; i < (int)m_path.size() - 1; i++)
			{
				m_region->Set(m_path[i].row, m_path[i].col, tileset.MakeTile(0));
			}
			m_path.clear();

			if (btn == 1)
			{
				// replace tile 
				m_region->Set(m_startTile.row, m_startTile.col, tileset.MakeTile(0));

				m_startTile.row = row;
				m_startTile.col = col;

				// replace tile 
				m_region->Set(row, col, tileset.MakeTile(7));
			}
			else if (btn == 2)
			{
				// replace tile 
				m_region->Set(m_goalTile.row, m_goalTile.col, tileset.MakeTile(0));

				m_goalTile.row = row;
				m_goalTile.col = col;

				// replace tile 
				m_region->Set(row, col, tileset.MakeTile(6));
			}
			else if (btn == 3)
			{


				// replace tile 
				m_region->Set(row, col, tileset.MakeTile(4));
			}
		}

		void OnMouseMove(int x, int y)
		{

		}

		// this method is fired up whenever the OnLap event is triggered from stopwatch
		void OnLap(double time)
		{
			// save current path
			std::vector<component::tile::Coord> m_prevPath = m_path;

			// find path
			math::geometry::Rect<int> map = { 0, 0, (int)m_region->GetWidth(), (int)m_region->GetHeight() };
			m_pathFinder.FindPath(
				map,
				m_startTile,
				m_goalTile,
				m_path
			);

			// get tileset
			engine::component::tile::Tileset<RenderableTile>& tileset = engine::cache::Registry<engine::component::tile::Tileset<RenderableTile>>::Instance().Get("12x1_384x32_tile");


			// clear tiles that were previous path
			for (int i = 1; i < (int)m_prevPath.size() - 1; i++)
			{
				m_region->Set(m_prevPath[i].row, m_prevPath[i].col, tileset.MakeTile(0));
			}

			// set tiles with new path
			for (int i = 1; i < (int)m_path.size() - 1; i++)
			{
				m_region->Set(m_path[i].row, m_path[i].col, tileset.MakeTile(1));
			}
		}

		// fun stuff. this is called on each loop of the message loop. this is where we draw!
		void OnIdle()
		{
			// call lap to get elapsed time and trigger OnLap event
			m_stopwatch.Lap<engine::timer::milliseconds>();

			m_input.Update();

			m_canvas->Clear({ 0.2f, 0.2f, 1.0f, 1.0f });

			// start the canvas. we can draw from here
			m_canvas->Begin();
			{
				m_rendererBatch->Begin();

				// get the tile region
				engine::component::tile::TileMap<RenderableTile> tilemap = m_region->MakeTileMap();

				// draw the tile region
				engine::graphics::tile::DrawTileMap(*m_rendererBatch, tilemap, m_tilesize, m_pos, { 1,1,1,1 });

				std::vector<engine::component::tile::Coord> wp = engine::navigation::tile::GetWayPoints(m_path);
				engine::graphics::navigation::DrawWaypoints(*m_rendererBatch, wp, m_tilesize, m_pos, { 1,1,1,1 }, 6.0f);


				wp = engine::navigation::tile::SmoothWayPoints(wp, [&tilemap](int row, int col) { return tilemap.IsInBounds(row, col) ? tilemap.Get(row, col)->IsWalkable() : false; });
				engine::graphics::navigation::DrawWaypoints(*m_rendererBatch, wp, m_tilesize, m_pos, { 0,1,0,1 }, 4.0f);

				wp = engine::navigation::tile::SmoothWayPoints(m_path, [&tilemap](int row, int col) { return tilemap.IsInBounds(row, col) ? tilemap.Get(row, col)->IsWalkable() : false; });
				engine::graphics::navigation::DrawWaypoints(*m_rendererBatch, wp, m_tilesize, m_pos, { 1,0,1,1 }, 2.0f);

				
				m_rendererBatch->End();
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
	};
}