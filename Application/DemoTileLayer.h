#pragma once

#include "Demo.h"
#include <State/State.h>
#include <Engine/Loader/AsyncLoader.h>

namespace demo
{
	class LoadTileLayerState : public engine::state::State<Demo>
	{
	private:
		engine::performance::FrameRateMonitor m_frameRateMonitor;
		engine::loader::IAsyncLoader* m_currentLoader;

		engine::loader::tile::AsyncTileMapLoader<RenderableTile, int> m_tileMapLoader;

		bool m_isFinished;

		std::unique_ptr<engine::graphics::resource::ISpriteAtlas> m_spriteAtlas;
		std::unique_ptr<engine::component::tile1::Tileset<RenderableTile>> m_tileset;
		std::unique_ptr<engine::component::tile1::TileLayer<RenderableTile>> m_layer;

	public:
		LoadTileLayerState();
		virtual ~LoadTileLayerState();

		virtual void Enter(Demo& owner) override;
		virtual void Exit(Demo& owner) override;
		virtual void Update(Demo& owner, double delta) override;
		virtual bool IsFinished(Demo& owner) override;
	};

	class RenderTileLayerState : public engine::state::State<Demo>
	{
	private:
		engine::performance::FrameRateMonitor m_frameRateMonitor;
		std::unique_ptr<engine::graphics::resource::ISpriteAtlas> m_spriteAtlas;
		std::unique_ptr<engine::component::tile1::Tileset<RenderableTile>> m_tileSet;
		std::unique_ptr<engine::component::tile1::TileLayer<RenderableTile>> m_layer;
		engine::math::SizeF m_viewportSize;

		void OnResize(size_t width, size_t height);

	public:
		RenderTileLayerState(
			std::unique_ptr<engine::component::tile1::TileLayer<RenderableTile>> layer,
			std::unique_ptr<engine::graphics::resource::ISpriteAtlas> spriteAtlas,
			std::unique_ptr<engine::component::tile1::Tileset<RenderableTile>> tileSet
		);
		virtual ~RenderTileLayerState();

		virtual void Enter(Demo& owner) override;
		virtual void Update(Demo& owner, double delta) override;
		virtual void Exit(Demo& owner) override;
		virtual bool IsFinished(Demo& owner) override;

		void OnMouseDown(int btn, int x, int y);
	};
};