
#pragma once

#include "Demo.h"
#include <State/State.h>
#include <Cache/BindCache.h>

namespace demo
{
	class LoadAsyncLoaderState : public state::State<Demo>
	{
	private:
		performance::FrameRateMonitor m_frameRateMonitor;

		engine::io::AsyncFileReader m_fileReader;
		engine::loader::tile::AsyncTileGridLoader<RenderableTile, int> m_tileGridLoader;
		engine::loader::tile::AsyncTileRegionLoader<RenderableTile, int> m_tileRegionLoader;
		engine::loader::tile::AsyncTileLayerLoader<RenderableTile, int> m_tileLayerLoader;
		engine::loader::tile::AsyncCSVMapToTileRegionLoader<RenderableTile, int> m_asyncCSVMapToTileRegionLoader;
		engine::loader::tile::AsyncTileMapLoader<RenderableTile, int> m_tileMapLoader;

		engine::loader::tile::AsyncTileLayerClearer<RenderableTile> m_tileLayerClearer;
		engine::loader::container::AsyncContainerClearer<engine::component::tile::Tile<RenderableTile>> m_tileGridClearer;
		engine::loader::container::AsyncContainerClearer<std::string> m_tableClearer;

		engine::utilities::parser::CSVParser m_csvParser;

		std::unique_ptr<engine::component::tile::TileGrid<RenderableTile>> m_grid;
		std::unique_ptr<engine::component::tile::TileRegion<RenderableTile>> m_region;
		engine::container::Table<std::string> m_table;

		std::string m_filePath;

		engine::loader::IAsyncLoader* m_currentLoader;



		bool m_isFinished;

		std::unique_ptr<graphics::renderable::ISpriteAtlas> m_spriteAtlas;
		std::unique_ptr<engine::component::tile::Tileset<RenderableTile>> m_tileset;
		std::unique_ptr<engine::component::tile::TileLayer<RenderableTile>> m_layer;

	public:
		LoadAsyncLoaderState(const std::string& filePath);
		virtual ~LoadAsyncLoaderState();

		virtual void Enter(Demo& owner) override;
		virtual void Exit(Demo& owner) override;
		virtual void Update(Demo& owner, double delta) override;
		virtual bool IsFinished(Demo& owner) override;
	};

	class RenderAsyncLoaderState : public state::State<Demo>
	{
	private:
		performance::FrameRateMonitor m_frameRateMonitor;
		std::unique_ptr<engine::component::tile::Tileset<RenderableTile>> m_tileSet;
		std::unique_ptr<engine::component::tile::TileLayer<RenderableTile>> m_layer;
		spatial::SizeF m_viewportSize;

		void OnResize(size_t width, size_t height);

	public:
		RenderAsyncLoaderState(
			std::unique_ptr<engine::component::tile::TileLayer<RenderableTile>> layer,
			std::unique_ptr<engine::component::tile::Tileset<RenderableTile>> tileSet
		);
		virtual ~RenderAsyncLoaderState();

		virtual void Enter(Demo& owner) override;
		virtual void Update(Demo& owner, double delta) override;
		virtual void Exit(Demo& owner) override;
		virtual bool IsFinished(Demo& owner) override;

		void OnMouseDown(int btn, int x, int y);
	};
};