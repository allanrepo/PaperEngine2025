#include <Graphics/Renderable/RenderSurface.h>
#include <Graphics/Resource/ITexture.h>
#include <Utilities/Logger.h>

engine::graphics::renderable::RenderSurface::RenderSurface(std::unique_ptr<engine::graphics::resource::ITexture> texture)
	:m_texture(std::move(texture))
{
}

// initialize methods
bool engine::graphics::renderable::RenderSurface::Initialize(unsigned int width, unsigned int height) 
{
	if (!m_texture->Initialize(width, height))
	{
		LOGERROR("Failed to initialize RenderSurface' texture resource.");
		return false;
	}

	return true;
}

// drawing methods
void engine::graphics::renderable::RenderSurface::Begin()
{
	m_texture->BeginDraw();
}

void engine::graphics::renderable::RenderSurface::Clear(float red, float green, float blue, float alpha)
{
	m_texture->Clear(red, green, blue, alpha);
}

void engine::graphics::renderable::RenderSurface::End()
{
	m_texture->EndDraw();
}

float engine::graphics::renderable::RenderSurface::GetWidth() const
{
	return static_cast<float>(m_texture->GetWidth());
}

float engine::graphics::renderable::RenderSurface::GetHeight() const
{
	return static_cast<float>(m_texture->GetHeight());
}

engine::spatial::SizeF engine::graphics::renderable::RenderSurface::GetSize() const
{
	return spatial::SizeF
	{
		static_cast<float>(m_texture->GetWidth()),
		static_cast<float>(m_texture->GetHeight())
	};
}

const engine::graphics::Sprite engine::graphics::renderable::RenderSurface::GetSprite() const
{
	return engine::graphics::Sprite(m_texture.get(), engine::math::geometry::RectF{ 0, 0, 1, 1 });
}

bool engine::graphics::renderable::RenderSurface::IsValid() const
{
	return m_texture->IsValid();
}

void engine::graphics::renderable::RenderSurface::Reset()
{
	m_texture->Reset();
}
