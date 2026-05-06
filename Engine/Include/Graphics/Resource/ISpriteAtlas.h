#pragma once
#include <Spatial/ISizeable.h>
#include <Math/Rect.h>
#include <vector>
#include <Core/Bindable.h>
#include <Graphics/Resource/ITexture.h>

namespace engine
{
	namespace graphics
	{
		class Sprite;

		namespace resource
		{
			class ITexture;

			class ISpriteAtlas : public spatial::ISizeable<float>// , public engine::graphics::renderable::IRenderable<>
			{

			public:
				virtual ~ISpriteAtlas() = default;
				virtual bool Initialize(const wchar_t* fileNamePath) = 0;
				virtual bool Initialize(unsigned int width, unsigned int height, const void* srcData, unsigned int bytesPerRow) = 0;
				virtual void AddUVRect(const engine::math::geometry::RectF& rect) = 0;
				virtual void AddUVRects(const std::vector<engine::math::geometry::RectF>& rects) = 0;
				virtual inline const engine::math::geometry::RectF GetUVRect(int index) const = 0;
				virtual inline size_t GetUVRectCount() const = 0;
				virtual engine::graphics::Sprite MakeSprite(int index, const engine::spatial::PositionF& pivot = {0,0}) const = 0;
				virtual void Reset() = 0;
				virtual const engine::graphics::Sprite GetSprite() const = 0;
				virtual bool IsValid() const = 0;
			};
		}
	}
}

