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
	class TestAnimation
	{
	private:
		std::unique_ptr<engine::win32::Window> m_window;
		std::unique_ptr<engine::graphics::ICanvas> m_canvas;
		std::unique_ptr<engine::graphics::renderer::IRenderer> m_renderer;
		engine::timer::StopWatch m_stopwatch;

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

		void CreateAnimation(const std::string& name, const std::wstring& filepath, const size_t row, const size_t col, const std::vector<int>& animationFrameIndice, float duration)
		{
			// create sprite atlas using factory. it will be stored in cache and will auto generate UV's based on given row and col
			engine::graphics::factory::SpriteAtlasFactory::Create(name, filepath, row, col);
			engine::graphics::resource::ISpriteAtlas& atlas = engine::cache::Registry<engine::graphics::resource::ISpriteAtlas>::Instance().Get(name);

			// create animation using factory. it will be stored in cache as well. we set loop to true for this animation
			engine::graphics::factory::AnimationFactory::Create(name, atlas, animationFrameIndice, duration, true);

			// create animator and load the animation. it will also be stored in cache. we can have multiple animators using the same animation
			engine::cache::Registry<engine::graphics::animation::Animator<engine::graphics::renderable::Sprite>>::Instance().Register(name, std::make_unique<engine::graphics::animation::Animator<engine::graphics::renderable::Sprite>>());

			// get animator from cache and play the animation. since we set loop to true, it will keep looping through the frames in the animation
			engine::graphics::animation::Animator<engine::graphics::renderable::Sprite>& animator = engine::cache::Registry<engine::graphics::animation::Animator<engine::graphics::renderable::Sprite>>::Instance().Get(name);
			animator.Play(engine::cache::Registry<engine::graphics::animation::Animation<engine::graphics::renderable::Sprite>>::Instance().Get(name));
		}

		void UpdateAnimation(const std::string& name, double delta)
		{
			engine::graphics::animation::Animator<engine::graphics::renderable::Sprite>& animator = engine::cache::Registry<engine::graphics::animation::Animator<engine::graphics::renderable::Sprite>>::Instance().Get(name);
			animator.Update(delta);
		}

		void DrawAnimation(const std::string& name, const engine::spatial::PositionF& pos, engine::graphics::ColorF color = { 1.0f, 1.0f, 1.0f, 1.0f }, float rotation = 0.0f)
		{
			engine::graphics::animation::Animator<engine::graphics::renderable::Sprite>& animator = engine::cache::Registry<engine::graphics::animation::Animator<engine::graphics::renderable::Sprite>>::Instance().Get(name);

			m_renderer->Draw(
				pos,			// position
				animator.GetCurrentFrame().element.GetSize(),	// get the sprite size from animator's current frame
				{1,1,1,0.5f},			// color
				rotation		// rotation
			);

			m_renderer->DrawRenderable(
				animator.GetCurrentFrame().element,				// get the sprite from animator's current frame
				pos,				// position
				animator.GetCurrentFrame().element.GetSize(),	// get the sprite size from animator's current frame
				color,			// color
				rotation
			);
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

			CreateAnimation("CharacterTest_Animation", L"../Assets/CharacterTest_2304x1536_12x8.png", 8, 12, { 12,13,14,15,16,17 }, 100.0f);
			CreateAnimation("Tree1", L"../Assets/tree_1x8_1536x256.png", 1, 8, { 0,1,2,3,4,5,6,7 }, 100.0f);
			CreateAnimation("Tree2", L"../Assets/tree_1x8_1536x192.png", 1, 8, { 0,1,2,3,4,5,6,7 }, 100.0f);
			CreateAnimation("Tree3", L"../Assets/tree1_1x8_1536x256.png", 1, 8, { 0,1,2,3,4,5,6,7 }, 100.0f);
			CreateAnimation("Tree4", L"../Assets/tree1_1x8_1536x192.png", 1, 8, { 0,1,2,3,4,5,6,7 }, 100.0f);


			// setup stopwatch to manage timing and start it
			m_stopwatch.OnLap += engine::event::Handler(this, &TestAnimation::OnLap);
			m_stopwatch.Start();
		}

		// this method is fired up whenever the OnLap event is triggered from stopwatch
		void OnLap(double time)
		{
			UpdateAnimation("CharacterTest_Animation", time);
			UpdateAnimation("Tree1", time);
			UpdateAnimation("Tree2", time);
			UpdateAnimation("Tree3", time);
			UpdateAnimation("Tree4", time);
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
					DrawAnimation("CharacterTest_Animation", { 100.0f, 100.0f });
					DrawAnimation("Tree1", { 400.0f, 100.0f });
					DrawAnimation("Tree2", { 600.0f, 200.0f });
					DrawAnimation("Tree3", { 800.0f, 200.0f });
					DrawAnimation("Tree4", { 1000.0f, 200.0f });
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