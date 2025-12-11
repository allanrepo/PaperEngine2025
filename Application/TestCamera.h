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
#include <Graphics/Renderable/ISpriteAtlas.h>
#include <Graphics/Renderable/SpriteAtlas.h>
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

#include "Utilities.h"

namespace TestCamera
{
	class RenderableTile
	{
	private:
		graphics::renderable::Sprite m_sprite;
		bool m_walkable;

	public:
		RenderableTile(const graphics::renderable::Sprite& sprite, bool walkable) :
			m_sprite(sprite),
			m_walkable(walkable)
		{
		}
		const graphics::renderable::Sprite& GetSprite() const
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
		std::unique_ptr<Win32::Window> m_window;
		std::unique_ptr<graphics::ICanvas> m_canvas;
		std::unique_ptr<graphics::renderer::IRenderer> m_renderer;
		std::unique_ptr<graphics::renderable::ISpriteAtlas> m_spriteAtlas;
		timer::StopWatch m_stopwatch;
		component::tile::Tileset<RenderableTile> m_tileset;
		component::tile::TileGrid<RenderableTile> m_tilegrid;
		spatial::SizeF m_tileSize{ 32.0f, 32.0f };
		spatial::CameraF m_camera;
		spatial::PositionF m_lastMousePos;
		bool m_isPanning = false;
		spatial::PositionF m_focusPos;


	public:
		Test():
			m_camera({ 250, 250, 720, 640 })
		{
			Win32::Window::OnInitialize += event::Handler(this, &Test::OnInitialize);
			Win32::Window::OnExit += event::Handler(this, &Test::OnExit);
			Win32::Window::OnIdle += event::Handler(this, &Test::OnIdle);

			input::Input::Instance().OnMouseDown += event::Handler(this, &Test::OnMouseDown);
			input::Input::Instance().OnMouseMove += event::Handler(this, &Test::OnMouseMove);
			input::Input::Instance().OnMouseUp += event::Handler(this, &Test::OnMouseUp);

			Win32::Window::Run();
		}

		void OnMouseMove(int x, int y)
		{
			// is we're holding down left mouse button and dragging it, pan the map
			if (m_isPanning)
			{
				// get the change in position and move camera position by that
				math::VecF delta = math::VecF((float)x, (float)y) - m_lastMousePos;
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
				spatial::PositionF pos((float)x, (float)y);
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
			m_window = std::make_unique<Win32::Window>();
			m_window->OnClose += event::Handler(this, &Test::OnWindowClose);
			m_window->OnCreate += event::Handler(this, &Test::OnWindowCreate);
			m_window->OnSize += event::Handler(this, &Test::OnWindowSize);
			m_window->OnWindowMessage += event::Handler(&input::Input::Instance(), &input::Input::ProcessWin32Message);

			m_window->Create(L"Test Camera", 1400, 1200);
		}

		// when window is created. we can now safely create resources dependent on window
		void OnWindowCreate(void* hWnd)
		{
			LOG("Window created...");

			// create dx11 canvas
			m_canvas = std::make_unique<graphics::Canvas>(std::make_unique<graphics::dx11::DX11CanvasImpl>());
			m_canvas->Initialize(hWnd);
			m_canvas->SetViewPort();
			LOG("Canvas (DX11) created...");

			// create dx11 renderer batched
			m_renderer = std::make_unique<graphics::renderer::Renderer>(std::make_unique<graphics::dx11::renderer::DX11RendererBatchImpl>());
			m_renderer->Initialize();
			LOG("Renderer (DX11) created...");

			// create sprite atlas manually for demo purpose
			m_spriteAtlas = std::make_unique<graphics::renderable::SpriteAtlas>(std::make_unique<graphics::dx11::resource::DX11TextureImpl>());

			// load sprite atlas from file manually for demo purpose
			m_spriteAtlas->Initialize(L"../Assets/4x1_128x32_tile.png");

			// load sprite atlas UVs from csv manually for demo purpose. we calculate UVs here by assuming a grid of 8 rows and 12 columns
			// in real scenario, you would use SpriteAtlasLoader to load from csv file 
			std::vector<math::geometry::RectF> uvs = utilities::graphics::CalcUV(1, 4, (int)m_spriteAtlas->GetWidth(), (int)m_spriteAtlas->GetHeight());
			for (math::geometry::RectF& rect : uvs)
			{
				m_spriteAtlas->AddUVRect(rect);
			}

			// register tiles
			m_tileset.Register(0, std::make_unique<RenderableTile>(m_spriteAtlas->MakeSprite(0), true)); // walkable
			m_tileset.Register(1, std::make_unique<RenderableTile>(m_spriteAtlas->MakeSprite(1), false)); // obstacle

			// load map into tile layer
			m_tilegrid = utilities::io::TileGridLoader<RenderableTile, int>::LoadFromCSV(
				"../Assets/32x32Map.csv",
				m_tileset,
				[](int row, int col, const int& cell, const component::tile::Tileset<RenderableTile>& tileset) -> component::tile::Tile<RenderableTile>
				{
					// this is safe. tileset will return "empty" tile if id is invalid. "empty" means does not have reference to tile data. tile is invalid
					return tileset.MakeTile(cell);
				}
			);

			// tell camera the size of the world. this will be the tile map
			m_camera.SetWorldSize(
				m_tilegrid.GetWidth() * m_tileSize.width,
				m_tilegrid.GetHeight() * m_tileSize.height
			);

			// setup stopwatch to manage timing and start it
			m_stopwatch.OnLap += event::Handler(this, &Test::OnLap);
			m_stopwatch.Start();
		}

		// this method is fired up whenever the OnLap event is triggered from stopwatch
		void OnLap(float time)
		{
		}

		// fun stuff. this is called on each loop of the message loop. this is where we draw!
		void OnIdle()
		{
			input::Input::Instance().Update();

			// call lap to get elapsed time and trigger OnLap event
			m_stopwatch.Lap<timer::milliseconds>();

			// does not need to do this every frame. just do this once everytime camera viewport changes
			m_renderer->SetClipRegion(m_camera.GetViewport());

			// start the canvas. we can draw from here
			m_canvas->Begin();
			{
				m_canvas->Clear(0.2f, 0.2f, 1.0f, 1.0f);

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
						spatial::PositionF camPos = m_camera.GetPosition();

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
						spatial::PositionF focusPos = m_camera.WorldToScreen(m_focusPos);

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
			m_canvas->Resize(static_cast<unsigned int>(nWidth), static_cast<unsigned int>(nHeight));
			m_canvas->SetViewPort();
		}


		void RenderTiles(component::tile::TileGrid<RenderableTile>& tilegrid, float alpha = 1.0f)
		{
			math::geometry::RectF vp = m_camera.GetViewport();
			spatial::PositionF camPos = m_camera.GetPosition();

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

					const component::tile::Tile<RenderableTile>& tile = tilegrid.GetTile(row, col);
					if (tile.isValid())
					{
						spatial::PositionF pos =
						{
							col * m_tileSize.width,
							row * m_tileSize.height
						};

						m_renderer->DrawRenderable(
							tile->GetSprite(),
							m_camera.WorldToScreen(pos),
							m_tileSize,
							graphics::ColorF{ 1.0f, 1.0f, 1.0f, alpha },
							0.0f
						);
					}
				}
			}
		}
	};
}