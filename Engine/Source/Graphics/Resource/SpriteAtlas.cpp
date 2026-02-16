#include <Graphics/Resource/SpriteAtlas.h>
#include <Graphics/Resource/ITexture.h>
#include <Utilities/Logger.h>
#include <Graphics/Renderable/Sprite.h>

graphics::resource::SpriteAtlas::SpriteAtlas(std::unique_ptr<::graphics::resource::ITexture> tex)
	:m_texture(std::move(tex))
{
}

bool graphics::resource::SpriteAtlas::Initialize(const wchar_t* fileNamePath)
{
	if (!m_texture->Initialize(fileNamePath))
	{
		LOGERROR("Failed to initialize SpriteSheet' texture resource.");
		return false;
	}

	return true;
}

void graphics::resource::SpriteAtlas::AddUVRect(const math::geometry::RectF& rect)
{
	m_nUVs.push_back(rect);
}

void graphics::resource::SpriteAtlas::AddUVRects(const std::vector<math::geometry::RectF>& rects)
{
	m_nUVs.insert(m_nUVs.end(), rects.begin(), rects.end());
}

const math::geometry::RectF graphics::resource::SpriteAtlas::GetUVRect(int index) const
{
	return m_nUVs[index];
}

inline size_t graphics::resource::SpriteAtlas::GetUVRectCount() const
{
	return m_nUVs.size();
}

graphics::renderable::Sprite graphics::resource::SpriteAtlas::MakeSprite(int index) const
{
	if(index < 0 || index >= static_cast<int>(m_nUVs.size()))
	{
		throw std::runtime_error("SpriteAtlas::MakeSprite - invalid index");
	}

	return graphics::renderable::Sprite(this, GetUVRect(index));
}

graphics::renderable::Sprite graphics::resource::SpriteAtlas::GetSprite() const
{
	return graphics::renderable::Sprite(this, math::geometry::RectF{ 0, 0, 1, 1 });
}

void graphics::resource::SpriteAtlas::Bind() const
{
	return m_texture->Bind();
}

bool graphics::resource::SpriteAtlas::CanBind() const
{
	return m_texture->CanBind();
}

float graphics::resource::SpriteAtlas::GetWidth() const
{
	return static_cast<float>(m_texture->GetWidth());
}

float graphics::resource::SpriteAtlas::GetHeight() const
{
	return static_cast<float>(m_texture->GetHeight());
}

spatial::SizeF graphics::resource::SpriteAtlas::GetSize() const
{
	return spatial::SizeF{
		static_cast<float>(m_texture->GetWidth()),
		static_cast<float>(m_texture->GetHeight())
	};
}

