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
#include <Timer/StopWatch.h>
#include <Graphics/Core/ICanvas.h>
#include <Graphics/Renderer/IRenderer.h>
#include <Win32/Window.h>
#include <memory>

namespace engine
{
	class Engine
	{
	private:
		std::unique_ptr<Win32::Window> m_window;
		std::unique_ptr<graphics::ICanvas> m_canvas;
		std::unique_ptr<graphics::renderer::IRenderer> m_renderer;
		timer::StopWatch m_stopwatch;

		void Initialize();
		void Idle();
		void Exit();

		void WindowClose();
		void WindowSize(size_t nWidth, size_t nHeight);
		void WindowCreate(void* hWnd);
		void ProcessWin32Message(UINT msg, WPARAM wParam, LPARAM lParam);

		void Lap(float delta);

	public:
		Engine(
			std::string title = "engine",
			std::string API = "DirectX11",
			std::string RenderMode = "Batch"
		);
		~Engine();

		event::Event<> OnStart;
		event::Event<> OnRender;
		event::Event<float> OnUpdate;
		event::Event<> OnEnd;
		event::Event<size_t, size_t> OnResize;
		event::Event<UINT, WPARAM, LPARAM> OnProcessWin32Message;

		timer::StopWatch& GetTimer()
		{
			return m_stopwatch;
		}

		graphics::renderer::IRenderer& GetRenderer()
		{
			return *m_renderer;
		}

		void Run();

	};
}

