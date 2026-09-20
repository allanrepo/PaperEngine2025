#include <Graphics/Resource/FontAtlas.h>
#include <Graphics/Core/Font.h>

engine::graphics::Font::Font(const engine::graphics::resource::IFontAtlas* fontAtlas)
	: m_view(fontAtlas)
{
}

engine::graphics::Sprite engine::graphics::Font::GetGlyph(const unsigned char character) const
{
	return m_view->GetGlyph(character);
}

engine::graphics::Sprite  engine::graphics::Font::GetSprite() const
{
	return m_view->GetSprite();
}

const float  engine::graphics::Font::GetWidth(const std::string& text) const
{
	return m_view->GetWidth(text);
}

const engine::math::SizeF  engine::graphics::Font::GetSize(const std::string& text) const
{
	return m_view->GetSize(text);
}

engine::graphics::Font engine::graphics::Font::MakeInvalidFont()
{
	return Font(nullptr);
}

bool  engine::graphics::Font::operator==(const Font& other) const
{
	return m_view == other.m_view;
}

bool  engine::graphics::Font::operator!=(const Font& other) const
{
	return !(*this == other);
}

float  engine::graphics::Font::GetWidth() const 
{
	return m_view->GetWidth();
}

float  engine::graphics::Font::GetHeight() const 
{
	return m_view->GetHeight();
}

engine::math::SizeF engine::graphics::Font::GetSize() const  
{
	return m_view->GetSize();
}

bool engine::graphics::Font::IsValid() const
{
	return m_view.IsValid();
}