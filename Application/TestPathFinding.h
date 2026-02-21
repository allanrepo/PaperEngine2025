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
#include <Graphics/Core/Draw.h>

using namespace engine;
using namespace engine::graphics;

namespace TestPathFinding
{
	class RenderableTile
	{
	private:
		engine::graphics::renderable::Sprite m_sprite;
		bool m_walkable;
		engine::graphics::animation::Animator<engine::graphics::renderable::Sprite>* m_animator;

	public:
		RenderableTile(const engine::graphics::renderable::Sprite& sprite, bool walkable, engine::graphics::animation::Animator<engine::graphics::renderable::Sprite>* animator = nullptr) :
			m_sprite(sprite),
			m_walkable(walkable),
			m_animator(animator)
		{
		}

		const engine::graphics::renderable::Sprite& GetSprite() const
		{
			if (m_animator)
			{
				return m_animator->GetCurrentFrame().element;
			}
			return m_sprite;
		}
		bool IsWalkable() const
		{
			return m_walkable;
		}
	};

	class Test
	{
	private:
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

		
		std::unique_ptr< engine::component::tile::TileRegion<RenderableTile>> m_region;
		engine::navigation::tile::PathFinder m_pathFinder;
		engine::component::tile::Coord m_startTile;
		engine::component::tile::Coord m_goalTile;

		std::vector<component::tile::Coord> m_path;


	public:
		Test():
			m_pos(50, 50),
			m_tilesize(32, 32),
			m_pathFinder(
				[this](int currRow, int currCol, int row, int col) -> bool
				{
					return m_region->Get(row, col)->IsWalkable();
				},
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
				engine::graphics::factory::SpriteAtlasFactory::Create("1x8_256x32_tile", L"../Assets/1x8_256x32_tile.png", 1, 8);
				engine::graphics::resource::ISpriteAtlas& atlas = engine::cache::Registry<engine::graphics::resource::ISpriteAtlas>::Instance().Get("1x8_256x32_tile");

				engine::cache::Registry<engine::component::tile::Tileset<RenderableTile>>::Instance().Register("1x8_256x32_tile", std::make_unique<engine::component::tile::Tileset<RenderableTile>>());
				engine::component::tile::Tileset<RenderableTile>& tileset = engine::cache::Registry<engine::component::tile::Tileset<RenderableTile>>::Instance().Get("1x8_256x32_tile");

				tileset.Register(0, std::make_unique<RenderableTile>(atlas.MakeSprite(0), true)); // walkable
				tileset.Register(1, std::make_unique<RenderableTile>(atlas.MakeSprite(1), true)); // obstacle
				tileset.Register(2, std::make_unique<RenderableTile>(atlas.MakeSprite(2), true)); // obstacle
				tileset.Register(3, std::make_unique<RenderableTile>(atlas.MakeSprite(3), true)); // obstacle
				tileset.Register(4, std::make_unique<RenderableTile>(atlas.MakeSprite(4), false)); // walkable
				tileset.Register(5, std::make_unique<RenderableTile>(atlas.MakeSprite(5), true)); // obstacle
				tileset.Register(6, std::make_unique<RenderableTile>(atlas.MakeSprite(6), true)); // obstacle
				tileset.Register(7, std::make_unique<RenderableTile>(atlas.MakeSprite(7), true)); // obstacle

				engine::container::Table<std::string> map({ 24, 16 }, "0");

				//engine::cache::Registry<engine::component::tile::TileRegion<RenderableTile>>::Instance().Register("1x8_256x32_tile", std::make_unique<engine::component::tile::TileRegion<RenderableTile>>());
				//engine::component::tile::TileRegion<RenderableTile>& region = engine::cache::Registry<engine::component::tile::TileRegion<RenderableTile>>::Instance().Get("1x8_256x32_tile");
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
			}

			// setup stopwatch to manage timing and start it
			m_stopwatch.OnLap += event::Handler(this, &Test::OnLap);
			m_stopwatch.Start();
		}

		void OnKeyDown(int key)
		{
			engine::component::tile::Tileset<RenderableTile>& tileset = engine::cache::Registry<engine::component::tile::Tileset<RenderableTile>>::Instance().Get("1x8_256x32_tile");

			switch (key)
			{
			//case 27: // escape
			//	m_pathFinder->SetMaxSteps(m_step = 0);
			//	break;
			//case 32: // space
			//	m_pathFinder->SetMaxSteps(m_step++);
			//	break;
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
			//case 51: // 3
			//	m_drawText = !m_drawText;
			//	break;
			//case 52: // 4
			//	m_drawPathFindingTiles = !m_drawPathFindingTiles;
			//	break;
			//case 53: // 5
			//	m_drawWaypoint = !m_drawWaypoint;
			//	break;
			//case 54: // 6
			//	m_pathFinder->EnableDiagonal(!m_pathFinder->IsDiagonalEnabled());
			//	break;
			//case 55: // 7
			//	m_pathFinder->EnableCutCorners(!m_pathFinder->IsCutCornersEnabled());
			//	break;
			//case 56: // 8
			//	m_pathFinder = (m_pathFinder == &m_pathFinderVector) ? &m_pathFinderPriorityQueue : &m_pathFinderVector;
			//	//m_pathFinder->SetMaxSteps(m_step = 0);
			//	break;
			//case 81: // q
			//	SetTileLayer(m_tileLayer, 24, 16, component::tile::TileInstance{ 0 });
			//	m_pathFinder->SetMaxSteps(m_step = 0);
			//	break;
			//case 87: // w
			//	m_tileLayer = engine::io::TileLayerLoader<int>::LoadFromCSV("PathFindingMap_24x16.csv", ',');
			//	m_pathFinder->SetMaxSteps(m_step = 0);
			//	break;
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
			engine::component::tile::Tileset<RenderableTile>& tileset = engine::cache::Registry<engine::component::tile::Tileset<RenderableTile>>::Instance().Get("1x8_256x32_tile");

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
			engine::component::tile::Tileset<RenderableTile>& tileset = engine::cache::Registry<engine::component::tile::Tileset<RenderableTile>>::Instance().Get("1x8_256x32_tile");


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
				DrawTileMap(tilemap, m_tilesize, m_pos);

				std::vector<engine::spatial::PositionF> wp = navigation::tile::GetWayPoints(m_path);
				DrawWaypoints(wp, m_tilesize, m_pos, { 1,1,1,1 });

				wp = SmoothWayPoints(wp, m_region->MakeTileMap());
				DrawWaypoints(wp, m_tilesize, m_pos, { 0,1,0,1 });


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

		void DrawTileMap(const engine::component::tile::TileMap<RenderableTile>& tilemap, const engine::spatial::SizeF& tilesize, const engine::spatial::PositionF& pos)
		{
			for (int row = 0; row <= tilemap->GetHeight(); ++row)
			{
				for (int col = 0; col <= tilemap->GetWidth(); ++col)
				{
					if (!tilemap->IsInBounds(row, col))
					{
						continue;
					}

					const engine::component::tile::Tile<RenderableTile>& tile = tilemap->Get(row, col);
					if (tile.isValid())
					{
						engine::spatial::PositionF origin =
						{
							col * tilesize.width,
							row * tilesize.height
						};


						m_rendererBatch->DrawRenderable(tile->GetSprite(), pos + origin, tilesize, engine::graphics::ColorF{ 1.0f, 1.0f, 1.0f, 1.0f }, 0.0f);
					}
				}
			}
		}

		void DrawWaypoints(const std::vector<engine::spatial::PositionF>& wp, const engine::spatial::SizeF& tilesize, const engine::spatial::PositionF& pos, const engine::graphics::ColorF& color)
		{
			for (size_t i = 1; i < wp.size(); i++)
			{
				engine::spatial::PositionF start
				{
					wp[i - 1].x * tilesize.width + tilesize.width / 2,
					wp[i - 1].y * tilesize.height + tilesize.height / 2
				};
				engine::spatial::PositionF end
				{
					wp[i].x * tilesize.width + tilesize.width / 2,
					wp[i].y * tilesize.height + tilesize.height / 2
				};

				start += pos;
				end += pos;

				engine::graphics::primitives::DrawLineSegment(*m_rendererBatch, start, end, color, 4.0f);
			}
		}
		std::vector<engine::spatial::PositionF> SmoothWayPoints(
			const std::vector<engine::spatial::PositionF>& waypoints,
			const engine::component::tile::TileMap<RenderableTile>& map
		)
		{
			std::vector<engine::spatial::PositionF> smoothed;

			if (waypoints.empty()) {
				return smoothed;
			}

			// Always keep the first waypoint
			smoothed.push_back(waypoints.front());

			size_t i = 0;
			while (i < waypoints.size() - 1) 
			{
				size_t j = waypoints.size() - 1;

				// Try to jump as far ahead as possible
				for (; j > i + 1; --j) 
				{
					if (IsRegionClear(waypoints[i], waypoints[j], map))
					{
						break; // found a clear jump
					}
				}

				// Keep the farthest reachable waypoint
				smoothed.push_back(waypoints[j]);
				i = j;
			}

			return smoothed;
		}

		bool IsRegionClear(
			const engine::spatial::PositionF& a,
			const engine::spatial::PositionF& b,
			const engine::component::tile::TileMap<RenderableTile>& map
		)
		{
			int mincol = (int)std::floor(std::min<float>(a.x, b.x));
			int maxcol = (int)std::floor(std::max<float>(a.x, b.x));
			int minrow = (int)std::floor(std::min<float>(a.y, b.y));
			int maxrow = (int)std::floor(std::max<float>(a.y, b.y));

			for (int row = minrow; row <= maxrow; ++row) {
				for (int col = mincol; col <= maxcol; ++col) {
					if (!map->Get(row, col)->IsWalkable()) {
						return false;
					}
				}
			}
			return true;
		}

		template<typename T, typename Predicate>
		bool IsRegionClear(
			const engine::spatial::PositionF& a,
			const engine::spatial::PositionF& b,
			const T& map,
			Predicate isWalkable
		)
		{
			int mincol = (int)std::floor(std::min<float>(a.x, b.x));
			int maxcol = (int)std::floor(std::max<float>(a.x, b.x));
			int minrow = (int)std::floor(std::min<float>(a.y, b.y));
			int maxrow = (int)std::floor(std::max<float>(a.y, b.y));

			for (int row = minrow; row <= maxrow; ++row) {
				for (int col = mincol; col <= maxcol; ++col) {
					if (!isWalkable(map.Get(row, col))) {
						return false;
					}
				}
			}
			return true;
		}

	};
}