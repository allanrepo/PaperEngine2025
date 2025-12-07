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
#include <Engine/Factory/SpriteAtlasFactory.h>
#include <Engine/Loader/SpriteAtlasLoader.h>
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
		// we are mocking the sprite class here for demo purpose so we can create sprite directly without using factory
		class MockSprite : public graphics::renderable::Sprite
		{
		public:
			MockSprite(graphics::renderable::ISpriteAtlas* spriteAtlas, math::geometry::RectF rect) :
				Sprite(spriteAtlas, rect)
			{
			}
		};

	private:
		std::unique_ptr<Win32::Window> m_window;
		std::unique_ptr<graphics::ICanvas> m_canvas;
		std::unique_ptr<graphics::renderer::IRenderer> m_renderer;
		std::unique_ptr<graphics::renderable::ISpriteAtlas> m_spriteAtlas;
		std::unique_ptr<MockSprite> m_sprite;
		spatial::SizeF m_spriteSize{};
		input::Input m_input;

	public:
		TestSprite()
		{
			Win32::Window::OnInitialize += event::Handler(this, &TestSprite::OnInitialize);
			Win32::Window::OnExit += event::Handler(this, &TestSprite::OnExit);
			Win32::Window::OnIdle += event::Handler(this, &TestSprite::OnIdle);

			Win32::Window::Run();
		}

		// function that will be called just before we enter into message loop
		void OnInitialize()
		{
			// create our window here
			m_window = std::make_unique<Win32::Window>();
			m_window->OnClose += event::Handler(this, &TestSprite::OnWindowClose);
			m_window->OnCreate += event::Handler(this, &TestSprite::OnWindowCreate);
			m_window->OnSize += event::Handler(this, &TestSprite::OnWindowSize);
			m_window->Create(L"Test Sprite", 1400, 900);
			m_window->OnWindowMessage += event::Handler(&m_input, &input::Input::ProcessWin32Message);

			m_input.OnMouseMove += event::Handler(this, &TestSprite::OnMouseMove);
		}


		void OnMouseMove(int x, int y)
		{
			// find the sprite cell from sprite atlas based on mouse position
			spatial::SizeF size = m_spriteAtlas->GetSize();

			// dividing by 2 because sprite atlas is drawn at half size
			int col = static_cast<int>(x / (m_spriteSize.width / 2));
			int row = static_cast<int>(y / (m_spriteSize.height / 2));

			// normalize the rect to UV coordinates
			math::geometry::RectF uvRect
			{
				col / 12.0f,
				row / 8.0f,
				(col + 1) / 12.0f,
				(row + 1) / 8.0f
			};

			// recreate sprite with new UV rect
			m_sprite = std::make_unique<MockSprite>(m_spriteAtlas.get(), uvRect);
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


			// create sprite atlas and load the image file as well as read the csv file for the UVs	
			m_spriteAtlas = graphics::factory::SpriteAtlasFactory::Create();
			if (!graphics::loader::SpriteAtlasLoader::Load(
				*m_spriteAtlas,
				"../Assets/CharacterTest_2304x1536_12x8.png",
				"../Assets/CharacterTest_2304x1536_12x8.csv"
			))
			{
				LOGERROR("Failed to load sprite atlas - " << "../Assets/CharacterTest_2304x1536_12x8.png");
			}
			
			// calculate sprite size based on atlas size and grid
			m_spriteSize.width = m_spriteAtlas->GetWidth() / 12; // assuming 12 columns
			m_spriteSize.height = m_spriteAtlas->GetHeight() / 8; // assuming 8 rows
			
			// create sprite
			m_sprite = std::make_unique<MockSprite>(m_spriteAtlas.get(), math::geometry::RectF{0.0f, 0.0f, 1.0f, 1.0f});

		}

		// fun stuff. this is called on each loop of the message loop. this is where we draw!
		void OnIdle()
		{
			m_input.Update();

			// start the canvas. we can draw from here
			m_canvas->Begin();
			{
				m_canvas->Clear(0.2f, 0.2f, 1.0f, 1.0f);

				m_renderer->Begin();
				{
					// draw the sprite atlas at half size
					m_renderer->DrawRenderable(*m_spriteAtlas,
						spatial::PositionF{ 0, 0 },
						spatial::SizeF{ m_spriteAtlas->GetWidth()/2, m_spriteAtlas->GetHeight()/2},
						graphics::ColorF{ 1,1,1,1 },
						0
					);

					// draw the selected sprite cell next to the sprite atlas
					m_renderer->DrawRenderable(*m_sprite,
						spatial::PositionF{ m_spriteAtlas->GetWidth() / 2 + 10, 0 },
						spatial::SizeF{ m_sprite->GetWidth(), m_sprite->GetHeight() },
						graphics::ColorF{ 1,1,1,1 },
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
			m_canvas->Resize(static_cast<unsigned int>(nWidth), static_cast<unsigned int>(nHeight));
			m_canvas->SetViewPort();
		}
	};
}