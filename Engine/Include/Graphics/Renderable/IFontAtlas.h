#pragma once
#include <Graphics/Renderable/IRenderable.h>
#include <Spatial/ISizeable.h>
#include <Spatial/Size.h>
#include <string>

namespace graphics
{
	namespace text
	{
		struct Glyph
		{
			math::geometry::RectF normUV;
			spatial::SizeF size;
		};

	}
	namespace renderable
	{
		class IFontAtlas : public graphics::renderable::IRenderable, public spatial::ISizeable<float>
		{
		protected:

		public:
			virtual ~IFontAtlas() = default;

			virtual bool Initialize(const std::string& fontName = "Arial", const unsigned int fontSize = 12) = 0;

			virtual void Reset() = 0;

			virtual const text::Glyph* GetGlyph(const unsigned char character) const = 0;

			// delete this 
			virtual bool GetNormalizedTexCoord(const unsigned char character, float& u0, float& v0, float& u1, float& v1) const = 0;

			virtual const float GetWidth(const unsigned char character) const = 0;
			virtual const float GetHeight(const unsigned char character) const = 0;

			// need to declare this IRenderable methods here because IFontAtlas has the same method name. it's a C++ thing.
			virtual float GetWidth() const = 0;
			virtual float GetHeight() const = 0;
			virtual spatial::SizeF GetSize() const = 0;
		};
	}
}