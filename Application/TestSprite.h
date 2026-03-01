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
#include <Engine/Factory/SpriteAtlasFactory.h>
#include <Graphics/Renderable/Sprite.h>
#include <Core/Input.h>

namespace test
{
	// this demo class shows how to use sprite atlas and sprite rendering
	// it does not use cache system to load resources for simplicity so sprite atlas and sprite are loaded directly from files
	// this is not optimal for real world application as resources should be cached to prevent reloading same resources multiple times
	// but this makes it easier to understand how sprite atlas and sprite rendering works	// 
	// sprite class is mocked here for simplicity so we can create sprite directly without using factory
	//
	// sprite atlas used in this demo is fully rendered in the screen at half size
	// moving the mouse cursor over the sprite atlas will calculate the sprite cell based on mouse position
	// and recreate the sprite to render only that cell from the sprite atlas
	class TestSprite
	{
	private:
		std::unique_ptr<engine::win32::Window> m_window;
		std::unique_ptr<engine::graphics::ICanvas> m_canvas;
		std::unique_ptr<engine::graphics::renderer::IRenderer> m_renderer;
		std::unique_ptr<engine::graphics::renderable::Sprite> m_sprite;
		engine::spatial::SizeF m_spriteSize{};
		engine::input::Input m_input;

	public:
		TestSprite()
		{
			engine::win32::Window::OnInitialize += engine::event::Handler(this, &TestSprite::OnInitialize);
			engine::win32::Window::OnExit += engine::event::Handler(this, &TestSprite::OnExit);
			engine::win32::Window::OnIdle += engine::event::Handler(this, &TestSprite::OnIdle);

			engine::win32::Window::Run();
		}

		// function that will be called just before we enter into message loop
		void OnInitialize()
		{
			// create our window here
			m_window = std::make_unique<engine::win32::Window>();
			m_window->OnClose += engine::event::Handler(this, &TestSprite::OnWindowClose);
			m_window->OnCreate += engine::event::Handler(this, &TestSprite::OnWindowCreate);
			m_window->OnSize += engine::event::Handler(this, &TestSprite::OnWindowSize);
			m_window->Create(L"Test Sprite", 1400, 900);
			m_window->OnWindowMessage += engine::event::Handler(&m_input, &engine::input::Input::ProcessWin32Message);

			m_input.MouseMoveEvent += engine::event::Handler(this, &TestSprite::OnMouseMove);
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

			// create sprite atlas using factory. it will be stored in cache and will auto generate UV's based on given row and col
			engine::graphics::factory::SpriteAtlasFactory::Create("CharacterTest_2304x1536_12x8", L"../Assets/CharacterTest_2304x1536_12x8.png", 8, 12);
			engine::graphics::resource::ISpriteAtlas& atlas = engine::cache::Registry<engine::graphics::resource::ISpriteAtlas>::Instance().Get("CharacterTest_2304x1536_12x8");
						
			// calculate sprite size based on atlas size and grid
			m_spriteSize.width = atlas.GetWidth() / 12; // assuming 12 columns
			m_spriteSize.height = atlas.GetHeight() / 8; // assuming 8 rows
			
			// create sprite
			m_sprite = std::make_unique<engine::graphics::renderable::Sprite>(atlas.MakeSprite(0));
		}

		void OnMouseMove(int x, int y)
		{
			engine::graphics::resource::ISpriteAtlas& atlas = engine::cache::Registry<engine::graphics::resource::ISpriteAtlas>::Instance().Get("CharacterTest_2304x1536_12x8");

			// find the sprite cell from sprite atlas based on mouse position
			engine::spatial::SizeF size = atlas.GetSize();

			// dividing by 2 because sprite atlas is drawn at half size
			int col = static_cast<int>(x / (m_spriteSize.width / 2));
			int row = static_cast<int>(y / (m_spriteSize.height / 2));

			int index = row * (int)(atlas.GetWidth()/m_spriteSize.width) + col;

			// recreate sprite with new UV rect
			if (index < atlas.GetUVRectCount())
			{
				m_sprite = std::make_unique<engine::graphics::renderable::Sprite>(atlas.MakeSprite(index));
			}
		}

		// fun stuff. this is called on each loop of the message loop. this is where we draw!
		void OnIdle()
		{
			m_input.Update();

			// start the canvas. we can draw from here
			m_canvas->Begin();
			{
				m_canvas->Clear({ 0.2f, 0.2f, 1.0f, 1.0f });

				m_renderer->Begin();
				{
					engine::graphics::resource::ISpriteAtlas& atlas = engine::cache::Registry<engine::graphics::resource::ISpriteAtlas>::Instance().Get("CharacterTest_2304x1536_12x8");

					// draw the sprite atlas at half size
					m_renderer->DrawRenderable(atlas.GetSprite(),
						engine::spatial::PositionF{ 0, 0 },
						engine::spatial::SizeF{ atlas.GetWidth()/2, atlas.GetHeight()/2},
						engine::graphics::ColorF{ 1,1,1,1 },
						0
					);

					// draw the selected sprite cell next to the sprite atlas
					m_renderer->DrawRenderable(*m_sprite,
						engine::spatial::PositionF{ atlas.GetWidth() / 2 + 10, 0 },
						engine::spatial::SizeF{ m_sprite->GetWidth(), m_sprite->GetHeight() },
						engine::graphics::ColorF{ 1,1,1,1 },
						0
					);
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
	};
}