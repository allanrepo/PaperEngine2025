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
	m_engine.OnStart += event::Handler(this, &Demo::OnStart);
	//m_engine.OnUpdate += event::Handler(this, &Demo::OnUpdate);
	m_engine.OnEnd += event::Handler(this, &Demo::OnExit);
	//m_engine.OnRender += event::Handler(this, &Demo::OnRender);
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

void demo::Demo::OnRender()
{
}

void demo::Demo::OnExit()
{
}
#pragma endregion

#pragma region launch state
demo::LaunchState::LaunchState() :
	m_frameRateMonitor(1.0f),
	m_logicFrameRateMonitor(1.0f),
	m_renderFrameRateMonitor(1.0f),
	m_physicsFrameRateMonitor(1.0f)
{

}

void demo::LaunchState::Enter(Demo& owner)
{
	// create font atlas for rendering text we will use fore demo
	m_fontAtlas = std::make_unique<graphics::renderable::FontAtlas>(std::make_unique<graphics::dx11::resource::DX11TextureImpl>());
	m_fontAtlas->Initialize("Arial", 24);
	LOG("[LaunchState] Font atlas created and initialized...");


	owner.Engine().m_scheduler += timer::Schedule(1.0f, this, &LaunchState::LogicUpdate);
	owner.Engine().m_scheduler += timer::Schedule(1 / 60.0f, this, &LaunchState::RenderUpdate);
	owner.Engine().m_scheduler += timer::Schedule(1 / 30.0f, this, &LaunchState::PhysicsUpdate);
	owner.Engine().m_scheduler += timer::Schedule(1.0f, std::function<void(float)>([this](float delta) 
		{ 
			m_physicsFrameRateMonitor.OnFrameCompleted(delta);
		}));


}
void demo::LaunchState::Exit(Demo& owner)
{
}
void demo::LaunchState::Update(Demo& owner, float delta)
{
	m_frameRateMonitor.OnFrameCompleted(delta);


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

	std::string fpsText = "FPS: " + std::to_string(static_cast<int>(m_frameRateMonitor.GetAverageFrameRate()));
	width = m_fontAtlas->GetWidth(fpsText);

	std::unique_ptr<engine::command::graphics::renderer::DrawTextCommand> drawFPSCmd =
		std::make_unique<engine::command::graphics::renderer::DrawTextCommand>(
			owner.Engine().Renderer(),
			*m_fontAtlas,
			fpsText,
			spatial::PositionF
			{
				owner.Engine().GetViewPort().GetWidth() - width - 10.0f,
				40
			},
			graphics::ColorF{ 1.0f, 1.0f, 1.0f, 1.0f }
		);
	owner.Engine().CommandQueue().Enqueue(std::move(drawFPSCmd));
	
	std::string fpsLogicText = "FPS(Logic): " + std::to_string(static_cast<float>(m_logicFrameRateMonitor.GetAverageFrameRate()));
	width = m_fontAtlas->GetWidth(fpsLogicText);
	std::unique_ptr<engine::command::graphics::renderer::DrawTextCommand> drawLogicFPSCmd =
		std::make_unique<engine::command::graphics::renderer::DrawTextCommand>(
			owner.Engine().Renderer(),
			*m_fontAtlas,
			fpsLogicText,
			spatial::PositionF
			{
				owner.Engine().GetViewPort().GetWidth() - width - 10.0f,
				70
			},
			graphics::ColorF{ 1.0f, 1.0f, 1.0f, 1.0f }
		);
	owner.Engine().CommandQueue().Enqueue(std::move(drawLogicFPSCmd));

	std::string fpsRenderText = "FPS(Render): " + std::to_string(static_cast<float>(m_renderFrameRateMonitor.GetAverageFrameRate()));
	width = m_fontAtlas->GetWidth(fpsRenderText);
	std::unique_ptr<engine::command::graphics::renderer::DrawTextCommand> drawRenderFPSCmd =
		std::make_unique<engine::command::graphics::renderer::DrawTextCommand>(
			owner.Engine().Renderer(),
			*m_fontAtlas,
			fpsRenderText,
			spatial::PositionF
			{
				owner.Engine().GetViewPort().GetWidth() - width - 10.0f,
				100
			},
			graphics::ColorF{ 1.0f, 1.0f, 1.0f, 1.0f }
		);
	owner.Engine().CommandQueue().Enqueue(std::move(drawRenderFPSCmd));

	std::string fpsPhysicsText = "FPS(Physics): " + std::to_string(static_cast<float>(m_physicsFrameRateMonitor.GetAverageFrameRate()));
	width = m_fontAtlas->GetWidth(fpsPhysicsText);
	std::unique_ptr<engine::command::graphics::renderer::DrawTextCommand> drawPhysicsFPSCmd =
		std::make_unique<engine::command::graphics::renderer::DrawTextCommand>(
			owner.Engine().Renderer(),
			*m_fontAtlas,
			fpsPhysicsText,
			spatial::PositionF
			{
				owner.Engine().GetViewPort().GetWidth() - width - 10.0f,
				130
			},
			graphics::ColorF{ 1.0f, 1.0f, 1.0f, 1.0f }
		);
	owner.Engine().CommandQueue().Enqueue(std::move(drawPhysicsFPSCmd));
}
bool demo::LaunchState::IsFinished(Demo& owner)
{
	return false;
}
void demo::LaunchState::LogicUpdate(float delta)
{
	m_logicFrameRateMonitor.OnFrameCompleted(delta);
}

void demo::LaunchState::RenderUpdate(float delta)
{
	m_renderFrameRateMonitor.OnFrameCompleted(delta);
}

void demo::LaunchState::PhysicsUpdate(float delta)
{
	m_physicsFrameRateMonitor.OnFrameCompleted(delta);
}
#pragma endregion

