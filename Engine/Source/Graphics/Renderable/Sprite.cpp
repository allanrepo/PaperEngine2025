#include <Graphics/Renderable/Sprite.h>
#include <Graphics/Resource/ISpriteAtlas.h>
#include <Graphics/Resource/SpriteAtlas.h>


engine::graphics::renderable::Sprite::Sprite(const engine::graphics::resource::ISpriteAtlas* spriteAtlas, math::geometry::RectF rect) :
	View<engine::graphics::resource::ISpriteAtlas>(spriteAtlas),
	m_rect(rect)
{
}

void engine::graphics::renderable::Sprite::Bind() const
{
	m_data->Bind();
}

bool engine::graphics::renderable::Sprite::CanBind() const
{
	return m_data->CanBind();
}

math::geometry::RectF engine::graphics::renderable::Sprite::GetUVRect() const
{
	return m_rect;
}

float engine::graphics::renderable::Sprite::GetWidth() const
{
	return m_data->GetWidth()*(m_rect.right - m_rect.left);
}

float engine::graphics::renderable::Sprite::GetHeight() const
{
	return m_data->GetHeight()*(m_rect.bottom - m_rect.top);
}

spatial::SizeF engine::graphics::renderable::Sprite::GetSize() const
{
	return spatial::SizeF{
		m_data->GetWidth()* (m_rect.right - m_rect.left),
		m_data->GetHeight()* (m_rect.bottom - m_rect.top)
	};
}

