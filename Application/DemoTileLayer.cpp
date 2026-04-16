#include "DemoTileLayer.h"
#include <algorithm>
#include <Graphics/Resource/DX11TextureImpl.h>

#pragma region LoadTileLayerState
demo::LoadTileLayerState::LoadTileLayerState() :
	m_isFinished(false),
	m_currentLoader(nullptr),
	m_tileMapLoader(0xF)
{
}

demo::LoadTileLayerState::~LoadTileLayerState()
{
}

void demo::LoadTileLayerState::Enter(Demo& owner)
{
	// create sprite atlas to be used by tilemap
	m_spriteAtlas = std::make_unique<engine::graphics::resource::SpriteAtlas>(std::make_unique<engine::graphics::dx11::resource::DX11TextureImpl>());

	// load sprite atlas from file manually for demo purpose
	m_spriteAtlas->Initialize(L"../Assets/4x1_128x32_tile.png");

	// load sprite atlas UVs from csv manually for demo purpose. we calculate UVs here by assuming a grid of 1 rows and 4 columns
	// in real scenario, you would use SpriteAtlasLoader to load from csv file 
	std::vector<engine::math::geometry::RectF> uvs = demo::CalcUV(1, 4, (int)m_spriteAtlas->GetWidth(), (int)m_spriteAtlas->GetHeight());
	for (engine::math::geometry::RectF& rect : uvs)
	{
		m_spriteAtlas->AddUVRect(rect);
	}

	// create tileset for tilemap and register tiles
	m_tileset = std::make_unique<engine::component::tile1::Tileset<RenderableTile>>();
	m_tileset->Register(0, std::make_unique<RenderableTile>(m_spriteAtlas->MakeSprite(0), true)); // walkable
	m_tileset->Register(1, std::make_unique<RenderableTile>(m_spriteAtlas->MakeSprite(1), false)); // obstacle
	m_tileset->Register(2, std::make_unique<RenderableTile>(m_spriteAtlas->MakeSprite(2), false)); // obstacle
	m_tileset->Register(3, std::make_unique<RenderableTile>(m_spriteAtlas->MakeSprite(3), false)); // obstacle


	// create our layer object
	m_layer = std::make_unique <engine::component::tile1::TileLayer<RenderableTile>>();


	std::unique_ptr<engine::job::JobChain> jobChain = std::make_unique<engine::job::JobChain>(owner.Engine().JobQueue());


	// define job to create tilemap object and load the csv data into it
	std::unique_ptr<engine::job::Job> loadTileMapJob = std::make_unique<engine::job::Job>(
		engine::job::Job(
			[this]()
			{
				m_tileMapLoader.Open(
					"..\\Assets\\256x256.csv",
					{ 128, 128 },
					[this](const int& cell) -> engine::component::tile1::Tile<RenderableTile>
					{
						// this is safe. tileset will return "empty" tile if id is invalid. "empty" means does not have reference to tile data. tile is invalid
						return m_tileset->MakeTile(cell);
					},
					*m_layer.get()
				);
			},
			[this]()
			{
				m_tileMapLoader.Update(0.001);
				m_currentLoader = &m_tileMapLoader;
			},
			true,
			[this]() { return m_tileMapLoader.IsDone(); },
			[this, &owner](){}
		));

	// define job to create tilemap object and load the csv data into it
	std::unique_ptr<engine::job::Job> loadTileMapJob1 = std::make_unique<engine::job::Job>(
		engine::job::Job(
			[this]()
			{
				m_tileMapLoader.Open(
					"..\\Assets\\256x256.csv",
					{ 64, 64 },
					[this](const int& cell) -> engine::component::tile1::Tile<RenderableTile>
					{
						// this is safe. tileset will return "empty" tile if id is invalid. "empty" means does not have reference to tile data. tile is invalid
						return m_tileset->MakeTile(cell);
					},
					* m_layer.get()
				);
			},
			[this]()
			{
				m_tileMapLoader.Update(0.001);
				m_currentLoader = &m_tileMapLoader;
			},
			true,
			[this]() { return m_tileMapLoader.IsDone(); },
			[this, &owner]()
			{
				m_isFinished = true;
				owner.QueueState(std::make_unique<RenderTileLayerState>(std::move(m_layer), std::move(m_spriteAtlas), std::move(m_tileset)));
			}
		));

	jobChain->AddJob(std::move(loadTileMapJob));
	jobChain->AddJob(std::move(loadTileMapJob1));
	owner.Engine().SubmitJob(std::move(jobChain));
}

void demo::LoadTileLayerState::Update(Demo& owner, double delta)
{
	// monitor frame rate
	m_frameRateMonitor.OnFrameCompleted(delta);

	// flush the draw commands on queue. we will queue new ones 
	owner.Engine().CommandQueue().Clear(engine::command::Type::Render);

	// draw progress bar
	if (m_currentLoader)
	{
		// show which loader is current and its progress
		std::string message = "Loading " + m_currentLoader->GetLabel() + " " + std::to_string(m_currentLoader->GetCurrent()) + "/" + std::to_string(m_currentLoader->GetTotal());
		owner.DrawTextCommand(message, { 50, 260 }, { 1,1,1,1 });

		// let's draw progress bar to show how much file reader has read compared to total size of the file
		float progress = (float)m_currentLoader->GetProgress();
		owner.DrawProgressBarCommand({ 50, 300 }, { 400, 40 }, (float)m_currentLoader->GetCurrent(), (float)m_currentLoader->GetTotal());
	}

	// render statistics
	std::list<std::string> logs;
	logs.push_back("State: LoadTileLayerState");
	logs.push_back("State FPS: " + std::to_string(static_cast<int>(m_frameRateMonitor.GetAverageFrameRate())));
	owner.DrawStatisticsCommand(logs);
}

void demo::LoadTileLayerState::Exit(Demo& owner)
{
}

bool demo::LoadTileLayerState::IsFinished(Demo& owner)
{
	return m_isFinished;
}

#pragma endregion 

#pragma region RenderTileLayerState
demo::RenderTileLayerState::RenderTileLayerState(
	std::unique_ptr<engine::component::tile1::TileLayer<RenderableTile>> layer,
	std::unique_ptr<engine::graphics::resource::ISpriteAtlas> spriteAtlas,
	std::unique_ptr<engine::component::tile1::Tileset<RenderableTile>> tileSet
) :
	m_layer(std::move(layer)),
	m_spriteAtlas(std::move(spriteAtlas)),
	m_tileSet(std::move(tileSet))
{
}

demo::RenderTileLayerState::~RenderTileLayerState()
{
}

void demo::RenderTileLayerState::Enter(Demo& owner)
{
	owner.Engine().ResizeEvent += engine::event::Handler(this, &RenderTileLayerState::OnResize);
}

void demo::RenderTileLayerState::Update(Demo& owner, double delta)
{
	// monitor frame rate
	m_frameRateMonitor.OnFrameCompleted(delta);

	// flush the draw commands on queue. we will queue new ones 
	owner.Engine().CommandQueue().Clear(engine::command::Type::Render);

	// queue command to draw tile layer
	owner.RenderTileLayerCommand(*m_layer.get(), 1.0f);

	// render statistics
	std::list<std::string> logs;
	logs.push_back("State: RenderTileLayerState");
	logs.push_back("State FPS: " + std::to_string(static_cast<int>(m_frameRateMonitor.GetAverageFrameRate())));
	owner.DrawStatisticsCommand(logs);
}

void demo::RenderTileLayerState::Exit(Demo& owner)
{
	owner.Engine().ResizeEvent -= engine::event::Handler(this, &RenderTileLayerState::OnResize);
}

bool demo::RenderTileLayerState::IsFinished(Demo& owner)
{
	return false;
}

void demo::RenderTileLayerState::OnResize(size_t width, size_t height)
{
	m_viewportSize = engine::spatial::SizeF({ static_cast<float>(width - 50), static_cast<float>(height - 50) });
}

#pragma endregion