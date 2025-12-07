// this test demonstrates how to create a simple animation system using the Animator and Animation classes.
// notes:
// - SpriteAtlas' constructors are protected, so we create MockSpriteAtlas classes to instantiate 
//	 them directly for testing purposes without using factories.
// - a function is defined to calculate UV rectangles for SpriteAtlas with the assumption that the layout is a grid. 
//	 the sprites in SpriteAtlas are also assumed to be sequenced row by row.
// - Animation is also created manually without using any factory or loader and frames were added directly
// - we needed to provide elapsed time to run the animation, so a stopwatch is used to measure time between frames
//   StopWatch's Lap method is called to measure and provide elapsed time per frame to the animator
// - since we use Sprite here to demonstrate animation, we actually test Sprite as well

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

namespace test
{
	// we are mocking the sprite atlas class here for demo purpose so we can create sprite directly without using factory
	class MockSpriteAtlas : public graphics::renderable::SpriteAtlas
	{
	public:
		MockSpriteAtlas(std::unique_ptr<graphics::resource::ITexture> tex) :
			SpriteAtlas(std::move(tex))
		{
		}
	};

	class TestAnimation
	{
	private:
		std::unique_ptr<Win32::Window> m_window;
		std::unique_ptr<graphics::ICanvas> m_canvas;
		std::unique_ptr<graphics::renderer::IRenderer> m_renderer;
		std::unique_ptr<MockSpriteAtlas> m_spriteAtlas;
		std::unique_ptr<graphics::animation::Animator<graphics::renderable::Sprite>> m_animator;
		std::unique_ptr<graphics::animation::Animation<graphics::renderable::Sprite>> m_animation;
		timer::StopWatch m_stopwatch;

	public:
		TestAnimation()
		{
			Win32::Window::OnInitialize += event::Handler(this, &TestAnimation::OnInitialize);
			Win32::Window::OnExit += event::Handler(this, &TestAnimation::OnExit);
			Win32::Window::OnIdle += event::Handler(this, &TestAnimation::OnIdle);

			Win32::Window::Run();
		}

		// function that will be called just before we enter into message loop
		void OnInitialize()
		{
			// create our window here
			m_window = std::make_unique<Win32::Window>();
			m_window->OnClose += event::Handler(this, &TestAnimation::OnWindowClose);
			m_window->OnCreate += event::Handler(this, &TestAnimation::OnWindowCreate);
			m_window->OnSize += event::Handler(this, &TestAnimation::OnWindowSize);
			m_window->Create(L"Test Sprite", 1400, 900);
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
			m_spriteAtlas = std::make_unique<MockSpriteAtlas>(std::make_unique<graphics::dx11::resource::DX11TextureImpl>());

			// load sprite atlas from file manually for demo purpose
			m_spriteAtlas->Initialize(L"../Assets/CharacterTest_2304x1536_12x8.png");

			// load sprite atlas UVs from csv manually for demo purpose. we calculate UVs here by assuming a grid of 8 rows and 12 columns
			// in real scenario, you would use SpriteAtlasLoader to load from csv file 
			std::vector<math::geometry::RectF> uvs = CalcUV(8, 12, (int)m_spriteAtlas->GetWidth(), (int)m_spriteAtlas->GetHeight());
			for(math::geometry::RectF& rect : uvs)
			{
				m_spriteAtlas->AddUVRect(rect);
			}

			// create animation manually and make it loop
			m_animation = std::make_unique<graphics::animation::Animation<graphics::renderable::Sprite>>();
			m_animation->loop = true;

			// load with walking animation frames manually
			for (int i = 12; i < 18; i++)
			{
				graphics::renderable::Sprite sprite = m_spriteAtlas->MakeSprite(i);
				m_animation->frames.push_back({ sprite, 100.0f });
			}

			// create animator and load the animation
			m_animator = std::make_unique<graphics::animation::Animator<graphics::renderable::Sprite>>();
			m_animator->Play(m_animation.get());

			// setup stopwatch to manage timing and start it
			m_stopwatch.OnLap += event::Handler(this, &TestAnimation::OnLap);
			m_stopwatch.Start();
		}

		// this method is fired up whenever the OnLap event is triggered from stopwatch
		void OnLap(float time)
		{
			m_animator->Update(time);
		}

		// fun stuff. this is called on each loop of the message loop. this is where we draw!
		void OnIdle()
		{
			// call lap to get elapsed time and trigger OnLap event
			m_stopwatch.Lap<timer::milliseconds>();

			// start the canvas. we can draw from here
			m_canvas->Begin();
			{
				m_canvas->Clear(0.2f, 0.2f, 1.0f, 1.0f);

				m_renderer->Begin();
				{
					m_renderer->DrawRenderable(
						m_animator->GetCurrentFrame()->element,				// get the sprite from animator's current frame
						spatial::PositionF {
						100.0f, 100.0f
					},				// position
						m_animator->GetCurrentFrame()->element.GetSize(),	// get the sprite size from animator's current frame
						graphics::ColorF{ 1.0f, 1.0f, 1.0f, 1.0f },			// color
						0.0f
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

		std::vector<math::geometry::RectF> CalcUV(int row, int col, int fileWidth, int fileHeight)
		{
			std::vector<math::geometry::RectF> uvs;
			float width = static_cast<float>(fileWidth / col);
			float height = static_cast<float>(fileHeight / row);
			float left = 0;
			float top = 0;
			float right = left + width;
			float bottom = top + height;

			for (int r = 0; r < row; r++)
			{
				for (int c = 0; c < col; c++)
				{
					left = width * c;
					top = height * r;
					right = left + width;
					bottom = top + height;

					left /= fileWidth;
					top /= fileHeight;
					right /= fileWidth;
					bottom /= fileHeight;

					uvs.push_back(math::geometry::RectF{ left, top, right, bottom });

					//LOG(std::to_string(left) << ", " << std::to_string(top) << ", " << std::to_string(right) << ", " << std::to_string(bottom));
				}
			}
			return uvs;
		}
	};
}