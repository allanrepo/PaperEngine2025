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
#include <Graphics/Resource/Texture.h>
#include <Graphics/Resource/DX11TextureImpl.h>
#include <Engine/Factory/TextureFactory.h>
#include <Graphics/Resource/IFontAtlas.h>
#include <Graphics/Resource/FontAtlas.h>
#include <Graphics/Renderable/RenderSurface.h>

namespace test
{
	class TestCanvas
	{
	private:
		std::unique_ptr<engine::win32::Window> m_window;
		std::unique_ptr<engine::graphics::ICanvas> m_canvas;
		std::unique_ptr<engine::graphics::renderer::IRenderer> m_renderer;
		std::unique_ptr<engine::graphics::renderable::IRenderSurface> m_renderSurface;
		std::unique_ptr<engine::graphics::resource::IFontAtlas> m_fontAtlas;
		std::unique_ptr<engine::graphics::resource::ISpriteAtlas> m_imageSurface;

	public:
		TestCanvas()
		{
			engine::win32::Window::OnInitialize += engine::event::Handler(this, &TestCanvas::OnInitialize);
			engine::win32::Window::OnExit += engine::event::Handler(this, &TestCanvas::OnExit);
			engine::win32::Window::OnIdle += engine::event::Handler(this, &TestCanvas::OnIdle);

			engine::win32::Window::Run();
		}

		// function that will be called just before we enter into message loop
		void OnInitialize()
		{
			// create our window here
			m_window = std::make_unique<engine::win32::Window>();
			m_window->OnClose += engine::event::Handler(this, &TestCanvas::OnWindowClose);
			m_window->OnCreate += engine::event::Handler(this, &TestCanvas::OnWindowCreate);
			m_window->OnSize += engine::event::Handler(this, &TestCanvas::OnWindowSize);
			m_window->Create(L"Test Canvas", 1400, 900);
		}

		// when window is created. we can now safely create resources dependent on window
		void OnWindowCreate(void* hWnd)
		{
			LOG("Window created...");

			// create dx11 canvas
			m_canvas = std::make_unique<engine::graphics::Canvas>(std::make_unique<engine::graphics::dx11::DX11CanvasImpl>());
			m_canvas->Initialize(hWnd);
			m_canvas->SetViewPort();

			// create dx11 renderer batched
			m_renderer = std::make_unique<engine::graphics::renderer::Renderer>(std::make_unique<engine::graphics::dx11::renderer::DX11RendererBatchImpl>());
			m_renderer->Initialize();

			// create font atlas
			m_fontAtlas = std::make_unique<engine::graphics::resource::FontAtlas>(std::make_unique<engine::graphics::resource::SpriteAtlas>(std::make_unique<engine::graphics::dx11::resource::DX11TextureImpl>()));
			m_fontAtlas->Initialize("Terminal", 12);

			// create image surface
			m_imageSurface = std::make_unique<engine::graphics::resource::SpriteAtlas>(std::make_unique<engine::graphics::resource::Texture>(std::make_unique<engine::graphics::dx11::resource::DX11TextureImpl>()));
			m_imageSurface->Initialize(L"../Assets/256x256.bmp");

			// create dx11 texture and use on drawable surface
			m_renderSurface = std::make_unique<engine::graphics::renderable::RenderSurface>(std::make_unique<engine::graphics::resource::Texture>(std::make_unique<engine::graphics::dx11::resource::DX11TextureImpl>()));

			// draw stuff on the drawable surface
			m_renderSurface->Initialize(256, 256);
			m_renderSurface->Clear(0, 1, 0, 1);

			// draw stuff on render surface 
			m_renderSurface->Begin();
			{
				m_renderer->Begin();
				{
					m_renderer->Draw(engine::spatial::PositionF{ 10, 10 }, engine::math::SizeF{ 128, 128 }, engine::graphics::ColorF{ 0.5f,0,0,1 }, 0);
					m_renderer->Draw(engine::spatial::PositionF{ 100, 100 }, engine::math::SizeF{ 96, 96 }, engine::graphics::ColorF{ 0, 0, 1.0f,0.5f }, 0);
					m_renderer->Draw(*m_fontAtlas, "Drawn on surface!!!", engine::spatial::PositionF{ 10, 200 }, engine::graphics::ColorF{ 0,0,0,1 });
				}
				m_renderer->End();
			}
			m_renderSurface->End();

		}

		// fun stuff. this is called on each loop of the message loop. this is where we draw!
		void OnIdle()
		{
			// start the canvas. we can draw from here
			m_canvas->Begin();
			{
				m_canvas->Clear({ 0.2f, 0.2f, 1.0f, 1.0f });

				m_renderer->Begin();
				{
					// draw a rectangle fill
					m_renderer->Draw(engine::spatial::PositionF{ 500, 50 }, engine::math::SizeF{ 100, 100 }, engine::graphics::ColorF{ 1,1,0,1 }, 0);

					// draw the drawable surface
					m_renderer->Draw(m_renderSurface->GetSprite(), engine::spatial::PositionF{200, 200}, m_renderSurface->GetSize(), engine::graphics::ColorF{1,1,1,1}, 0);

					// draw text
					m_renderer->Draw(*m_fontAtlas, "Hello World", engine::spatial::PositionF{ 200, 500 }, engine::graphics::ColorF{ 0,1,1,1 });

					// draw image surface
					m_renderer->Draw(m_imageSurface->GetSprite(), engine::spatial::PositionF{ 500, 200}, m_imageSurface->GetSize(), engine::graphics::ColorF{1,1,1,1}, 0);
				}
				m_renderer->End();
			}
			// end the canvas. we don't draw anything past this.
			m_canvas->End();
		}

		void OnExit()
		{

		}

		void OnWindowClose()
		{
			LOG("Window closed...");
		}

		void OnWindowSize(size_t nWidth, size_t nHeight)
		{
			LOG("Window resized to: " + std::to_string(nWidth) + ", " + std::to_string(nHeight));
			m_canvas->Resize({ static_cast<unsigned int>(nWidth), static_cast<unsigned int>(nHeight) });
			m_canvas->SetViewPort();
		}
	};
}