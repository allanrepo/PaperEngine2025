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
#include <Graphics/Renderable/_IFontAtlas.h>
#include <Graphics/Renderable/_FontAtlas.h>
#include <Graphics/Resource/SpriteAtlas.h>


#include "Utilities.h"

namespace TestFont
{
	class Test
	{
	private:
	private:
		std::unique_ptr<engine::win32::Window> m_window;
		std::unique_ptr<engine::graphics::ICanvas> m_canvas;
		std::unique_ptr<engine::graphics::renderer::IRenderer> m_rendererBatch;
		std::unique_ptr<engine::graphics::renderer::IRenderer> m_rendererImmediate;

		engine::input::Input m_input;
		std::unique_ptr<engine::graphics::resource::IFontAtlas> m_FontAtlas;
		std::unique_ptr<engine::graphics::renderable::IFontAtlas> m_fontAtlas;
		int m_toggleDraw = 0;
		std::string m_text;

	public:
		Test()
		{
			engine::win32::Window::OnInitialize += engine::event::Handler(this, &Test::OnInitialize);
			engine::win32::Window::OnExit += engine::event::Handler(this, &Test::OnExit);
			engine::win32::Window::OnIdle += engine::event::Handler(this, &Test::OnIdle);
			engine::win32::Window::Run();
		}

		// function that will be called just before we enter into message loop
		void OnInitialize()
		{
			// create our window here
			m_window = std::make_unique<engine::win32::Window>();
			m_window->OnClose += engine::event::Handler(this, &Test::OnWindowClose);
			m_window->OnCreate += engine::event::Handler(this, &Test::OnWindowCreate);
			m_window->OnSize += engine::event::Handler(this, &Test::OnWindowSize);
			m_window->Create(L"Test Font", 1400, 900);
			m_window->OnWindowMessage += engine::event::Handler(&m_input, &engine::input::Input::ProcessWin32Message);

			m_input.MouseDownEvent += engine::event::Handler(this, &Test::OnMouseDown);
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
			m_rendererBatch = std::make_unique<engine::graphics::renderer::Renderer>(std::make_unique<engine::graphics::dx11::renderer::DX11RendererBatchImpl>());
			m_rendererBatch->Initialize();
			LOG("Renderer Batch (DX11) created...");

			// create dx11 renderer batched
			m_rendererImmediate = std::make_unique<engine::graphics::renderer::Renderer>(std::make_unique<engine::graphics::dx11::renderer::DX11RendererImmediateImpl>());
			m_rendererImmediate->Initialize();
			LOG("Renderer Immediate (DX11) created...");

			m_FontAtlas = std::make_unique<engine::graphics::resource::FontAtlas>(std::make_unique<engine::graphics::resource::SpriteAtlas>(std::make_unique<engine::graphics::dx11::resource::DX11TextureImpl>()));
			m_FontAtlas->Initialize("Terminal", 12);

			m_fontAtlas = std::make_unique<engine::graphics::renderable::FontAtlas>(std::make_unique<engine::graphics::dx11::resource::DX11TextureImpl>());
			m_fontAtlas->Initialize("Terminal", 12);

			for (unsigned char c = 32; c <= 127; c++)
			{
				m_text += c;
			}
		}

		void OnMouseDown(int btn, int x, int y)
		{
			m_toggleDraw < 3 ? m_toggleDraw++ : m_toggleDraw = 0;
		}

		// fun stuff. this is called on each loop of the message loop. this is where we draw!
		void OnIdle()
		{
			m_input.Update();

			// start the canvas. we can draw from here
			m_canvas->Begin();
			{
				int repeat = 1;
				engine::graphics::renderer::IRenderer* renderer;
				switch (m_toggleDraw)
				{
				case 0: 
					renderer = m_rendererImmediate.get();
					m_canvas->Clear({ 0.2f, 0.2f, 1.0f, 1.0f }); 
					break;
				case 1: 
					renderer = m_rendererImmediate.get();
					m_canvas->Clear({ 0.2f, 0.5f, 0.5f, 1.0f });
					break;
				case 2: 
					renderer = m_rendererImmediate.get();
					m_canvas->Clear({ 0.2f, 0.5f, 0.2f, 1.0f });
					break;
				case 3: 
					renderer = m_rendererImmediate.get();
					m_canvas->Clear({ 0.5f, 0.5f, 1.0f, 0.2f });
					break;
				default:
					renderer = m_rendererBatch.get();
					m_canvas->Clear({ 0, 0, 0, 0 });
					break;
				}

				renderer->Begin();
				{
					std::string title;
					for (int i = 0; i < repeat; i++)
					{
						switch (m_toggleDraw)
						{
						case 0:
						{
							title = "draw characters one at a time using old font";
							unsigned char ch = 32;
							for (int row = 0; row < 32; row++)
							{
								for (int col = 0; col < 96; col++)
								{
									renderer->DrawChar(*m_fontAtlas, ch,
										engine::spatial::PositionF{ 50 + col * 12.0f, 50 + row * 25.0f },
										engine::graphics::ColorF{ 1,1,1,1 },
										0
									);
									ch++;
									if (ch > 127) ch = 32;
								}
							}
							break;
						}
						case 1:
						{
							title = "draw characters one at a time using new font";
							unsigned char ch = 32;
							for (int row = 0; row < 32; row++)
							{
								for (int col = 0; col < 96; col++)
								{
									engine::graphics::renderable::Sprite glyph = m_FontAtlas->GetGlyph(ch);
									renderer->DrawRenderable(glyph,
										engine::spatial::PositionF{ 50 + col * 12.0f, 50 + row * 25.0f },
										glyph.GetSize(),
										engine::graphics::ColorF{ 1,1,1,1 },
										0
									);
									ch++;
									if (ch > 127) ch = 32;
								}
							}
							break;
						}
						case 2:
						{
							title = "draw ASCII characters in string using old font";
							for (int row = 0; row < 32; row++)
							{
								renderer->DrawText(*m_fontAtlas, m_text, engine::spatial::PositionF{ 50, 50 + row * 25.0f }, engine::graphics::ColorF{ 1,1,1,1 });
							}
							break;
						}
						case 3:
						{
							title = "draw ASCII characters in string using new font";
							for (int row = 0; row < 32; row++)
							{
								renderer->DrawText(*m_FontAtlas, m_text, engine::spatial::PositionF{ 50, 50 + row * 25.0f }, engine::graphics::ColorF{ 1,1,1,1 });
							}
							break;

						}
						}
					}

					renderer->DrawText(
						*m_FontAtlas,
						title,
						engine::spatial::PositionF{ 650.0f, 10.0f },
						engine::graphics::ColorF{ 1,1,1,1 }
					);
				}
				renderer->End();
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