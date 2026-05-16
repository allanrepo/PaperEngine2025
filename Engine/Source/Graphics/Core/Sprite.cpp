#include <Graphics/Core/Sprite.h>
#include <Graphics/Resource/ISpriteAtlas.h>
#include <Graphics/Resource/SpriteAtlas.h>

engine::graphics::Sprite::Sprite(const engine::graphics::resource::ITexture* texture, const engine::math::geometry::RectF& rect, const engine::spatial::PositionF& pivot):
	m_view(texture),
	m_rect(rect),
	m_pivot(pivot)
{
	// if making invalid sprite, size will be 0,0
	m_size = spatial::SizeF{
	(texture != nullptr? m_view->GetWidth() : 0.0f) * (m_rect.right - m_rect.left),
	(texture != nullptr ? m_view->GetHeight() : 0.0f) * (m_rect.bottom - m_rect.top)
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

void engine::graphics::Sprite::SetPivot(const engine::spatial::PositionF& pos)
{
	m_pivot = pos;
}

engine::spatial::PositionF engine::graphics::Sprite::GetPivot() const
{
	return m_pivot;
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

engine::graphics::Sprite engine::graphics::Sprite::MakeInvalidSprite()
{
	return Sprite(nullptr, { 0,0,0,0 }, { 0,0 });
}

// Convert normalized pivot (0–1) into pixel coordinates
engine::spatial::PositionF engine::graphics::Sprite::GetPivotInPixels() const
{
	return {
		m_pivot.x * m_size.width,
		m_pivot.y * m_size.height
	};
}

// Set pivot using pixel coordinates, internally converting back to normalized
void engine::graphics::Sprite::SetPivotInPixels(const engine::spatial::PositionF& pixelPos)
{
	// set pivot only if size is valid. otherwise, keep it unchanged
	if (m_size.width != 0 && m_size.height != 0)
	{
		m_pivot.x = pixelPos.x / m_size.width;
		m_pivot.y = pixelPos.y / m_size.height;
	}
}


