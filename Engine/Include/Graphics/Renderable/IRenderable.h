#pragma once
#include <Math/Rect.h>

namespace graphics::renderable
{
	class IRenderable
	{
	public:
		virtual ~IRenderable() = default;

		virtual void Bind() const = 0;
		virtual bool CanBind() const = 0;
		virtual math::geometry::RectF GetUVRect() const = 0;
	};
}
