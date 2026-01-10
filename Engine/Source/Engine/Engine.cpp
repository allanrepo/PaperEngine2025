#include <Engine/Engine.h>
#include <Engine/Factory/CanvasFactory.h>
#include <Engine/Factory/RendererFactory.h>
#include <Cache/Registry.h>
#include <Cache/Dictionary.h>
#include <Utilities/Logger.h>
#include <Utilities/Utilities.h>
#include <Core/Input.h>

engine::Engine::Engine(
	std::string title,
	std::string API,
	std::string RenderMode
)
{
	// let's setup environment config first before we do anything else
	cache::Registry<cache::Dictionary<>>::Instance().Register("EnvironmentConfig", std::make_unique<cache::Dictionary<>>());
	cache::Dictionary<>& environmentConfig = cache::Registry<cache::Dictionary<>>::Instance().Get("EnvironmentConfig");

	environmentConfig.Register("Title", title);
	environmentConfig.Register("API", API);
	environmentConfig.Register("RenderMode", RenderMode);

	// register our lookup table into cache. this is lookup table for sprite names to corresponding sprite atlas
	cache::Registry<cache::Dictionary<>>::Instance().Register("SpriteToAtlasMap", std::make_unique<cache::Dictionary<>>());

	// register our image file to atlas UVs lookup table into cache. our atlas UV's are stored in csv file
	cache::Registry<cache::Dictionary<>>::Instance().Register("AtlasToUVRectsMap", std::make_unique<cache::Dictionary<>>());

	// now we can setup window event handlers
	Win32::Window::OnInitialize += event::Handler(this, &Engine::Initialize);
	Win32::Window::OnExit += event::Handler(this, &Engine::Exit);
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

	// create our window here
	m_window = std::make_unique<Win32::Window>();
	m_window->OnClose += event::Handler(this, &Engine::WindowClose);
	m_window->OnCreate += event::Handler(this, &Engine::WindowCreate);
	m_window->OnSize += event::Handler(this, &Engine::WindowSize);
	//m_window->Create(utilities::Text::ToWide(environmentConfig.Get("Title")).c_str(), 1400, 900);
	m_window->Create(L"window title", 1400, 900);
	m_window->OnWindowMessage += event::Handler(this, &Engine::ProcessWin32Message);
	//m_window->OnWindowMessage += event::Handler(&input::Input::Instance(), &input::Input::ProcessWin32Message);
}

void engine::Engine::ProcessWin32Message(UINT msg, WPARAM wParam, LPARAM lParam)
{
	OnProcessWin32Message(msg, wParam, lParam);
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


	// create utility font atlas

	// emit event that we are ready to start
	OnStart();
	LOG("[ENGINE] Start event happened...");

	// setup stopwatch to manage timing
	m_stopwatch.OnLap += event::Handler(this, &Engine::Lap);
	m_stopwatch.Start();
	LOG("[ENGINE] Timer started...");

}

void engine::Engine::Lap(float delta)
{
	input::Input::Instance().Update();

	m_scheduler.Update(delta);

	//m_commandQueue.Dispatch(engine::command::Type::Logic, true);

	//OnUpdate(delta);

	// start the canvas. we can draw from here
	m_canvas->Begin();
	{
		m_canvas->Clear({ 0.2f, 0.2f, 1.0f, 1.0f });

		m_canvas->SetViewPort();

		// render sprites using immediate renderer. renders a bunch of quads and text at the right side of screen
		m_renderer->Begin();
		{
			// dispatch render commands on queue and clear them after dispatch
			m_commandQueue.Dispatch(engine::command::Type::Render, true);

			// emit render event
			//OnRender();
		}
		m_renderer->End();
	}
	// end the canvas. we don't draw anything past this.
	m_canvas->End();
}

void engine::Engine::Idle()
{
	m_stopwatch.Lap<timer::seconds>();
}

void engine::Engine::Exit()
{
	OnEnd();
	LOG("[ENGINE] End event happened...");
}


void engine::Engine::WindowClose()
{

}

void engine::Engine::WindowSize(size_t width, size_t height)
{
	LOG("[ENGINE] window resized to " << width << " height = " << height << "...");
	m_canvas->Resize({ static_cast<unsigned int>(width), static_cast<unsigned int>(height) });
	OnResize(width, height);
}

