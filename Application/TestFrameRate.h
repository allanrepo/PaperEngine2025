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
#include <limits.h>
#include <Graphics/Renderable/IFontAtlas.h>
#include <Graphics/Renderable/FontAtlas.h>
#include <Timer/StopWatch.h>
#include <Timer/Scheduler.h>
#include <Performance/FrameRateMonitor.h>
#include <Graphics/Resource/DX11TextureImpl.h>
#include <Core/Singleton.h>

namespace testFrameRate
{
	class Test: public core::Singleton<Test>
	{
	private:
		friend class core::Singleton<Test>;

		std::unique_ptr<Win32::Window> m_window;
		std::unique_ptr<graphics::ICanvas> m_canvas;
		std::unique_ptr<graphics::renderer::IRenderer> m_renderer;
		timer::StopWatch m_stopwatch;
		std::unique_ptr<graphics::renderable::IFontAtlas> m_fontAtlas;
		timer::Scheduler m_scheduler;
		performance::FrameRateMonitor m_frameRateMonitor1;
		performance::FrameRateMonitor m_frameRateMonitor2;
		performance::FrameRateMonitor m_frameRateMonitor3;
		performance::FrameRateMonitor m_frameRateMonitor4;

		Test() :
			m_frameRateMonitor1(1.0f),
			m_frameRateMonitor2(1.0f),
			m_frameRateMonitor3(1.0f),
			m_frameRateMonitor4(1.0f)
		{
			Win32::Window::OnInitialize += event::Handler(this, &Test::OnInitialize);
			Win32::Window::OnIdle += event::Handler(this, &Test::OnIdle);
		}

	public:
		void Run()
		{
			Win32::Window::Run();
		}

		// function that will be called just before we enter into message loop
		void OnInitialize()
		{
			// create our window here
			m_window = std::make_unique<Win32::Window>();
			m_window->OnCreate += event::Handler(this, &Test::OnWindowCreate);
			m_window->OnSize += event::Handler(this, &Test::OnWindowSize);
			m_window->Create(L"Test Frame Rate Monitor and Scheduler", 1400, 900);
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

			// create font atlas
			m_fontAtlas = std::make_unique<graphics::renderable::FontAtlas>(std::make_unique<graphics::dx11::resource::DX11TextureImpl>());
			m_fontAtlas->Initialize("Arial", 24);
			LOG("Font atlas created and initialized...");

			// subscribe to scheduler using class method
			m_scheduler += timer::Schedule(1 / 30.0f, this, &Test::ScheduledEventHandlerMethod, false, 5);

			// subscribe to scheduler using regular function (using static method from Test class, same thing as regular function)
			m_scheduler += timer::Schedule(1 / 60.0f, &Test::ScheduledEventHandlerFunction);

			// subscribe to scheduler using lambda
			m_scheduler += timer::Schedule(1.0f / 1.0f, std::function<void(float)>([this](float delta)
				{
					m_frameRateMonitor1.OnFrameCompleted(delta);
				}));

			// setup stopwatch to manage timing and start it
			m_stopwatch.OnLap += event::Handler(this, &Test::OnLap);
			m_stopwatch.Start();
			LOG("Stopwatch started...");	
		}

		void ScheduledEventHandlerMethod(float delta)
		{
			m_frameRateMonitor2.OnFrameCompleted(delta);
		}

		static void ScheduledEventHandlerFunction(float delta)
		{
			testFrameRate::Test::Instance().m_frameRateMonitor3.OnFrameCompleted(delta);
		}

		// this method is fired up whenever the OnLap event is triggered from stopwatch
		void OnLap(float time)
		{
			m_scheduler.Update(time);

			// this monitors the frame rate of the main loop
			m_frameRateMonitor4.OnFrameCompleted(time);

			// start the canvas. we can draw from here
			m_canvas->Begin();
			{
				m_canvas->Clear({ 0.2f, 0.2f, 1.0f, 1.0f });

				m_renderer->Begin();
				{
					std::string Text = "The application subscribes to scheduler using Lambda, class method, and free function";
					m_renderer->DrawText(*m_fontAtlas, Text, { 10, 200 }, { 1,1,1,1 });
					Text = "FrameRateMonitor for each subscribed listeners are attached, with measure range of 1 second";
					m_renderer->DrawText(*m_fontAtlas, Text, { 10, 230 }, { 1,1,1,1 });
					Text = "FrameRateMonitor is also attached to main loop to monitor main FPS, with measure range of 1 second";
					m_renderer->DrawText(*m_fontAtlas, Text, { 10, 260 }, { 1,1,1,1 });
					Text = "Note: The test class is a singleton so it can provide a static method as a free function";
					m_renderer->DrawText(*m_fontAtlas, Text, { 10, 290 }, { 1,1,1,1 });

					std::string fpsText = "FPS: " + std::to_string(static_cast<int>(m_frameRateMonitor4.GetAverageFrameRate()));
					m_renderer->DrawText(*m_fontAtlas, fpsText, { 10, 330 }, { 1,1,1,1 });

					std::string fpsLambdaText = "FPS(Lambda 1 FPS): " + std::to_string(static_cast<int>(m_frameRateMonitor1.GetAverageFrameRate()));
					m_renderer->DrawText(*m_fontAtlas, fpsLambdaText, { 10, 360 }, { 1,1,1,1 });

					std::string fpsMethodText = "FPS(Method 30 FPS): " + std::to_string(static_cast<float>(m_frameRateMonitor2.GetAverageFrameRate()));
					m_renderer->DrawText(*m_fontAtlas, fpsMethodText, { 10, 390 }, { 1,1,1,1 });

					std::string fpsFuncText = "FPS(Free Function 60 FPS): " + std::to_string(static_cast<float>(m_frameRateMonitor3.GetAverageFrameRate()));
					m_renderer->DrawText(*m_fontAtlas, fpsFuncText, { 10, 420 }, { 1,1,1,1 });
				}
				m_renderer->End();
			}
			m_canvas->End();
		}

		// fun stuff. this is called on each loop of the message loop. this is where we draw!
		void OnIdle()
		{
			// call lap to get elapsed time and trigger OnLap event
			m_stopwatch.Lap<timer::seconds>();
		}

		void OnWindowSize(size_t nWidth, size_t nHeight)
		{
			LOG("Window resized to: " + std::to_string(nWidth) + ", " + std::to_string(nHeight));
			m_canvas->Resize({ static_cast<unsigned int>(nWidth), static_cast<unsigned int>(nHeight) });
			m_canvas->SetViewPort();
		}
	};

}

