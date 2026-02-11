#pragma once

#include <Utilities/CSVParser.h>
#include <Engine/Loader/AsyncLoader.h>
#include <Engine/Engine.h>
#include <state/State.h>
#include <state/StateMachine.h>
#include <Command/ICommand.h>
#include <Performance/FrameRateMonitor.h>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <Components/Tile.h>
#include <Graphics/Renderable/SpriteAtlas.h>
#include <Containers/Table.h>
#include <IO/ASyncFileReader.h>
#include <unordered_map>
#include <Graphics/Resource/DX11TextureImpl.h>
#include <Graphics/Renderable/FontAtlas.h>
#include <Engine/Manager/TileMapManager.h>
#include <Engine/Manager/TileSetManager.h>

using namespace engine;

namespace demo
{
	class LoadTileLayerState;
	class DrawTileMapCommand;
	class DrawTileRegionCommand;
};

namespace demo
{
	std::vector<math::geometry::RectF> CalcUV(int row, int col, int fileWidth, int fileHeight);


	class RenderableTile
	{
	private:
		graphics::renderable::Sprite m_sprite;
		bool m_walkable;

	public:
		RenderableTile(const graphics::renderable::Sprite& sprite, bool walkable) :
			m_sprite(sprite),
			m_walkable(walkable)
		{
		}
		const graphics::renderable::Sprite& GetSprite() const
		{
			return m_sprite;
		}
		bool IsWalkable() const
		{
			return m_walkable;
		}
	};

	class DrawTileMapCommand : public command::graphics::renderer::DrawCommandBase
	{
	private:
		spatial::PositionF m_pos;
		component::tile::TileMap<RenderableTile> m_tilemap;

	public:
		DrawTileMapCommand(
			::graphics::renderer::IRenderer& renderer,
			component::tile::TileMap<RenderableTile> tilemap,
			spatial::PositionF pos,
			float alpha = 1.0f
		) :
			DrawCommandBase(renderer),
			m_tilemap(tilemap),
			m_pos(pos)
		{
		}

		void Execute() override
		{
			spatial::SizeF tileSize{ 8.0f, 8.0f };

			for (int row = 0; row <= m_tilemap->GetHeight(); ++row)
			{
				for (int col = 0; col <= m_tilemap->GetWidth(); ++col)
				{
					if (!m_tilemap->IsInBounds(row, col))
					{
						continue;
					}

					const component::tile::Tile<RenderableTile>& tile = m_tilemap->Get(row, col);
					if (tile.isValid())
					{
						spatial::PositionF pos =
						{
							col * tileSize.width,
							row * tileSize.height
						};

						pos += m_pos;

						m_renderer.DrawRenderable(tile->GetSprite(), pos, tileSize, graphics::ColorF{ 1.0f, 1.0f, 1.0f, 1.0f }, 0.0f);
					}
				}
			}
		}
	};

	class Demo1
	{
	private:
		engine::Engine m_engine;
		std::unique_ptr<graphics::renderable::IFontAtlas> m_fontAtlas;
		std::unique_ptr<graphics::renderable::ISpriteAtlas> m_spriteAtlas;
		//std::unique_ptr<component::tile::Tileset<RenderableTile>> m_tileset;

		engine::manager::TileSetManager<RenderableTile> m_tileSetManager;
		engine::manager::TileMapManager<RenderableTile> m_tileMapManager;

		int stage = 0;

	public:
		Demo1() :
			m_engine("Test State Machine", "DirectX11", "Batch", 1000)
		{
			// subscribe to start event of the engine. we do all initialization of our components here e.g. state machine
			m_engine.StartEvent += event::Handler(this, &Demo1::OnStart);

			// subscribe our OnUpdate() to engine's scheduler. the scheduler runs on engine's main loop. 
			// the scheduler is updated by engine's main loop elapsed time per frame. 
			// the scheduler fires up event that is registered at specific interval.
			// we subscribe to be notified every x elapsed time so we can update ourselves at consistent frame rate
			// in our demo this is where we update our state machine
			m_engine.Scheduler() += timer::Schedule(1.0f / 6000.0, this, &Demo1::OnUpdate, true, 1);

			// let the engine run!
			m_engine.Run();
		}
		virtual ~Demo1() {}
		void OnStart()
		{
			// create font atlas for rendering text we will use fore demo
			m_fontAtlas = std::make_unique<graphics::renderable::FontAtlas>(std::make_unique<graphics::dx11::resource::DX11TextureImpl>());
			m_fontAtlas->Initialize("Terminal", 12);
			LOG("[Demo] Font atlas created and initialized...");

			// create sprite atlas to be used by tilemap
			m_spriteAtlas = std::make_unique<graphics::renderable::SpriteAtlas>(std::make_unique<graphics::dx11::resource::DX11TextureImpl>());

			// load sprite atlas from file manually for demo purpose
			m_spriteAtlas->Initialize(L"../Assets/4x1_128x32_tile.png");

			// load sprite atlas UVs from csv manually for demo purpose. we calculate UVs here by assuming a grid of 1 rows and 4 columns
			// in real scenario, you would use SpriteAtlasLoader to load from csv file 
			std::vector<math::geometry::RectF> uvs = demo::CalcUV(1, 4, (int)m_spriteAtlas->GetWidth(), (int)m_spriteAtlas->GetHeight());
			for (math::geometry::RectF& rect : uvs)
			{
				m_spriteAtlas->AddUVRect(rect);
			}
			LOG("Sprite atlas created...");

			// create tileset for tilemap and register tiles
			m_tileSetManager.Create("debugTileSet");
			m_tileSetManager.Register("debugTileSet", 0, std::make_unique<RenderableTile>(m_spriteAtlas->MakeSprite(0), true));		// walkable
			m_tileSetManager.Register("debugTileSet", 1, std::make_unique<RenderableTile>(m_spriteAtlas->MakeSprite(1), false));	// obstacle
			m_tileSetManager.Register("debugTileSet", 2, std::make_unique<RenderableTile>(m_spriteAtlas->MakeSprite(2), false));	// obstacle
			m_tileSetManager.Register("debugTileSet", 3, std::make_unique<RenderableTile>(m_spriteAtlas->MakeSprite(3), false));	// obstacle	
			LOG("Tilesets generated...");
		}

		void OnUpdate(double delta)
		{
			if (stage == 0)
			{
				m_tileMapManager.Load(
					"debugMap",
					"..\\Assets\\256x256.csv",
					[this](const int& cell) -> component::tile::Tile<RenderableTile>
					{
						return m_tileSetManager.MakeTile("debugTileSet", cell);
					},
					m_engine.JobQueue());
				stage = 1;
			}

			if (m_tileMapManager.GetState("debugMap") == engine::manager::TileMapManager<RenderableTile>::MapState::Loaded && stage == 1)
			{
				stage = 2;
			}

			// flush the draw commands on queue. we will queue new ones 
			m_engine.CommandQueue().Clear(engine::command::Type::Render);

			if (stage == 1)
			{
				DrawTextCommand("Loading Map..." + std::to_string(m_tileMapManager.GetProgress("debugMap")), { 50, 300 }, { 1,1,1,1 });
			}
			if (stage == 2)
			{
				//DrawTextCommand("Map loaded!", { 50, 300 }, { 1,1,1,1 });

				RenderTileMapCommand(m_tileMapManager.GetTileMap("debugMap"));
			}


			// draw statistics via command
			std::list<std::string> logs;
			DrawStatisticsCommand(logs);
		}

		void DrawStatisticsCommand(const std::list<std::string>& logs)
		{
			float tab = 15.0f;

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

		// helper function to draw text at top-right screen. this is for showing statistics like FPS
		void DrawTextCommandTopRightScreen(const std::string& text, float y)
		{
			// render text showing which state are we in
			float width = m_fontAtlas->GetWidth(text);
			float height = m_fontAtlas->GetHeight();

			std::unique_ptr<engine::command::graphics::renderer::DrawTextCommand> drawTextCmd =
				std::make_unique<engine::command::graphics::renderer::DrawTextCommand>(
					m_engine.Renderer(),
					*m_fontAtlas,
					text,
					spatial::PositionF
					{
						m_engine.GetViewPort().GetWidth() - width - 10.0f,
						y
					},
					graphics::ColorF{ 1.0f, 1.0f, 1.0f, 1.0f }
				);
			m_engine.CommandQueue().Enqueue(std::move(drawTextCmd));
		}

		void DrawTextCommand(const std::string& text, spatial::PositionF pos, graphics::ColorF color)
		{
			// render text showing which state are we in
			float width = m_fontAtlas->GetWidth(text);
			float height = m_fontAtlas->GetHeight();

			std::unique_ptr<engine::command::graphics::renderer::DrawTextCommand> drawTextCmd =
				std::make_unique<engine::command::graphics::renderer::DrawTextCommand>(
					m_engine.Renderer(),
					*m_fontAtlas,
					text,
					pos,
					color
				);
			m_engine.CommandQueue().Enqueue(std::move(drawTextCmd));
		}

		void RenderTileMapCommand(component::tile::TileMap<RenderableTile> map, float alpha = 1.0f)
		{
			std::unique_ptr<DrawTileMapCommand> cmd =
				std::make_unique<DrawTileMapCommand>(
					m_engine.Renderer(),
					map,
					spatial::PositionF{ 50,50 }
				);
			m_engine.CommandQueue().Enqueue(std::move(cmd));
		}
	};

	class Demo
	{
	private:
		engine::Engine m_engine;
		state::StateMachine<Demo> m_stateMachine;
		std::unique_ptr<graphics::renderable::IFontAtlas> m_fontAtlas;
		std::unique_ptr<state::State<Demo>> m_state;

		engine::manager::TileSetManager<RenderableTile> m_tileSetManager;
		engine::manager::TileMapManager<RenderableTile> m_tileMapManager;

	public:
		Demo(std::unique_ptr<state::State<Demo>> state);
		virtual ~Demo();
		void OnStart();
		void OnUpdate(double delta);

		engine::Engine& Engine()
		{
			return m_engine;
		}

		void DrawTextCommandTopRightScreen(const std::string& text, float y);
		void DrawProgressBarCommand(spatial::PositionF pos, spatial::SizeF size, float current, float total);
		void DrawTextCommand(const std::string& text, spatial::PositionF pos, graphics::ColorF color);
		void DrawStatisticsCommand(const std::list<std::string>& logs);

		void RenderTileGridCommand(component::tile::TileGrid<RenderableTile>& tilegrid, float alpha = 1.0f);
		void RenderTileRegionCommand(component::tile::TileRegion<RenderableTile>& tilegrid, float alpha = 1.0f);
		void RenderTileLayerCommand(component::tile::TileLayer<RenderableTile>& tilegrid, float alpha = 1.0f);

		void SetState(std::unique_ptr<state::State<Demo>> state);
		void QueueState(std::unique_ptr<state::State<Demo>> state);

		bool LoadMap(const std::string& filename);

	};

	class DrawTileGridCommand : public command::graphics::renderer::DrawCommandBase
	{
	private:
		spatial::PositionF m_pos;
		component::tile::TileGrid<RenderableTile>& m_tilegrid;

	public:
		DrawTileGridCommand(
			::graphics::renderer::IRenderer& renderer,
			component::tile::TileGrid<RenderableTile>& tilegrid, 
			spatial::PositionF pos,
			float alpha = 1.0f
			) :
			DrawCommandBase(renderer),
			m_tilegrid(tilegrid),
			m_pos(pos)
		{
		}

		void Execute() override
		{
			spatial::SizeF tileSize{ 8.0f, 8.0f };

			for (int row = 0; row <= m_tilegrid.GetHeight(); ++row)
			{
				for (int col = 0; col <= m_tilegrid.GetWidth(); ++col)
				{
					if (!m_tilegrid.IsInBounds(row, col))
					{
						continue;
					}

					const component::tile::Tile<RenderableTile>& tile = m_tilegrid.Get(row, col);
					if (tile.isValid())
					{
						spatial::PositionF pos =
						{
							col * tileSize.width,
							row * tileSize.height
						};

						pos += m_pos;

						m_renderer.DrawRenderable(tile->GetSprite(), pos, tileSize, graphics::ColorF{ 1.0f, 1.0f, 1.0f, 1.0f }, 0.0f);
					}
				}
			}
		}
	};



	class DrawTileRegionCommand : public command::graphics::renderer::DrawCommandBase
	{
	private:
		spatial::PositionF m_pos;
		component::tile::TileRegion<RenderableTile>& m_region;

	public:
		DrawTileRegionCommand(
			::graphics::renderer::IRenderer& renderer,
			component::tile::TileRegion<RenderableTile>& region,
			spatial::PositionF pos,
			float alpha = 1.0f
		) :
			DrawCommandBase(renderer),
			m_region(region),
			m_pos(pos)
		{
		}

		void Execute() override
		{
			spatial::SizeF tileSize{ 8.0f, 8.0f };

			for (int row = 0; row <= m_region.GetHeight(); ++row)
			{
				for (int col = 0; col <= m_region.GetWidth(); ++col)
				{
					if (!m_region.IsInBounds(row, col))
					{
						continue;
					}

					const component::tile::Tile<RenderableTile>& tile = m_region.Get(row, col);
					if (tile.isValid())
					{
						spatial::PositionF pos =
						{
							col * tileSize.width,
							row * tileSize.height
						};

						pos += m_pos;

						m_renderer.DrawRenderable(tile->GetSprite(), pos, tileSize, graphics::ColorF{ 1.0f, 1.0f, 1.0f, 1.0f }, 0.0f);
					}
				}
			}
		}
	};

	class DrawTileLayerCommand : public command::graphics::renderer::DrawCommandBase
	{
	private:
		spatial::PositionF m_pos;
		component::tile::TileLayer<RenderableTile>& m_layer;

	public:
		DrawTileLayerCommand(
			::graphics::renderer::IRenderer& renderer,
			component::tile::TileLayer<RenderableTile>& layer,
			spatial::PositionF pos,
			float alpha = 1.0f
		) :
			DrawCommandBase(renderer),
			m_layer(layer),
			m_pos(pos)
		{
		}

		void Execute() override
		{
			spatial::SizeF tileSize{ 8.0f, 8.0f };

			size_t regionRows = m_layer.GetHeight();
			size_t regionCols = m_layer.GetWidth();

			size_t regionPosY = 0;
			for (int currRegionRow = 0; currRegionRow < regionRows; currRegionRow++)
			{
				size_t regionPosX = 0;
				for (int currRegionCol = 0; currRegionCol < regionCols; currRegionCol++)
				{
					component::tile::TileRegion<RenderableTile>& region = m_layer.Get(currRegionRow, currRegionCol);

					size_t regionCols = region.GetWidth();
					size_t regionRows = region.GetHeight();


					for (int currTileRow = 0; currTileRow < regionRows; currTileRow++)
					{
						for (int currTileCol = 0; currTileCol < regionCols; currTileCol++)
						{
							if (!region.IsInBounds(currTileRow, currTileCol))
							{
								continue;
							}

							const component::tile::Tile<RenderableTile>& tile = region.Get(currTileRow, currTileCol);
							if (!tile.isValid())
							{
								continue;
							}
							
							spatial::PositionF pos =
							{
								(regionPosX + currTileCol) * tileSize.width,
								(regionPosY + currTileRow) * tileSize.height
							};

							pos += m_pos;

							m_renderer.DrawRenderable(tile->GetSprite(), pos, tileSize, graphics::ColorF{ 1.0f, 1.0f, 1.0f, 1.0f }, 0.0f);
						}
					}

					// advance region position X
					regionPosX += region.GetWidth();
				}

				// advance region position Y for next row
				regionPosY += m_layer.Get(currRegionRow, 0).GetHeight();	
			}
		}
	};

	class LoadSequentialState : public state::State<Demo>
	{
	private:
		performance::FrameRateMonitor m_frameRateMonitor;
		io::AsyncFileReader m_fileReader;
		std::deque<std::string> m_files;
		std::string m_currFile;
		int m_readSize;
		bool m_isFinished;

	public:
		LoadSequentialState();
		virtual ~LoadSequentialState();

		virtual void Enter(Demo& owner) override;
		virtual void Exit(Demo& owner) override;
		virtual void Update(Demo& owner, double delta) override;
		virtual bool IsFinished(Demo& owner) override;

		void OnMouseDown(int btn, int x, int y);
	};

	class LoadSimultaneousState : public state::State<Demo>
	{
	private:
		performance::FrameRateMonitor m_frameRateMonitor;
		bool m_isFinished;
		io::AsyncFileReader m_fileReader0;
		io::AsyncFileReader m_fileReader1;
		io::AsyncFileReader m_fileReader2;
		int m_nFilesInProgress;

	public:
		LoadSimultaneousState();
		virtual ~LoadSimultaneousState();

		virtual void Enter(Demo& owner) override;
		virtual void Exit(Demo& owner) override;
		virtual void Update(Demo& owner, double delta) override;
		virtual bool IsFinished(Demo& owner) override;

		void OnMouseDown(int btn, int x, int y);
	};

	class LoadTileRegionState : public state::State<Demo>
	{
	private:
		performance::FrameRateMonitor m_frameRateMonitor;
		io::AsyncFileReader m_fileReader;
		engine::utilities::parser::CSVParser m_csvParser;
		engine::container::Table<std::string> m_table;
		engine::loader::tile::AsyncTileRegionLoader<RenderableTile, int> m_tileRegionLoader;

		bool m_isFinished;
		bool m_csvTableLoaded;
		bool m_tileRegionLoaded;

		std::unique_ptr<graphics::renderable::ISpriteAtlas> m_spriteAtlas;
		std::unique_ptr<component::tile::Tileset<RenderableTile>> m_tileset;
		std::unique_ptr<component::tile::TileRegion<RenderableTile>> m_region;

	public:
		LoadTileRegionState();
		virtual ~LoadTileRegionState();

		virtual void Enter(Demo& owner) override;
		virtual void Exit(Demo& owner) override;
		virtual void Update(Demo& owner, double delta) override;
		virtual bool IsFinished(Demo& owner) override;

		void OnMouseDown(int btn, int x, int y);
	};

	class RenderTileRegionState : public state::State<Demo>
	{
	private:
		performance::FrameRateMonitor m_frameRateMonitor;
		std::unique_ptr<graphics::renderable::ISpriteAtlas> m_spriteAtlas;
		std::unique_ptr<component::tile::Tileset<RenderableTile>> m_tileSet;
		std::unique_ptr<component::tile::TileRegion<RenderableTile>> m_region;

	public:
		RenderTileRegionState(
			std::unique_ptr<component::tile::TileRegion<RenderableTile>> region,
			std::unique_ptr<graphics::renderable::ISpriteAtlas> spriteAtlas,
			std::unique_ptr<component::tile::Tileset<RenderableTile>> tileSet
		);
		virtual ~RenderTileRegionState();

		virtual void Enter(Demo& owner) override;
		virtual void Update(Demo& owner, double delta) override;
		virtual void Exit(Demo& owner) override;
		virtual bool IsFinished(Demo& owner) override;

		void OnMouseDown(int btn, int x, int y);
	};


}
