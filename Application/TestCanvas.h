#pragma once

#include <Win32/Window.h>
#include <Core/Event.h>
#include <Utilities/Logger.h>
#include <Graphics/Core/ICanvas.h>
#include <Graphics/Core/Canvas.h>
#include <Graphics/Core/DX11CanvasImpl.h>

namespace test
{
	class TestCanvas
	{
	private:
		std::unique_ptr<Win32::Window> window;
		std::unique_ptr<graphics::ICanvas> canvas;

	public:
		TestCanvas()
		{
			Win32::Window::OnInitialize += event::Handler(this, &TestCanvas::OnInitialize);
			Win32::Window::OnExit += event::Handler(this, &TestCanvas::OnExit);
			Win32::Window::OnIdle += event::Handler(this, &TestCanvas::OnIdle);

			Win32::Window::Run();
		}

		// function that will be called just before we enter into message loop
		void OnInitialize()
		{
			// create our window here
			window = std::make_unique<Win32::Window>();
			window->OnClose += event::Handler(this, &TestCanvas::OnWindowClose);
			window->OnCreate += event::Handler(this, &TestCanvas::OnWindowCreate);
			window->OnSize += event::Handler(this, &TestCanvas::OnWindowSize);
			window->Create(L"Test Win32", 1400, 900);
		}

		// when window is created. we can now safely create resources dependent on window
		void OnWindowCreate(void* hWnd)
		{
			LOG("Window created...");

			// create dx11 canvas
			canvas = std::make_unique<graphics::Canvas>(std::make_unique<graphics::dx11::DX11CanvasImpl>());
			canvas->Initialize(hWnd);			
		}



		// fun stuff. this is called on each loop of the message loop. this is where we draw!
		void OnIdle()
		{
			// start the canvas. we can draw from here
			canvas->Begin();
			{
				canvas->Clear(0.2f, 0.2f, 1.0f, 1.0f);
			}
			// end the canvas. we don't draw anything past this.
			canvas->End();
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
			canvas->Resize(static_cast<unsigned int>(nWidth), static_cast<unsigned int>(nHeight));
		}
	};
}