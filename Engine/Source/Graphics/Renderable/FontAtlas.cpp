#include <Win32/GDIUtility.h>
#include <Graphics/Resource/ITexture.h>
#include <Graphics/Renderable/FontAtlas.h>

graphics::renderable::FontAtlas::FontAtlas(std::unique_ptr<graphics::resource::ITexture> tex)
	: texture(std::move(tex))
{
}

void graphics::renderable::FontAtlas::Reset()
{
	texture->Reset();
	// TODO: glyphs are not populated yet
	glyphs.clear();
}

bool graphics::renderable::FontAtlas::Initialize(const std::string& fontName, const unsigned int fontSize)
{
	// generate font atlas bitmap data
	unsigned int width, height;
	unsigned int* srcData = nullptr;
	//std::vector<std::array<float, 4>> glyphNormalizedCoords;
	// ensure HDC is cleaned up when it goes out of scope
	utilities::OnOutOfScope cleanupHDC([=]
		{
			if (srcData != nullptr) delete[] srcData;
		});
	{
		// create a font atlas of specified font drawn in a bitmap array
		// also get the width and height of the bitmap so it will be used for creating texture of that size to draw the bitmap
		// also get the array of coords (normalized) that map each characters in the bitmap
		if (!Win32::GDIUtility::GenerateFontAtlas(&srcData, width, height, m_textNormalizedCoords, fontName, fontSize))
		{
			LOGERROR("Failed to create font resource.");
			return false;
		}
	}

	// TODO: convert mapped data into glyphs
	{

	}

	// initialize texture
	// load font atlas data into texture
	{
		if (!texture->Initialize(width, height, srcData, width * sizeof(unsigned int)))
		{
			LOGERROR("Failed to initialize FontAtlas' texture resource.");
			return false;
		}
	}

	return true;
}

// TODO: glyphs need to be populated during Initialize
const graphics::text::Glyph* graphics::renderable::FontAtlas::GetGlyph(const unsigned char character) const
{
	if (character < 32 || character > 127)
	{
		throw std::runtime_error("Invalid character in getting glyph.");
		LOGERROR("Invalid character in getting glyph. Character: " << character);
	}

	throw std::runtime_error("Glyphs are not populated yet in FontAtlas.");

	return &glyphs[character - 32];
}

bool graphics::renderable::FontAtlas::GetNormalizedTexCoord(const unsigned char character, float& u0, float& v0, float& u1, float& v1) const
{
	if (character < 32 || character > 127)
	{
		LOGERROR("Invalid character specified in getting texture coordinates. Integer value must be between 33 to 127. Specified value is " << std::to_string(character));
		return false;
	}
	u0 = m_textNormalizedCoords[character - 32][0]; // left
	v0 = m_textNormalizedCoords[character - 32][1]; // top
	u1 = m_textNormalizedCoords[character - 32][2]; // right 
	v1 = m_textNormalizedCoords[character - 32][3]; // bottom
	return true;
}

math::geometry::RectF graphics::renderable::FontAtlas::GetUVRect() const
{
	return math::geometry::RectF{ 0, 0, 1, 1 };
}

void graphics::renderable::FontAtlas::Bind() const
{
	texture->Bind();
}

bool graphics::renderable::FontAtlas::CanBind() const
{
	return texture->CanBind();
}

const float graphics::renderable::FontAtlas::GetWidth(const unsigned char character) const
{
	if (character < 32 || character > 127)
	{
		LOGERROR("Invalid character specified in getting texture coordinates. Integer value must be between 33 to 127. Specified value is " << std::to_string(character));
		throw std::exception("Invalid character specified in getting texture coordinates. Integer value must be between 33 to 127.");
	}
	float u0 = m_textNormalizedCoords[character - 32][0]; // left
	float u1 = m_textNormalizedCoords[character - 32][2]; // right 

	return texture->GetWidth() * (u1 - u0);
}

const float graphics::renderable::FontAtlas::GetHeight(const unsigned char character) const
{
	if (character < 32 || character > 127)
	{
		LOGERROR("Invalid character specified in getting texture coordinates. Integer value must be between 33 to 127. Specified value is " << std::to_string(character));
		throw std::exception("Invalid character specified in getting texture coordinates. Integer value must be between 33 to 127.");
	}

	float v0 = m_textNormalizedCoords[character - 32][1]; // top
	float v1 = m_textNormalizedCoords[character - 32][3]; // bottom

	return texture->GetHeight() * (v1 - v0);
}

const float graphics::renderable::FontAtlas::GetWidth(const std::string& text) const
{
	float total = 0.0f;
	for (unsigned char c : text)
	{
		if (c < 32 || c > 127)
			continue; // or handle error/logging

		total += GetWidth(c);
		// optionally add advance/kerning if your Glyph stores it
	}
	return total;
}

float graphics::renderable::FontAtlas::GetWidth() const
{
	return static_cast<float>(texture->GetWidth());
}

float graphics::renderable::FontAtlas::GetHeight() const
{
	return static_cast<float>(texture->GetHeight());
}

spatial::SizeF graphics::renderable::FontAtlas::GetSize() const
{
	return spatial::SizeF
	{
		static_cast<float>(texture->GetWidth()),
		static_cast<float>(texture->GetHeight())
	};
}

