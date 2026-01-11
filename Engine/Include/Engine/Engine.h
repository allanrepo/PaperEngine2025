/*
* design consideration:
* -	on handling render system
*	-	the engine knows the the type of API and render mode to use but it doesn't really know how to create them, nor does it care.
*	-	the engine just registers the requested API and render mode into config cache
* 	-	the engine will just use factories to create the canvas and renderer based on the type requests in the config cache
*   -	the factories will refer to config cache to create the appropriate types of canvas and renderer 
*	- 	this way, the engine is decoupled from the actual implementations of canvas and renderer
*	- 	it promotes a more modular architecture and flexibility for future updates
*	- 	the engine can easily switch between different APIs or rendering techniques without refactoring the core logic
*/

#pragma once
#include <Timer/Pulse.h>
#include <Timer/StopWatch.h>
#include <Timer/Scheduler.h>
#include <Graphics/Core/ICanvas.h>
#include <Graphics/Renderer/IRenderer.h>
#include <Command/ICommand.h>
#include <Command/CommandQueue.h>
#include <Win32/Window.h>
#include <Performance/FrameRateMonitor.h>
#include <Timer/FrameRateController.h>

#include <memory>
#include <deque>
#include <vector>
#include <list>

namespace engine
{

	class Engine
	{
	private:
		std::unique_ptr<Win32::Window> m_window;
		std::unique_ptr<graphics::ICanvas> m_canvas;
		std::unique_ptr<graphics::renderer::IRenderer> m_renderer;
		timer::StopWatch m_stopwatch;
		command::CommandQueue m_commandQueue;
		performance::FrameRateMonitor m_mainLoopMonitor;
		performance::FrameRateMonitor m_renderMonitorMonitor;
		timer::FrameRateController m_renderController;

		void Initialize();
		void Idle();
		void Exit();

		void WindowClose();
		void WindowSize(size_t nWidth, size_t nHeight);
		void WindowCreate(void* hWnd);
		void ProcessWin32Message(UINT msg, WPARAM wParam, LPARAM lParam);

		void Lap(float delta);

		void DebugShowStatistics(float delta);

		void OnRender(float delta);

	public:

		struct Statistics
		{
			float mainLoopAverageFPS;
			float mainLoopLastFPS;
			float renderAverageFPS;
			float renderLastFPS;
		};

		Engine(
			std::string title = "engine",
			std::string API = "DirectX11",
			std::string RenderMode = "Batch"
		);
		~Engine();

		event::Event<> StartEvent;
		event::Event<> EndEvent;
		event::Event<size_t, size_t> ResizeEvent;
		event::Event<UINT, WPARAM, LPARAM> ProcessWin32MessageEvent;

		timer::StopWatch& Timer()
		{
			return m_stopwatch;
		}

		graphics::renderer::IRenderer& Renderer()
		{
			return *m_renderer;
		}

		command::CommandQueue& CommandQueue()
		{
			return m_commandQueue;
		}

		math::geometry::RectF GetViewPort() const
		{
			return m_canvas->GetViewPort();
		}

		void Run();

		Statistics GetStatistics()
		{
			return Statistics
			{
				m_mainLoopMonitor.GetAverageFrameRate(),
				m_mainLoopMonitor.GetLastFrameRate(),
				m_renderMonitorMonitor.GetAverageFrameRate(),
				m_renderMonitorMonitor.GetLastFrameRate(),
			};
		}

		timer::Scheduler m_scheduler;

	};
}

