#include "DemoAsyncLoader.h"

#include <algorithm>
#include <Graphics/Resource/DX11TextureImpl.h>
#include <Cache/Registry.h>

#pragma region LoadAsyncLoaderState
demo::LoadAsyncLoaderState::LoadAsyncLoaderState(const std::string& filePath) :
	m_filePath(filePath),	
	m_isFinished(false),
	m_fileReader(0x4),
	m_currentLoader(nullptr),
	m_tileMapLoader(0xF),
	m_tileGridLoader(),
	m_tileRegionLoader(),
	m_tileLayerClearer()
{
}

demo::LoadAsyncLoaderState::~LoadAsyncLoaderState()
{
}

void demo::LoadAsyncLoaderState::Enter(Demo& owner)
{
	// create sprite atlas to be used by tilemap
	engine::cache::Registry<engine::graphics::resource::ISpriteAtlas>::Instance().Register("atlasForDemo", std::make_unique<engine::graphics::resource::SpriteAtlas>(std::make_unique<engine::graphics::dx11::resource::DX11TextureImpl>()));


	// load sprite atlas from file manually for demo purpose
	engine::graphics::resource::ISpriteAtlas& atlas = engine::cache::Registry<engine::graphics::resource::ISpriteAtlas>::Instance().Get("atlasForDemo");
	atlas.Initialize(L"../Assets/4x1_128x32_tile.png");

	// load sprite atlas UVs from csv manually for demo purpose. we calculate UVs here by assuming a grid of 1 rows and 4 columns
	atlas.AddUVRects(demo::CalcUV(1, 4, (int)atlas.GetWidth(), (int)atlas.GetHeight()));
	LOG("Sprite atlas created...");

	// create tileset for tilemap and register tiles
	m_tileset = std::make_unique<engine::component::tile::Tileset<RenderableTile>>();
	m_tileset->Register(0, std::make_unique<RenderableTile>(atlas.MakeSprite(0), true)); // walkable
	m_tileset->Register(1, std::make_unique<RenderableTile>(atlas.MakeSprite(1), false)); // obstacle
	m_tileset->Register(2, std::make_unique<RenderableTile>(atlas.MakeSprite(2), false)); // obstacle
	m_tileset->Register(3, std::make_unique<RenderableTile>(atlas.MakeSprite(3), false)); // obstacle
	LOG("Tilesets generated...");

	// specify the tilemap file to read. it must be csv file
	m_fileReader.Open(m_filePath);
	LOG("Loading ... " << m_filePath);

	// chain our events where CSV parser listens to file reader when it extract chunk of data from file
	m_fileReader.ProcessChunkEvent += engine::event::Handler(&m_csvParser, &engine::utilities::parser::CSVParser::ParseChunk);
	m_fileReader.EndOfFileFoundEvent += engine::event::Handler(&m_csvParser, &engine::utilities::parser::CSVParser::ParseRemaining);

	// chain CSV table to CSV parser to acquire row of data from CSV Parser when it parse chunk of data and extracts rows of CSV data
	m_csvParser.ParseRowEvent += engine::event::Handler(&m_table, &engine::container::Table<std::string>::AddRow);
	m_csvParser.ParseRemainingEvent += engine::event::Handler(&m_table, &engine::container::Table<std::string>::AddRange);

	// create our tile objects
	m_layer = std::make_unique <engine::component::tile::TileLayer<RenderableTile>>();
	m_grid = std::make_unique <engine::component::tile::TileGrid<RenderableTile>>();
	m_region = std::make_unique <engine::component::tile::TileRegion<RenderableTile>>();


	// define job to read map file in chunks
	std::unique_ptr<engine::job::Job> readTileMapFileJob = std::make_unique<engine::job::Job>(
		engine::job::Job(
			[this](){ m_currentLoader = &m_fileReader; }, 
			[this](){ m_fileReader.Update(0.001); },
			true,
			[this]() { return m_fileReader.IsDone(); },
			[this](){}
		));

	// define job to create tilegrid object and load the csv data into it
	std::unique_ptr<engine::job::Job> loadTileGridJob = std::make_unique<engine::job::Job>(
		engine::job::Job(
			[this]()
			{
				m_tileGridLoader.Begin("Loading TileGrid", *m_grid.get(), m_table,  
					[this](const int& cell) -> engine::component::tile::Tile<RenderableTile>
					{						
						return m_tileset->MakeTile(cell);
					});
				m_currentLoader = &m_tileGridLoader;
			},
			[this](){ m_tileGridLoader.Update(0.0001); },
			true,
			[this]() { return m_tileGridLoader.IsDone(); },
			[this, &owner](){}
		));

	// define job to create tileregion object and load the csv data into it
	std::unique_ptr<engine::job::Job> loadTileRegionJob = std::make_unique<engine::job::Job>(
		engine::job::Job(
			[this]()
			{
				m_tileRegionLoader.Begin("Loading TileRegion", *m_region.get(), m_table, 
					[this](const int& cell) -> engine::component::tile::Tile<RenderableTile>
					{
						return m_tileset->MakeTile(cell);
					});
				m_currentLoader = &m_tileRegionLoader;
			},
			[this]() { m_tileRegionLoader.Update(0.0001); },
			true,
			[this]() { return m_tileRegionLoader.IsDone(); },
			[this, &owner]() {}
		));

	// define job to create tilelayer object and load the csv data into it
	std::unique_ptr<engine::job::Job> loadTileLayerJob = std::make_unique<engine::job::Job>(
		engine::job::Job(
			[this]()
			{
				m_tileLayerLoader.Begin("Loading TileLayer", *m_layer.get(), m_table, { 32, 32 },
					[this](const int& cell) -> engine::component::tile::Tile<RenderableTile>
					{
						return m_tileset->MakeTile(cell);
					});
				m_currentLoader = &m_tileLayerLoader;
			},
			[this]() { m_tileLayerLoader.Update(0.0001); },
			true,
			[this]() { return m_tileLayerLoader.IsDone(); },
			[this, &owner]() {}
		));


	// define job to unload data from tilegrid object
	std::unique_ptr<engine::job::Job> unloadTileGridJob = std::make_unique<engine::job::Job>(
		engine::job::Job(
			[this]()
			{
				m_tileGridClearer.Begin("Unloading TileGrid", *m_grid.get(), 0.01);
				m_currentLoader = &m_tileGridClearer;
			},
			[this](){ m_tileGridClearer.Update(0.0001); },
			true,
			[this]() { return m_tileGridClearer.IsDone(); },
			[this, &owner](){}
		));

	// define job to unload data from tileregion object
	std::unique_ptr<engine::job::Job> unloadTileRegionJob = std::make_unique<engine::job::Job>(
		engine::job::Job(
			[this]()
			{
				m_tileGridClearer.Begin("Unloading TileRegion", *m_region.get(), 0.01);
				m_currentLoader = &m_tileGridClearer;
			},
			[this]() { m_tileGridClearer.Update(0.0001); },
			true,
			[this]() { return m_tileGridClearer.IsDone(); },
			[this, &owner]() {}
		));

	// define job to unload data from tilelayer object
	std::unique_ptr<engine::job::Job> unloadTileLayerJob = std::make_unique<engine::job::Job>(
		engine::job::Job(
			[this]()
			{
				m_tileLayerClearer.Begin("Unloading TileLayer", *m_layer.get(), 0.01);
				m_currentLoader = &m_tileLayerClearer;
			},
			[this]() { m_tileLayerClearer.Update(0.0001); },
			true,
			[this]() { return m_tileLayerClearer.IsDone(); },
			[this, &owner]() 
			{
			}
		));

	// define job to create tilemap object and load the csv data into it
	std::unique_ptr<engine::job::Job> loadTileMapJob = std::make_unique<engine::job::Job>(
		engine::job::Job(
			[this]()
			{
				m_tileMapLoader.Open(
					m_filePath,
					{ 128, 128 },
					[this](const int& cell) -> engine::component::tile::Tile<RenderableTile>
					{
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
			[this, &owner]() 
			{
				m_isFinished = true;
				owner.QueueState(std::make_unique<RenderAsyncLoaderState>(std::move(m_layer), std::move(m_tileset)));
			}
		));

	// define job to unload data from table object
	std::unique_ptr<engine::job::Job> unloadTableJob = std::make_unique<engine::job::Job>(
		engine::job::Job(
			[this]()
			{
				m_tableClearer.Begin("Unloading table", m_table, 0.01);
				m_currentLoader = &m_tableClearer;
			},
			[this]() { m_tableClearer.Update(0.0001); },
			true,
			[this]() { return m_tableClearer.IsDone(); },
			[this, &owner](){}
		));

	// define job to create tileregion object and load the csv data into it
	std::unique_ptr<engine::job::Job> loadCSVMapToTileRegionJob = std::make_unique<engine::job::Job>(
		engine::job::Job(
			[this]()
			{
				m_asyncCSVMapToTileRegionLoader.Open(
					m_filePath,
					[this](const int& cell) -> engine::component::tile::Tile<RenderableTile>
					{
						return m_tileset->MakeTile(cell);
					},
					*m_region.get()
				);
			},
			[this]()
			{
				m_asyncCSVMapToTileRegionLoader.Update(0.001);
				m_currentLoader = &m_asyncCSVMapToTileRegionLoader;
			},
			true,
			[this]() { return m_asyncCSVMapToTileRegionLoader.IsDone(); },
			[this, &owner]()
			{
			}
		));

	std::unique_ptr<engine::job::JobChain> jobChain = std::make_unique<engine::job::JobChain>(owner.Engine().JobQueue());



	//jobChain->AddJob(std::move(readTileMapFileJob));
	//jobChain->AddJob(std::move(loadTileGridJob));
	//jobChain->AddJob(std::move(loadTileRegionJob));
	//jobChain->AddJob(std::move(loadTileLayerJob));
	jobChain->AddJob(std::move(loadCSVMapToTileRegionJob));
	//jobChain->AddJob(std::move(unloadTableJob));
	//jobChain->AddJob(std::move(unloadTileGridJob));
	//jobChain->AddJob(std::move(unloadTileRegionJob));
	//jobChain->AddJob(std::move(unloadTileLayerJob));
	jobChain->AddJob(std::move(loadTileMapJob));
	owner.Engine().SubmitJob(std::move(jobChain));
}

void demo::LoadAsyncLoaderState::Update(Demo& owner, double delta)
{
	// monitor frame rate
	m_frameRateMonitor.OnFrameCompleted(delta);

	// flush the draw commands on queue. we will queue new ones.  
	owner.Engine().CommandQueue().Clear(engine::command::Type::Render);

	// draw progress bar
	if (m_currentLoader)
	{
		// show which loader is current and its progress
		std::string message = m_currentLoader->GetLabel() + "... " + std::to_string(m_currentLoader->GetCurrent()) + "/" + std::to_string(m_currentLoader->GetTotal());
		owner.DrawTextCommand(message, { 50, 260 }, { 1,1,1,1 });

		// let's draw progress bar to show how much file reader has read compared to total size of the file
		float progress = (float)m_tileLayerLoader.GetProgress();
		owner.DrawProgressBarCommand({ 50, 300 }, { 400, 40 }, (float)m_currentLoader->GetCurrent(), (float)m_currentLoader->GetTotal());
	}

	// render statistics
	std::list<std::string> logs;
	logs.push_back("State: LoadTileLayerState");
	logs.push_back("State FPS: " + std::to_string(static_cast<int>(m_frameRateMonitor.GetAverageFrameRate())));
	owner.DrawStatisticsCommand(logs);
}

void demo::LoadAsyncLoaderState::Exit(Demo& owner)
{
	m_fileReader.ProcessChunkEvent -= engine::event::Handler(&m_csvParser, &engine::utilities::parser::CSVParser::ParseChunk);
	m_fileReader.EndOfFileFoundEvent -= engine::event::Handler(&m_csvParser, &engine::utilities::parser::CSVParser::ParseRemaining);

	m_csvParser.ParseRowEvent -= engine::event::Handler(&m_table, &engine::container::Table<std::string>::AddRow);
	m_csvParser.ParseRemainingEvent -= engine::event::Handler(&m_table, &engine::container::Table<std::string>::AddRange);
}

bool demo::LoadAsyncLoaderState::IsFinished(Demo& owner)
{
	return m_isFinished;
}

#pragma endregion 

#pragma region RenderAsyncLoaderState
demo::RenderAsyncLoaderState::RenderAsyncLoaderState(
	std::unique_ptr<engine::component::tile::TileLayer<RenderableTile>> layer,
	std::unique_ptr<engine::component::tile::Tileset<RenderableTile>> tileSet
) :
	m_layer(std::move(layer)),
	m_tileSet(std::move(tileSet))
{
}

demo::RenderAsyncLoaderState::~RenderAsyncLoaderState()
{
}

void demo::RenderAsyncLoaderState::Enter(Demo& owner)
{
	owner.Engine().ResizeEvent += engine::event::Handler(this, &RenderAsyncLoaderState::OnResize);
}

void demo::RenderAsyncLoaderState::Update(Demo& owner, double delta)
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

void demo::RenderAsyncLoaderState::Exit(Demo& owner)
{
	owner.Engine().ResizeEvent -= engine::event::Handler(this, &RenderAsyncLoaderState::OnResize);
}

bool demo::RenderAsyncLoaderState::IsFinished(Demo& owner)
{
	return false;
}

void demo::RenderAsyncLoaderState::OnResize(size_t width, size_t height)
{
	m_viewportSize = engine::spatial::SizeF({ static_cast<float>(width - 50), static_cast<float>(height - 50) });
}

#pragma endregion