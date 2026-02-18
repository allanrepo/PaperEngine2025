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
#include <Graphics/Resource/SpriteAtlas.h>
#include <Cache/Registry.h>
#include <Graphics/Animation/Animation.h>
#include <Timer/StopWatch.h>

//#include "Actor.h"

//using namespace engine::win32;
//using namespace engine::graphics;
//using namespace engine::graphics::renderer;
//using namespace engine::input;
//using namespace engine::component;
//using namespace engine::event;
//using namespace engine::graphics::dx11;
//using namespace engine::graphics::dx11::renderer;
//using namespace engine::graphics::resource;
//using namespace engine::graphics::dx11::resource;
//using namespace engine::graphics::factory;
//using namespace engine::cache;

//using Sprite = engine::graphics::renderable::Sprite;
//using Animation = engine::graphics::animation::Animation<Sprite>;
//using Window = engine::win32::Window;
//using ICanvas = engine::graphics::ICanvas;
//using IRenderer = engine::graphics::renderer::IRenderer;
//using IFontAtlas = engine::graphics::resource::IFontAtlas;
//using Input = engine::input::Input;

using namespace engine;
using namespace engine::graphics;

namespace TestActor
{
	class Test
	{
	private:
	private:
		std::unique_ptr<win32::Window> m_window;
		std::unique_ptr<ICanvas> m_canvas;
		std::unique_ptr<renderer::IRenderer> m_rendererBatch;

		input::Input m_input;
		std::unique_ptr<resource::IFontAtlas> m_FontAtlas;

		std::unique_ptr<Actor> m_actor;

		Animation<Sprite> m_anim;

		timer::StopWatch m_stopwatch;

	public:
		Test()
		{
			win32::Window::OnInitialize += event::Handler(this, &Test::OnInitialize);
			win32::Window::OnExit += event::Handler(this, &Test::OnExit);
			win32::Window::OnIdle += event::Handler(this, &Test::OnIdle);
			win32::Window::Run();
		}

		// function that will be called just before we enter into message loop
		void OnInitialize()
		{
			// create our window here
			m_window = std::make_unique<win32::Window>();
			m_window->OnClose += event::Handler(this, &Test::OnWindowClose);
			m_window->OnCreate += event::Handler(this, &Test::OnWindowCreate);
			m_window->OnSize += event::Handler(this, &Test::OnWindowSize);
			m_window->Create(L"Test Font", 1400, 900);
			m_window->OnWindowMessage += event::Handler(&m_input, &input::Input::ProcessWin32Message);

			m_input.MouseDownEvent += event::Handler(this, &Test::OnMouseDown);
		}

		// when window is created. we can now safely create resources dependent on window
		void OnWindowCreate(void* hWnd)
		{
			LOG("Window created...");

			// create dx11 canvas
			m_canvas = std::make_unique<Canvas>(std::make_unique<dx11::DX11CanvasImpl>());
			m_canvas->Initialize(hWnd);
			m_canvas->SetViewPort();
			LOG("Canvas (DX11) created...");

			// create dx11 renderer batched
			m_rendererBatch = std::make_unique<renderer::Renderer>(std::make_unique<dx11::renderer::DX11RendererBatchImpl>());
			m_rendererBatch->Initialize();
			LOG("Renderer Batch (DX11) created...");
			
			// create font
			m_FontAtlas = std::make_unique<resource::FontAtlas>(std::make_unique<resource::SpriteAtlas>(std::make_unique<dx11::resource::DX11TextureImpl>()));
			m_FontAtlas->Initialize("Terminal", 12);

			// load actor animations
			{
				// create sprite atlas hero actor
				factory::SpriteAtlasFactory::Create("Hero", L"../Assets/CharacterTest_2304x1536_12x8.png", 8, 12);
				resource::ISpriteAtlas& atlas = cache::Registry<resource::ISpriteAtlas>::Instance().Get("Hero");

				// load animation in cache
				factory::AnimationFactory::Create("hero idle right", atlas, { 0, 1, 2, 3, 4, 5 }, 100, true);
				factory::AnimationFactory::Create("hero idle left", atlas, { 6, 7, 8, 9, 10, 11 }, 100, true);
				factory::AnimationFactory::Create("hero walk right", atlas, { 12, 13, 14, 15, 16, 17 }, 100, true);
				factory::AnimationFactory::Create("hero walk left", atlas, { 18, 19, 20, 21, 22, 23, }, 100, true);

			}
			// create sprite atlas for tilemap (grass, wall, etc...)

			// create actor 
			m_actor = std::make_unique<Actor>("Actor");

			// setup stopwatch to manage timing and start it
			m_stopwatch.OnLap += event::Handler(this, &Test::OnLap);
			m_stopwatch.Start();

		}

		void OnMouseDown(int btn, int x, int y)
		{
			m_actor->WalkTo({ (float)x, (float)y }, 0.25f);
		}

		// this method is fired up whenever the OnLap event is triggered from stopwatch
		void OnLap(double time)
		{
			m_actor->Update(time);
		}

		// fun stuff. this is called on each loop of the message loop. this is where we draw!
		void OnIdle()
		{
			// call lap to get elapsed time and trigger OnLap event
			m_stopwatch.Lap<engine::timer::milliseconds>();

			m_input.Update();


			m_canvas->Clear({ 0.2f, 0.2f, 1.0f, 1.0f });

			// start the canvas. we can draw from here
			m_canvas->Begin();
			{
				m_rendererBatch->Begin();

				m_rendererBatch->Draw(m_actor->GetPosition(), m_actor->GetSprite().GetSize(), { 1,1,1,0.5f }, 0);
				m_rendererBatch->DrawRenderable(m_actor->GetSprite(), m_actor->GetPosition(), m_actor->GetSprite().GetSize(), {1,1,1,1}, 0);

				std::string str = std::to_string(m_actor->GetPosition().x) + ", " + std::to_string(m_actor->GetPosition().y);
				m_rendererBatch->DrawText(*m_FontAtlas, str, { 500, 5 }, { 1,1,1,1 });

				m_rendererBatch->End();
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