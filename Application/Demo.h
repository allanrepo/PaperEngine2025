#include <Utilities/CSVParser.h>
#include <Engine/Loader/AsyncTileGridLoader.h>
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

using namespace engine;

namespace demo
{
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
		
	class Demo
	{
	private:
		engine::Engine m_engine;
		state::StateMachine<Demo> m_stateMachine;
		std::unique_ptr<graphics::renderable::IFontAtlas> m_fontAtlas;
		component::tile::TileRegion<RenderableTile>* m_region;
		component::tile::TileLayer<RenderableTile>* m_layer;

	public:
		Demo();
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

		void RenderTileGridCommand(component::tile::TileGrid<RenderableTile>& tilegrid, float alpha = 1.0f);
		void RenderTileRegionCommand(component::tile::TileRegion<RenderableTile>& tilegrid, float alpha = 1.0f);
		void RenderTileLayerCommand(component::tile::TileLayer<RenderableTile>& tilegrid, float alpha = 1.0f);

		void SetState(std::unique_ptr<state::State<Demo>> state);
		void QueueState(std::unique_ptr<state::State<Demo>> state);
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

					const component::tile::Tile<RenderableTile>& tile = m_tilegrid.GetTile(row, col);
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

					const component::tile::Tile<RenderableTile>& tile = m_region.GetTile(row, col);
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
					component::tile::TileRegion region = m_layer.GetRegion(currRegionRow, currRegionCol);

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

							const component::tile::Tile<RenderableTile>& tile = region.GetTile(currTileRow, currTileCol);
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
				regionPosY += m_layer.GetRegion(currRegionRow, 0).GetHeight();	
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

	class LoadTileMapState : public state::State<Demo>
	{
	private:
		performance::FrameRateMonitor m_frameRateMonitor;
		io::AsyncFileReader m_fileReader;
		engine::utilities::parser::CSVParser m_csvParser;
		component::tile::loader::AsyncTileGridLoader<RenderableTile, int> m_tileGridLoader;
		container::Table<std::string> m_table;

		bool m_isFinished;
		bool m_csvTableLoaded;
		bool m_tileGridLoaded;

		std::unique_ptr<graphics::renderable::ISpriteAtlas> m_spriteAtlas;
		std::unique_ptr<component::tile::Tileset<RenderableTile>> m_tileset;
		std::unique_ptr<component::tile::TileGrid<RenderableTile>> m_tilegrid;

	public:
		LoadTileMapState();
		virtual ~LoadTileMapState();

		virtual void Enter(Demo& owner) override;
		virtual void Exit(Demo& owner) override;
		virtual void Update(Demo& owner, double delta) override;
		virtual bool IsFinished(Demo& owner) override;

		void OnMouseDown(int btn, int x, int y);
	};

	class RenderTileMapState : public state::State<Demo>
	{
	private:
		performance::FrameRateMonitor m_frameRateMonitor;
		std::unique_ptr<graphics::renderable::ISpriteAtlas> m_spriteAtlas;
		std::unique_ptr<component::tile::Tileset<RenderableTile>> m_tileSet;
		std::unique_ptr<component::tile::TileGrid<RenderableTile>> m_tileGrid;

	public:
		RenderTileMapState(
			std::unique_ptr<component::tile::TileGrid<RenderableTile>> tileGrid,
			std::unique_ptr<graphics::renderable::ISpriteAtlas> spriteAtlas,
			std::unique_ptr<component::tile::Tileset<RenderableTile>> tileSet
		);
		virtual ~RenderTileMapState();

		virtual void Enter(Demo& owner) override;
		virtual void Update(Demo& owner, double delta) override;
		virtual void Exit(Demo& owner) override;
		virtual bool IsFinished(Demo& owner) override;

		void OnMouseDown(int btn, int x, int y);
	};

	class LoadTileRegionState : public state::State<Demo>
	{
	private:
		performance::FrameRateMonitor m_frameRateMonitor;
		io::AsyncFileReader m_fileReader;
		engine::utilities::parser::CSVParser m_csvParser;
		container::Table<std::string> m_table;
		component::tile::loader::AsyncTileRegionLoader<RenderableTile, int> m_tileRegionLoader;

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

	class LoadTileLayerState : public state::State<Demo>
	{
	private:
		performance::FrameRateMonitor m_frameRateMonitor;
		io::AsyncFileReader m_fileReader;
		engine::utilities::parser::CSVParser m_csvParser;
		container::Table<std::string> m_table;
		component::tile::loader::AsyncTileLayerLoader<RenderableTile, int> m_tileLayerLoader;

		bool m_isFinished;
		bool m_csvTableLoaded;
		bool m_tileLayerLoaded;

		std::unique_ptr<graphics::renderable::ISpriteAtlas> m_spriteAtlas;
		std::unique_ptr<component::tile::Tileset<RenderableTile>> m_tileset;
		std::unique_ptr<component::tile::TileLayer<RenderableTile>> m_layer;

	public:
		LoadTileLayerState();
		virtual ~LoadTileLayerState();

		virtual void Enter(Demo& owner) override;
		virtual void Exit(Demo& owner) override;
		virtual void Update(Demo& owner, double delta) override;
		virtual bool IsFinished(Demo& owner) override;

		void OnMouseDown(int btn, int x, int y);
	};

	class RenderTileLayerState : public state::State<Demo>
	{
	private:
		performance::FrameRateMonitor m_frameRateMonitor;
		std::unique_ptr<graphics::renderable::ISpriteAtlas> m_spriteAtlas;
		std::unique_ptr<component::tile::Tileset<RenderableTile>> m_tileSet;
		std::unique_ptr<component::tile::TileLayer<RenderableTile>> m_layer;

	public:
		RenderTileLayerState(
			std::unique_ptr<component::tile::TileLayer<RenderableTile>> layer,
			std::unique_ptr<graphics::renderable::ISpriteAtlas> spriteAtlas,
			std::unique_ptr<component::tile::Tileset<RenderableTile>> tileSet
		);
		virtual ~RenderTileLayerState();

		virtual void Enter(Demo& owner) override;
		virtual void Update(Demo& owner, double delta) override;
		virtual void Exit(Demo& owner) override;
		virtual bool IsFinished(Demo& owner) override;

		void OnMouseDown(int btn, int x, int y);
	};
}
