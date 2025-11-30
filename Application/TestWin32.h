#pragma once

#include <Win32/Window.h>
#include <Core/Event.h>
#include <Utilities/Logger.h>

namespace test
{
	class TestWin32
	{
	private:
		std::unique_ptr<Win32::Window> window;

	public:
		TestWin32()
		{
			Win32::Window::OnInitialize += event::Handler(this, &TestWin32::OnInitialize);
			Win32::Window::OnExit += event::Handler(this, &TestWin32::OnExit);
			Win32::Window::OnIdle += event::Handler(this, &TestWin32::OnIdle);

			Win32::Window::Run();
		}

		// function that will be called just before we enter into message loop
		void OnInitialize()
		{
			// create our window here
			window = std::make_unique<Win32::Window>();
			window->OnClose += event::Handler(this, &TestWin32::OnWindowClose);
			window->OnCreate += event::Handler(this, &TestWin32::OnWindowCreate);
			window->OnSize += event::Handler(this, &TestWin32::OnWindowSize);
			window->Create(L"Test Win32", 1400, 900);
		}

		// when window is created
		void OnWindowCreate(void* hWnd)
		{
			LOG("Window created...");
		}



		// fun stuff. this is called on each loop of the message loop. this is where we draw!
		void OnIdle()
		{
		}

		void OnExit()
		{

		}

		void OnWindowClose()
		{
			LOG("Window closed...");
		}

		void OnWindowSize(size_t nWidth, size_t nHeight)
		{
			LOG("Window resized to: " + std::to_string(nWidth) + ", " + std::to_string(nHeight));
		}
	};
}