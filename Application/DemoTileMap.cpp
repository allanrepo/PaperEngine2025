#include "DemoTileMap.h"

#include <algorithm>
#include <Graphics/Resource/DX11TextureImpl.h>

#pragma region LoadTileMapState
demo::LoadTileMapState::LoadTileMapState(const std::string& filePath) :
	m_isFinished(false),
	m_mapFileName(filePath)
{
}

demo::LoadTileMapState::~LoadTileMapState()
{
}

void demo::LoadTileMapState::Enter(Demo& owner)
{
	// access  
}

void demo::LoadTileMapState::Update(Demo& owner, double delta)
{
	// monitor frame rate
	m_frameRateMonitor.OnFrameCompleted(delta);

	// flush the draw commands on queue. we will queue new ones.  
	owner.Engine().CommandQueue().Clear(engine::command::Type::Render);

	// render statistics
	std::list<std::string> logs;
	logs.push_back("State: LoadTileMapState");
	logs.push_back("State FPS: " + std::to_string(static_cast<int>(m_frameRateMonitor.GetAverageFrameRate())));
	owner.DrawStatisticsCommand(logs);
}

void demo::LoadTileMapState::Exit(Demo& owner)
{
}

bool demo::LoadTileMapState::IsFinished(Demo& owner)
{
	return m_isFinished;
}

#pragma endregion 

#pragma region RenderTileMapState
demo::RenderTileMapState::RenderTileMapState(
) 
{
}

demo::RenderTileMapState::~RenderTileMapState()
{
}

void demo::RenderTileMapState::Enter(Demo& owner)
{
	owner.Engine().ResizeEvent += event::Handler(this, &RenderTileMapState::OnResize);
}

void demo::RenderTileMapState::Update(Demo& owner, double delta)
{
	// monitor frame rate
	m_frameRateMonitor.OnFrameCompleted(delta);

	// flush the draw commands on queue. we will queue new ones 
	owner.Engine().CommandQueue().Clear(engine::command::Type::Render);

	// render statistics
	std::list<std::string> logs;
	logs.push_back("State: RenderTileLayerState");
	logs.push_back("State FPS: " + std::to_string(static_cast<int>(m_frameRateMonitor.GetAverageFrameRate())));
	owner.DrawStatisticsCommand(logs);
}

void demo::RenderTileMapState::Exit(Demo& owner)
{
	owner.Engine().ResizeEvent -= event::Handler(this, &RenderTileMapState::OnResize);
}

bool demo::RenderTileMapState::IsFinished(Demo& owner)
{
	return false;
}

void demo::RenderTileMapState::OnResize(size_t width, size_t height)
{
	m_viewportSize = spatial::SizeF({ static_cast<float>(width - 50), static_cast<float>(height - 50) });
}

#pragma endregion