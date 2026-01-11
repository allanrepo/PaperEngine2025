#include "Demo.h"

#include <Graphics/Renderable/FontAtlas.h>
#include <Graphics/Resource/DX11TextureImpl.h>
#include <Utilities/Logger.h>
#include <algorithm>


#pragma region demo
demo::Demo::Demo() :
	m_engine("Test State Machine", "DirectX11", "Batch"),
	m_stateMachine(this)
{
	m_engine.StartEvent += event::Handler(this, &Demo::OnStart);

	m_engine.m_scheduler += timer::Schedule(1.0f/60.0f, this, &Demo::OnUpdate);

	m_engine.Run();
}

demo::Demo::~Demo()
{
}

void demo::Demo::OnStart()
{
	// set initial state
	m_stateMachine.Set(std::make_unique<LaunchState>());
}

void demo::Demo::OnUpdate(float delta)
{
	m_stateMachine.Update(delta);
}

#pragma endregion

#pragma region launch state
demo::LaunchState::LaunchState() :
	m_frameRateMonitor(1.0f)
{

}

void demo::LaunchState::Enter(Demo& owner)
{
	// create font atlas for rendering text we will use fore demo
	m_fontAtlas = std::make_unique<graphics::renderable::FontAtlas>(std::make_unique<graphics::dx11::resource::DX11TextureImpl>());
	m_fontAtlas->Initialize("Arial", 24);
	LOG("[LaunchState] Font atlas created and initialized...");
}

void demo::LaunchState::Exit(Demo& owner)
{
}

void demo::LaunchState::Update(Demo& owner, float delta)
{
	m_frameRateMonitor.OnFrameCompleted(delta);

	// flush the draw commands on queue. we will queue new ones 
	owner.Engine().CommandQueue().Clear(engine::command::Type::Render);

	// get engine performance statistics
	engine::Engine::Statistics stats = owner.Engine().GetStatistics();
	
	// render text showing which state are we in
	float width = m_fontAtlas->GetWidth("State: LaunchState");
	float height = m_fontAtlas->GetHeight();

	std::unique_ptr<engine::command::graphics::renderer::DrawTextCommand> drawTextCmd =
		std::make_unique<engine::command::graphics::renderer::DrawTextCommand>(
			owner.Engine().Renderer(),
			*m_fontAtlas,
			"State: LaunchState",
			spatial::PositionF
			{
				owner.Engine().GetViewPort().GetWidth() - width - 10.0f,
				10
			},
			graphics::ColorF{ 1.0f, 1.0f, 1.0f, 1.0f }
		);
	owner.Engine().CommandQueue().Enqueue(std::move(drawTextCmd));

	std::string text = "State FPS: " +std::to_string(static_cast<int>(m_frameRateMonitor.GetAverageFrameRate()));
	width = m_fontAtlas->GetWidth(text);

	drawTextCmd =
		std::make_unique<engine::command::graphics::renderer::DrawTextCommand>(
			owner.Engine().Renderer(),
			*m_fontAtlas,
			text,
			spatial::PositionF
			{
				owner.Engine().GetViewPort().GetWidth() - width - 10.0f,
				40
			},
			graphics::ColorF{ 1.0f, 1.0f, 1.0f, 1.0f }
		);
	owner.Engine().CommandQueue().Enqueue(std::move(drawTextCmd));

	text = "Render FPS: " + std::to_string(static_cast<int>(stats.renderAverageFPS));
	width = m_fontAtlas->GetWidth(text);

	drawTextCmd =
		std::make_unique<engine::command::graphics::renderer::DrawTextCommand>(
			owner.Engine().Renderer(),
			*m_fontAtlas,
			text,
			spatial::PositionF
			{
				owner.Engine().GetViewPort().GetWidth() - width - 10.0f,
				70
			},
			graphics::ColorF{ 1.0f, 1.0f, 1.0f, 1.0f }
		);
	owner.Engine().CommandQueue().Enqueue(std::move(drawTextCmd));

	text = "Main Loop FPS: " + std::to_string(static_cast<int>(stats.mainLoopAverageFPS));
	width = m_fontAtlas->GetWidth(text);

	drawTextCmd =
		std::make_unique<engine::command::graphics::renderer::DrawTextCommand>(
			owner.Engine().Renderer(),
			*m_fontAtlas,
			text,
			spatial::PositionF
			{
				owner.Engine().GetViewPort().GetWidth() - width - 10.0f,
				100
			},
			graphics::ColorF{ 1.0f, 1.0f, 1.0f, 1.0f }
		);
	owner.Engine().CommandQueue().Enqueue(std::move(drawTextCmd));
}

bool demo::LaunchState::IsFinished(Demo& owner)
{
	return false;
}

#pragma endregion

