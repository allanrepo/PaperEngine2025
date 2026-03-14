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
#include <Graphics/Resource/SpriteAtlas.h>
#include <Containers/Table.h>
#include <IO/ASyncFileReader.h>
#include <unordered_map>
#include <Graphics/Resource/DX11TextureImpl.h>
#include <Graphics/Resource/FontAtlas.h>
#include <Engine/Manager/TileMapManager.h>
#include <Engine/Manager/TileSetManager.h>
#include <Graphics/Animation/Animation.h>
#include <Math/Vector.h>
#include <Engine/Manager/AnimatedTileSetManager.h>
#include <Engine/Factory/AnimationFactory.h>
#include <Spatial/Camera.h>
#include "Actor.h"

//using namespace engine;

namespace demo
{
	class LoadTileLayerState;
	template<typename T>
	class DrawTileMapCommand;
	class DrawTileRegionCommand;
};

namespace demo
{
	std::vector<engine::math::geometry::RectF> CalcUV(int row, int col, int fileWidth, int fileHeight);
	
	class RenderableTile
	{
	private:
		engine::graphics::Sprite m_sprite;
		bool m_walkable;
		engine::graphics::animation::Animator<engine::graphics::Sprite>* m_animator;

	public:
		RenderableTile(const engine::graphics::Sprite& sprite, bool walkable, engine::graphics::animation::Animator<engine::graphics::Sprite>* animator = nullptr) :
			m_sprite(sprite),
			m_walkable(walkable),
			m_animator(animator)
		{
		}

		const engine::graphics::Sprite& GetSprite() const
		{
			if (m_animator)
			{
				return m_animator->GetCurrent();
			}
			return m_sprite;
		}
		bool IsWalkable() const
		{
			return m_walkable;
		}
	};

	template<typename T>
	class DrawTileMapCommand : public engine::command::graphics::renderer::DrawCommandBase
	{
	private:
		engine::spatial::PositionF m_pos;
		engine::component::tile::TileMap<T> m_tilemap;
		engine::spatial::SizeF m_tilesize;
		float m_alpha;
		engine::math::VecF m_scale;
		engine::math::VecF m_offset;

	public:
		DrawTileMapCommand(
			engine::graphics::renderer::IRenderer& renderer,
			engine::component::tile::TileMap<T> tilemap,
			engine::spatial::PositionF pos = {50.0f, 50.0f},
			engine::spatial::SizeF tilesize = { 8.0f, 8.0f },
			engine::math::VecF offset = {0,0},
			engine::math::VecF scale = {1,1},
			float alpha = 1.0f
		) :
			DrawCommandBase(renderer),
			m_tilemap(tilemap),
			m_pos(pos),
			m_tilesize(tilesize),
			m_alpha(alpha),
			m_scale(scale),
			m_offset(offset)
		{
		}

		void Execute() override
		{
			for (int row = 0; row <= m_tilemap.GetHeight(); ++row)
			{
				for (int col = 0; col <= m_tilemap.GetWidth(); ++col)
				{
					if (!m_tilemap.IsInBounds(row, col))
					{
						continue;
					}

					const engine::component::tile::Tile<T>& tile = m_tilemap.Get(row, col);
					if (tile.IsValid())
					{
						engine::spatial::PositionF pos =
						{
							col * m_tilesize.width,
							row * m_tilesize.height
						};

						pos += m_pos + m_offset;

						engine::spatial::SizeF tilesize =
						{
							m_tilesize.width * m_scale.x,
							m_tilesize.height * m_scale.y
						};
												
						m_renderer.Draw(tile->GetSprite(), pos, tilesize, engine::graphics::ColorF{ 1.0f, 1.0f, 1.0f, m_alpha }, 0.0f);
					}
				}
			}
		}
	};

	template<typename T>
	class DrawTileMapOnViewPortCommand : public engine::command::graphics::renderer::DrawCommandBase
	{
	private:
		engine::spatial::PositionF m_pos;
		engine::component::tile::TileMap<T> m_tilemap;
		engine::spatial::SizeF m_tilesize;
		float m_alpha;
		engine::math::VecF m_scale;
		engine::math::VecF m_offset;
		engine::spatial::CameraF& m_camera;

	public:
		DrawTileMapOnViewPortCommand(
			engine::graphics::renderer::IRenderer& renderer,
			engine::component::tile::TileMap<T> tilemap,
			engine::spatial::CameraF& camera,
			engine::spatial::PositionF pos = { 50.0f, 50.0f },
			engine::spatial::SizeF tilesize = { 8.0f, 8.0f },
			engine::math::VecF offset = { 0,0 },
			engine::math::VecF scale = { 1,1 },
			float alpha = 1.0f
		) :
			DrawCommandBase(renderer),
			m_tilemap(tilemap),
			m_pos(pos),
			m_tilesize(tilesize),
			m_alpha(alpha),
			m_scale(scale),
			m_offset(offset),
			m_camera(camera)
		{
		}

		void Execute() override
		{
			engine::math::geometry::RectF vp = m_camera.GetViewport();
			engine::spatial::PositionF camPos = m_camera.GetPosition();

			// the whole tilemap might be bigger than viewport. so we may not need to draw all the tiles as some are outside viewport
			// calculate the start and end tile row and column that is visible in viewport and we will only render them
			int left = (int)(camPos.x / m_tilesize.width);
			int top = (int)(camPos.y / m_tilesize.height);

			// add 1 tile as we need to draw 1 tile bigger than viewport to handle offst
			int right = (int)((camPos.x + vp.GetWidth()) / m_tilesize.width);
			int bottom = (int)((camPos.y + vp.GetHeight()) / m_tilesize.height);

			for (int row = top; row <= bottom; ++row)
			{
				for (int col = left; col <= right; ++col)
				{
					// defensive. just in case we have tile that is out of bounds, skip it.
					if (!m_tilemap.IsInBounds(row, col))
					{
						continue;
					}

					// get the tile
					const engine::component::tile::Tile<T>& tile = m_tilemap.Get(row, col);

					// defensive. we're never sure if the tile has valid sprite, so do check
					if (tile.IsValid())
					{
						// this will be the top-left position of this tile in map coordinate.
						engine::spatial::PositionF pos =
						{
							col * m_tilesize.width,
							row * m_tilesize.height
						};

						// m_pos is the position of map in the world coordinate. it is the top-left position of map in the world
						// typically, it is 0,0 as the map itself is the world. but in this function, we are not assuming it.
						// we will take give m_pos as the top-left position of the map in the world
						// so we translate this tile's top-left position with m_pos
						pos += m_pos;

						pos += m_offset;

						engine::spatial::SizeF tilesize =
						{
							m_tilesize.width * m_scale.x,
							m_tilesize.height * m_scale.y
						};

						m_renderer.Draw(
							tile->GetSprite(),
							m_camera.WorldToScreen(pos),
							tilesize,
							engine::graphics::ColorF{ 1.0f, 1.0f, 1.0f, m_alpha },
							0.0f
						);
					}
				}
			}


		}
	};

	class Demo
	{
	private:
		using AnimatedTile = engine::component::tile::AnimatedTile;
		using AnimatedTileSetManager = engine::manager::AnimatedTileSetManager;

		engine::Engine m_engine;
		engine::state::StateMachine<Demo> m_stateMachine;
		std::unique_ptr<engine::graphics::resource::IFontAtlas> m_fontAtlas;
		std::unique_ptr<engine::state::State<Demo>> m_state;

		AnimatedTileSetManager m_tileSetManager;
		engine::manager::TileMapManager<AnimatedTile> m_tileMapManager;

	public:
		Demo(std::unique_ptr<engine::state::State<Demo>> state);
		virtual ~Demo();
		void OnStart();
		void OnUpdate(double delta);

		engine::Engine& Engine(){ return m_engine; }
		AnimatedTileSetManager& TileSetManager() { return m_tileSetManager; }
		engine::manager::TileMapManager<AnimatedTile>& TileMapManager() { return m_tileMapManager; }

		void DrawTextCommandTopRightScreen(const std::string& text, float y);
		void DrawProgressBarCommand(engine::spatial::PositionF pos, engine::spatial::SizeF size, float current, float total);
		void DrawTextCommand(const std::string& text, engine::spatial::PositionF pos, engine::graphics::ColorF color);
		void DrawStatisticsCommand(const std::list<std::string>& logs);

		template<typename T>
		void RenderTileMapCommand(
			engine::component::tile::TileMap<T> map,
			engine::spatial::PositionF pos = { 50.0f, 50.0f },
			engine::spatial::SizeF tilesize = { 8.0f, 8.0f },
			engine::math::VecF offset = { 0,0 },
			engine::math::VecF scale = { 1,1 },
			float alpha = 1.0f)
		{
			std::unique_ptr<DrawTileMapCommand<T>> cmd =
				std::make_unique<DrawTileMapCommand<T>>(
					m_engine.Renderer(),
					map,
					pos,
					tilesize,
					offset,
					scale,
					alpha
				);
			m_engine.CommandQueue().Enqueue(std::move(cmd));
		}

		template<typename T>
		void RenderTileMapOnViewPortCommand(
			engine::component::tile::TileMap<T> map,
			engine::spatial::CameraF& camera,
			engine::spatial::PositionF pos = { 50.0f, 50.0f },
			engine::spatial::SizeF tilesize = { 8.0f, 8.0f },
			engine::math::VecF offset = { 0,0 },
			engine::math::VecF scale = { 1,1 },
			float alpha = 1.0f)
		{
			std::unique_ptr<DrawTileMapOnViewPortCommand<T>> cmd =
				std::make_unique<DrawTileMapOnViewPortCommand<T>>(
					m_engine.Renderer(),
					map,
					camera,
					pos,
					tilesize,
					offset,
					scale,
					alpha
				);
			m_engine.CommandQueue().Enqueue(std::move(cmd));
		}

		void RenderTileGridCommand(engine::component::tile::TileGrid<RenderableTile>& tilegrid, float alpha = 1.0f);
		void RenderTileRegionCommand(engine::component::tile::TileRegion<RenderableTile>& tilegrid, float alpha = 1.0f);
		void RenderTileLayerCommand(engine::component::tile::TileLayer<RenderableTile>& tilegrid, float alpha = 1.0f);

		void SetState(std::unique_ptr<engine::state::State<Demo>> state);
		void QueueState(std::unique_ptr<engine::state::State<Demo>> state);

		bool LoadMap(const std::string& filename);


	};

	class DrawTileGridCommand : public engine::command::graphics::renderer::DrawCommandBase
	{
	private:
		engine::spatial::PositionF m_pos;
		engine::component::tile::TileGrid<RenderableTile>& m_tilegrid;

	public:
		DrawTileGridCommand(
			engine::graphics::renderer::IRenderer& renderer,
			engine::component::tile::TileGrid<RenderableTile>& tilegrid,
			engine::spatial::PositionF pos,
			float alpha = 1.0f
			) :
			DrawCommandBase(renderer),
			m_tilegrid(tilegrid),
			m_pos(pos)
		{
		}

		void Execute() override
		{
			engine::spatial::SizeF tileSize{ 8.0f, 8.0f };

			for (int row = 0; row <= m_tilegrid.GetHeight(); ++row)
			{
				for (int col = 0; col <= m_tilegrid.GetWidth(); ++col)
				{
					if (!m_tilegrid.IsInBounds(row, col))
					{
						continue;
					}

					const engine::component::tile::Tile<RenderableTile>& tile = m_tilegrid.Get(row, col);
					if (tile.IsValid())
					{
						engine::spatial::PositionF pos =
						{
							col * tileSize.width,
							row * tileSize.height
						};

						pos += m_pos;

						m_renderer.Draw(tile->GetSprite(), pos, tileSize, engine::graphics::ColorF{ 1.0f, 1.0f, 1.0f, 1.0f }, 0.0f);
					}
				}
			}
		}
	};

	class DrawTileRegionCommand : public engine::command::graphics::renderer::DrawCommandBase
	{
	private:
		engine::spatial::PositionF m_pos;
		engine::component::tile::TileRegion<RenderableTile>& m_region;

	public:
		DrawTileRegionCommand(
			engine::graphics::renderer::IRenderer& renderer,
			engine::component::tile::TileRegion<RenderableTile>& region,
			engine::spatial::PositionF pos,
			float alpha = 1.0f
		) :
			DrawCommandBase(renderer),
			m_region(region),
			m_pos(pos)
		{
		}

		void Execute() override
		{
			engine::spatial::SizeF tileSize{ 8.0f, 8.0f };

			for (int row = 0; row <= m_region.GetHeight(); ++row)
			{
				for (int col = 0; col <= m_region.GetWidth(); ++col)
				{
					if (!m_region.IsInBounds(row, col))
					{
						continue;
					}

					const engine::component::tile::Tile<RenderableTile>& tile = m_region.Get(row, col);
					if (tile.IsValid())
					{
						engine::spatial::PositionF pos =
						{
							col * tileSize.width,
							row * tileSize.height
						};

						pos += m_pos;

						m_renderer.Draw(tile->GetSprite(), pos, tileSize, engine::graphics::ColorF{ 1.0f, 1.0f, 1.0f, 1.0f }, 0.0f);
					}
				}
			}
		}
	};

	class DrawTileLayerCommand : public engine::command::graphics::renderer::DrawCommandBase
	{
	private:
		engine::spatial::PositionF m_pos;
		engine::component::tile::TileLayer<RenderableTile>& m_layer;

	public:
		DrawTileLayerCommand(
			engine::graphics::renderer::IRenderer& renderer,
			engine::component::tile::TileLayer<RenderableTile>& layer,
			engine::spatial::PositionF pos,
			float alpha = 1.0f
		) :
			DrawCommandBase(renderer),
			m_layer(layer),
			m_pos(pos)
		{
		}

		void Execute() override
		{
			engine::spatial::SizeF tileSize{ 8.0f, 8.0f };

			size_t regionRows = m_layer.GetHeight();
			size_t regionCols = m_layer.GetWidth();

			size_t regionPosY = 0;
			for (int currRegionRow = 0; currRegionRow < regionRows; currRegionRow++)
			{
				size_t regionPosX = 0;
				for (int currRegionCol = 0; currRegionCol < regionCols; currRegionCol++)
				{
					engine::component::tile::TileRegion<RenderableTile>& region = m_layer.Get(currRegionRow, currRegionCol);

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

							const engine::component::tile::Tile<RenderableTile>& tile = region.Get(currTileRow, currTileCol);
							if (!tile.IsValid())
							{
								continue;
							}
							
							engine::spatial::PositionF pos =
							{
								(regionPosX + currTileCol) * tileSize.width,
								(regionPosY + currTileRow) * tileSize.height
							};

							pos += m_pos;

							m_renderer.Draw(tile->GetSprite(), pos, tileSize, engine::graphics::ColorF{ 1.0f, 1.0f, 1.0f, 1.0f }, 0.0f);
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

	class DemoState : public engine::state::State<Demo>
	{
	private:
		using AnimatedTile = engine::component::tile::AnimatedTile;

		engine::performance::FrameRateMonitor m_frameRateMonitor;
		bool m_isFinished;

	public:
		DemoState();
		virtual ~DemoState();

		virtual void Enter(Demo& owner) override;
		virtual void Exit(Demo& owner) override;
		virtual void Update(Demo& owner, double delta) override;
		virtual bool IsFinished(Demo& owner) override;

		void OnMouseDown(int btn, int x, int y);
	};

	class DemoStateCameraMap : public engine::state::State<Demo>
	{
	private:
		using AnimatedTile = engine::component::tile::AnimatedTile;

		engine::performance::FrameRateMonitor m_frameRateMonitor;
		bool m_isFinished;

		engine::spatial::CameraF m_camera;
		engine::spatial::PositionF m_lastMousePos;
		bool m_isPanning;
		engine::spatial::PositionF m_focusPos;
		engine::spatial::SizeF m_tileSize;


	public:
		DemoStateCameraMap();
		virtual ~DemoStateCameraMap();

		virtual void Enter(Demo& owner) override;
		virtual void Exit(Demo& owner) override;
		virtual void Update(Demo& owner, double delta) override;
		virtual bool IsFinished(Demo& owner) override;

		void OnMouseMove(int x, int y);
		void OnMouseDown(int btn, int x, int y);
		void OnMouseUp(int btn, int x, int y);
	};

	class DemoStateActor : public engine::state::State<Demo>
	{
	private:
		using AnimatedTile = engine::component::tile::AnimatedTile;
		using Sprite = engine::graphics::Sprite;
		using Animator = engine::graphics::animation::Animator<Sprite>;
		using Animation = engine::graphics::animation::Animation<Sprite>;
		using AnimationFactory = engine::graphics::factory::AnimationFactory;

		engine::performance::FrameRateMonitor m_frameRateMonitor;
		bool m_isFinished;

		engine::spatial::CameraF m_camera;
		engine::spatial::PositionF m_lastMousePos;
		bool m_isPanning;
		engine::spatial::PositionF m_focusPos;
		engine::spatial::SizeF m_tileSize;


	public:
		DemoStateActor();
		virtual ~DemoStateActor();

		virtual void Enter(Demo& owner) override;
		virtual void Exit(Demo& owner) override;
		virtual void Update(Demo& owner, double delta) override;
		virtual bool IsFinished(Demo& owner) override;

		void OnMouseMove(int x, int y);
		void OnMouseDown(int btn, int x, int y);
		void OnMouseUp(int btn, int x, int y);
	};


	class DemoStatePathFinding : public engine::state::State<Demo>
	{
	private:
		engine::performance::FrameRateMonitor m_frameRateMonitor;
		bool m_isFinished;

	public:
		DemoStatePathFinding();
		virtual ~DemoStatePathFinding();

		virtual void Enter(Demo& owner) override;
		virtual void Exit(Demo& owner) override;
		virtual void Update(Demo& owner, double delta) override;
		virtual bool IsFinished(Demo& owner) override;

		void OnMouseDown(int btn, int x, int y);
	};
}
