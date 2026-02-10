#include <Graphics/Renderable/FontAtlas.h>
#include <Graphics/Resource/DX11TextureImpl.h>
#include <Utilities/Logger.h>
#include <algorithm>
#include <Core/Input.h>
#include "Demo.h"
#include <Job/IJob.h>
#include <Job/Job.h>
#include <Graphics/Renderable/SpriteAtlas.h>
#include "Utilities.h"
#include <Containers/Table.h>


using namespace engine;
using namespace app;

#pragma region demo methods
std::vector<math::geometry::RectF> demo::CalcUV(int row, int col, int fileWidth, int fileHeight)
{
	std::vector<math::geometry::RectF> uvs;
	float width = static_cast<float>(fileWidth / col);
	float height = static_cast<float>(fileHeight / row);
	float left = 0;
	float top = 0;
	float right = left + width;
	float bottom = top + height;

	for (int r = 0; r < row; r++)
	{
		for (int c = 0; c < col; c++)
		{
			left = width * c;
			top = height * r;
			right = left + width;
			bottom = top + height;

			left /= fileWidth;
			top /= fileHeight;
			right /= fileWidth;
			bottom /= fileHeight;

			uvs.push_back(math::geometry::RectF{ left, top, right, bottom });
		}
	}
	return uvs;
}
#pragma endregion

#pragma region demo
demo::Demo::Demo(std::unique_ptr<state::State<Demo>> state) :
	m_engine("Test State Machine", "DirectX11", "Batch", 1000),
	m_stateMachine(this),
	m_state(std::move(state))
{
	// subscribe to start event of the engine. we do all initialization of our components here e.g. state machine
	m_engine.StartEvent += event::Handler(this, &Demo::OnStart);

	// subscribe our OnUpdate() to engine's scheduler. the scheduler runs on engine's main loop. 
	// the scheduler is updated by engine's main loop elapsed time per frame. 
	// the scheduler fires up event that is registered at specific interval.
	// we subscribe to be notified every x elapsed time so we can update ourselves at consistent frame rate
	// in our demo this is where we update our state machine
	m_engine.Scheduler() += timer::Schedule(1.0f / 6000.0, this, &Demo::OnUpdate, true, 1);

	// let the engine run!
	m_engine.Run();
}

demo::Demo::~Demo()
{
}

void demo::Demo::OnStart()
{
	// create font atlas for rendering text we will use fore demo
	m_fontAtlas = std::make_unique<graphics::renderable::FontAtlas>(std::make_unique<graphics::dx11::resource::DX11TextureImpl>());
	m_fontAtlas->Initialize("Terminal", 12);
	LOG("[Demo] Font atlas created and initialized...");

	// set initial state
	m_stateMachine.Set(std::move(m_state));
	LOG("[Demo] State machine set to LaunchState...");
}

void demo::Demo::OnUpdate(double delta)
{
	m_stateMachine.Update(delta);
}

void demo::Demo::SetState(std::unique_ptr<state::State<Demo>> state)
{
	m_stateMachine.Set(std::move(state));
}

void demo::Demo::QueueState(std::unique_ptr<state::State<Demo>> state)
{
	m_stateMachine.Queue(std::move(state));
}

bool demo::Demo::LoadMap(const std::string& filename)
{
	return false;
}

// helper function to draw text at top-right screen. this is for showing statistics like FPS
void demo::Demo::DrawTextCommandTopRightScreen(const std::string& text, float y)
{
	// render text showing which state are we in
	float width = m_fontAtlas->GetWidth(text);
	float height = m_fontAtlas->GetHeight();

	std::unique_ptr<engine::command::graphics::renderer::DrawTextCommand> drawTextCmd =
		std::make_unique<engine::command::graphics::renderer::DrawTextCommand>(
			Engine().Renderer(),
			*m_fontAtlas,
			text,
			spatial::PositionF
			{
				Engine().GetViewPort().GetWidth() - width - 10.0f,
				y
			},
			graphics::ColorF{ 1.0f, 1.0f, 1.0f, 1.0f }
		);
	Engine().CommandQueue().Enqueue(std::move(drawTextCmd));
}


void demo::Demo::DrawProgressBarCommand(spatial::PositionF pos, spatial::SizeF size, float current, float total)
{
	std::unique_ptr<engine::command::graphics::renderer::DrawQuadCommand> drawQuadCmd =
		std::make_unique<engine::command::graphics::renderer::DrawQuadCommand>(
			m_engine.Renderer(),
			pos,
			size,
			graphics::ColorF{ 1.0f, 0.0f, 0.0f, 1.0f },
			0.0f
		);
	m_engine.CommandQueue().Enqueue(std::move(drawQuadCmd));

	drawQuadCmd =
		std::make_unique<engine::command::graphics::renderer::DrawQuadCommand>(
			m_engine.Renderer(),
			pos,
			spatial::SizeF
			{
				size.width * current / total,
				size.height
			},
			graphics::ColorF{ 0.0f, 1.0f, 0.0f, 1.0f },
			0.0f
		);
	m_engine.CommandQueue().Enqueue(std::move(drawQuadCmd));
}

void demo::Demo::DrawTextCommand(const std::string& text, spatial::PositionF pos, graphics::ColorF color)
{
	// render text showing which state are we in
	float width = m_fontAtlas->GetWidth(text);
	float height = m_fontAtlas->GetHeight();

	std::unique_ptr<engine::command::graphics::renderer::DrawTextCommand> drawTextCmd =
		std::make_unique<engine::command::graphics::renderer::DrawTextCommand>(
			Engine().Renderer(),
			*m_fontAtlas,
			text,
			pos,
			color
		);
	Engine().CommandQueue().Enqueue(std::move(drawTextCmd));
}

void demo::Demo::DrawStatisticsCommand(const std::list<std::string> &logs)
{
	float tab = 15.0f;
	//for(const std::string& str : logs)
	//{
	//	DrawTextCommandTopRightScreen(str, tab);
	//	tab += 20;
	//}

	// get engine performance statistics
	engine::Engine::Statistics stats = m_engine.GetStatistics();

	std::list<std::string> statLogs;
	statLogs.insert(statLogs.end(), logs.begin(), logs.end());
	statLogs.push_back("Render FPS: " + std::to_string(static_cast<int>(stats.renderAverageFPS)));
	statLogs.push_back("Main Loop Ave FPS: " + std::to_string(static_cast<int>(stats.mainLoopAverageFPS)));
	statLogs.push_back("Main Loop Last FPS: " + std::to_string(static_cast<int>(stats.mainLoopLastFPS)));
		
	for (const std::string& str : statLogs)
	{
		DrawTextCommandTopRightScreen(str, tab);
		tab += 20;
	}
}

void demo::Demo::RenderTileGridCommand(component::tile::TileGrid<RenderableTile>& tilegrid, float alpha)
{
	std::unique_ptr<DrawTileGridCommand> cmd =
		std::make_unique<DrawTileGridCommand>(
			Engine().Renderer(),
			tilegrid,
			spatial::PositionF{50,50}
		);
	Engine().CommandQueue().Enqueue(std::move(cmd));

	return;

	//math::geometry::RectF vp = m_camera.GetViewport();
	//spatial::PositionF camPos = m_camera.GetPosition();

	//int left = (int)(camPos.x / m_tileSize.width);
	//int top = (int)(camPos.y / m_tileSize.height);
	//int right = (int)((camPos.x + vp.GetWidth()) / m_tileSize.width);
	//int bottom = (int)((camPos.y + vp.GetHeight()) / m_tileSize.height);

	spatial::SizeF tileSize{ 16.0f, 16.0f };	 

	for (int row = 0; row <= tilegrid.GetHeight(); ++row)
	{
		for (int col = 0; col <= tilegrid.GetWidth(); ++col)
		{
			if (!tilegrid.IsInBounds(row, col))
			{
				continue;
			}

			const component::tile::Tile<RenderableTile>& tile = tilegrid.Get(row, col);
			if (tile.isValid())
			{
				spatial::PositionF pos =
				{
					col * tileSize.width,
					row * tileSize.height
				};

				std::unique_ptr<engine::command::graphics::renderer::DrawRenderableCommand> cmd =
					std::make_unique<engine::command::graphics::renderer::DrawRenderableCommand>(
						Engine().Renderer(),
						tile->GetSprite(),						
						pos,
						tileSize,
						graphics::ColorF{ 1.0f, 1.0f, 1.0f, alpha },
						0.0f
					);
				Engine().CommandQueue().Enqueue(std::move(cmd));
			}
		}
	}
}

void demo::Demo::RenderTileRegionCommand(component::tile::TileRegion<RenderableTile>& region, float alpha)
{
	std::unique_ptr<DrawTileRegionCommand> cmd =
		std::make_unique<DrawTileRegionCommand>(
			Engine().Renderer(),
			region,
			spatial::PositionF{ 50,50 }
		);
	Engine().CommandQueue().Enqueue(std::move(cmd));
}

void demo::Demo::RenderTileLayerCommand(component::tile::TileLayer<RenderableTile>& layer, float alpha)
{
	std::unique_ptr<DrawTileLayerCommand> cmd =
		std::make_unique<DrawTileLayerCommand>(
			Engine().Renderer(),
			layer,
			spatial::PositionF{ 50,50 }
		);
	Engine().CommandQueue().Enqueue(std::move(cmd));
}

#pragma endregion

#pragma region LoadSequentialState 
demo::LoadSequentialState::LoadSequentialState() :
	m_isFinished(false),
	m_frameRateMonitor(1.0f),
	m_readSize(0)
{

}

demo::LoadSequentialState::~LoadSequentialState()
{
	LOG("[LoadSequentialState] destroyed");
}

void demo::LoadSequentialState::Enter(Demo& owner)
{
	// specify list of files to load and read
	m_files.push_back("small.csv");
	m_files.push_back("big.csv");
	m_files.push_back("huge.csv");

	// set starting read size
	m_readSize = 0;

	owner.QueueState(std::make_unique<LoadSimultaneousState>());
}

void demo::LoadSequentialState::Update(Demo& owner, double delta)
{
	// monitor frame rate
	m_frameRateMonitor.OnFrameCompleted(delta);

	// in this state, we want to keep loading/reading file data from our list of the files we specified until all of them are loaded/read
	if (m_fileReader.IsDone() ||	// file reader reached EOF of the current file it is reading
		!m_fileReader.IsOpen()			// file reader has yet to read any file from the list
		)
	{
		// if we still have files in our list to read, we fetch one of them and queue job to read them
		if (!m_files.empty())
		{
			// open first file available from the list and remove it from the list since it will now be processed
			m_fileReader.Open(m_files.front());
			LOG("Loading " << m_files.front());
			m_currFile = m_files.front();
			m_files.pop_front();

			// increment read size so succeeding files will be read faster
			m_readSize += 0x1FFF;

			// queue a job in the engine to read this file. 
			// it will be persistent so this job will be executed repeatedly until reader reach EOF
			// the beauty of using JobQueue is that even though the state requests this job, it is not going to do it.
			// the engine itself will do it so it will not affect the frame rate of this state.
			owner.Engine().SubmitJob(std::make_unique<engine::job::Job>(
				engine::job::Job(
				nullptr,
				[this]()
				{
					m_fileReader.Update(0.01);

					// delay per read to slow it down 
					timer::StopWatch sw;
					sw.Start();
					while (sw.Peek<timer::milliseconds>() < 1)
					{
						// busy wait
					}
					sw.Stop();
				},
				true,
				[this]()
				{
					return m_fileReader.IsDone();
				},
				[this]()
				{
					// if we done with reading this file and there are no more files on queue to read, we are done!
					if (m_files.empty())
					{
						// subscribe to input controller so we can accept mouse clicks to switch state
						input::Input::Instance().MouseDownEvent += event::Handler(this, &LoadSequentialState::OnMouseDown);
					}
				}
			)));
		}
	}

	// flush the draw commands on queue. we will queue new ones 
	owner.Engine().CommandQueue().Clear(engine::command::Type::Render);

	std::string loadMessage = m_files.empty() && m_fileReader.IsDone() ? "All files done loading. Click screen to switch to next demo.": "Loading  " + m_currFile + " ...";
	owner.DrawTextCommand(loadMessage, { 50, 260 }, { 1,1,1,1 });

	// let's draw progress bar to show how much file reader has read compared to total size of the file
	owner.DrawProgressBarCommand({ 50, 300 }, { 400, 40 }, static_cast<float>(m_fileReader.GetCurrent()), static_cast<float>(m_fileReader.GetTotal()));

	// get engine performance statistics
	engine::Engine::Statistics stats = owner.Engine().GetStatistics();

	// show FPS on top-right of screen
	owner.DrawTextCommandTopRightScreen("State: LoadSequentialState", 10.0f);
	std::string text = "State FPS: " + std::to_string(static_cast<int>(m_frameRateMonitor.GetAverageFrameRate()));
	owner.DrawTextCommandTopRightScreen(text, 40.0f);
	text = "Render FPS: " + std::to_string(static_cast<int>(stats.renderAverageFPS));
	owner.DrawTextCommandTopRightScreen(text, 70.0f);
	text = "Main Loop Ave FPS: " + std::to_string(static_cast<int>(stats.mainLoopAverageFPS));
	owner.DrawTextCommandTopRightScreen(text, 100.0f);
	text = "Main Loop Last FPS: " + std::to_string(static_cast<int>(stats.mainLoopLastFPS));
	owner.DrawTextCommandTopRightScreen(text, 130.0f);
}

void demo::LoadSequentialState::Exit(Demo& owner)
{
	input::Input::Instance().MouseDownEvent -= event::Handler(this, &LoadSequentialState::OnMouseDown);
}


bool demo::LoadSequentialState::IsFinished(Demo& owner)
{
	return m_isFinished;
}


void demo::LoadSequentialState::OnMouseDown(int btn, int x, int y)
{
	m_isFinished = true;
}

#pragma endregion

#pragma region LoadSimultaneousState
demo::LoadSimultaneousState::LoadSimultaneousState():
	m_isFinished(false),
	m_nFilesInProgress(0)
{
}

demo::LoadSimultaneousState::~LoadSimultaneousState()
{
	LOG("[LoadSimultaneousState] destroyed");
}

void demo::LoadSimultaneousState::Enter(Demo& owner)
{

	//in entry point, we will open all files for reading simultaneously. we also queue job to read them simultaneously as well
	m_nFilesInProgress = 3;
	int readSize = 0xFFF;
	m_fileReader0.Open("small.csv");
	LOG("Loading small.csv");
	owner.Engine().SubmitJob(std::make_unique<engine::job::Job>(
		engine::job::Job(
			nullptr,
			[this, readSize]()
			{
				m_fileReader0.Update(0.01);

				// delay per read to slow it down 
				timer::StopWatch sw;
				sw.Start();
				while (sw.Peek<timer::milliseconds>() < 0.2)
				{
					// busy wait
				}
				sw.Stop();
			},
			true,
			[this]()
			{
				return m_fileReader0.IsDone();
			},
			[this]()
			{
				m_nFilesInProgress--;
				if (!m_nFilesInProgress)
				{
					// subscribe to input controller so we can accept mouse clicks to switch state
					input::Input::Instance().MouseDownEvent += event::Handler(this, &LoadSimultaneousState::OnMouseDown);
				}
			}
		)));

	m_fileReader1.Open("big.csv");
	LOG("Loading big.csv");
	readSize += 0xFFF;
	owner.Engine().SubmitJob(std::make_unique<engine::job::Job>(
		engine::job::Job(
			nullptr,
			[this, readSize]()
			{
				m_fileReader1.Update(0.01);

				// delay per read to slow it down 
				timer::StopWatch sw;
				sw.Start();
				while (sw.Peek<timer::milliseconds>() < 0.2)
				{
					// busy wait
				}
				sw.Stop();
			},
			true,
			[this]()
			{
				return m_fileReader1.IsDone();
			},
			[this]()
			{
				m_nFilesInProgress--;
				if (!m_nFilesInProgress)
				{
					// subscribe to input controller so we can accept mouse clicks to switch state
					input::Input::Instance().MouseDownEvent += event::Handler(this, &LoadSimultaneousState::OnMouseDown);
				}
			}
		)));

	m_fileReader2.Open("huge.csv");
	LOG("Loading huge.csv");
	readSize += 0xFFF;
	owner.Engine().SubmitJob(std::make_unique<engine::job::Job>(
		engine::job::Job(
			nullptr,
			[this, readSize]()
			{
				m_fileReader2.Update(0.01);

				// delay per read to slow it down 
				timer::StopWatch sw;
				sw.Start();
				while (sw.Peek<timer::milliseconds>() < 0.2)
				{
					// busy wait
				}
				sw.Stop();
			},
			true,
			[this]()
			{
				return m_fileReader2.IsDone();
			},
			[this]()
			{
				m_nFilesInProgress--;
				if (!m_nFilesInProgress)
				{
					// subscribe to input controller so we can accept mouse clicks to switch state
					input::Input::Instance().MouseDownEvent += event::Handler(this, &LoadSimultaneousState::OnMouseDown);
				}
			}
		)));

	owner.QueueState(std::make_unique<LoadSequentialState>());
}

void demo::LoadSimultaneousState::Update(Demo& owner, double delta)
{
	// flush the draw commands on queue. we will queue new ones 
	owner.Engine().CommandQueue().Clear(engine::command::Type::Render);

	std::string loadMessage = m_nFilesInProgress ? "Loading  " + std::to_string(m_nFilesInProgress) + " file" + (m_nFilesInProgress > 1? "s":"") + "..." : "All files loaded.";
	owner.DrawTextCommand(loadMessage, { 50, 260 }, { 1,1,1,1 });

	// let's draw progress bar to show how much file reader has read compared to total size of the file
	owner.DrawProgressBarCommand({ 50, 300 }, { 400, 40 }, static_cast<float>(m_fileReader0.GetCurrent()), static_cast<float>(m_fileReader0.GetTotal()));
	owner.DrawProgressBarCommand({ 50, 360 }, { 400, 40 }, static_cast<float>(m_fileReader1.GetCurrent()), static_cast<float>(m_fileReader1.GetTotal()));
	owner.DrawProgressBarCommand({ 50, 420 }, { 400, 40 }, static_cast<float>(m_fileReader2.GetCurrent()), static_cast<float>(m_fileReader2.GetTotal()));

	// get engine performance statistics
	engine::Engine::Statistics stats = owner.Engine().GetStatistics();

	// show FPS on top-right of screen
	owner.DrawTextCommandTopRightScreen("State: LoadSimultaneousState", 10.0f);
	std::string text = "State FPS: " + std::to_string(static_cast<int>(m_frameRateMonitor.GetAverageFrameRate()));
	owner.DrawTextCommandTopRightScreen(text, 40.0f);
	text = "Render FPS: " + std::to_string(static_cast<int>(stats.renderAverageFPS));
	owner.DrawTextCommandTopRightScreen(text, 70.0f);
	text = "Main Loop Ave FPS: " + std::to_string(static_cast<int>(stats.mainLoopAverageFPS));
	owner.DrawTextCommandTopRightScreen(text, 100.0f);
	text = "Main Loop Last FPS: " + std::to_string(static_cast<int>(stats.mainLoopLastFPS));
	owner.DrawTextCommandTopRightScreen(text, 130.0f);
}

void demo::LoadSimultaneousState::Exit(Demo& owner)
{
	input::Input::Instance().MouseDownEvent -= event::Handler(this, &LoadSimultaneousState::OnMouseDown);
}

bool demo::LoadSimultaneousState::IsFinished(Demo& owner)
{
	return m_isFinished;
}

void demo::LoadSimultaneousState::OnMouseDown(int btn, int x, int y)
{
	m_isFinished = true;
}
#pragma endregion

#pragma region LoadTileMapState
demo::LoadTileMapState::LoadTileMapState():
	m_isFinished(false),
	m_csvTableLoaded(false),
	m_tileGridLoaded(false)
{
}

demo::LoadTileMapState::~LoadTileMapState()
{
}

void demo::LoadTileMapState::Enter(Demo& owner)
{
	// create sprite atlas to be used by tilemap
	m_spriteAtlas = std::make_unique<graphics::renderable::SpriteAtlas>(std::make_unique<graphics::dx11::resource::DX11TextureImpl>());

	// load sprite atlas from file manually for demo purpose
	m_spriteAtlas->Initialize(L"../Assets/4x1_128x32_tile.png");

	// load sprite atlas UVs from csv manually for demo purpose. we calculate UVs here by assuming a grid of 1 rows and 4 columns
	// in real scenario, you would use SpriteAtlasLoader to load from csv file 
	std::vector<math::geometry::RectF> uvs = app::utilities::graphics::CalcUV(1, 4, (int)m_spriteAtlas->GetWidth(), (int)m_spriteAtlas->GetHeight());
	for (math::geometry::RectF& rect : uvs)
	{
		m_spriteAtlas->AddUVRect(rect);
	}

	// create tileset for tilemap and register tiles
	m_tileset = std::make_unique<component::tile::Tileset<RenderableTile>>();
	m_tileset->Register(0, std::make_unique<RenderableTile>(m_spriteAtlas->MakeSprite(0), true)); // walkable
	m_tileset->Register(1, std::make_unique<RenderableTile>(m_spriteAtlas->MakeSprite(1), false)); // obstacle
	m_tileset->Register(2, std::make_unique<RenderableTile>(m_spriteAtlas->MakeSprite(2), false)); // obstacle
	m_tileset->Register(3, std::make_unique<RenderableTile>(m_spriteAtlas->MakeSprite(3), false)); // obstacle

	// specify the tilemap file to read. it must be csv file
	m_fileReader.Open("..\\Assets\\128x128Map.csv");
	LOG("Loading ..\\Assets\\128x128Map.csv");
	//m_fileReader.Open("..\\Assets\\tilemap.csv");
	//LOG("Loading ..\\Assets\\tilemap.csv");

	// chain our events where CSV parser listens to file reader when it extract chunk of data from file
	m_fileReader.ProcessChunkEvent += event::Handler(&m_csvParser, &engine::utilities::parser::CSVParser::ParseChunk);
	m_fileReader.EndOfFileFoundEvent += event::Handler(&m_csvParser, &engine::utilities::parser::CSVParser::ParseRemaining);

	// add wait time for this event
	//m_fileReader.ProcessChunkEvent += event::Handler(std::function<void(const char*, size_t)>(
	//	[](const char* data, size_t len)
	//	{
	//		// force 1ms delay here for simulation to slow down reading so we can observe
	//		timer::StopWatch sw;
	//		sw.Start();
	//		while (sw.Peek<timer::milliseconds>() < 0.1)
	//		{
	//			// busy wait
	//		}
	//		sw.Stop();
	//	}));

	// chain CSV table to CSV parser to acquire row of data from CSV Parser when it parse chunk of data and extracts rows of CSV data
	//m_csvParser.ParseRowEvent += event::Handler(&m_csvTable, &container::StringTable::AddRow);
	m_csvParser.ParseRowEvent += event::Handler(&m_table, &container::Table<std::string>::AddRow);
	m_csvParser.ParseRemainingEvent += event::Handler(&m_table, &container::Table<std::string>::AddRange);

	// create our tilegrid object
	m_tilegrid = std::make_unique <component::tile::TileGrid<RenderableTile>>();

	// queue job to read the file
	owner.Engine().SubmitJob(std::make_unique<engine::job::Job>(
		engine::job::Job(
			nullptr,
			[this]()
			{
				m_fileReader.Update(0.00000000001);
			},
			true,
			[this]()
			{
				return m_fileReader.IsDone();
			},
			[this, &owner]()
			{
				m_csvTableLoaded = true;


				m_tileGridLoader.Begin(
					"TileGrid",
					*m_tilegrid.get(),
					m_table,
					[this](const int& cell) -> component::tile::Tile<RenderableTile>
					{
						// this is safe. tileset will return "empty" tile if id is invalid. "empty" means does not have reference to tile data. tile is invalid
						return m_tileset->MakeTile(cell);
					});
				
				// in the job's done event, queue another job to create a tilemap object and load the csv data into it
				owner.Engine().SubmitJob(std::make_unique<engine::job::Job>(
					engine::job::Job(
						nullptr,
						[this]()
						{
							m_tileGridLoader.Update(0.001);

							// force 1ms delay here for simulation to slow down reading so we can observe
							timer::StopWatch sw;
							sw.Start();
							while (sw.Peek<timer::milliseconds>() < 0.1)
							{
								// busy wait
							}
							sw.Stop();
						},
						true,
						[this]()
						{
							return m_tileGridLoader.IsDone();
						},
						[this]()
						{
							m_tileGridLoaded = true;
						}
					)
				));
			}
		)));	
}

void demo::LoadTileMapState::Update(Demo& owner, double delta)
{
	// monitor frame rate
	m_frameRateMonitor.OnFrameCompleted(delta);

	// flush the draw commands on queue. we will queue new ones 
	owner.Engine().CommandQueue().Clear(engine::command::Type::Render);

	if (!m_tileGridLoaded)
	{
		if (m_csvTableLoaded)
		{
			std::string message = std::to_string(m_tileGridLoader.GetCurrent()) + "/" + std::to_string(m_tileGridLoader.GetTotal());
			owner.DrawTextCommand(message, { 50, 260 }, { 1,1,1,1 });

			// let's draw progress bar to show how much file reader has read compared to total size of the file
			owner.DrawProgressBarCommand(
				{ 50, 300 }, { 400, 40 }, 
				static_cast<float>(m_tileGridLoader.GetCurrent()), 
				static_cast<float>(m_tileGridLoader.GetTotal())
			);
		}
		else
		{
			std::string message = m_fileReader.IsDone() ? "" : "Reading data from file " + m_fileReader.GetLabel() + "...";
			owner.DrawTextCommand(message, { 50, 260 }, { 1,1,1,1 });

			// let's draw progress bar to show how much file reader has read compared to total size of the file
			owner.DrawProgressBarCommand({ 50, 300 }, { 400, 40 }, static_cast<float>(m_fileReader.GetCurrent()), static_cast<float>(m_fileReader.GetTotal()));
		}
	}
	else
	{
		owner.SetState(std::make_unique<RenderTileMapState>(std::move(m_tilegrid), std::move(m_spriteAtlas), std::move(m_tileset)));
	}

	// get engine performance statistics
	engine::Engine::Statistics stats = owner.Engine().GetStatistics();

	// show FPS on top-right of screen
	owner.DrawTextCommandTopRightScreen("State: LoadTileMapState", 10.0f);
	std::string text = "State FPS: " + std::to_string(static_cast<int>(m_frameRateMonitor.GetAverageFrameRate()));
	owner.DrawTextCommandTopRightScreen(text, 40.0f);
	text = "Render FPS: " + std::to_string(static_cast<int>(stats.renderAverageFPS));
	owner.DrawTextCommandTopRightScreen(text, 70.0f);
	text = "Main Loop Ave FPS: " + std::to_string(static_cast<int>(stats.mainLoopAverageFPS));
	owner.DrawTextCommandTopRightScreen(text, 100.0f);
	text = "Main Loop Last FPS: " + std::to_string(static_cast<int>(stats.mainLoopLastFPS));
	owner.DrawTextCommandTopRightScreen(text, 130.0f);
}

void demo::LoadTileMapState::Exit(Demo& owner)
{
	m_fileReader.ProcessChunkEvent -= event::Handler(&m_csvParser, &engine::utilities::parser::CSVParser::ParseChunk);
	m_fileReader.EndOfFileFoundEvent -= event::Handler(&m_csvParser, &engine::utilities::parser::CSVParser::ParseRemaining);

	m_csvParser.ParseRowEvent -= event::Handler(&m_table, &container::Table<std::string>::AddRow);
	m_csvParser.ParseRemainingEvent -= event::Handler(&m_table, &container::Table<std::string>::AddRange);
}

bool demo::LoadTileMapState::IsFinished(Demo& owner)
{
	return m_isFinished;
}

#pragma endregion

#pragma region RenderTileMapState
demo::RenderTileMapState::RenderTileMapState(
	std::unique_ptr<component::tile::TileGrid<RenderableTile>> tileGrid,
	std::unique_ptr<graphics::renderable::ISpriteAtlas> spriteAtlas,
	std::unique_ptr<component::tile::Tileset<RenderableTile>> tileSet
):
	m_tileGrid(std::move(tileGrid)),
	m_spriteAtlas(std::move(spriteAtlas)),
	m_tileSet(std::move(tileSet))
{
}

demo::RenderTileMapState::~RenderTileMapState()
{
}

void demo::RenderTileMapState::Enter(Demo& owner)
{
}

void demo::RenderTileMapState::Update(Demo& owner, double delta)
{
	// monitor frame rate
	m_frameRateMonitor.OnFrameCompleted(delta);

	// flush the draw commands on queue. we will queue new ones 
	owner.Engine().CommandQueue().Clear(engine::command::Type::Render);

	owner.RenderTileGridCommand(*m_tileGrid, 1.0f);

	// get engine performance statistics
	engine::Engine::Statistics stats = owner.Engine().GetStatistics();

	// show FPS on top-right of screen
	owner.DrawTextCommandTopRightScreen("State: RenderTileMapState", 10.0f);
	std::string text = "State FPS: " + std::to_string(static_cast<int>(m_frameRateMonitor.GetAverageFrameRate()));
	owner.DrawTextCommandTopRightScreen(text, 40.0f);
	text = "Render FPS: " + std::to_string(static_cast<int>(stats.renderAverageFPS));
	owner.DrawTextCommandTopRightScreen(text, 70.0f);
	text = "Main Loop Ave FPS: " + std::to_string(static_cast<int>(stats.mainLoopAverageFPS));
	owner.DrawTextCommandTopRightScreen(text, 100.0f);
	text = "Main Loop Last FPS: " + std::to_string(static_cast<int>(stats.mainLoopLastFPS));
	owner.DrawTextCommandTopRightScreen(text, 130.0f);
}

void demo::RenderTileMapState::Exit(Demo& owner)
{
}

bool demo::RenderTileMapState::IsFinished(Demo& owner)
{
	return false;
}

#pragma endregion

#pragma region LoadTileRegionState
demo::LoadTileRegionState::LoadTileRegionState() :
	m_isFinished(false),
	m_csvTableLoaded(false),
	m_tileRegionLoaded(false)
{
}

demo::LoadTileRegionState::~LoadTileRegionState()
{
}

void demo::LoadTileRegionState::Enter(Demo& owner)
{
	// create sprite atlas to be used by tilemap
	m_spriteAtlas = std::make_unique<graphics::renderable::SpriteAtlas>(std::make_unique<graphics::dx11::resource::DX11TextureImpl>());

	// load sprite atlas from file manually for demo purpose
	m_spriteAtlas->Initialize(L"../Assets/4x1_128x32_tile.png");

	// load sprite atlas UVs from csv manually for demo purpose. we calculate UVs here by assuming a grid of 1 rows and 4 columns
	// in real scenario, you would use SpriteAtlasLoader to load from csv file 
	std::vector<math::geometry::RectF> uvs = app::utilities::graphics::CalcUV(1, 4, (int)m_spriteAtlas->GetWidth(), (int)m_spriteAtlas->GetHeight());
	for (math::geometry::RectF& rect : uvs)
	{
		m_spriteAtlas->AddUVRect(rect);
	}

	// create tileset for tilemap and register tiles
	m_tileset = std::make_unique<component::tile::Tileset<RenderableTile>>();
	m_tileset->Register(0, std::make_unique<RenderableTile>(m_spriteAtlas->MakeSprite(0), true)); // walkable
	m_tileset->Register(1, std::make_unique<RenderableTile>(m_spriteAtlas->MakeSprite(1), false)); // obstacle
	m_tileset->Register(2, std::make_unique<RenderableTile>(m_spriteAtlas->MakeSprite(2), false)); // obstacle
	m_tileset->Register(3, std::make_unique<RenderableTile>(m_spriteAtlas->MakeSprite(3), false)); // obstacle

	// specify the tilemap file to read. it must be csv file
	//m_fileReader.Open("..\\Assets\\128x128Map.csv");
	//LOG("Loading ..\\Assets\\128x128Map.csv");
	m_fileReader.Open("..\\Assets\\69x71Map.csv");
	LOG("Loading ..\\Assets\\69x71Map.csv");

	m_fileReader.ProcessChunkEvent += event::Handler(&m_csvParser, &engine::utilities::parser::CSVParser::ParseChunk);
	m_fileReader.EndOfFileFoundEvent += event::Handler(&m_csvParser, &engine::utilities::parser::CSVParser::ParseRemaining);

	// chain CSV table to CSV parser to acquire row of data from CSV Parser when it parse chunk of data and extracts rows of CSV data
	m_csvParser.ParseRowEvent += event::Handler(&m_table, &container::Table<std::string>::AddRow);
	m_csvParser.ParseRemainingEvent += event::Handler(&m_table, &container::Table<std::string>::AddRange);


	// create our region object
	m_region = std::make_unique <component::tile::TileRegion<RenderableTile>>();

	// queue job to read the file
	owner.Engine().SubmitJob(std::make_unique<engine::job::Job>(
		engine::job::Job(
			nullptr,
			[this]()
			{
				m_fileReader.Update(0.01);
			},
			true,
			[this]()
			{
				return m_fileReader.IsDone();
			},
			[this, &owner]()
			{
				m_csvTableLoaded = true;

				m_tileRegionLoader.Begin(
					"TileRegion",
					*m_region.get(),
					m_table,
					[this](const int& cell) -> component::tile::Tile<RenderableTile>
					{
						// this is safe. tileset will return "empty" tile if id is invalid. "empty" means does not have reference to tile data. tile is invalid
						return m_tileset->MakeTile(cell);
					});

				// in the job's done event, queue another job to create a tilemap object and load the csv data into it
				owner.Engine().SubmitJob(std::make_unique<engine::job::Job>(
					engine::job::Job(
						nullptr,
						[this]()
						{
							m_tileRegionLoader.Update(0.01);

							// force 1ms delay here for simulation to slow down reading so we can observe
							timer::StopWatch sw;
							sw.Start();
							while (sw.Peek<timer::milliseconds>() < 0.1)
							{
								// busy wait
							}
							sw.Stop();
						},
						true,
						[this]()
						{
							return m_tileRegionLoader.IsDone();
						},
						[this]()
						{
							m_tileRegionLoaded = true;
						}
					)
				));
			}
		)));
}

void demo::LoadTileRegionState::Update(Demo& owner, double delta)
{
	// monitor frame rate
	m_frameRateMonitor.OnFrameCompleted(delta);

	// flush the draw commands on queue. we will queue new ones 
	owner.Engine().CommandQueue().Clear(engine::command::Type::Render);

	if (!m_tileRegionLoaded)
	{
		if (m_csvTableLoaded)
		{
			std::string message = std::to_string(m_tileRegionLoader.GetCurrent()) + "/" + std::to_string(m_tileRegionLoader.GetTotal());
			owner.DrawTextCommand(message, { 50, 260 }, { 1,1,1,1 });

			// let's draw progress bar to show how much file reader has read compared to total size of the file
			owner.DrawProgressBarCommand(
				{ 50, 300 }, { 400, 40 },
				static_cast<float>(m_tileRegionLoader.GetCurrent()),
				static_cast<float>(m_tileRegionLoader.GetTotal())
			);
		}
		else
		{
			std::string message = m_fileReader.IsDone() ? "" : "Reading data from file " + m_fileReader.GetLabel() + "...";
			owner.DrawTextCommand(message, { 50, 260 }, { 1,1,1,1 });

			// let's draw progress bar to show how much file reader has read compared to total size of the file
			owner.DrawProgressBarCommand({ 50, 300 }, { 400, 40 }, static_cast<float>(m_fileReader.GetCurrent()), static_cast<float>(m_fileReader.GetTotal()));
		}
	}
	else
	{
		owner.SetState(std::make_unique<RenderTileRegionState>(std::move(m_region), std::move(m_spriteAtlas), std::move(m_tileset)));
	}

	// get engine performance statistics
	engine::Engine::Statistics stats = owner.Engine().GetStatistics();

	// show FPS on top-right of screen
	owner.DrawTextCommandTopRightScreen("State: LoadTileRegionState", 10.0f);
	std::string text = "State FPS: " + std::to_string(static_cast<int>(m_frameRateMonitor.GetAverageFrameRate()));
	owner.DrawTextCommandTopRightScreen(text, 40.0f);
	text = "Render FPS: " + std::to_string(static_cast<int>(stats.renderAverageFPS));
	owner.DrawTextCommandTopRightScreen(text, 70.0f);
	text = "Main Loop Ave FPS: " + std::to_string(static_cast<int>(stats.mainLoopAverageFPS));
	owner.DrawTextCommandTopRightScreen(text, 100.0f);
	text = "Main Loop Last FPS: " + std::to_string(static_cast<int>(stats.mainLoopLastFPS));
	owner.DrawTextCommandTopRightScreen(text, 130.0f);
}

void demo::LoadTileRegionState::Exit(Demo& owner)
{
	m_fileReader.ProcessChunkEvent -= event::Handler(&m_csvParser, &engine::utilities::parser::CSVParser::ParseChunk);
	m_fileReader.EndOfFileFoundEvent -= event::Handler(&m_csvParser, &engine::utilities::parser::CSVParser::ParseRemaining);

	m_csvParser.ParseRowEvent -= event::Handler(&m_table, &container::Table<std::string>::AddRow);
	m_csvParser.ParseRemainingEvent -= event::Handler(&m_table, &container::Table<std::string>::AddRange);
}

bool demo::LoadTileRegionState::IsFinished(Demo& owner)
{
	return m_isFinished;
}

#pragma endregion

#pragma region RenderTileRegionState
demo::RenderTileRegionState::RenderTileRegionState(
	std::unique_ptr<component::tile::TileRegion<RenderableTile>> region,
	std::unique_ptr<graphics::renderable::ISpriteAtlas> spriteAtlas,
	std::unique_ptr<component::tile::Tileset<RenderableTile>> tileSet
) :
	m_region(std::move(region)),
	m_spriteAtlas(std::move(spriteAtlas)),
	m_tileSet(std::move(tileSet))
{
}

demo::RenderTileRegionState::~RenderTileRegionState()
{
}

void demo::RenderTileRegionState::Enter(Demo& owner)
{
}

void demo::RenderTileRegionState::Update(Demo& owner, double delta)
{
	// monitor frame rate
	m_frameRateMonitor.OnFrameCompleted(delta);

	// flush the draw commands on queue. we will queue new ones 
	owner.Engine().CommandQueue().Clear(engine::command::Type::Render);

	owner.RenderTileRegionCommand(*m_region.get(), 1.0f);

	// get engine performance statistics
	engine::Engine::Statistics stats = owner.Engine().GetStatistics();

	// show FPS on top-right of screen
	owner.DrawTextCommandTopRightScreen("State: RenderTileRegionState", 10.0f);
	std::string text = "State FPS: " + std::to_string(static_cast<int>(m_frameRateMonitor.GetAverageFrameRate()));
	owner.DrawTextCommandTopRightScreen(text, 40.0f);
	text = "Render FPS: " + std::to_string(static_cast<int>(stats.renderAverageFPS));
	owner.DrawTextCommandTopRightScreen(text, 70.0f);
	text = "Main Loop Ave FPS: " + std::to_string(static_cast<int>(stats.mainLoopAverageFPS));
	owner.DrawTextCommandTopRightScreen(text, 100.0f);
	text = "Main Loop Last FPS: " + std::to_string(static_cast<int>(stats.mainLoopLastFPS));
	owner.DrawTextCommandTopRightScreen(text, 130.0f);
}

void demo::RenderTileRegionState::Exit(Demo& owner)
{
}

bool demo::RenderTileRegionState::IsFinished(Demo& owner)
{
	return false;
}

#pragma endregion


