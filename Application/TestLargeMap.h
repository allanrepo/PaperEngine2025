#pragma once

#include <Win32/Window.h>
#include <Core/Event.h>
#include <Utilities/Logger.h>
#include <Graphics/Core/ICanvas.h>
#include <Graphics/Core/Canvas.h>
#include <Graphics/Core/DX11CanvasImpl.h>
#include <Graphics/Renderer/IRenderer.h>
#include <Graphics/Renderer/DX11RendererBatchImpl.h>
#include <Graphics/Renderer/Renderer.h>
#include <Graphics/Resource/ISpriteAtlas.h>
#include <Graphics/Resource/SpriteAtlas.h>
#include <Engine/Factory/SpriteAtlasFactory.h>
#include <Engine/Loader/SpriteAtlasLoader.h>
#include <Graphics/Renderable/Sprite.h>
#include <Core/Input.h>
#include <Graphics/Animation/Animation.h>
#include <Graphics/Resource/DX11TextureImpl.h>
#include <Timer/StopWatch.h>
#include <Components/Tile.h>
#include <Spatial/Camera.h>
#include <Spatial/Position.h>
#include <State/State.h>

#include "Utilities.h"

namespace TestLargeMap
{
	class RenderableTile
	{
	private:
		engine::graphics::renderable::Sprite m_sprite;
		bool m_walkable;

	public:
		RenderableTile(const engine::graphics::renderable::Sprite& sprite, bool walkable) :
			m_sprite(sprite),
			m_walkable(walkable)
		{
		}
		const engine::graphics::renderable::Sprite& GetSprite() const
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
	private:
		std::unique_ptr<engine::win32::Window> m_window;
		std::unique_ptr<engine::graphics::ICanvas> m_canvas;
		std::unique_ptr<engine::graphics::renderer::IRenderer> m_renderer;
		std::unique_ptr<engine::graphics::resource::ISpriteAtlas> m_spriteAtlas;
		engine::timer::StopWatch m_stopwatch;
		engine::component::tile::Tileset<RenderableTile> m_tileset;
		engine::component::tile::TileGrid<RenderableTile> m_tilegrid;
		engine::spatial::SizeF m_tileSize{ 32.0f, 32.0f };
		engine::spatial::CameraF m_camera;
		engine::spatial::PositionF m_lastMousePos;
		bool m_isPanning = false;
		engine::spatial::PositionF m_focusPos;
		int v;

	public:
		Test() :
			m_camera({ 250, 250, 720, 640 })
		{
			engine::win32::Window::OnInitialize += engine::event::Handler(this, &Test::OnInitialize);
			engine::win32::Window::OnExit += engine::event::Handler(this, &Test::OnExit);
			engine::win32::Window::OnIdle += engine::event::Handler(this, &Test::OnIdle);

			engine::input::Input::Instance().MouseDownEvent += engine::event::Handler(this, &Test::OnMouseDown);
			engine::input::Input::Instance().MouseMoveEvent += engine::event::Handler(this, &Test::OnMouseMove);
			engine::input::Input::Instance().MouseUpEvent += engine::event::Handler(this, &Test::OnMouseUp);

			engine::win32::Window::Run();
		}

		void OnMouseMove(int x, int y)
		{
			// is we're holding down left mouse button and dragging it, pan the map
			if (m_isPanning)
			{
				// get the change in position and move camera position by that
				engine::math::VecF delta = engine::math::VecF((float)x, (float)y) - m_lastMousePos;
				m_camera.MoveBy(delta);

				// remember the last mouse position
				m_lastMousePos = { (float)x, (float)y };
			}
		}

		void OnMouseDown(int btn, int x, int y)
		{
			// this button is for panning the camera
			if (btn == 1)
			{
				m_isPanning = true;
				m_lastMousePos = { (float)x, (float)y };
			}
			// if this button is clicked, move our focus object in this position
			if (btn == 2)
			{
				// this is screen position and convert it to world position
				engine::spatial::PositionF pos((float)x, (float)y);
				pos = m_camera.ScreenToWorld(pos);
				m_focusPos = pos;

				// pan the camera such that the focus object is at center of the viewport, if possible
				m_camera.CenterOn(m_focusPos);
			}
		}

		void OnMouseUp(int btn, int x, int y)
		{
			m_isPanning = false;
		}

		// function that will be called just before we enter into message loop
		void OnInitialize()
		{
			// create our window here
			m_window = std::make_unique<engine::win32::Window>();
			m_window->OnClose += engine::event::Handler(this, &Test::OnWindowClose);
			m_window->OnCreate += engine::event::Handler(this, &Test::OnWindowCreate);
			m_window->OnSize += engine::event::Handler(this, &Test::OnWindowSize);
			m_window->OnWindowMessage += engine::event::Handler(&engine::input::Input::Instance(), &engine::input::Input::ProcessWin32Message);

			m_window->Create(L"Test Camera", 1400, 1200);
		}

		// when window is created. we can now safely create resources dependent on window
		void OnWindowCreate(void* hWnd)
		{
			LOG("Window created...");

			// create dx11 canvas
			m_canvas = std::make_unique<engine::graphics::Canvas>(std::make_unique<engine::graphics::dx11::DX11CanvasImpl>());
			m_canvas->Initialize(hWnd);
			m_canvas->SetViewPort();
			LOG("Canvas (DX11) created...");

			// create dx11 renderer batched
			m_renderer = std::make_unique<engine::graphics::renderer::Renderer>(std::make_unique<engine::graphics::dx11::renderer::DX11RendererBatchImpl>());
			m_renderer->Initialize();
			LOG("Renderer (DX11) created...");

			// create sprite atlas manually for demo purpose
			m_spriteAtlas = std::make_unique<engine::graphics::resource::SpriteAtlas>(std::make_unique<engine::graphics::dx11::resource::DX11TextureImpl>());

			// load sprite atlas from file manually for demo purpose
			m_spriteAtlas->Initialize(L"../Assets/4x1_128x32_tile.png");

			// load sprite atlas UVs from csv manually for demo purpose. we calculate UVs here by assuming a grid of 8 rows and 12 columns
			// in real scenario, you would use SpriteAtlasLoader to load from csv file 
			std::vector<engine::math::geometry::RectF> uvs = app::utilities::graphics::CalcUV(1, 4, (int)m_spriteAtlas->GetWidth(), (int)m_spriteAtlas->GetHeight());
			for (engine::math::geometry::RectF& rect : uvs)
			{
				m_spriteAtlas->AddUVRect(rect);
			}

			// register tiles
			m_tileset.Register(0, std::make_unique<RenderableTile>(m_spriteAtlas->MakeSprite(0), true)); // walkable
			m_tileset.Register(1, std::make_unique<RenderableTile>(m_spriteAtlas->MakeSprite(1), false)); // obstacle

			// load map into tile layer
			{
				engine::io::AsyncFileReader	fileReader(0xFF);
				fileReader.Open("../Assets/32x32Map.csv");

				engine::utilities::parser::CSVParser csvParser;
				fileReader.ProcessChunkEvent += engine::event::Handler(&csvParser, &engine::utilities::parser::CSVParser::ParseChunk);
				fileReader.EndOfFileFoundEvent += engine::event::Handler(&csvParser, &engine::utilities::parser::CSVParser::ParseRemaining);
				engine::container::Table<std::string> table;

				csvParser.ParseRowEvent += engine::event::Handler(&table, &engine::container::Table<std::string>::AddRow);
				csvParser.ParseRemainingEvent += engine::event::Handler(&table, &engine::container::Table<std::string>::AddRange);

				fileReader.SyncReadAll(0xFF, 5.0);

				engine::loader::tile::AsyncTileGridLoader<RenderableTile, int> tileLoader;
				tileLoader.LoadImmediate(
					m_tilegrid,
					table,
					[this](const int& cell) -> engine::component::tile::Tile<RenderableTile>
					{
						// this is safe. tileset will return "empty" tile if id is invalid. "empty" means does not have reference to tile data. tile is invalid
						return m_tileset.MakeTile(cell);
					},
					0xFFFF,
					5.0
				);

				fileReader.ProcessChunkEvent -= engine::event::Handler(&csvParser, &engine::utilities::parser::CSVParser::ParseChunk);
				fileReader.EndOfFileFoundEvent -= engine::event::Handler(&csvParser, &engine::utilities::parser::CSVParser::ParseRemaining);
				csvParser.ParseRowEvent -= engine::event::Handler(&table, &engine::container::Table<std::string>::AddRow);
				csvParser.ParseRemainingEvent -= engine::event::Handler(&table, &engine::container::Table<std::string>::AddRange);
			}

			// tell camera the size of the world. this will be the tile map
			m_camera.SetWorldSize(
				m_tilegrid.GetWidth() * m_tileSize.width,
				m_tilegrid.GetHeight() * m_tileSize.height
			);

			// setup stopwatch to manage timing and start it
			m_stopwatch.OnLap += engine::event::Handler(this, &Test::OnLap);
			m_stopwatch.Start();
		}

		// this method is fired up whenever the OnLap event is triggered from stopwatch
		void OnLap(double time)
		{
		}

		// fun stuff. this is called on each loop of the message loop. this is where we draw!
		void OnIdle()
		{
			engine::input::Input::Instance().Update();

			// call lap to get elapsed time and trigger OnLap event
			m_stopwatch.Lap<engine::timer::milliseconds>();

			// does not need to do this every frame. just do this once everytime camera viewport changes
			m_renderer->SetClipRegion(m_camera.GetViewport());

			// start the canvas. we can draw from here
			m_canvas->Begin();
			{
				m_canvas->Clear({ 0.2f, 0.2f, 1.0f, 1.0f });

				m_renderer->Begin();
				{
					m_renderer->EnableClipping(false);
					RenderTiles(m_tilegrid, 0.3f);

					m_renderer->EnableClipping(true);
					RenderTiles(m_tilegrid);

					m_renderer->EnableClipping(false);

					// draw a red rectangle that shows where the camera position is (top-left position of viewport)
					{
						// get the position of camera in world coordinates
						engine::spatial::PositionF camPos = m_camera.GetPosition();

						// convert to screen position
						camPos = m_camera.WorldToScreen(camPos);

						// then draw. add a little offset to make the rectangle centered on position
						m_renderer->Draw(
							{ camPos.x - 10, camPos.y - 10 },
							{ 20, 20 },
							{ 1,0,0,1 },
							0
						);
					}

					// draw a yellow rectangle that shows where the "focus" object is in the map (or the screen)
					{
						// focus object's position is in world space (map). convert to screen position
						engine::spatial::PositionF focusPos = m_camera.WorldToScreen(m_focusPos);

						// then draw. add a little offset to make the rectangle centered on position
						m_renderer->Draw(
							{ focusPos.x - 10, focusPos.y - 10 },
							{ 20, 20 },
							{ 1,1,0,1 },
							0
						);
					}
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

		void RenderTiles(engine::component::tile::TileGrid<RenderableTile>& tilegrid, float alpha = 1.0f)
		{
			engine::math::geometry::RectF vp = m_camera.GetViewport();
			engine::spatial::PositionF camPos = m_camera.GetPosition();

			int left = (int)(camPos.x / m_tileSize.width);
			int top = (int)(camPos.y / m_tileSize.height);
			int right = (int)((camPos.x + vp.GetWidth()) / m_tileSize.width);
			int bottom = (int)((camPos.y + vp.GetHeight()) / m_tileSize.height);

			for (int row = top; row <= bottom; ++row)
			{
				for (int col = left; col <= right; ++col)
				{
					if (!tilegrid.IsInBounds(row, col))
					{
						continue;
					}

					const engine::component::tile::Tile<RenderableTile>& tile = tilegrid.Get(row, col);
					if (tile.isValid())
					{
						engine::spatial::PositionF pos =
						{
							col * m_tileSize.width,
							row * m_tileSize.height
						};

						m_renderer->DrawRenderable(
							tile->GetSprite(),
							m_camera.WorldToScreen(pos),
 							m_tileSize,
							engine::graphics::ColorF{ 1.0f, 1.0f, 1.0f, alpha },
							0.0f
						);
					}
				}
			}
		}


	};
}