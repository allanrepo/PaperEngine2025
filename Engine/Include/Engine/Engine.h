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

		void OnInitialize();
		void OnIdle();
		void OnExit();

		void OnWindowClose();
		void OnWindowSize(size_t nWidth, size_t nHeight);
		void OnWindowCreate(void* hWnd);
		void ProcessWin32Message(UINT msg, WPARAM wParam, LPARAM lParam);

		void OnLap(float delta);

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

