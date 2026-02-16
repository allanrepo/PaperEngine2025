
#pragma once

#include "Demo.h"
#include <State/State.h>
#include <Cache/BindCache.h>

namespace demo
{
	class LoadTileMapState : public state::State<Demo>
	{
	private:
		engine::performance::FrameRateMonitor m_frameRateMonitor;
		bool m_isFinished;
		std::string m_mapFileName;

	public:
		LoadTileMapState(const std::string& filePath);
		virtual ~LoadTileMapState() = default;

		virtual void Enter(Demo& owner) override;
		virtual void Exit(Demo& owner) override;
		virtual void Update(Demo& owner, double delta) override;
		virtual bool IsFinished(Demo& owner) override;
	};

	class RenderTileMapState : public state::State<Demo>
	{
	private:
		engine::performance::FrameRateMonitor m_frameRateMonitor;
		engine::spatial::SizeF m_viewportSize;

		void OnResize(size_t width, size_t height);

	public:
		RenderTileMapState();
		virtual ~RenderTileMapState() = default;

		virtual void Enter(Demo& owner) override;
		virtual void Update(Demo& owner, double delta) override;
		virtual void Exit(Demo& owner) override;
		virtual bool IsFinished(Demo& owner) override;

		void OnMouseDown(int btn, int x, int y);
	};
};