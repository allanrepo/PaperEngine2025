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
#include <Graphics/Resource/ISpriteAtlas.h>
#include <Graphics/Resource/SpriteAtlas.h>
#include <Engine/Factory/SpriteAtlasFactory.h>
#include <Graphics/Renderable/Sprite.h>
#include <Core/Input.h>
#include <Graphics/Animation/Animation.h>
#include <Graphics/Resource/DX11TextureImpl.h>
#include <Timer/StopWatch.h>
#include <Engine/Factory/AnimationFactory.h>

namespace test
{
	// we are mocking the sprite atlas class here for demo purpose so we can create sprite directly without using factory
	class MockSpriteAtlas : public engine::graphics::resource::SpriteAtlas
	{
	public:
		MockSpriteAtlas(std::unique_ptr<engine::graphics::resource::ITexture> tex) :
			SpriteAtlas(std::move(tex))
		{
		}
	};

	class TestAnimation
	{
	private:
		std::unique_ptr<engine::win32::Window> m_window;
		std::unique_ptr<engine::graphics::ICanvas> m_canvas;
		std::unique_ptr<engine::graphics::renderer::IRenderer> m_renderer;
		std::unique_ptr<engine::graphics::animation::Animator<engine::graphics::renderable::Sprite>> m_animator;
		engine::timer::StopWatch m_stopwatch;
		engine::graphics::animation::Animation<engine::graphics::renderable::Sprite> m_anim;

	public:
		TestAnimation()
		{
			engine::win32::Window::OnInitialize += engine::event::Handler(this, &TestAnimation::OnInitialize);
			engine::win32::Window::OnExit += engine::event::Handler(this, &TestAnimation::OnExit);
			engine::win32::Window::OnIdle += engine::event::Handler(this, &TestAnimation::OnIdle);

			engine::win32::Window::Run();
		}

		// function that will be called just before we enter into message loop
		void OnInitialize()
		{
			// create our window here
			m_window = std::make_unique<engine::win32::Window>();
			m_window->OnClose += engine::event::Handler(this, &TestAnimation::OnWindowClose);
			m_window->OnCreate += engine::event::Handler(this, &TestAnimation::OnWindowCreate);
			m_window->OnSize += engine::event::Handler(this, &TestAnimation::OnWindowSize);
			m_window->Create(L"TestAnimation", 1400, 900);
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

	
			m_anim.loop = true;

			// load with walking animation frames manually
			for (int i = 12; i < 18; i++)
			{
				engine::graphics::renderable::Sprite sprite = atlas.MakeSprite(i);
				m_anim.frames.push_back({ sprite, 100.0f });
			}

			// create animator and load the animation
			m_animator = std::make_unique<engine::graphics::animation::Animator<engine::graphics::renderable::Sprite>>();
			m_animator->Play(m_anim);

			// setup stopwatch to manage timing and start it
			m_stopwatch.OnLap += engine::event::Handler(this, &TestAnimation::OnLap);
			m_stopwatch.Start();
		}

		// this method is fired up whenever the OnLap event is triggered from stopwatch
		void OnLap(double time)
		{
			m_animator->Update(time);
		}

		// fun stuff. this is called on each loop of the message loop. this is where we draw!
		void OnIdle()
		{
			// call lap to get elapsed time and trigger OnLap event
			m_stopwatch.Lap<engine::timer::milliseconds>();

			// start the canvas. we can draw from here
			m_canvas->Begin();
			{
				m_canvas->Clear({ 0.2f, 0.2f, 1.0f, 1.0f });

				m_renderer->Begin();
				{
					m_renderer->DrawRenderable(
						m_animator->GetCurrentFrame().element,				// get the sprite from animator's current frame
						engine::spatial::PositionF {
						100.0f, 100.0f
					},				// position
						m_animator->GetCurrentFrame().element.GetSize(),	// get the sprite size from animator's current frame
						engine::graphics::ColorF{ 1.0f, 1.0f, 1.0f, 1.0f },			// color
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
			m_canvas->Resize({ static_cast<unsigned int>(nWidth), static_cast<unsigned int>(nHeight) });
			m_canvas->SetViewPort();
		}

		//std::vector<math::geometry::RectF> CalcUV(int row, int col, int fileWidth, int fileHeight)
		//{
		//	std::vector<math::geometry::RectF> uvs;
		//	float width = static_cast<float>(fileWidth / col);
		//	float height = static_cast<float>(fileHeight / row);
		//	float left = 0;
		//	float top = 0;
		//	float right = left + width;
		//	float bottom = top + height;

		//	for (int r = 0; r < row; r++)
		//	{
		//		for (int c = 0; c < col; c++)
		//		{
		//			left = width * c;
		//			top = height * r;
		//			right = left + width;
		//			bottom = top + height;

		//			left /= fileWidth;
		//			top /= fileHeight;
		//			right /= fileWidth;
		//			bottom /= fileHeight;

		//			uvs.push_back(math::geometry::RectF{ left, top, right, bottom });

		//			//LOG(std::to_string(left) << ", " << std::to_string(top) << ", " << std::to_string(right) << ", " << std::to_string(bottom));
		//		}
		//	}
		//	return uvs;
		//}
	};
}