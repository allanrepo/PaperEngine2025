#include "DemoTileMap.h"

#include <algorithm>
#include <Graphics/Resource/DX11TextureImpl.h>
#include <Engine/Factory/SpriteAtlasFactory.h>
#include <Cache/Registry.h>
#include <Engine/Manager/TileMapManager.h>

#pragma region LoadTileMapState
demo::LoadTileMapState::LoadTileMapState(const std::string& filePath) :
	m_isFinished(false),
	m_mapFileName(filePath)
{
}

void demo::LoadTileMapState::Enter(Demo& owner)
{
	// using factory, create sprite atlas
	engine::graphics::factory::SpriteAtlasFactory::Create("demoTileMapAtlas", L"../Assets/4x1_128x32_tile.png", std::vector<engine::math::geometry::RectF>());

	// add UV rects on our sprite atlas
	engine::graphics::resource::ISpriteAtlas& atlas = engine::cache::Registry<engine::graphics::resource::ISpriteAtlas>::Instance().Get("demoTileMapAtlas");
	atlas.AddUVRects(demo::CalcUV(1, 4, (int)atlas.GetWidth(), (int)atlas.GetHeight()));

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
demo::RenderTileMapState::RenderTileMapState() :
	m_viewportSize({})
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
	m_viewportSize = engine::spatial::SizeF({ static_cast<float>(width - 50), static_cast<float>(height - 50) });
}

#pragma endregion