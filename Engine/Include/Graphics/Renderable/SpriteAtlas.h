#pragma once
#include <memory>
#include <vector>
#include <Graphics/Renderable/ISpriteAtlas.h>

// forward declare
namespace graphics
{
	namespace factory
	{
		class SpriteAtlasFactory;
	}
	namespace resource
	{
		class Sprite;
		class ITexture;
	}
}

namespace graphics::renderable
{
	class SpriteAtlas : public graphics::renderable::ISpriteAtlas
	{
	private:
		friend class graphics::factory::SpriteAtlasFactory;

		std::shared_ptr<graphics::resource::ITexture> m_texture;
		std::vector<math::geometry::RectF> m_nUVs;

	public:
		SpriteAtlas(std::unique_ptr<graphics::resource::ITexture> tex);
		virtual ~SpriteAtlas() = default;

		// ISpriteAtlas methods implementation
		virtual bool Initialize(const wchar_t* fileNamePath) override final;
		virtual void AddUVRect(math::geometry::RectF rect) override final;
		virtual const inline math::geometry::RectF GetUVRect(int index) const override final;
		virtual inline size_t GetUVRectCount() const override final;
		virtual graphics::renderable::Sprite MakeSprite(int index) const override final;

		// ISizeable methods implementation
		virtual float GetWidth() const override final;
		virtual float GetHeight() const override final;
		virtual spatial::SizeF GetSize() const override final;

		// IRenderable methods implementation
		virtual math::geometry::RectF GetUVRect() const override final;
		virtual void Bind() const override final;
		virtual bool CanBind() const override final;
	};
}

