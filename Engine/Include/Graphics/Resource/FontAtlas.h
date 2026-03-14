#pragma once
#include <Graphics/Resource/IFontAtlas.h>
#include <Graphics/Resource/ISpriteAtlas.h>

using namespace engine::spatial;

namespace engine
{
	namespace graphics
	{
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

				bool Initialize(const std::string& fontName = "Arial", const unsigned int fontSize = 12) override final;

				void Reset() override final;

				engine::graphics::Sprite GetGlyph(const unsigned char character) const override final;
				engine::graphics::Sprite GetSprite() const override final;

				// get size of characters
				const float GetWidth(const unsigned char character) const override final;
				const float GetHeight(const unsigned char character) const override final;

				// get size of a string
				const float GetWidth(const std::string& text) const override final;

				// ISizeable methods
				float GetWidth() const override final;
				float GetHeight() const override final;
				SizeF GetSize() const override final;
			};
		}
	}
}