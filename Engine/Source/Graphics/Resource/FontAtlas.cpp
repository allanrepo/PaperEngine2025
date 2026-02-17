#include <Graphics/Resource/FontAtlas.h>
#include <Utilities/Utilities.h>
#include <Utilities/Logger.h>
#include <Win32/GDIUtility.h>
#include <Math/Rect.h>
#include <vector>

//using namespace engine::graphics::resource;
using namespace std;
using namespace engine::spatial;

engine::graphics::resource::FontAtlas::FontAtlas(std::unique_ptr<engine::graphics::resource::ISpriteAtlas> spriteAtlas):
	m_spriteAtlas(std::move(spriteAtlas))
{

}

bool engine::graphics::resource::FontAtlas::Initialize(const string& fontName, const unsigned int fontSize)
{

	// ensure source data is cleaned up when it goes out of scope
	unsigned int* srcData = nullptr;
	engine::utilities::OnOutOfScope cleanupSource([=]
		{
			if (srcData != nullptr) delete[] srcData;
		});

	unsigned int width, height; //  this will be the width and height of the bitmap where the text will be rendered
	std::vector<engine::math::geometry::RectF> textUVs; // glyph UVs (normalized coordinates of characters in bitmap
	{
		// generate font atlas bitmap data
		// create a font atlas of specified font drawn in a bitmap array
		// also get the width and height of the bitmap so it will be used for creating texture of that size to draw the bitmap
		// also get the array of coords (normalized) that map each characters in the bitmap
		if (!engine::win32::GDIUtility::GenerateFontAtlas(&srcData, width, height, textUVs, fontName, fontSize))
		{
			LOGERROR("Failed to create font resource.");
			return false;
		}
	}

	// initialize texture
	// load font atlas data into texture
	{
		if (!m_spriteAtlas->Initialize(width, height, srcData, width * sizeof(unsigned int)))
		{
			LOGERROR("Failed to initialize SpriteAtlas resource.");
			return false;
		}
	}

	// load text UVs. this list should contain all 96 ASCII characters decimal value from 32-127.
	m_spriteAtlas->AddUVRects(textUVs);

	// create glyphs (sprites) for each ASCII character and store 
	m_glyphs.clear();
	m_glyphs.reserve(100);
	for (int i = 0; i < m_spriteAtlas->GetUVRectCount(); i++)
	{
		m_glyphs.push_back(m_spriteAtlas->MakeSprite(i));
	}

	return true;
}

engine::graphics::renderable::Sprite engine::graphics::resource::FontAtlas::GetGlyph(const unsigned char character) const
{
	if (character < 32 || character > 127)
	{
		LOGERROR("Invalid character specified in getting texture coordinates. Integer value must be between 33 to 127. Specified value is " << std::to_string(character));
		throw std::runtime_error("Invalid character specified in getting texture coordinates. Integer value must be between 33 to 127");
	}

	return m_glyphs[character - 32];
}

engine::graphics::renderable::Sprite engine::graphics::resource::FontAtlas::GetSprite() const
{
	return m_spriteAtlas->GetSprite();
}


void engine::graphics::resource::FontAtlas::Reset()
{
	// TODO: sprite atlas don't have Reset(). if it does, call it here.
	m_glyphs.clear();
}

// get size of characters
const float engine::graphics::resource::FontAtlas::GetWidth(const unsigned char character) const
{
	if (character < 32 || character > 127)
	{
		LOGERROR("Invalid character specified in getting texture coordinates. Integer value must be between 33 to 127. Specified value is " << std::to_string(character));
		throw std::runtime_error("Invalid character specified in getting texture coordinates. Integer value must be between 33 to 127.");
	}

	return m_glyphs[character - 32].GetWidth();
}

const float engine::graphics::resource::FontAtlas::GetHeight(const unsigned char character) const
{
	if (character < 32 || character > 127)
	{
		LOGERROR("Invalid character specified in getting texture coordinates. Integer value must be between 33 to 127. Specified value is " << std::to_string(character));
		throw std::runtime_error("Invalid character specified in getting texture coordinates. Integer value must be between 33 to 127.");
	}

	return m_glyphs[character - 32].GetHeight();
}

// get size of a string
const float engine::graphics::resource::FontAtlas::GetWidth(const std::string& text) const
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

// ISizeable methods
float engine::graphics::resource::FontAtlas::GetWidth() const
{
	return static_cast<float>(m_spriteAtlas->GetWidth());
}

float engine::graphics::resource::FontAtlas::GetHeight() const
{
	return static_cast<float>(m_spriteAtlas->GetHeight());
}

SizeF engine::graphics::resource::FontAtlas::GetSize() const
{
	return spatial::SizeF
	{
		static_cast<float>(m_spriteAtlas->GetWidth()),
		static_cast<float>(m_spriteAtlas->GetHeight())
	};
}
