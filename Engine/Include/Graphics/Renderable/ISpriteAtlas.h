#pragma once
#include <Graphics/Renderable/IRenderable.h>
#include <Graphics/Renderable/Sprite.h>
#include <Spatial/ISizeable.h>

namespace graphics::renderable
{
	class ISpriteAtlas : public graphics::renderable::IRenderable, public spatial::ISizeable<float>
	{

	public:
		virtual ~ISpriteAtlas() = default;
		virtual bool Initialize(const wchar_t* fileNamePath) = 0;
		virtual void AddUVRect(math::geometry::RectF rect) = 0;
		virtual inline const math::geometry::RectF GetUVRect(int index) const = 0;
		virtual inline size_t GetUVRectCount() const = 0;
		virtual graphics::renderable::Sprite MakeSprite(int index) const = 0;
	};
}
