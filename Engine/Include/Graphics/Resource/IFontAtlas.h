#pragma once
#include <Graphics/Core/Sprite.h>
#include <Math/Size.h>
#include <Spatial/ISizeable.h>
#include <string>

using namespace engine::spatial;

namespace engine
{
	namespace graphics
	{
		class Font;

		namespace resource
		{
			class IFontAtlas : public spatial::ISizeable<float>
			{
			protected:

			public:
				virtual ~IFontAtlas() = default;

				// resource management
				virtual bool Initialize(const std::string& fontName = "Arial", const size_t fontSize = 12) = 0;
				virtual void Reset() = 0;

				// get an instance of a glyph in sprite form
				virtual engine::graphics::Sprite GetGlyph(const unsigned char character) const = 0;

				// get an instance of the actual font atlas image resource in sprite form
				virtual engine::graphics::Sprite GetSprite() const = 0;

				// get size of characters
				virtual const float GetWidth(const unsigned char character) const = 0;
				virtual const float GetHeight(const unsigned char character) const = 0;

				// get size of a string
				virtual const float GetWidth(const std::string& text) const = 0;
				virtual const engine::math::SizeF GetSize(const std::string& text) const = 0;

				// ISizeable methods
				virtual float GetWidth() const = 0;
				virtual float GetHeight() const = 0;
				virtual engine::math::SizeF GetSize() const = 0;

				// get a view of font atlas
				virtual engine::graphics::Font MakeFont() const = 0;
			};
		}
	}
}

