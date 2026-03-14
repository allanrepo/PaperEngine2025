#include <Graphics/Core/Sprite.h>
#include <Graphics/Resource/ISpriteAtlas.h>
#include <Graphics/Resource/SpriteAtlas.h>

engine::graphics::Sprite::Sprite(const engine::graphics::resource::ITexture* texture, const engine::math::geometry::RectF& rect, const engine::spatial::PositionF& anchor):
	m_view(texture),
	m_rect(rect),
	m_anchor(anchor)
{
	m_size = spatial::SizeF{
	m_view->GetWidth() * (m_rect.right - m_rect.left),
	m_view->GetHeight() * (m_rect.bottom - m_rect.top)
	};
}

bool engine::graphics::Sprite::IsValid() const
{
	return m_view.IsValid();
}

void engine::graphics::Sprite::Bind() const
{
	m_view->Bind();
}

bool engine::graphics::Sprite::CanBind() const
{
	return m_view->CanBind();
}

engine::math::geometry::RectF engine::graphics::Sprite::GetUVRect() const
{
	return m_rect;
}

void engine::graphics::Sprite::SetAnchor(const engine::spatial::PositionF& pos)
{
	m_anchor = pos;
}

engine::spatial::PositionF engine::graphics::Sprite::GetAnchor() const
{
	return m_anchor;
}

float engine::graphics::Sprite::GetWidth() const
{
	return m_size.width;
}

float engine::graphics::Sprite::GetHeight() const
{
	return m_size.height;
}

engine::spatial::SizeF engine::graphics::Sprite::GetSize() const
{
	return m_size;
}

