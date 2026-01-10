#pragma once
/*
on launch
	- the application will be on idle state not rendering anything since the map is not yet loaded
	- when user clicks on screen, state changes to map loading state
on map loading state
	- load the map from file asyncronously. Once loaded, state changes to map rendering state
	- while loading, display a loading message and progress bar
on map rendering state
	- render the map and allow user to pan around
	- user can click on a button to unload the map and return to idle state
*/

#include <Win32/Window.h>
#include <Core/Event.h>
#include <Utilities/Logger.h>
#include <Graphics/Core/ICanvas.h>
#include <Graphics/Core/Canvas.h>
#include <Graphics/Core/DX11CanvasImpl.h>
#include <Graphics/Renderer/IRenderer.h>
#include <Graphics/Renderer/DX11RendererBatchImpl.h>
#include <Graphics/Renderer/Renderer.h>
#include <Graphics/Renderable/ISpriteAtlas.h>
#include <Graphics/Renderable/SpriteAtlas.h>
#include <Engine/Factory/SpriteAtlasFactory.h>
#include <Engine/Loader/SpriteAtlasLoader.h>
#include <Graphics/Renderable/Sprite.h>
#include <Core/Input.h>
#include <Graphics/Animation/Animation.h>
#include <Graphics/Resource/DX11TextureImpl.h>
#include <Timer/StopWatch.h>
#include <Timer/Pulse.h>
#include <limits.h>
#include <State/State.h>
#include <State/StateMachine.h>	
#include <Graphics/Renderable/IFontAtlas.h>
#include <Graphics/Renderable/FontAtlas.h>
#include <Command/CommandQueue.h>
#include <Command/ICommand.h>
#include <Command/DrawCommand.h>

#include "Utilities.h"

namespace TestAsyncFileReader
{
	class Test;

	class LaunchState;
	class LoadResourcesState;

}
namespace TestAsyncFileReader
{



	// a wrapper class to calculate simple frame rate at specified interval
	// it averages the frame rate over the interval to avoid too much fluctuation
	class SimpleFrameRateCounter
	{
	private:
		// calculate frame rate every pulse interval
		timer::Pulse m_frameRateRefresher;

		// pulse to refresh frame rate display so it doesn't flicker too much when frame rate changes
		bool m_updateFrameRateDisplay = false;

		// accumulated frame rate
		float m_loopElapsedTimeAccumulator = 0.0f;
		size_t m_loopFrameCount = 0;

		// store latest calculated frame rate
		float m_currentFrameRate = 0.0f;

	public:
		SimpleFrameRateCounter(float interval = 1000.0f):
			m_frameRateRefresher(interval, timer::Pulse::Mode::Persistent)
		{
			// assign frame rate refresher event handler. we feed it with a lambda function (C++11 feature) that sets the update flag to true
			// we are using std::function wrapper to wrap the lambda. This is because the event handler requires a std::function object
			// we could have also created a separate method for this instead of using lambda as this is opportunity to test this feature
			m_frameRateRefresher.OnInterval += event::Handler(std::function<void(float)>([this](float interval)
				{
					m_updateFrameRateDisplay = true;
				}));
			LOG("Frame rate refresher pulse created...");
		}

		virtual ~SimpleFrameRateCounter()
		{
		}

		void Update(float time)
		{
			// update the frame rate refresher pulse
			m_frameRateRefresher.Update(time);

			// if it's time to update frame rate display, calculate it now
			if (m_updateFrameRateDisplay)
			{
				// we assume time is in milliseconds
				m_currentFrameRate = m_loopFrameCount / m_loopElapsedTimeAccumulator * 1000.0f;
				m_updateFrameRateDisplay = false;
				m_loopElapsedTimeAccumulator = 0;
				m_loopFrameCount = 0;
			}

			// accumulate time and frame count
			m_loopElapsedTimeAccumulator += time;
			m_loopFrameCount++;
		}

		float GetCurrentFrameRate() const
		{
			return m_currentFrameRate;
		}
	};

	class AsyncFileReader
	{
	private:
		std::ifstream m_filestream;
		std::string m_filename;

	public:
		AsyncFileReader()
		{
		}

		virtual ~AsyncFileReader()
		{
		}

		bool Open(const std::string& filename)
		{
			// ensure clean state
			Close();

			// open the file 
			m_filename = filename;
			m_filestream.open(m_filename.c_str(), std::ios::binary);
			if (!m_filestream.is_open())
			{
				return false;
			}

			return true;
		}

		void Close()
		{
			if (m_filestream.is_open())
			{
				m_filestream.close();
			}
		}

		void ProcessChunk(const char* data, size_t len)
		{
			// streaming state-machine parser: accumulate chars into m_currentCell, 
			// // push on delimiter/newline, handle CRLF, quoted fields if needed. 

			// put temporary delay to simulate processing time

			timer::StopWatch sw;

			sw.Start();
			while (sw.Peek<timer::milliseconds>() < 16.0f)
			{
				// busy wait
			}
			sw.Stop();
		}

		bool Update(size_t maxBytesPerRead = 0xFF)
		{
			// is file open?
			if (!m_filestream.is_open())
			{
				return false;
			}

			// did we finished reading the file already?
			if (m_filestream.eof())
			{
				return true;
			}

			// read chunk size data
			std::vector<char> buffer(maxBytesPerRead);
			m_filestream.read(buffer.data(), buffer.size());

			std::streamsize n = m_filestream.gcount();
			if (n > 0)
			{
				ProcessChunk(buffer.data(), static_cast<size_t>(n));
			}

			// returns true if reached EOF already. false otherwise
			return m_filestream.eof();
		}

		long GetFileSizeLong()
		{
			// is file open?
			if (!m_filestream.is_open())
			{
				return -1;
			}

			// store current position. if tellg fails, it returns -1
			std::streampos currentPos = m_filestream.tellg();
			if (currentPos == std::streampos(-1))
			{
				return -1; // tellg failed
			}

			// clear any eof flags before seeking
			m_filestream.clear(); 

			// seek to end.
			m_filestream.seekg(0, std::ios::end);

			// if seekg failed, the stream state will be bad. restore to original position and return -1
			if(!m_filestream.good())
			{
				// attempt to clear bad state
				m_filestream.clear(); 

				// restore original position
				m_filestream.seekg(currentPos);

				// return -1 to indicate failure
				return -1;
			}


			std::streampos endPos = m_filestream.tellg();
			if (endPos == std::streampos(-1))
			{
				// attempt to clear bad state
				m_filestream.clear();

				// restore original position
				m_filestream.seekg(currentPos);

				// return -1 to indicate failure
				return -1;
			}

			// seek back to original position
			m_filestream.clear();
			m_filestream.seekg(currentPos);
			if (!m_filestream.good())
			{
				// attempt to clear bad state
				m_filestream.clear();

				// failed to restore position. return -1
				return -1;
			}


			// convert to long long for safe comparison
			long long size64 = static_cast<long long>(endPos);
			if (size64 < 0)
			{
				return -1;
			}

			// Note: call the member function through a pointer-to-function style to stop macro expansion to fix conflix with max macro on Windows
			if (static_cast<unsigned long long>(size64) > static_cast<unsigned long long>((std::numeric_limits<long>::max)()))
			{
				return -1; // would overflow long 
			}

			// return size. warning: this cast may lose data if file is larger than what long can hold
			return static_cast<long>(size64);
		}

		long GetNumberOfBytesReadLong()
		{
			// is file open?
			if (!m_filestream.is_open())
			{
				return -1;
			}

			// get current position. if tellg fails, it returns -1
			std::streampos currentPos = m_filestream.tellg();
			if (currentPos == std::streampos(-1))
			{
				return -1; // tellg failed
			}

			// convert to long long for safe comparison
			long long size64 = static_cast<long long>(currentPos); 
			if (size64 < 0)
			{
				return -1;
			}

			// Note: call the member function through a pointer-to-function style to stop macro expansion to fix conflix with max macro on Windows
			if (static_cast<unsigned long long>(size64) > static_cast<unsigned long long>((std::numeric_limits<long>::max)()))
			{
				return -1; // would overflow long 
			}

			// return size. warning: this cast may lose data if file is larger than what long can hold
			return static_cast<long>(size64);
		}
	};

	class Test
	{
	private:
		std::unique_ptr<Win32::Window> m_window;
		std::unique_ptr<graphics::ICanvas> m_canvas;
		std::unique_ptr<graphics::renderer::IRenderer> m_renderer;
		timer::StopWatch m_stopwatch;
		AsyncFileReader m_fileReader;
		std::unique_ptr<graphics::renderable::IFontAtlas> m_fontAtlas;

		// frame rate counter
		SimpleFrameRateCounter m_frameRateCounter;

		// command manager
		engine::command::CommandQueue m_commandQueue;

		// application state machine
		state::StateMachine<TestAsyncFileReader::Test> m_stateMachine;

	public:
		Test():
			m_stateMachine(this),
			m_frameRateCounter(500.0f) // update frame rate every 500 ms
		{
			Win32::Window::OnInitialize += event::Handler(this, &Test::OnInitialize);
			Win32::Window::OnExit += event::Handler(this, &Test::OnExit);
			Win32::Window::OnIdle += event::Handler(this, &Test::OnIdle);

			input::Input::Instance().OnMouseDown += event::Handler(this, &Test::OnMouseDown);

			Win32::Window::Run();
		}

		// function that will be called just before we enter into message loop
		void OnInitialize()
		{
			// create our window here
			m_window = std::make_unique<Win32::Window>();
			m_window->OnClose += event::Handler(this, &Test::OnWindowClose);
			m_window->OnCreate += event::Handler(this, &Test::OnWindowCreate);
			m_window->OnSize += event::Handler(this, &Test::OnWindowSize);
			m_window->OnWindowMessage += event::Handler(&input::Input::Instance(), &input::Input::ProcessWin32Message);
			m_window->Create(L"Test Sprite", 1400, 900);
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

			// setup stopwatch to manage timing and start it
			m_stopwatch.OnLap += event::Handler(this, &Test::OnLap);
			m_stopwatch.Start();
			LOG("Stopwatch started...");

			// set initial state to launch state

			//std::unique_ptr<LoadResourcesState> launchState = std::make_unique<LoadResourcesState>();

			//m_stateMachine.Set(std::make_unique<LoadResourcesState>());

		}

		graphics::renderer::IRenderer& GetRenderer() const
		{
			return *m_renderer;
		}

		graphics::renderable::IFontAtlas& GetFontAtlas() const
		{
			return *m_fontAtlas;
		}

		engine::command::CommandQueue& GetCommandQueue()
		{
			return m_commandQueue;
		}

		graphics::ICanvas& GetCanvas() const
		{
			return *m_canvas;
		}

		// this method is fired up whenever the OnLap event is triggered from stopwatch
		void OnLap(float time)
		{
			m_fileReader.Update(0xFFF);

			m_stateMachine.Update(time);
			//m_frameRateCounter.Update(time);
		}


		// fun stuff. this is called on each loop of the message loop. this is where we draw!
		void OnIdle()
		{
			input::Input::Instance().Update();

			// call lap to get elapsed time and trigger OnLap event
			m_stopwatch.Lap<timer::milliseconds>();

			// start the canvas. we can draw from here
			m_canvas->Begin();
			{
				m_canvas->Clear({ 0.2f, 0.2f, 1.0f, 1.0f });

				m_renderer->Begin();
				{
					// execute render commands on queue. clear commands after dispatching
					m_commandQueue.Dispatch(engine::command::Type::Render, true);

					//long fileSize = m_fileReader.GetFileSizeLong();
					//long bytesRead = m_fileReader.GetNumberOfBytesReadLong();
					//float size = 600.0f;
					//float sizeRead = size * (static_cast<float>(bytesRead) / static_cast<float>(fileSize > 0 ? fileSize : 0));

					//m_renderer->Draw(
					//	spatial::PositionF{ 100.0f, 400.0f },				// position
					//	spatial::SizeF{ size, 50.0f },					// size
					//	graphics::ColorF{ 1.0f, 0.0f, 0.0f, 1.0f },			// color
					//	0.0f
					//);

					//m_renderer->Draw(
					//	spatial::PositionF{ 100.0f, 400.0f },				// position
					//	spatial::SizeF{ sizeRead, 50.0f },					// size
					//	graphics::ColorF{ 0.0f, 1.0f, 0.0f, 1.0f },			// color
					//	0.0f
					//);

					//// render frame rate at top-right corner
					//{
					//	std::string fps = "FPS: " + std::to_string(static_cast<int>(m_frameRateCounter.GetCurrentFrameRate()));
					//	float textWidth = m_fontAtlas->GetWidth(fps);

					//	math::geometry::RectF vp = m_canvas->GetViewPort();

					//	spatial::PositionF pos{
					//		vp.right - textWidth - 10.0f,
					//		10.0f
					//	};

					//	m_renderer->DrawText(
					//		*m_fontAtlas,
					//		"FPS: " + std::to_string(static_cast<int>(m_frameRateCounter.GetCurrentFrameRate())),
					//		pos,
					//		graphics::ColorF{ 1.0f, 1.0f, 1.0f, 1.0f }
					//	);
					//}

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

		void OnMouseDown(int btn, int x, int y)
		{
			if (btn == 1)
			{
				// open a file to read asynchronously
				if (m_fileReader.Open("big.csv"))
				{
					LOG("Opened large CSV file for asynchronous reading.");
				}
				else
				{
					LOG("Failed to open large CSV file for asynchronous reading.");
					return;
				}
			}
			if (btn == 2)
			{
			}
		}
	};


	class LaunchState : public state::State<TestAsyncFileReader::Test>
	{
	private:
	public:
		LaunchState()
		{
		}
		virtual ~LaunchState() = default;

		virtual void Enter(TestAsyncFileReader::Test& owner) override
		{
		}
		virtual void Exit(TestAsyncFileReader::Test& owner) override
		{
		}
		virtual void Update(TestAsyncFileReader::Test& owner, float delta) override
		{
			// render text showing which state are we in
			float width = owner.GetFontAtlas().GetWidth("State: LaunchState");
			float height = owner.GetFontAtlas().GetHeight();

			math::geometry::RectF vp = owner.GetCanvas().GetViewPort();

			owner.GetCommandQueue().Enqueue(
				std::make_unique<engine::command::graphics::renderer::DrawTextCommand>(
					owner.GetRenderer(),
					owner.GetFontAtlas(),
					"State: LaunchState",
					spatial::PositionF
					{
						vp.GetWidth() - owner.GetFontAtlas().GetWidth("State: LaunchState") - 10.0f,
						owner.GetFontAtlas().GetHeight()
					},
					graphics::ColorF{ 1.0f, 1.0f, 1.0f, 1.0f }
				));

		}
		virtual bool IsFinished(TestAsyncFileReader::Test& owner) override
		{
			return false;
		}
	};



	class LoadResourcesState : public state::State<Test>
	{
	private:
	public:
		LoadResourcesState()
		{
		}
		virtual ~LoadResourcesState() = default;

		virtual void Enter(Test& owner) override
		{
		}
		virtual void Exit(Test& owner) override
		{
		}
		virtual void Update(Test& owner, float delta) override
		{
		}
		virtual bool IsFinished(Test& owner) override
		{
			return false;
		}
	};


	class IdleState : public state::State<Test>
	{
	private:
	public:
		virtual void Enter(Test& owner) override
		{
		}
		virtual void Exit(Test& owner) override
		{
		}
		virtual void Update(Test& owner, float delta) override
		{
		}
		virtual bool IsFinished(Test& owner) override
		{
			return false;
		}
	};

	class ExitState : public state::State<Test>
	{
	private:
	public:
		virtual void Enter(Test& owner) override
		{
		}
		virtual void Exit(Test& owner) override
		{
		}
		virtual void Update(Test& owner, float delta) override
		{
		}
		virtual bool IsFinished(Test& owner) override
		{
			return false;
		}
	};

}

