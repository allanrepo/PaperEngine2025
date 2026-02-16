#pragma once
#include <memory>
#include <vector>
#include <Graphics/Resource/ISpriteAtlas.h>

// forward declare
namespace engine::graphics
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

namespace engine::graphics::resource
{
	class SpriteAtlas : public engine::graphics::resource::ISpriteAtlas
	{
	private:
		friend class ::engine::graphics::factory::SpriteAtlasFactory;

		std::shared_ptr<::engine::graphics::resource::ITexture> m_texture;
		std::vector<engine::math::geometry::RectF> m_nUVs;

	public:
		SpriteAtlas(std::unique_ptr<::engine::graphics::resource::ITexture> tex);
		virtual ~SpriteAtlas() = default;

		// ISpriteAtlas methods implementation
		virtual bool Initialize(const wchar_t* fileNamePath) override final;
		virtual void AddUVRect(const engine::math::geometry::RectF& rect) override final;
		virtual void AddUVRects(const std::vector<engine::math::geometry::RectF>& rects) override final;
		virtual const inline engine::math::geometry::RectF GetUVRect(int index) const override final;
		virtual inline size_t GetUVRectCount() const override final;
		virtual engine::graphics::renderable::Sprite MakeSprite(int index) const override final;
		virtual engine::graphics::renderable::Sprite GetSprite() const override final;

		// ISizeable methods implementation
		virtual float GetWidth() const override final;
		virtual float GetHeight() const override final;
		virtual spatial::SizeF GetSize() const override final;

		// IBindable methods implementation
		virtual void Bind() const override final;
		virtual bool CanBind() const override final;
	};
}

