#include <Graphics/Resource/SpriteAtlas.h>
#include <Graphics/Resource/ITexture.h>
#include <Utilities/Logger.h>
#include <Graphics/Core/Sprite.h>

engine::graphics::resource::SpriteAtlas::SpriteAtlas(std::unique_ptr<::engine::graphics::resource::ITexture> tex)
	:m_texture(std::move(tex))
{
}

bool engine::graphics::resource::SpriteAtlas::Initialize(const wchar_t* fileNamePath)
{
	if (!m_texture->Initialize(fileNamePath))
	{
		LOGERROR("Failed to initialize SpriteAtlas' texture resource.");
		return false;
	}

	return true;
}

bool engine::graphics::resource::SpriteAtlas::Initialize(unsigned int width, unsigned int height, const void* srcData, unsigned int bytesPerRow)
{
	if (!m_texture->Initialize(width, height, srcData, bytesPerRow))
	{
		LOGERROR("Failed to initialize SpriteAtlas' texture resource.");
		return false;
	}
	return true;
}

void engine::graphics::resource::SpriteAtlas::AddUVRect(const engine::math::geometry::RectF& rect)
{
	m_nUVs.push_back(rect);
}

void engine::graphics::resource::SpriteAtlas::AddUVRects(const std::vector<engine::math::geometry::RectF>& rects)
{
	m_nUVs.insert(m_nUVs.end(), rects.begin(), rects.end());
}

const engine::math::geometry::RectF engine::graphics::resource::SpriteAtlas::GetUVRect(int index) const
{
	return m_nUVs[index];
}

inline size_t engine::graphics::resource::SpriteAtlas::GetUVRectCount() const
{
	return m_nUVs.size();
}

engine::graphics::Sprite engine::graphics::resource::SpriteAtlas::MakeSprite(int index, const engine::spatial::PositionF& anchor) const
{
	if(index < 0 || index >= static_cast<int>(m_nUVs.size()))
	{
		throw std::runtime_error("SpriteAtlas::MakeSprite - invalid index");
	}

	return engine::graphics::Sprite(m_texture.get(), GetUVRect(index), anchor);
}

const engine::graphics::Sprite engine::graphics::resource::SpriteAtlas::GetSprite() const
{
	return engine::graphics::Sprite(m_texture.get(), engine::math::geometry::RectF{0, 0, 1, 1});
}

bool engine::graphics::resource::SpriteAtlas::IsValid() const
{
	return m_texture->IsValid();
}

void engine::graphics::resource::SpriteAtlas::Reset()
{
	m_texture->Reset();
}

float engine::graphics::resource::SpriteAtlas::GetWidth() const
{
	return static_cast<float>(m_texture->GetWidth());
}

float engine::graphics::resource::SpriteAtlas::GetHeight() const
{
	return static_cast<float>(m_texture->GetHeight());
}

engine::spatial::SizeF engine::graphics::resource::SpriteAtlas::GetSize() const
{
	return spatial::SizeF{
		static_cast<float>(m_texture->GetWidth()),
		static_cast<float>(m_texture->GetHeight())
	};
}

