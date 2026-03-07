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
#include <Graphics/Renderable/IRenderable.h>
#include <Spatial/ISizeable.h>
#include <Core/View.h>
#include <memory>

namespace engine::graphics
{
	namespace resource
	{
		class ISpriteAtlas;
		class SpriteAtlas;
	}

	namespace renderable
	{
		// is renderable
		// has pointer to sprite atlas 
		// has source rect (normalized rectangular coordinates or UV)
		// Sprite is sprite. Is is NOT a SpriteAtlas.It has a View SpriteAtlas instead.
		// 
		// design consideration: 
		// change View<T> as composition instead of inheritance. in inheritance, Sprite is behaving like SpriteAtlas
		// View<T> -> operator allows Sprite to call SpriteAtlas methods. making View<T> protected prevents that but it is visible in intellisense
		// that is confusing. Sprite is sprite. Is is NOT a SpriteAtlas.It has a View SpriteAtlas instead.
		class Sprite : public engine::graphics::renderable::IRenderable, public spatial::ISizeable<float>
		{
		private:
			engine::math::geometry::RectF m_rect;
			engine::spatial::SizeF m_size;
			core::View<engine::graphics::resource::ISpriteAtlas> m_view;
			engine::spatial::PositionF m_anchor;

			friend class engine::graphics::resource::SpriteAtlas;
			friend class engine::graphics::resource::ISpriteAtlas;

		protected:
			// use this constructor if you have the sprite atlas and the source rect
			//Sprite(const engine::graphics::resource::ISpriteAtlas* spriteAtlas, engine::math::geometry::RectF rect);
			Sprite(const engine::graphics::resource::ISpriteAtlas* spriteAtlas, const engine::math::geometry::RectF& rect, const engine::spatial::PositionF& anchor = { 0,0 });

		public:
			~Sprite() = default;

			inline bool IsValid() const
			{
				return m_view.IsValid();
			}

			// ISizeable methods implementation
			virtual float GetWidth() const override final;
			virtual float GetHeight() const override final;
			virtual spatial::SizeF GetSize() const override final;

			// IRenderable methods implementation
			virtual void Bind() const override final;
			virtual bool CanBind() const override final;
			virtual engine::math::geometry::RectF GetUVRect() const override final;
			void SetAnchor(const engine::spatial::PositionF& pos) override final;
			engine::spatial::PositionF GetAnchor() const override final;
		};
	}


}

