#pragma once
#include <memory>
#include <vector>
#include <Graphics/Resource/ISpriteAtlas.h>
#include <Spatial/Position.h>

namespace engine::graphics
{
	namespace factory
	{
		class SpriteAtlasFactory;
	}
	namespace renderable
	{
		class Sprite;
	}

	namespace resource
	{
		class ITexture;

		class SpriteAtlas : public engine::graphics::resource::ISpriteAtlas
		{
		private:
			friend class ::engine::graphics::factory::SpriteAtlasFactory;

			std::unique_ptr<engine::graphics::resource::ITexture> m_texture;
			std::vector<engine::math::geometry::RectF> m_nUVs;

		public:
			SpriteAtlas(std::unique_ptr<engine::graphics::resource::ITexture> tex);
			virtual ~SpriteAtlas() = default;

			// ISpriteAtlas methods implementation
			virtual bool Initialize(const wchar_t* fileNamePath) override final;
			bool Initialize(unsigned int width, unsigned int height, const void* srcData, unsigned int bytesPerRow) override final;
			virtual void AddUVRect(const engine::math::geometry::RectF& rect) override final;
			virtual void AddUVRects(const std::vector<engine::math::geometry::RectF>& rects) override final;
			virtual const inline engine::math::geometry::RectF GetUVRect(int index) const override final;
			virtual inline size_t GetUVRectCount() const override final;
			virtual engine::graphics::Sprite MakeSprite(int index, const engine::spatial::PositionF& anchor = {0,0}) const override final;
			void Reset() override final;
			const engine::graphics::Sprite GetSprite() const override final;
			bool IsValid() const override final;

			// ISizeable methods implementation
			virtual float GetWidth() const override final;
			virtual float GetHeight() const override final;
			virtual spatial::SizeF GetSize() const override final;


		};

	}
}

