#pragma once
#include <Graphics/Renderer/IRenderer.h>
#include <Spatial/Position.h>
#include <Graphics/Core/Color.h>

namespace engine
{
	namespace graphics
	{
		namespace primitives
		{
			void DrawLineSegment(
				engine::graphics::renderer::IRenderer& renderer,
				const engine::spatial::PositionF& start,
				const engine::spatial::PositionF& end,
				const engine::graphics::ColorF& color = { 1, 1, 1, 1 },
				float thickness = 1.0f
			);

			void DrawCircleOutline(
				engine::graphics::renderer::IRenderer& renderer,
				const engine::spatial::PositionF& center,
				float radius,
				const engine::graphics::ColorF& color = { 1, 1, 1, 1 },
				float thickness = 1.0f,
				int segments = 32
			);
		}
	}
}


