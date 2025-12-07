// description:
// Sprite is a renderable object that represents a portion of a sprite atlas defined by a UV rectangle.
// it is a "view" into a sprite atlas and does not own the texture data itself.
// It is a renderable object that can be bound for rendering operations.
// 
// parameters:
// - spriteAtlas: Pointer to the ISpriteAtlas that contains the texture and UV data.
// - rect: A RectF defining the UV coordinates (normalized) within the sprite atlas.
//
// methods:
// - GetWidth(): Returns the width of the sprite in pixels. it is calculated based on the sprite atlas width and the UV rectangle.
// - GetHeight(): Returns the height of the sprite in pixels. its calculated based on the sprite atlas height and the UV rectangle.
// - GetSize(): Returns the size of the sprite as a SizeF structure containing width and height in pixels.
// - Bind(): Binds the sprite's texture for rendering by delegating to the sprite atlas.
// - CanBind(): Checks if the sprite's texture can be bound for rendering by delegating to the sprite atlas.
// - GetUVRect(): Returns the UV rectangle defining the portion of the sprite atlas used by this sprite.

#pragma once
#include <Graphics/Renderable/IRenderable.h>
#include <Spatial/ISizeable.h>
#include <memory>

// forward declare
namespace graphics
{
	namespace loader
	{
		class SpriteLoader;
	}

	namespace renderable
	{
		class ISpriteAtlas;
		class SpriteAtlas;
	}
}

namespace graphics::renderable
{
	// is renderable
	// has pointer to sprite atlas 
	// has source rect (normalized rectangular coordinates or UV)
	class Sprite : public graphics::renderable::IRenderable, public spatial::ISizeable<float>
	{
	private:
		const graphics::renderable::ISpriteAtlas* m_spriteAtlas;
		math::geometry::RectF m_rect;

		friend class graphics::renderable::SpriteAtlas;
		friend class graphics::renderable::ISpriteAtlas;

	public:
		// use this constructor if you have the sprite atlas and the source rect, or you can use factory to get the sprite
		Sprite(const graphics::renderable::ISpriteAtlas* spriteAtlas, math::geometry::RectF rect);
		~Sprite() = default;

	public:

		// ISizeable methods implementation
		virtual const float GetWidth() const override final;
		virtual const float GetHeight() const override final;
		virtual const spatial::SizeF GetSize() const override final;

		// IRenderable methods implementation
		virtual void Bind() const override final;
		virtual bool CanBind() const override final;
		virtual math::geometry::RectF GetUVRect() const override final;
	};
}
