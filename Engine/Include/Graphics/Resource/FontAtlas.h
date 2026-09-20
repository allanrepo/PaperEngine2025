#pragma once
#include <Graphics/Resource/IFontAtlas.h>
#include <Graphics/Resource/ISpriteAtlas.h>

using namespace engine::spatial;

namespace engine
{
	namespace graphics
	{
		class Font;

		namespace resource
		{
			class FontAtlas : public IFontAtlas
			{
			protected:
				std::unique_ptr<engine::graphics::resource::ISpriteAtlas> m_spriteAtlas;
				std::vector<engine::graphics::Sprite> m_glyphs;
				
			public:
				FontAtlas(std::unique_ptr<engine::graphics::resource::ISpriteAtlas> spriteAtlas);
				virtual ~FontAtlas() = default;

				// resource management
				bool Initialize(const std::string& fontName = "Arial", const size_t fontSize = 12) override final;
				void Reset() override final;

				// get an instance of a glyph in sprite form
				engine::graphics::Sprite GetGlyph(const unsigned char character) const override final;
				
				// get an instance of the actual font atlas image resource in sprite form
				engine::graphics::Sprite GetSprite() const override final;

				// get size of characters
				const float GetWidth(const unsigned char character) const override final;
				const float GetHeight(const unsigned char character) const override final;

				// get size of a string
				const float GetWidth(const std::string& text) const override final;
				const engine::math::SizeF GetSize(const std::string& text) const override final;

				// ISizeable methods
				float GetWidth() const override final;
				float GetHeight() const override final;
				engine::math::SizeF GetSize() const override final;

				// get a view of font atlas
				engine::graphics::Font MakeFont() const override final;
			};
		}
	}
}

