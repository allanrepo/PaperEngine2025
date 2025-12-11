#include <Graphics/Renderable/Sprite.h>
#include <Graphics/Renderable/ISpriteAtlas.h>
#include <Graphics/Renderable/SpriteAtlas.h>


graphics::renderable::Sprite::Sprite(const graphics::renderable::ISpriteAtlas* spriteAtlas, math::geometry::RectF rect) :
	View<graphics::renderable::ISpriteAtlas>(spriteAtlas),
	m_rect(rect)
{
}

void graphics::renderable::Sprite::Bind() const
{
	m_data->Bind();
}

bool graphics::renderable::Sprite::CanBind() const
{
	return m_data->CanBind();
}

math::geometry::RectF graphics::renderable::Sprite::GetUVRect() const
{
	return m_rect;
}

float graphics::renderable::Sprite::GetWidth() const
{
	return m_data->GetWidth()*(m_rect.right - m_rect.left);
}

float graphics::renderable::Sprite::GetHeight() const
{
	return m_data->GetHeight()*(m_rect.bottom - m_rect.top);
}

spatial::SizeF graphics::renderable::Sprite::GetSize() const
{
	return spatial::SizeF{
		m_data->GetWidth()* (m_rect.right - m_rect.left),
		m_data->GetHeight()* (m_rect.bottom - m_rect.top)
	};
}

