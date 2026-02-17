#pragma once
#include <Graphics/Renderable/Sprite.h>
#include <Spatial/Size.h>
#include <Spatial/ISizeable.h>
#include <string>

using namespace engine::spatial;

namespace engine
{
	namespace graphics
	{
		namespace resource
		{
			class IFontAtlas : public spatial::ISizeable<float>
			{
			protected:

			public:
				virtual ~IFontAtlas() = default;

				virtual bool Initialize(const std::string& fontName = "Arial", const unsigned int fontSize = 12) = 0;

				virtual void Reset() = 0;

				virtual engine::graphics::renderable::Sprite GetGlyph(const unsigned char character) const = 0;

				virtual engine::graphics::renderable::Sprite GetSprite() const = 0;

				// get size of characters
				virtual const float GetWidth(const unsigned char character) const = 0;
				virtual const float GetHeight(const unsigned char character) const = 0;

				// get size of a string
				virtual const float GetWidth(const std::string& text) const = 0;

				// ISizeable methods
				virtual float GetWidth() const = 0;
				virtual float GetHeight() const = 0;
				virtual SizeF GetSize() const = 0;
			};
		}
	}
}

