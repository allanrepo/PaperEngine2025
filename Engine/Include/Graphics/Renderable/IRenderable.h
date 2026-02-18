#pragma once
#include <Math/Rect.h>
#include <Spatial/Position.h>

namespace engine::graphics::renderable
{
	class IRenderable
	{
	public:
		virtual ~IRenderable() = default;

		virtual void Bind() const = 0;
		virtual bool CanBind() const = 0;
		virtual engine::math::geometry::RectF GetUVRect() const = 0;

		virtual void SetAnchor(const engine::spatial::PositionF& pos) = 0;
		virtual engine::spatial::PositionF GetAnchor() const = 0;
	};
}
