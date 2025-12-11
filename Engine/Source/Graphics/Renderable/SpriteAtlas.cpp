#include <Graphics/Renderable/SpriteAtlas.h>
#include <Graphics/Resource/ITexture.h>
#include <Utilities/Logger.h>
#include <Graphics/Renderable/Sprite.h>

graphics::renderable::SpriteAtlas::SpriteAtlas(std::unique_ptr<graphics::resource::ITexture> tex)
	:m_texture(std::move(tex))
{
}

bool graphics::renderable::SpriteAtlas::Initialize(const wchar_t* fileNamePath)
{
	if (!m_texture->Initialize(fileNamePath))
	{
		LOGERROR("Failed to initialize SpriteSheet' texture resource.");
		return false;
	}

	return true;
}

void graphics::renderable::SpriteAtlas::AddUVRect(math::geometry::RectF rect)
{
	m_nUVs.push_back(rect);
}

const math::geometry::RectF graphics::renderable::SpriteAtlas::GetUVRect(int index) const
{
	return m_nUVs[index];
}

inline size_t graphics::renderable::SpriteAtlas::GetUVRectCount() const
{
	return m_nUVs.size();
}

graphics::renderable::Sprite graphics::renderable::SpriteAtlas::MakeSprite(int index) const
{
	if(index < 0 || index >= static_cast<int>(m_nUVs.size()))
	{
		throw std::runtime_error("SpriteAtlas::MakeSprite - invalid index");
	}

	return graphics::renderable::Sprite(this, GetUVRect(index));
}


math::geometry::RectF graphics::renderable::SpriteAtlas::GetUVRect() const
{
	return math::geometry::RectF{ 0, 0, 1, 1 };
}

void graphics::renderable::SpriteAtlas::Bind() const
{
	return m_texture->Bind();
}

bool graphics::renderable::SpriteAtlas::CanBind() const
{
	return m_texture->CanBind();
}

float graphics::renderable::SpriteAtlas::GetWidth() const
{
	return static_cast<float>(m_texture->GetWidth());
}

float graphics::renderable::SpriteAtlas::GetHeight() const
{
	return static_cast<float>(m_texture->GetHeight());
}

spatial::SizeF graphics::renderable::SpriteAtlas::GetSize() const
{
	return spatial::SizeF{
		static_cast<float>(m_texture->GetWidth()),
		static_cast<float>(m_texture->GetHeight())
	};
}

