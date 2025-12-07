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
#include <Graphics/Renderable/DrawableSurface.h>
#include <Graphics/Resource/DX11Texture.h>
#include <Graphics/Resource/Texture.h>
#include <Graphics/Resource/DX11TextureImpl.h>
#include <Engine/Factory/TextureFactory.h>
#include <Graphics/Renderable/IFontAtlas.h>
#include <Graphics/Renderable/FontAtlas.h>
#include <Graphics/Renderable/IImageSurface.h>
#include <Graphics/Renderable/ImageSurface.h>

namespace test
{
	class TestCanvas
	{
	private:
		std::unique_ptr<Win32::Window> m_window;
		std::unique_ptr<graphics::ICanvas> m_canvas;
		std::unique_ptr<graphics::renderer::IRenderer> m_renderer;
		std::unique_ptr<graphics::renderable::IDrawableSurface> m_drawableSurface;
		std::unique_ptr<graphics::renderable::IFontAtlas> m_fontAtlas;
		std::unique_ptr<graphics::renderable::IImageSurface> m_imageSurface;

	public:
		TestCanvas()
		{
			Win32::Window::OnInitialize += event::Handler(this, &TestCanvas::OnInitialize);
			Win32::Window::OnExit += event::Handler(this, &TestCanvas::OnExit);
			Win32::Window::OnIdle += event::Handler(this, &TestCanvas::OnIdle);

			Win32::Window::Run();
		}

		// function that will be called just before we enter into message loop
		void OnInitialize()
		{
			// create our window here
			m_window = std::make_unique<Win32::Window>();
			m_window->OnClose += event::Handler(this, &TestCanvas::OnWindowClose);
			m_window->OnCreate += event::Handler(this, &TestCanvas::OnWindowCreate);
			m_window->OnSize += event::Handler(this, &TestCanvas::OnWindowSize);
			m_window->Create(L"Test Canvas", 1400, 900);
		}

		// when window is created. we can now safely create resources dependent on window
		void OnWindowCreate(void* hWnd)
		{
			LOG("Window created...");

			// create dx11 canvas
			m_canvas = std::make_unique<graphics::Canvas>(std::make_unique<graphics::dx11::DX11CanvasImpl>());
			m_canvas->Initialize(hWnd);
			m_canvas->SetViewPort();

			// create dx11 renderer batched
			m_renderer = std::make_unique<graphics::renderer::Renderer>(std::make_unique<graphics::dx11::renderer::DX11RendererBatchImpl>());
			m_renderer->Initialize();

			// create dx11 texture and use on drawable surface
			m_drawableSurface = std::make_unique<graphics::renderable::DrawableSurface>(std::make_unique<graphics::resource::Texture>(std::make_unique<graphics::dx11::resource::DX11TextureImpl>()));
			//m_drawableSurface = std::make_unique<graphics::renderable::DrawableSurface>(std::make_unique<graphics::dx11::resource::DX11Texture>());

			// draw stuff on the drawable surface
			m_drawableSurface->Initialize(128, 128);
			m_drawableSurface->Begin();
			{
				m_drawableSurface->Clear(0, 0.5f, 0, 1);
				m_drawableSurface->Begin();
				{
					m_renderer->Draw(spatial::PositionF{ 32, 32 }, spatial::SizeF{ 64, 64 }, graphics::ColorF{ 0.5f,0,0,1 }, 0);
					m_renderer->Draw(spatial::PositionF{ 48, 56 }, spatial::SizeF{ 64, 48 }, graphics::ColorF{ 0,0,0.5f,1 }, 0);
				}
				m_drawableSurface->End();
			}
			m_drawableSurface->End();

			// create font atlas
			m_fontAtlas = std::make_unique<graphics::renderable::FontAtlas>(std::make_unique<graphics::resource::Texture>(std::make_unique<graphics::dx11::resource::DX11TextureImpl>()));
			m_fontAtlas->Initialize("Comic Sans", 32);

			// create image surface
			m_imageSurface = std::make_unique<graphics::renderable::ImageSurface>(std::make_unique<graphics::resource::Texture>(std::make_unique<graphics::dx11::resource::DX11TextureImpl>()));
			m_imageSurface->Initialize(L"../Assets/256x256.bmp");
		}

		// fun stuff. this is called on each loop of the message loop. this is where we draw!
		void OnIdle()
		{
			// start the canvas. we can draw from here
			m_canvas->Begin();
			{
				m_canvas->Clear(0.2f, 0.2f, 1.0f, 1.0f);

				m_renderer->Begin();
				{
					// draw a rectangle fill
					m_renderer->Draw(spatial::PositionF{ 100, 100 }, spatial::SizeF{ 100, 100 }, graphics::ColorF{ 1,1,0,1 }, 0);

					// draw the drawable surface
					m_renderer->DrawRenderable(*m_drawableSurface, spatial::PositionF{ 250, 250 }, m_drawableSurface->GetSize(), graphics::ColorF{ 1,1,1,1 }, 0);

					// draw text
					m_renderer->DrawText(*m_fontAtlas, "Hello World", spatial::PositionF{ 250, 200 }, graphics::ColorF{ 0,1,1,1 });

					// draw image surface
					m_renderer->DrawRenderable(*m_imageSurface, spatial::PositionF{ 400, 250 }, m_imageSurface->GetSize(), graphics::ColorF{ 1,1,1,1 }, 0);
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
			m_canvas->Resize(static_cast<unsigned int>(nWidth), static_cast<unsigned int>(nHeight));
			m_canvas->SetViewPort();
		}
	};
}