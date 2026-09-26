#pragma once
#include <Spatial/ISizeable.h>
#include <Core/View.h>

namespace engine
{
	namespace graphics
	{
		class Sprite;

		namespace resource
		{
			class FontAtlas;
			class IFontAtlas;
		}

		class Font : public engine::spatial::ISizeable<float>
		{
		private:
			friend class engine::graphics::resource::FontAtlas;

			core::View<engine::graphics::resource::IFontAtlas> m_view;

			// only FontAtlas can create Fonts.
			Font(const engine::graphics::resource::IFontAtlas* fontAtlas);

		public:
			// get an instance of a glyph in sprite form
			engine::graphics::Sprite GetGlyph(const unsigned char character) const;

			// get an instance of the actual font atlas image resource in sprite form
			engine::graphics::Sprite GetSprite() const;

			// get size of a string
			const float GetWidth(const std::string& text) const;

			// get size of a string
			const engine::math::SizeF GetSize(const std::string& text) const;

			// used for creating default font to initialize a font object with no font atlas reference
			static Font MakeInvalidFont();

			// operators
			bool operator==(const Font& other) const;
			bool operator!=(const Font& other) const;

			bool IsValid() const;

			// ISizeable methods
			float GetWidth() const override final;
			float GetHeight() const override final;
			engine::math::SizeF GetSize() const override final;

		};
	}
}