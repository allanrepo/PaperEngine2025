#include <Graphics/Renderable/Sprite.h>
#include <Graphics/Resource/ISpriteAtlas.h>
#include <Graphics/Resource/SpriteAtlas.h>


engine::graphics::renderable::Sprite::Sprite(const engine::graphics::resource::ISpriteAtlas* spriteAtlas, engine::math::geometry::RectF rect) :
	//View<engine::graphics::resource::ISpriteAtlas>(spriteAtlas),
	m_view(spriteAtlas),
	m_rect(rect),
	m_anchor({0,0})
{
	// precompute size relative to spriteatlas size. it is a lot faster than trying to compute it repeatedly on GetSize()
	// performance hit accumulates as more sprites are rendered
	m_size = spatial::SizeF{
		m_view->GetWidth() * (m_rect.right - m_rect.left),
		m_view->GetHeight() * (m_rect.bottom - m_rect.top)
	};
}

void engine::graphics::renderable::Sprite::Bind() const
{
	m_view->Bind();
}

bool engine::graphics::renderable::Sprite::CanBind() const
{
	return m_view->CanBind();
}

engine::math::geometry::RectF engine::graphics::renderable::Sprite::GetUVRect() const
{
	return m_rect;
}

void engine::graphics::renderable::Sprite::SetAnchor(const engine::spatial::PositionF& pos)
{
	m_anchor = pos;
}

engine::spatial::PositionF engine::graphics::renderable::Sprite::GetAnchor() const
{
	return m_anchor;
}

float engine::graphics::renderable::Sprite::GetWidth() const
{
	return m_size.width;
}

float engine::graphics::renderable::Sprite::GetHeight() const
{
	return m_size.height;
}

engine::spatial::SizeF engine::graphics::renderable::Sprite::GetSize() const
{
	return m_size;
}

