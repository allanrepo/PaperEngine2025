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
#include <Graphics/Resource/ISpriteAtlas.h>
#include <Graphics/Resource/SpriteAtlas.h>
#include <Engine/Factory/SpriteAtlasFactory.h>
#include <Engine/Loader/SpriteAtlasLoader.h>
#include <Graphics/Renderable/Sprite.h>
#include <Core/Input.h>
#include <Graphics/Animation/Animation.h>
#include <Graphics/Resource/DX11TextureImpl.h>
#include <Timer/StopWatch.h>
#include <Timer/Pulse.h>
#include <State/State.h>
#include <State/StateMachine.h>	
#include <Graphics/Renderable/IFontAtlas.h>
#include <Graphics/Renderable/FontAtlas.h>
#include <Command/CommandQueue.h>
#include <Command/ICommand.h>
#include <Command/DrawCommand.h>
#include <Performance/FrameRateMonitor.h>
#include <limits.h>
#include "Utilities.h"

namespace TestAsyncFileReader
{
	class Test;
	class LaunchState;
	class LoadState;
}

namespace TestAsyncFileReader
{
	class Job 
	{
	public:
		// The actual work
		std::function<void()> task;

		// Persistent flag
		bool persistent;

		// Optional completion condition
		std::function<bool()> isDone;

		Job(
			std::function<void()> fn,
			bool persistent = false,
			std::function<bool()> done = nullptr
		): 
			task(std::move(fn)), 
			persistent(persistent), 
			isDone(std::move(done)) 
		{
		}
	};

	class JobQueue 
	{
	private:
		std::deque<Job> m_jobs;

	public:
		void Submit(const Job& job) {
			m_jobs.push_back(job);
		}

		void Update(size_t maxJobs = SIZE_MAX)
		{
			size_t count = 0;
			for (auto it = m_jobs.begin(); it != m_jobs.end() && count < maxJobs;) 
			{
				it->task(); // run the job

				if (!it->persistent) 
				{
					// one-shot job → remove immediately
					it = m_jobs.erase(it);
				}
				else {
					// persistent job → check condition
					if (it->isDone && it->isDone()) 
					{
						it = m_jobs.erase(it); // finished → remove
					}
					else 
					{
						++it; // keep it for next frame
					}
				}
				count++;
			}
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
			// put temporary delay to simulate processing time

			timer::StopWatch sw;

			sw.Start();
			while (sw.Peek<timer::milliseconds>() < 0.25)
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
				int nn = 0;
				timer::StopWatch sw;
				sw.Start();
				while (sw.Peek<timer::milliseconds>() < 1)
				{
					nn++;
					ProcessChunk(buffer.data(), static_cast<size_t>(n));
				}
				if (nn > 1)
				{
				//	LOG("read count: " << std::to_string(nn));
				}
			}

			// returns true if reached EOF already. false otherwise
			return m_filestream.eof();
		}

		long GetFileSizeInBytesLong()
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
	
		bool IsEndOfFile() const
		{
			return m_filestream.eof();
		}

		bool IsOpen() const
		{
			return m_filestream.is_open();
		}
	};

	class Test
	{
	private:
		std::unique_ptr<Win32::Window> m_window;
		std::unique_ptr<engine::graphics::ICanvas> m_canvas;
		std::unique_ptr<engine::graphics::renderer::IRenderer> m_renderer;
		timer::StopWatch m_stopwatch;
		AsyncFileReader m_fileReader;
		AsyncFileReader m_fileReader1;
		AsyncFileReader m_fileReader2;
		std::deque<std::string> m_files;
		std::unique_ptr<engine::graphics::renderable::IFontAtlas> m_fontAtlas;

		performance::FrameRateMonitor m_frameRateMonitor;
		performance::FrameRateMonitor m_stateFrameRateMonitor;

		timer::FrameRateController m_frameRateController;

		timer::Scheduler m_scheduler;
		
		JobQueue m_jobQueue;

		// command manager
		engine::command::CommandQueue m_commandQueue;

		// application state machine
		state::StateMachine<TestAsyncFileReader::Test> m_stateMachine;

	public:
		Test():
			m_stateMachine(this),
			m_frameRateMonitor(1.0),
			m_stateFrameRateMonitor(1.0),
			m_frameRateController(60.0)
		{
			Win32::Window::OnInitialize += event::Handler(this, &Test::OnInitialize);
			Win32::Window::OnExit += event::Handler(this, &Test::OnExit);
			Win32::Window::OnIdle += event::Handler(this, &Test::OnIdle);

			input::Input::Instance().MouseDownEvent += event::Handler(this, &Test::OnMouseDown);

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
			m_canvas = std::make_unique<engine::graphics::Canvas>(std::make_unique<engine::graphics::dx11::DX11CanvasImpl>());
			m_canvas->Initialize(hWnd);
			m_canvas->SetViewPort();
			LOG("Canvas (DX11) created...");

			// create dx11 renderer batched
			m_renderer = std::make_unique<engine::graphics::renderer::Renderer>(std::make_unique<engine::graphics::dx11::renderer::DX11RendererBatchImpl>());
			m_renderer->Initialize();
			LOG("Renderer (DX11) created...");

			// create font atlas
			m_fontAtlas = std::make_unique<engine::graphics::renderable::FontAtlas>(std::make_unique<engine::graphics::dx11::resource::DX11TextureImpl>());
			m_fontAtlas->Initialize("Arial", 24);
			LOG("Font atlas created and initialized...");

			// setup stopwatch to manage timing and start it
			m_stopwatch.OnLap += event::Handler(this, &Test::OnLap);
			m_stopwatch.Start();
			LOG("Stopwatch started...");

			m_frameRateController += event::Handler(this, &Test::OnUpdateFileReader);

			m_scheduler += timer::Schedule(1.0/1000.0, this, &Test::OnUpdateFileReader, true, 1);

			m_files.push_back("small.csv");
			m_files.push_back("big.csv");
			m_files.push_back("huge.csv");

		}

		void SetState(std::unique_ptr<state::State<TestAsyncFileReader::Test>> state)
		{
			m_stateMachine.Set(std::move(state));
		}

		void DrawProgressBarCommand(spatial::PositionF pos, spatial::SizeF size, float current, float total)
		{
			std::unique_ptr<engine::command::graphics::renderer::DrawQuadCommand> drawQuadCmd =
				std::make_unique<engine::command::graphics::renderer::DrawQuadCommand>(
					*m_renderer,
					pos,
					size,
					engine::graphics::ColorF{ 1.0f, 0.0f, 0.0f, 1.0f },
					0.0f
				);
			m_commandQueue.Enqueue(std::move(drawQuadCmd));

			drawQuadCmd =
				std::make_unique<engine::command::graphics::renderer::DrawQuadCommand>(
					*m_renderer,
					pos,
					spatial::SizeF
					{
						size.width * current / total,
						size.height
					},
					engine::graphics::ColorF{ 0.0f, 1.0f, 0.0f, 1.0f },
					0.0f
				);
			m_commandQueue.Enqueue(std::move(drawQuadCmd));
		}

		// helper function to draw text at top-right screen. this is for showing statistics like FPS
		void DrawTextCommandTopRightScreen(const std::string& text, float y)
		{
			// render text showing which state are we in
			float width = m_fontAtlas->GetWidth(text);
			float height = m_fontAtlas->GetHeight();

			std::unique_ptr<engine::command::graphics::renderer::DrawTextCommand> drawTextCmd =
				std::make_unique<engine::command::graphics::renderer::DrawTextCommand>(
					*m_renderer,
					*m_fontAtlas,
					text,
					spatial::PositionF
					{
						m_canvas->GetViewPort().GetWidth() - width - 10.0f,
						y
					},
					engine::graphics::ColorF{ 1.0f, 1.0f, 1.0f, 1.0f }
				);
			m_commandQueue.Enqueue(std::move(drawTextCmd));
		}

		engine::graphics::renderer::IRenderer& GetRenderer() const
		{
			return *m_renderer;
		}

		engine::graphics::renderable::IFontAtlas& GetFontAtlas() const
		{
			return *m_fontAtlas;
		}

		engine::command::CommandQueue& GetCommandQueue()
		{
			return m_commandQueue;
		}

		engine::graphics::ICanvas& GetCanvas() const
		{
			return *m_canvas;
		}

		JobQueue& GetJobQueue()
		{
			return m_jobQueue;
		}

		void OnUpdateFileReader(double time)
		{
			m_stateFrameRateMonitor.OnFrameCompleted(time);

			if (m_fileReader.IsEndOfFile() || !m_fileReader.IsOpen())
			{
				if (m_files.empty())
				{
					// time to get out of loading state
				}
				else
				{
					m_fileReader.Open(m_files.front());
					LOG("Loading " << m_files.front());
					m_files.pop_front();

					m_jobQueue.Submit(Job(
						[this]()
						{
							m_fileReader.Update(0x3FFF);
						},
						true,
						[this]()
						{
							return m_fileReader.IsEndOfFile();
						}
					));
				}

			}

			//// read the file in chunks
			//m_fileReader.Update(0x3FFF);

			//if (m_fileReader.IsEndOfFile() || !m_fileReader.IsOpen())
			//{
			//	m_fileReader.Close();
			//	if (!m_files.empty())
			//	{
			//		m_fileReader.Open(m_files.front());
			//		LOG("Loading " << m_files.front());
			//		m_files.pop_front();
			//	}
			//	else
			//	{
			//		// time to get out of loading state
			//		//LOG("All files loaded. state is done.");
			//	}
			//}
		}

		// this method is fired up whenever the OnLap event is triggered from stopwatch
		void OnLap(double time)
		{
			// monitor main loop's frame rate
			m_frameRateMonitor.OnFrameCompleted(time);

			//m_frameRateController.Update(time);

			m_scheduler.Update(time);

			m_jobQueue.Update();

			// before issuing any draw command for this frame, flush any draw command on queue first. 
			m_commandQueue.Clear(engine::command::Type::Render);

			// we consolidate all rendering calls here since this is considered to be the rendering loop

			// draw progress bar for first filestream
			DrawProgressBarCommand({ 50, 300 }, { 400, 50 },
				static_cast<float>(!m_fileReader.IsOpen() ? 0 : m_fileReader.IsEndOfFile() ? 1 : m_fileReader.GetNumberOfBytesReadLong()), // if EOF, pass 1 so we render progress bar at 100% progress
				static_cast<float>(m_fileReader.IsEndOfFile() ? 1 : m_fileReader.GetFileSizeInBytesLong()) // if EOF, pass 1 so we render progress bar at 100% progress
			);

			//// draw progress bar for second filestream
			//DrawProgressBarCommand({ 50, 360 }, { 400, 50 },
			//	static_cast<float>(!m_fileReader1.IsOpen() ? 0 : m_fileReader1.IsEndOfFile() ? 1 : m_fileReader1.GetNumberOfBytesReadLong()), // if EOF, pass 1 so we render progress bar at 100% progress
			//	static_cast<float>(m_fileReader1.IsEndOfFile() ? 1 : m_fileReader1.GetFileSizeInBytesLong()) // if EOF, pass 1 so we render progress bar at 100% progress
			//);

			//// draw progress bar for third filestream
			//DrawProgressBarCommand({ 50, 420 }, { 400, 50 },
			//	static_cast<float>(!m_fileReader2.IsOpen() ? 0 : m_fileReader2.IsEndOfFile() ? 1 : m_fileReader2.GetNumberOfBytesReadLong()), // if EOF, pass 1 so we render progress bar at 100% progress
			//	static_cast<float>(m_fileReader2.IsEndOfFile() ? 1 : m_fileReader2.GetFileSizeInBytesLong()) // if EOF, pass 1 so we render progress bar at 100% progress
			//);

			// display FPS via draw command
			std::string text = "FPS(Render): " + std::to_string(static_cast<int>(m_frameRateMonitor.GetAverageFrameRate()));
			DrawTextCommandTopRightScreen(text, 10.0f);

			// display state FPS via draw command
			text = "FPS(State): " + std::to_string(static_cast<int>(m_stateFrameRateMonitor.GetAverageFrameRate()));
			DrawTextCommandTopRightScreen(text, 40.0f);
		}


		// fun stuff. this is called on each loop of the message loop. this is where we draw!
		void OnIdle()
		{
			input::Input::Instance().Update();

			// call lap to get elapsed time and trigger OnLap event
			m_stopwatch.Lap<timer::seconds>();

			// start the canvas. we can draw from here
			m_canvas->Begin();
			{
				m_canvas->Clear({ 0.2f, 0.2f, 1.0f, 1.0f });

				m_renderer->Begin();
				{
					// execute render commands on queue. 
					m_commandQueue.Dispatch(engine::command::Type::Render, false);
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
				m_fileReader.Open("big.csv");
				m_fileReader1.Open("big.csv");
				m_fileReader2.Open("big.csv");
			}
			if (btn == 2)
			{
			}
		}
	};

	class LoadState : public state::State<TestAsyncFileReader::Test>
	{
	private:
		//std::deque<std::string> m_files;
		//std::unique_ptr<engine::graphics::renderable::IFontAtlas> m_fontAtlas;
		//performance::FrameRateMonitor m_frameRateMonitor;
		//AsyncFileReader m_fileReader;

	public:
		LoadState()
		{
		}
		virtual ~LoadState() = default;

		virtual void Enter(TestAsyncFileReader::Test& owner) override
		{
			//// identify files we need to read and load and store them in list
			//m_files.push_back("small.csv");
			//m_files.push_back("big.csv");
			//m_files.push_back("huge.csv");

			//// we're going to render some text here so we create font resource
			//m_fontAtlas = std::make_unique<engine::graphics::renderable::FontAtlas>(std::make_unique<graphics::dx11::resource::DX11TextureImpl>());
			//m_fontAtlas->Initialize("Arial", 24);
			//LOG("Font atlas created and initialized...");
		}
		virtual void Exit(TestAsyncFileReader::Test& owner) override
		{
		}
		virtual void Update(TestAsyncFileReader::Test& owner, double delta) override
		{
			// monitor frame rate here
			//m_frameRateMonitor.OnFrameCompleted(delta);

			// if file reader is not reading anything, get file from list and open it
			// queue job to read file
			// if file read is done, get again from list until it is empty.
			// once file list is empty, our state is done!
			//if (m_fileReader.IsEndOfFile() || !m_fileReader.IsOpen())
			//{
			//	if (m_files.empty())
			//	{
			//		// time to get out of loading state
			//	}
			//	else
			//	{
			//		m_fileReader.Open(m_files.front());
			//		LOG("Loading " << m_files.front());
			//		m_files.pop_front();

			//		owner.GetJobQueue().Submit(Job(
			//			[this]()
			//			{
			//				m_fileReader.Update(0x3FFF);
			//			},
			//			true,
			//			[this]()
			//			{
			//				return m_fileReader.IsEndOfFile();
			//			}
			//		));
			//	}
			//}

			// clear draw command queue

			// queue draw command to draw text showing current file to being loaded

			// queue draw command to draw progress bar

			// queue draw command to draw statistics

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
		virtual void Update(Test& owner, double delta) override
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
		virtual void Update(Test& owner, double delta) override
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
		virtual void Update(Test& owner, double delta) override
		{
		}
		virtual bool IsFinished(Test& owner) override
		{
			return false;
		}
	};

}

