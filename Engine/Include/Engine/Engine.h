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
#include <Command/DrawCommand.h>
#include <Win32/Window.h>
#include <Performance/FrameRateMonitor.h>
#include <Timer/FrameRateController.h>
#include <Graphics/Core/Sprite.h>
#include <Job/IJob.h>
#include <Job/Job.h>
#include <memory>
#include <deque>
#include <vector>
#include <list>
#include <algorithm>

namespace engine
{

	class Engine
	{
	private:
		std::unique_ptr<engine::win32::Window> m_window;
		std::unique_ptr<engine::graphics::ICanvas> m_canvas;
		std::unique_ptr<engine::graphics::renderer::IRenderer> m_renderer;
		engine::timer::StopWatch m_stopwatch;
		command::CommandQueue m_commandQueue;
		performance::FrameRateMonitor m_mainLoopMonitor;
		performance::FrameRateMonitor m_renderMonitorMonitor;
		timer::FrameRateController m_renderController;
		timer::Scheduler m_scheduler;
		engine::job::JobQueue m_jobQueue;

		void Initialize();
		void Idle();
		void Exit();

		void WindowClose();
		void WindowSize(size_t nWidth, size_t nHeight);
		void WindowCreate(void* hWnd);
		void ProcessWin32Message(UINT msg, WPARAM wParam, LPARAM lParam);

		void Lap(double delta);

		void OnRender(double delta);

	public:

		struct Statistics
		{
			double mainLoopAverageFPS;
			double mainLoopLastFPS;
			double renderAverageFPS;
			double renderLastFPS;
		};

		Engine(
			std::string title = "engine",
			std::string API = "DirectX11",
			std::string RenderMode = "Batch",
			double renderFPS = 60.0
		);
		~Engine();

		engine::event::Event<> StartEvent;
		engine::event::Event<> EndEvent;
		engine::event::Event<size_t, size_t> ResizeEvent;
		engine::event::Event<UINT, WPARAM, LPARAM> ProcessWin32MessageEvent;

		engine::timer::StopWatch& Timer()
		{
			return m_stopwatch;
		}

		::engine::graphics::renderer::IRenderer& Renderer()
		{
			return *m_renderer;
		}

		command::CommandQueue& CommandQueue()
		{
			return m_commandQueue;
		}

		engine::math::geometry::RectF GetViewPort() const
		{
			return m_canvas->GetViewPort();
		}

		timer::Scheduler& Scheduler()
		{
			return m_scheduler;
		}

		engine::job::JobQueue& JobQueue()
		{
			return m_jobQueue;
		}

		void SubmitJob(std::unique_ptr<engine::job::IJob> job)
		{
			m_jobQueue.Submit(std::move(job));
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

		void QueueEnableClipRegionCommand(engine::math::geometry::RectF region)
		{
			m_commandQueue.Enqueue(std::make_unique<engine::command::graphics::renderer::SetClipRegionCommand>(*m_renderer, region, true));
		}

		void QueueDisableClipRegionCommand()
		{
			m_commandQueue.Enqueue(std::make_unique<engine::command::graphics::renderer::SetClipRegionCommand>(*m_renderer, engine::math::geometry::RectF{}, false));
		}

		void QueueDrawQuadCommand(engine::spatial::PositionF pos, spatial::SizeF size, ::engine::graphics::ColorF color, float rot)
		{
			m_commandQueue.Enqueue(std::make_unique<engine::command::graphics::renderer::DrawQuadCommand>(*m_renderer, pos, size, color, rot));
		}

		void QueueDrawSpriteCommand(::engine::graphics::Sprite sprite, engine::spatial::PositionF pos, spatial::SizeF size, ::engine::graphics::ColorF color, float rot)
		{
			m_commandQueue.Enqueue(std::make_unique<engine::command::graphics::renderer::DrawSpriteCommand>(*m_renderer, sprite, pos, size, color, rot));
		}

	};
}

