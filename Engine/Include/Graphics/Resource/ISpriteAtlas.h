#pragma once
#include <Graphics/Renderable/Sprite.h>
#include <Spatial/ISizeable.h>
#include <vector>
#include <Core/Bindable.h>

//namespace engine
//{
	namespace graphics
	{
		namespace resource
		{
			class ISpriteAtlas : public core::IBindable, public spatial::ISizeable<float>
			{

			public:
				virtual ~ISpriteAtlas() = default;
				virtual bool Initialize(const wchar_t* fileNamePath) = 0;
				virtual void AddUVRect(const math::geometry::RectF& rect) = 0;
				virtual void AddUVRects(const std::vector<math::geometry::RectF>& rects) = 0;
				virtual inline const math::geometry::RectF GetUVRect(int index) const = 0;
				virtual inline size_t GetUVRectCount() const = 0;
				virtual ::graphics::renderable::Sprite MakeSprite(int index) const = 0;
				virtual ::graphics::renderable::Sprite GetSprite() const = 0;

			};
		}
	}
//}

