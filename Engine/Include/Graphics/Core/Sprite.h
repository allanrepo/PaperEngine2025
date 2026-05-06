// DESCRIPTION:
// Sprite is a renderable object that represents a portion of a sprite atlas defined by a UV rectangle.
// it is a "view" into a sprite atlas and does not own the texture data itself.
// it is a renderable object that can be bound for rendering operations.
// only sprite atlas can create sprites since its constructor is protected and sprite atlas is a friend class.
// 
// PARAMETERS:
// - spriteAtlas: Pointer to the ISpriteAtlas that contains the texture and UV data.
// - rect: A RectF defining the UV coordinates (normalized) within the sprite atlas.
//
// METHODS:
// - GetWidth(): Returns the width of the sprite in pixels. it is calculated based on the sprite atlas width and the UV rectangle.
// - GetHeight(): Returns the height of the sprite in pixels. its calculated based on the sprite atlas height and the UV rectangle.
// - GetSize(): Returns the size of the sprite as a SizeF structure containing width and height in pixels.
// - Bind(): Binds the sprite's texture for rendering by delegating to the sprite atlas.
// - CanBind(): Checks if the sprite's texture can be bound for rendering by delegating to the sprite atlas.
// - GetUVRect(): Returns the UV rectangle defining the portion of the sprite atlas used by this sprite.

#pragma once
#include <Spatial/ISizeable.h>
#include <Math/Rect.h>
#include <Core/View.h>
#include <Core/Bindable.h>
#include <memory>

namespace engine::graphics
{
	namespace resource
	{
		class SpriteAtlas;
		class ITexture;
	}

	namespace renderable
	{
		class RenderSurface;
	}

	// has pointer to sprite atlas 
	// has source rect (normalized rectangular coordinates or UV)
	// Sprite is sprite. Is is NOT a SpriteAtlas.It has a View SpriteAtlas instead.
	class Sprite : public engine::spatial::ISizeable<float>, public engine::core::IBindable
	{
	private:
		engine::math::geometry::RectF m_rect;
		engine::spatial::SizeF m_size;
		core::View<engine::graphics::resource::ITexture> m_view;
		engine::spatial::PositionF m_pivot;

		friend class engine::graphics::resource::SpriteAtlas;
		friend class engine::graphics::renderable::RenderSurface;

	protected:
		// use this constructor if you have the sprite atlas and the source rect
		Sprite(const engine::graphics::resource::ITexture* texture, const engine::math::geometry::RectF& rect, const engine::spatial::PositionF& pivot = { 0,0 });

	public:
		~Sprite() = default;

		// Sprite methods
		bool IsValid() const;
		engine::math::geometry::RectF GetUVRect() const;
		void SetPivot(const engine::spatial::PositionF& pos);
		engine::spatial::PositionF GetPivot() const;
		void SetPivotInPixels(const engine::spatial::PositionF& pixelPos);
		engine::spatial::PositionF GetPivotInPixels() const;

		// helper method to generate an "empty" or "invalid" sprite
		static Sprite MakeInvalidSprite();

		// IBindable methods
		void Bind() const override final;
		bool CanBind() const override final;

		// ISizeable methods implementation
		float GetWidth() const override final;
		float GetHeight() const override final;
		spatial::SizeF GetSize() const override final;

	};

}

