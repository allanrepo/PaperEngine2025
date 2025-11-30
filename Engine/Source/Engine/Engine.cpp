#include <Engine/Engine.h>
#include <Engine/Factory/CanvasFactory.h>
#include <Cache/Registry.h>
#include <Cache/Dictionary.h>
#include <Utilities/Logger.h>

engine::Engine::Engine(
	std::string API,
	std::string RenderMode
)
{
	// let's setup environment config first before we do anything else
	cache::Registry<cache::Dictionary<>>::Instance().Register("EnvironmentConfig", std::make_unique<cache::Dictionary<>>());
	cache::Dictionary<>& environmentConfig = cache::Registry<cache::Dictionary<>>::Instance().Get("EnvironmentConfig");

	environmentConfig.Register("API", API);
	environmentConfig.Register("RenderMode", RenderMode);

	// register our lookup table into cache. this is lookup table for sprite names to corresponding sprite atlas
	cache::Registry<cache::Dictionary<>>::Instance().Register("SpriteToAtlasMap", std::make_unique<cache::Dictionary<>>());

	// register our image file to atlas UVs lookup table into cache. our atlas UV's are stored in csv file
	cache::Registry<cache::Dictionary<>>::Instance().Register("AtlasToUVRectsMap", std::make_unique<cache::Dictionary<>>());

	// now we can setup window event handlers
	Win32::Window::OnInitialize += event::Handler(this, &Engine::OnInitialize);
	Win32::Window::OnExit += event::Handler(this, &Engine::OnExit);
	Win32::Window::OnIdle += event::Handler(this, &Engine::OnIdle);
}

engine::Engine::~Engine()
{
}

void engine::Engine::Run()
{
	// now run the window message loop
	Win32::Window::Run();
}

void engine::Engine::OnInitialize()
{
	// create our window here
	m_window = std::make_unique<Win32::Window>();
	m_window->OnClose += event::Handler(this, &Engine::OnWindowClose);
	m_window->OnCreate += event::Handler(this, &Engine::OnWindowCreate);
	m_window->OnSize += event::Handler(this, &Engine::OnWindowSize);
	m_window->Create(L"Test Bedding", 1400, 900);
	m_window->OnWindowMessage += event::Handler(this, &Engine::ProcessWin32Message);
	//m_window->OnWindowMessage += event::Handler(&input::Input::Instance(), &input::Input::ProcessWin32Message);
}

void engine::Engine::ProcessWin32Message(UINT msg, WPARAM wParam, LPARAM lParam)
{
	OnProcessWin32Message(msg, wParam, lParam);
}

void engine::Engine::OnWindowCreate(void* hWnd)
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

	// create utility font atlas

	// emit event that we are ready to start
	OnStart();
	LOG("[ENGINE] Start event happened...");

	// setup stopwatch to manage timing
	m_stopwatch.OnLap += event::Handler(this, &Engine::OnLap);
	m_stopwatch.Start();
	LOG("[ENGINE] Timer started...");
}

void engine::Engine::OnLap(float delta)
{
	//input::Input::Instance().Update();
	OnUpdate(delta);

	// start the canvas. we can draw from here
	m_canvas->Begin();
	{
		m_canvas->Clear(0.2f, 0.2f, 1.0f, 1.0f);
	}
	// end the canvas. we don't draw anything past this.
	m_canvas->End();
}

void engine::Engine::OnIdle()
{
	m_stopwatch.Lap<timer::milliseconds>();
}

void engine::Engine::OnExit()
{

}


void engine::Engine::OnWindowClose()
{

}

void engine::Engine::OnWindowSize(size_t width, size_t height)
{
	LOG("[ENGINE] window resized to " << width << " height = " << height << "...");
	m_canvas->Resize(static_cast<unsigned int>(width), static_cast<unsigned int>(height));
	OnResize(width, height);
}


