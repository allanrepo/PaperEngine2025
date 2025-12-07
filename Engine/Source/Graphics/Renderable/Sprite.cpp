#include <Graphics/Renderable/Sprite.h>
#include <Graphics/Renderable/ISpriteAtlas.h>

graphics::renderable::Sprite::Sprite(const graphics::renderable::ISpriteAtlas* spriteAtlas, math::geometry::RectF rect) :
	m_rect(rect),
	m_spriteAtlas(spriteAtlas)
{
}

void graphics::renderable::Sprite::Bind() const
{
	m_spriteAtlas->Bind();
}

bool graphics::renderable::Sprite::CanBind() const
{
	return m_spriteAtlas->CanBind();
}

math::geometry::RectF graphics::renderable::Sprite::GetUVRect() const
{
	return m_rect;
}

const float graphics::renderable::Sprite::GetWidth() const
{
	return m_spriteAtlas->GetWidth()*(m_rect.right - m_rect.left);
}

const float graphics::renderable::Sprite::GetHeight() const
{
	return m_spriteAtlas->GetHeight()*(m_rect.bottom - m_rect.top);
}

const spatial::SizeF graphics::renderable::Sprite::GetSize() const
{
	return spatial::SizeF{
		m_spriteAtlas->GetWidth()* (m_rect.right - m_rect.left),
		m_spriteAtlas->GetHeight()* (m_rect.bottom - m_rect.top)
	};
}


