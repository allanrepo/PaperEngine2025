#pragma once
#include <Graphics/Renderable/IFontAtlas.h>
#include <string>
#include <memory>
#include <vector>

namespace graphics
{
	namespace resource
	{
		class ITexture;
	}
}

namespace graphics::renderable
{
	class FontAtlas : public graphics::renderable::IFontAtlas
	{
	private:
		std::unique_ptr<graphics::resource::ITexture> texture;

		// TODO: glyphs are not populated yet. it is also not defined yet. its data structure is defined in Text/Glyph.h but it may not be final
		std::vector<text::Glyph> glyphs;
		std::vector<std::array<float, 4>> m_textNormalizedCoords;

	public:
		FontAtlas(std::unique_ptr<graphics::resource::ITexture> tex);

		FontAtlas() = default;
		virtual ~FontAtlas() = default;

		// cannot be copied
		FontAtlas(const FontAtlas&) = delete;
		FontAtlas& operator=(const FontAtlas&) = delete;

		virtual void Reset() override final;

		virtual bool Initialize(const std::string& fontName = "Arial", const unsigned int fontSize = 12) override final;

		virtual const text::Glyph* GetGlyph(const unsigned char character) const override final;

		virtual bool GetNormalizedTexCoord(const unsigned char character, float& u0, float& v0, float& u1, float& v1) const override final;

		virtual const float GetWidth(const unsigned char character) const override final;
		virtual const float GetHeight(const unsigned char character) const override final;
		virtual const float GetWidth(const std::string& text) const override final;

		// ISizeable methods implementation
		virtual float GetWidth() const override final;
		virtual float GetHeight() const override final;
		virtual spatial::SizeF GetSize() const override final;

		// IRenderable methods implementation
		virtual math::geometry::RectF GetUVRect() const override final;
		virtual void Bind() const override final;
		virtual bool CanBind() const override final;
	};
}
