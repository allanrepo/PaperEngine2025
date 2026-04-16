#include <Graphics/Resource/FontAtlas.h>
#include <Graphics/Resource/DX11TextureImpl.h>
#include <Utilities/Logger.h>
#include <algorithm>
#include <Core/Input.h>
#include "Demo.h"
#include <Job/IJob.h>
#include <Job/Job.h>
#include <Graphics/Resource/SpriteAtlas.h>
#include <Containers/Table.h>
#include <Engine/Factory/SpriteAtlasFactory.h>
#include <Cache/Registry.h>

#pragma region demo

std::vector<engine::math::geometry::RectF> demo::CalcUV(int row, int col, int fileWidth, int fileHeight)
{
	std::vector<engine::math::geometry::RectF> uvs;
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

			uvs.push_back(engine::math::geometry::RectF{ left, top, right, bottom });
		}
	}
	return uvs;
}

demo::Demo::Demo(std::unique_ptr<engine::state::State<Demo>> state) :
	m_engine("Test State Machine", "DirectX11", "Batch", 1000),
	m_stateMachine(this),
	m_state(std::move(state))
{
	// subscribe to start event of the engine. we do all initialization of our components here e.g. state machine
	m_engine.StartEvent += engine::event::Handler(this, &Demo::OnStart);

	// subscribe our OnUpdate() to engine's scheduler. the scheduler runs on engine's main loop. 
	// the scheduler is updated by engine's main loop elapsed time per frame. 
	// the scheduler fires up event that is registered at specific interval.
	// we subscribe to be notified every x elapsed time so we can update ourselves at consistent frame rate
	// in our demo this is where we update our state machine
	m_engine.Scheduler() += engine::timer::Schedule(1.0f / 100.0f, this, &Demo::OnUpdate, true, 1);

	// let the engine run!
	m_engine.Run();
}

demo::Demo::~Demo()
{
}

void demo::Demo::OnStart()
{
	// create font atlas for rendering text we will use fore demo
	m_fontAtlas = std::make_unique<engine::graphics::resource::FontAtlas>(std::make_unique<engine::graphics::resource::SpriteAtlas>(std::make_unique<engine::graphics::dx11::resource::DX11TextureImpl>()));
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

void demo::Demo::SetState(std::unique_ptr<engine::state::State<Demo>> state)
{
	m_stateMachine.Set(std::move(state));
}

void demo::Demo::QueueState(std::unique_ptr<engine::state::State<Demo>> state)
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
			engine::spatial::PositionF
			{
				Engine().GetViewPort().GetWidth() - width - 10.0f,
				y
			},
			engine::graphics::ColorF{ 1.0f, 1.0f, 1.0f, 1.0f }
		);
	Engine().CommandQueue().Enqueue(std::move(drawTextCmd));
}


void demo::Demo::DrawProgressBarCommand(engine::spatial::PositionF pos, engine::spatial::SizeF size, float current, float total)
{
	std::unique_ptr<engine::command::graphics::renderer::DrawQuadCommand> drawQuadCmd =
		std::make_unique<engine::command::graphics::renderer::DrawQuadCommand>(
			m_engine.Renderer(),
			pos,
			size,
			engine::graphics::ColorF{ 1.0f, 0.0f, 0.0f, 1.0f },
			0.0f
		);
	m_engine.CommandQueue().Enqueue(std::move(drawQuadCmd));

	drawQuadCmd =
		std::make_unique<engine::command::graphics::renderer::DrawQuadCommand>(
			m_engine.Renderer(),
			pos,
			engine::spatial::SizeF
			{
				size.width * current / total,
				size.height
			},
			engine::graphics::ColorF{ 0.0f, 1.0f, 0.0f, 1.0f },
			0.0f
		);
	m_engine.CommandQueue().Enqueue(std::move(drawQuadCmd));
}

void demo::Demo::DrawTextCommand(const std::string& text, engine::spatial::PositionF pos, engine::graphics::ColorF color)
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

void demo::Demo::RenderTileGridCommand(engine::component::tile1::TileGrid<RenderableTile>& tilegrid, float alpha)
{
	std::unique_ptr<DrawTileGridCommand> cmd =
		std::make_unique<DrawTileGridCommand>(
			Engine().Renderer(),
			tilegrid,
			engine::spatial::PositionF{50,50}
		);
	Engine().CommandQueue().Enqueue(std::move(cmd));

	return;

	//math::geometry::RectF vp = m_camera.GetViewport();
	//spatial::PositionF camPos = m_camera.GetPosition();

	//int left = (int)(camPos.x / m_tileSize.width);
	//int top = (int)(camPos.y / m_tileSize.height);
	//int right = (int)((camPos.x + vp.GetWidth()) / m_tileSize.width);
	//int bottom = (int)((camPos.y + vp.GetHeight()) / m_tileSize.height);

	engine::spatial::SizeF tileSize{ 16.0f, 16.0f };

	for (int row = 0; row <= tilegrid.GetHeight(); ++row)
	{
		for (int col = 0; col <= tilegrid.GetWidth(); ++col)
		{
			if (!tilegrid.IsInBounds(row, col))
			{
				continue;
			}

			const engine::component::tile1::Tile<RenderableTile>& tile = tilegrid.Get(row, col);
			if (tile.IsValid())
			{
				engine::spatial::PositionF pos =
				{
					col * tileSize.width,
					row * tileSize.height
				};

				std::unique_ptr<engine::command::graphics::renderer::DrawCommand> cmd =
					std::make_unique<engine::command::graphics::renderer::DrawCommand>(
						Engine().Renderer(),
						tile->GetSprite(),						
						pos,
						tileSize,
						engine::graphics::ColorF{ 1.0f, 1.0f, 1.0f, alpha },
						0.0f
					);
				Engine().CommandQueue().Enqueue(std::move(cmd));
			}
		}
	}
}

void demo::Demo::RenderTileRegionCommand(engine::component::tile1::TileRegion<RenderableTile>& region, float alpha)
{
	std::unique_ptr<DrawTileRegionCommand> cmd =
		std::make_unique<DrawTileRegionCommand>(
			Engine().Renderer(),
			region,
			engine::spatial::PositionF{ 50,50 }
		);
	Engine().CommandQueue().Enqueue(std::move(cmd));
}

void demo::Demo::RenderTileLayerCommand(engine::component::tile1::TileLayer<RenderableTile>& layer, float alpha)
{
	std::unique_ptr<DrawTileLayerCommand> cmd =
		std::make_unique<DrawTileLayerCommand>(
			Engine().Renderer(),
			layer,
			engine::spatial::PositionF{ 50,50 }
		);
	Engine().CommandQueue().Enqueue(std::move(cmd));
}

#pragma endregion

#pragma region DemoState 
demo::DemoState::DemoState() :
	m_isFinished(false),
	m_frameRateMonitor(1.0f)
{
	LOG("[DemoState] created");
}

demo::DemoState::~DemoState()
{
	LOG("[DemoState] destroyed");
}

void demo::DemoState::Enter(Demo& owner)
{
	// create map tileset and load tiles into it
	{
		// create sprite atlas for tilemap (grass, wall, etc...)
		engine::graphics::factory::SpriteAtlasFactory::Create("576x384TileSet", L"../Assets/576x384px_6x9tile_TileMap.png", 6, 9);
		engine::graphics::resource::ISpriteAtlas& atlas = engine::cache::Registry<engine::graphics::resource::ISpriteAtlas>::Instance().Get("576x384TileSet");

		// create animated tileset for tilemap (grass, wall, etc...)
		owner.TileSetManager().Create("576x384TileSet");

		// each sprite from atlas is a static tile (single frame), so we create tile from each sprite
		for (int i = 0; i < atlas.GetUVRectCount(); i++)
		{
			// create animation. these tilemaps are static. so their animations are 1 frame only
			engine::graphics::animation::Animation<engine::graphics::Sprite> anim = engine::graphics::factory::AnimationFactory::Create(atlas, { i }, 0.1f, true);

			// when instancing AnimatedTile, it sets the animation passed in constructor as current animation
			owner.TileSetManager().Register("576x384TileSet", i, std::make_unique<AnimatedTile>(true, "default", anim));
		}
	}

	// create water splash tile animation
	{
		// create sprite atlas for the water splash animation
		engine::graphics::factory::SpriteAtlasFactory::Create("3072x192TileSet", L"../Assets/3072x192px_1x17tile_waterfoam.png", 1, 16);
		engine::graphics::resource::ISpriteAtlas& atlas = engine::cache::Registry<engine::graphics::resource::ISpriteAtlas>::Instance().Get("3072x192TileSet");

		// load all sprite atlas's sprites into animation object
		engine::graphics::animation::Animation<engine::graphics::Sprite> anim = engine::graphics::factory::AnimationFactory::Create(atlas, 0.1f, true);

		// create tileset
		owner.TileSetManager().Create("splashTileset");

		// add our splash animation to tile index 1 and set it as default animation
		owner.TileSetManager().Register("splashTileset", 1, std::make_unique<AnimatedTile>(true, "splash", anim));
	}

	// create tilemap with animated tiles
	{
		owner.TileMapManager().LoadImmediate(
			"map_splashAnim",
			"..\\Assets\\16x16_2.csv",
			[&owner](const int& cell) -> engine::component::tile1::Tile<AnimatedTile>
			{
				return owner.TileSetManager().MakeTile("splashTileset", cell);
			});

		owner.TileMapManager().LoadImmediate(
			"demoTileMap",
			"..\\Assets\\16x16Map.csv",
			[&owner](const int& cell) -> engine::component::tile1::Tile<AnimatedTile>
			{
				return owner.TileSetManager().MakeTile("576x384TileSet", cell);
			});

		owner.TileMapManager().LoadImmediate(
			"map_1",
			"..\\Assets\\16x16_1.csv",
			[&owner](const int& cell) -> engine::component::tile1::Tile<AnimatedTile>
			{
				return owner.TileSetManager().MakeTile("576x384TileSet", cell);
			});
	}
}


void demo::DemoState::Update(Demo& owner, double delta)
{
	// monitor frame rate
	m_frameRateMonitor.OnFrameCompleted(delta);
	
	// update animations of tiles
	owner.TileSetManager().Update(delta);

	// flush the draw commands on queue. we will queue new ones 
	owner.Engine().CommandQueue().Clear(engine::command::Type::Render);


	//engine::graphics::resource::ISpriteAtlas& atlas = cache::Registry<engine::graphics::resource::ISpriteAtlas>::Instance().Get("576x384TileSet");
	//{
	//	std::unique_ptr<engine::command::graphics::renderer::DrawCommand> DrawCmd =
	//		std::make_unique<engine::command::graphics::renderer::DrawCommand>(
	//			owner.Engine().Renderer(),
	//			atlas,
	//			spatial::PositionF{ 50.0f, 50.0f },
	//			spatial::SizeF{ 576.0f,384.0f },
	//			::graphics::ColorF{ 1.0f,1.0f,1.0f,1.0f },
	//			0.0f
	//		);
	//	owner.Engine().CommandQueue().Enqueue(std::move(DrawCmd));
	//}


	// draw the tilemap via command
	owner.RenderTileMapCommand<AnimatedTile>(owner.TileMapManager().GetTileMap("map_splashAnim"), { 50, 50 }, { 32, 32 }, { -32,-34 }, { 3,3 }, 1.0f);
	owner.RenderTileMapCommand<AnimatedTile>(owner.TileMapManager().GetTileMap("demoTileMap"), { 50, 50 }, { 32, 32 }, { 0,0 }, { 1,1 }, 1.0f);
	owner.RenderTileMapCommand<AnimatedTile>(owner.TileMapManager().GetTileMap("map_1"), { 50, 50 }, { 32, 32 }, { 0,0 }, { 1,1 }, 1.0f);

	// render statistics
	std::list<std::string> logs;
	logs.push_back("State: DemoState");
	logs.push_back("State FPS: " + std::to_string(static_cast<int>(m_frameRateMonitor.GetAverageFrameRate())));
	owner.DrawStatisticsCommand(logs);
}

void demo::DemoState::Exit(Demo& owner)
{
	engine::input::Input::Instance().MouseDownEvent -= engine::event::Handler(this, &DemoState::OnMouseDown);
}


bool demo::DemoState::IsFinished(Demo& owner)
{
	return m_isFinished;
}


void demo::DemoState::OnMouseDown(int btn, int x, int y)
{
	m_isFinished = true;
}

#pragma endregion

#pragma region DemoStateCameraMap 
demo::DemoStateCameraMap::DemoStateCameraMap() :
	m_isFinished(false),
	m_frameRateMonitor(1.0f),
	m_camera({ 100, 100, 800, 600 }),
	m_isPanning(false),
	m_focusPos({}),
	m_lastMousePos({}),
	m_tileSize({ 64, 64 })
{
	LOG("[DemoStateCameraMap] created");
}

demo::DemoStateCameraMap::~DemoStateCameraMap()
{
	LOG("[DemoStateCameraMap] destroyed");
}

void  demo::DemoStateCameraMap::OnMouseMove(int x, int y)
{
	// is we're holding down left mouse button and dragging it, pan the map
	if (m_isPanning)
	{
		// get the change in position and move camera position by that
		engine::math::VecF delta = engine::math::VecF((float)x, (float)y) - m_lastMousePos;
		m_camera.MoveBy(delta);

		// remember the last mouse position
		m_lastMousePos = { (float)x, (float)y };
	}
}

void  demo::DemoStateCameraMap::OnMouseDown(int btn, int x, int y)
{
	// this button is for panning the camera
	if (btn == 1)
	{
		m_isPanning = true;
		m_lastMousePos = { (float)x, (float)y };
	}
	// if this button is clicked, move our focus object in this position
	if (btn == 2)
	{
		// this is screen position and convert it to world position
		engine::spatial::PositionF pos((float)x, (float)y);
		pos = m_camera.ScreenToWorld(pos);
		m_focusPos = pos;

		// pan the camera such that the focus object is at center of the viewport, if possible
		m_camera.CenterOn(m_focusPos);
	}
}

void  demo::DemoStateCameraMap::OnMouseUp(int btn, int x, int y)
{
	m_isPanning = false;
}

void demo::DemoStateCameraMap::Enter(Demo& owner)
{
	// subscribe to mouse input for camera control
	{
		engine::input::Input::Instance().MouseDownEvent += engine::event::Handler(this, &DemoStateCameraMap::OnMouseDown);
		engine::input::Input::Instance().MouseMoveEvent += engine::event::Handler(this, &DemoStateCameraMap::OnMouseMove);
		engine::input::Input::Instance().MouseUpEvent += engine::event::Handler(this, &DemoStateCameraMap::OnMouseUp);
	}

	// create map tileset and load tiles into it
	{
		// create sprite atlas for tilemap (grass, wall, etc...)
		engine::graphics::factory::SpriteAtlasFactory::Create("576x384TileSet", L"../Assets/576x384px_6x9tile_TileMap.png", 6, 9);
		engine::graphics::resource::ISpriteAtlas& atlas = engine::cache::Registry<engine::graphics::resource::ISpriteAtlas>::Instance().Get("576x384TileSet");

		// create animated tileset for tilemap (grass, wall, etc...)
		owner.TileSetManager().Create("576x384TileSet");

		// each sprite from atlas is a static tile (single frame), so we create tile from each sprite
		for (int i = 0; i < atlas.GetUVRectCount(); i++)
		{
			// create animation. these tilemaps are static. so their animations are 1 frame only
			engine::graphics::animation::Animation<engine::graphics::Sprite> anim = engine::graphics::factory::AnimationFactory::Create(atlas, { i }, 0.1f, true);

			// when instancing AnimatedTile, it sets the animation passed in constructor as current animation
			owner.TileSetManager().Register("576x384TileSet", i, std::make_unique<AnimatedTile>(true, "default", anim));
		}
	}

	// create water splash tile animation
	{
		// create sprite atlas for the water splash animation
		engine::graphics::factory::SpriteAtlasFactory::Create("3072x192TileSet", L"../Assets/3072x192px_1x17tile_waterfoam.png", 1, 16);
		engine::graphics::resource::ISpriteAtlas& atlas = engine::cache::Registry<engine::graphics::resource::ISpriteAtlas>::Instance().Get("3072x192TileSet");

		// load all sprite atlas's sprites into animation object
		engine::graphics::animation::Animation<engine::graphics::Sprite> anim = engine::graphics::factory::AnimationFactory::Create(atlas, 0.1f, true);

		// create tileset
		owner.TileSetManager().Create("splashTileset");

		// add our splash animation to tile index 1 and set it as default animation
		owner.TileSetManager().Register("splashTileset", 1, std::make_unique<AnimatedTile>(true, "splash", anim));
	}

	// create tilemap with animated tiles
	{
		owner.TileMapManager().LoadImmediate(
			"map_splashAnim",
			"..\\Assets\\16x16_2.csv",
			[&owner](const int& cell) -> engine::component::tile1::Tile<AnimatedTile>
			{
				return owner.TileSetManager().MakeTile("splashTileset", cell);
			});

		owner.TileMapManager().LoadImmediate(
			"demoTileMap",
			"..\\Assets\\16x16Map.csv",
			[&owner](const int& cell) -> engine::component::tile1::Tile<AnimatedTile>
			{
				return owner.TileSetManager().MakeTile("576x384TileSet", cell);
			});

		owner.TileMapManager().LoadImmediate(
			"map_1",
			"..\\Assets\\16x16_1.csv",
			[&owner](const int& cell) -> engine::component::tile1::Tile<AnimatedTile>
			{
				return owner.TileSetManager().MakeTile("576x384TileSet", cell);
			});
	}

	// configure camera 
	{
		// tell camera the size of the world. this will be the tile map
		m_camera.SetWorldSize(
			owner.TileMapManager().GetTileMap("demoTileMap").GetWidth() * m_tileSize.width,
			owner.TileMapManager().GetTileMap("demoTileMap").GetHeight() * m_tileSize.height
		);
	}
}


void demo::DemoStateCameraMap::Update(Demo& owner, double delta)
{
	// monitor frame rate
	m_frameRateMonitor.OnFrameCompleted(delta);

	// update input
	engine::input::Input::Instance().Update();

	// update animations of tiles
	owner.TileSetManager().Update(delta);

	// flush the draw commands on queue. we will queue new ones 
	owner.Engine().CommandQueue().Clear(engine::command::Type::Render);

	// set camera viewport as clip region.
	owner.Engine().QueueEnableClipRegionCommand(m_camera.GetViewport());

	// draw dark background in viewport so we know the boundaries of viewport
	engine::math::geometry::RectF vp = m_camera.GetViewport();
	owner.Engine().QueueDrawQuadCommand(vp.GetTopLeft(), vp.GetSize(), engine::graphics::ColorF{ 0.2f,0.2f,0.2f,1 }, 0.0f);

	// this is the position of map's top-left in the world.
	engine::spatial::PositionF pos = {0,0};

	// draw the tilemap via command
	owner.RenderTileMapOnViewPortCommand<AnimatedTile>(owner.TileMapManager().GetTileMap("map_splashAnim"), m_camera, pos, m_tileSize, { -64,-68 }, { 3,3 }, 1.0f);
	owner.RenderTileMapOnViewPortCommand<AnimatedTile>(owner.TileMapManager().GetTileMap("demoTileMap"), m_camera, pos, m_tileSize, { 0,0 }, { 1,1 }, 1.0f);
	owner.RenderTileMapOnViewPortCommand<AnimatedTile>(owner.TileMapManager().GetTileMap("map_1"), m_camera, pos, m_tileSize, { 0,0 }, { 1,1 }, 1.0f);

	// disable clip region so we can render anywhere again
	owner.Engine().QueueDisableClipRegionCommand();

	// render statistics
	std::list<std::string> logs;
	logs.push_back("State: DemoStateCameraMap");
	logs.push_back("State FPS: " + std::to_string(static_cast<int>(m_frameRateMonitor.GetAverageFrameRate())));
	owner.DrawStatisticsCommand(logs);

}

void demo::DemoStateCameraMap::Exit(Demo& owner)
{
	{
		engine::input::Input::Instance().MouseDownEvent -= engine::event::Handler(this, &DemoStateCameraMap::OnMouseDown);
		engine::input::Input::Instance().MouseMoveEvent -= engine::event::Handler(this, &DemoStateCameraMap::OnMouseMove);
		engine::input::Input::Instance().MouseUpEvent -= engine::event::Handler(this, &DemoStateCameraMap::OnMouseUp);
	}
}


bool demo::DemoStateCameraMap::IsFinished(Demo& owner)
{
	return m_isFinished;
}

#pragma endregion

#pragma region DemoStatePathFinding 
demo::DemoStatePathFinding::DemoStatePathFinding() :
	m_isFinished(false),
	m_frameRateMonitor(1.0f)
{
	LOG("[DemoStatePathFinding] created");
}

demo::DemoStatePathFinding::~DemoStatePathFinding()
{
	LOG("[DemoStatePathFinding] destroyed");
}

void demo::DemoStatePathFinding::Enter(Demo& owner)
{
}


void demo::DemoStatePathFinding::Update(Demo& owner, double delta)
{
	// monitor frame rate
	m_frameRateMonitor.OnFrameCompleted(delta);

	// update animations of tiles
	owner.TileSetManager().Update(delta);

	// flush the draw commands on queue. we will queue new ones 
	owner.Engine().CommandQueue().Clear(engine::command::Type::Render);

	// render statistics
	std::list<std::string> logs;
	logs.push_back("State: DemoStatePathFinding");
	logs.push_back("State FPS: " + std::to_string(static_cast<int>(m_frameRateMonitor.GetAverageFrameRate())));
	owner.DrawStatisticsCommand(logs);
}

void demo::DemoStatePathFinding::Exit(Demo& owner)
{
	engine::input::Input::Instance().MouseDownEvent -= engine::event::Handler(this, &DemoStatePathFinding::OnMouseDown);
}


bool demo::DemoStatePathFinding::IsFinished(Demo& owner)
{
	return m_isFinished;
}


void demo::DemoStatePathFinding::OnMouseDown(int btn, int x, int y)
{
	m_isFinished = true;
}

#pragma endregion


#pragma region DemoStateActor 
demo::DemoStateActor::DemoStateActor() :
	m_isFinished(false),
	m_frameRateMonitor(1.0f),
	m_camera({ 100, 100, 800, 600 }),
	m_isPanning(false),
	m_focusPos({}),
	m_lastMousePos({}),
	m_tileSize({ 64, 64 })
{
	LOG("[DemoStateActor] created");
}

demo::DemoStateActor::~DemoStateActor()
{
	LOG("[DemoStateActor] destroyed");
}

void  demo::DemoStateActor::OnMouseMove(int x, int y)
{
	// is we're holding down left mouse button and dragging it, pan the map
	if (m_isPanning)
	{
		// get the change in position and move camera position by that
		engine::math::VecF delta = engine::math::VecF((float)x, (float)y) - m_lastMousePos;
		m_camera.MoveBy(delta);

		// remember the last mouse position
		m_lastMousePos = { (float)x, (float)y };
	}
}

void  demo::DemoStateActor::OnMouseDown(int btn, int x, int y)
{
	// this button is for panning the camera
	if (btn == 1)
	{
		m_isPanning = true;
		m_lastMousePos = { (float)x, (float)y };
	}
	// if this button is clicked, move our focus object in this position
	if (btn == 2)
	{
		// this is screen position and convert it to world position
		engine::spatial::PositionF pos((float)x, (float)y);
		pos = m_camera.ScreenToWorld(pos);
		m_focusPos = pos;

		// pan the camera such that the focus object is at center of the viewport, if possible
		m_camera.CenterOn(m_focusPos);
	}
}

void  demo::DemoStateActor::OnMouseUp(int btn, int x, int y)
{
	m_isPanning = false;
}

void demo::DemoStateActor::Enter(Demo& owner)
{
	// subscribe to mouse input for camera control
	{
		engine::input::Input::Instance().MouseDownEvent += engine::event::Handler(this, &DemoStateActor::OnMouseDown);
		engine::input::Input::Instance().MouseMoveEvent += engine::event::Handler(this, &DemoStateActor::OnMouseMove);
		engine::input::Input::Instance().MouseUpEvent += engine::event::Handler(this, &DemoStateActor::OnMouseUp);
	}

	{
		// create sprite atlas for tilemap (grass, wall, etc...)
		engine::graphics::factory::SpriteAtlasFactory::Create("Character", L"../Assets/CharacterTest_2304x1536_12x8.png", 8, 12);
		engine::graphics::resource::ISpriteAtlas& atlas = engine::cache::Registry<engine::graphics::resource::ISpriteAtlas>::Instance().Get("Character");

		// create our animator
		engine::cache::Registry<Animator>::Instance().Register("Character", std::make_unique<Animator>());
		Animator& animator = engine::cache::Registry<Animator>::Instance().Get("Character");

		// create our animation
		
		engine::cache::Registry<Animation>::Instance().Register("idle right face", std::make_unique<Animation>(AnimationFactory::Create(atlas, { 0, 1, 2, 3, 4, 5 }, 0.1f, true, PositionF{ 0.5f, 0.5f })));
		animator.Play(engine::cache::Registry<Animation>::Instance().Get("idle right face"));
	}
}


void demo::DemoStateActor::Update(Demo& owner, double delta)
{
	// monitor frame rate
	m_frameRateMonitor.OnFrameCompleted(delta);

	// update input
	engine::input::Input::Instance().Update();

	// update animator
	Animator& animator = engine::cache::Registry<Animator>::Instance().Get("Character");
	animator.Update(delta);

	// flush the draw commands on queue. we will queue new ones 
	owner.Engine().CommandQueue().Clear(engine::command::Type::Render);

	owner.Engine().QueueDrawSpriteCommand(animator.GetCurrent(), engine::spatial::PositionF{ 250.0f, 250.0f }, animator.GetCurrent().GetSize(), engine::graphics::ColorF{ 1.0f,1.0f,1.0f,1.0f }, 0.0f);

	// render statistics
	std::list<std::string> logs;
	logs.push_back("State: DemoStateActor");
	logs.push_back("State FPS: " + std::to_string(static_cast<int>(m_frameRateMonitor.GetAverageFrameRate())));
	owner.DrawStatisticsCommand(logs);

}

void demo::DemoStateActor::Exit(Demo& owner)
{
	{
		engine::input::Input::Instance().MouseDownEvent -= engine::event::Handler(this, &DemoStateActor::OnMouseDown);
		engine::input::Input::Instance().MouseMoveEvent -= engine::event::Handler(this, &DemoStateActor::OnMouseMove);
		engine::input::Input::Instance().MouseUpEvent -= engine::event::Handler(this, &DemoStateActor::OnMouseUp);
	}
}


bool demo::DemoStateActor::IsFinished(Demo& owner)
{
	return m_isFinished;
}

#pragma endregion