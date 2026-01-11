#include <Engine/Engine.h>
#include <Engine/Factory/CanvasFactory.h>
#include <Engine/Factory/RendererFactory.h>
#include <Cache/Registry.h>
#include <Cache/Dictionary.h>
#include <Utilities/Logger.h>
#include <Utilities/Utilities.h>
#include <Core/Input.h>

#include <iomanip>

engine::Engine::Engine(
	std::string title,
	std::string API,
	std::string RenderMode
):
	m_renderController(600.0f)
{
	// let's setup environment config first before we do anything else
	cache::Registry<cache::Dictionary<>>::Instance().Register("EnvironmentConfig", std::make_unique<cache::Dictionary<>>());
	cache::Dictionary<>& environmentConfig = cache::Registry<cache::Dictionary<>>::Instance().Get("EnvironmentConfig");

	// register environment config e.g. API, render mode
	environmentConfig.Register("Title", title);
	environmentConfig.Register("API", API);
	environmentConfig.Register("RenderMode", RenderMode);

	// register our lookup table into cache. this is lookup table for sprite names to corresponding sprite atlas
	//cache::Registry<cache::Dictionary<>>::Instance().Register("SpriteToAtlasMap", std::make_unique<cache::Dictionary<>>());

	// register our image file to atlas UVs lookup table into cache. our atlas UV's are stored in csv file
	//cache::Registry<cache::Dictionary<>>::Instance().Register("AtlasToUVRectsMap", std::make_unique<cache::Dictionary<>>());

	// now we can setup window event handlers...

	// this event will fire up just before window application is created
	Win32::Window::OnInitialize += event::Handler(this, &Engine::Initialize);

	// this event will fire up after window application closes
	Win32::Window::OnExit += event::Handler(this, &Engine::Exit);

	// this event will fire up on every windows message loop (main loop)
	Win32::Window::OnIdle += event::Handler(this, &Engine::Idle);
}

engine::Engine::~Engine()
{
}

void engine::Engine::Run()
{
	// now run the window message loop
	Win32::Window::Run();
}

void engine::Engine::Initialize()
{
	// get environment config from cache
	cache::Dictionary<>& environmentConfig = cache::Registry<cache::Dictionary<>>::Instance().Get("EnvironmentConfig");

	// create our window application object here
	m_window = std::make_unique<Win32::Window>();

	// subscribe the window application's events...

	// this will fire up when window application closes
	m_window->OnClose += event::Handler(this, &Engine::WindowClose);

	// this will fire up after window application is created
	m_window->OnCreate += event::Handler(this, &Engine::WindowCreate);

	// this will fire up when window resizes
	m_window->OnSize += event::Handler(this, &Engine::WindowSize);

	// this will fire up whenever there is a window message the window application needs to process
	// stuff like keyboard and mouse inputs can be handled here
	m_window->OnWindowMessage += event::Handler(this, &Engine::ProcessWin32Message);

	// now let's create the window application
	m_window->Create(L"window title", 1400, 900);
}

void engine::Engine::ProcessWin32Message(UINT msg, WPARAM wParam, LPARAM lParam)
{
	ProcessWin32MessageEvent(msg, wParam, lParam);
}

void engine::Engine::WindowCreate(void* hWnd)
{
	// get environment config from cache
	cache::Dictionary<>& environmentConfig = cache::Registry<cache::Dictionary<>>::Instance().Get("EnvironmentConfig");

	// create canvas
	m_canvas = graphics::CanvasFactory::Create();
	if(!m_canvas)
	{
		throw std::exception("Failed to create canvas.");
	}
	if (!m_canvas->Initialize(hWnd))
	{
		throw std::exception("Failed to initialize canvas.");
	}
	LOG("[ENGINE] Using canvas type: " << m_canvas->GetTypeName());
	
	// create sprite renderer
	m_renderer = graphics::factory::RendererFactory::Create();
	if (!m_renderer)
	{
		throw std::exception("Failed to create sprite renderer.");
	}
	if (!m_renderer->Initialize())
	{
		throw std::exception("Failed to initialize sprite renderer.");
	}
	LOG("[ENGINE] Using sprite renderer mode: " << environmentConfig.Get("RenderMode"));

	// emit event that we are ready to start
	StartEvent();
	LOG("[ENGINE] Start event happened...");

	// setup stopwatch to manage timing
	m_stopwatch.OnLap += event::Handler(this, &Engine::Lap);
	m_stopwatch.Start();
	LOG("[ENGINE] Timer started...");

	//m_scheduler += timer::Schedule(1.0f, this, &Engine::DebugShowStatistics);

	m_renderController += event::Handler(this, &Engine::OnRender);

}

void engine::Engine::Lap(float delta)
{
	m_mainLoopMonitor.OnFrameCompleted(delta);

	input::Input::Instance().Update();

	m_scheduler.Update(delta);

	// accumulator += delta
	// if accumulator > target - render, accumulator = 0;
	// if accumulator < target - delta + accumulator

	m_renderController.Update(delta);

	//if (m_renderController.IsReady(delta))
	//{
	//	OnRender(m_renderController.GetLastElapsedTime());
	//}

	return;


	//OnUpdate(delta);


}

void engine::Engine::DebugShowStatistics(float delta)
{
	LOG("[ENGINE] MAIN LOOP FPS: " << std::setprecision(15) << m_mainLoopMonitor.GetAverageFrameRate());
	LOG("[ENGINE] RENDER FPS: " << std::setprecision(15) << m_renderMonitorMonitor.GetAverageFrameRate());
}

void engine::Engine::OnRender(float delta)
{
	// monitor render loop's frame rate
	m_renderMonitorMonitor.OnFrameCompleted(delta);

	// start the canvas. we can draw from here
	m_canvas->Begin();
	{
		// clear the screen TODO: provide setter for color
		m_canvas->Clear({ 0.2f, 0.2f, 1.0f, 1.0f });

		// set viewport
		m_canvas->SetViewPort();

		// render block
		m_renderer->Begin();
		{
			// dispatch render commands on queue 
			m_commandQueue.Dispatch(engine::command::Type::Render, false);
		}
		m_renderer->End();
	}
	// end the canvas. we don't draw anything past this.
	m_canvas->End();
}

void engine::Engine::Idle()
{
	// we use stopwatch to measure the elapsed time between this frame and the last frame. 
	// the elapsed time is passed into the event emmited by stopwatch when lap is executed
	m_stopwatch.Lap<timer::seconds>();
}

void engine::Engine::Exit()
{
	EndEvent();
	LOG("[ENGINE] End event happened...");
}


void engine::Engine::WindowClose()
{
}

void engine::Engine::WindowSize(size_t width, size_t height)
{
	LOG("[ENGINE] window resized to " << width << " height = " << height << "...");
	m_canvas->Resize({ static_cast<unsigned int>(width), static_cast<unsigned int>(height) });
	ResizeEvent(width, height);
}

