#include <Graphics/Renderable/DrawableSurface.h>
#include <Graphics/Resource/ITexture.h>
#include <Utilities/Logger.h>

graphics::renderable::DrawableSurface::DrawableSurface(std::unique_ptr<graphics::resource::ITexture> tex)
	:texture(std::move(tex))
{
}

// initialize methods
bool graphics::renderable::DrawableSurface::Initialize(unsigned int width, unsigned int height) 
{
	if (!texture->Initialize(width, height))
	{
		LOGERROR("Failed to initialize DrawableSurface' texture resource.");
		return false;
	}

	return true;
}

math::geometry::RectF graphics::renderable::DrawableSurface::GetUVRect() const
{
	return math::geometry::RectF{ 0, 0, 1, 1 };
}

void graphics::renderable::DrawableSurface::Bind() const
{
	texture->Bind();
}

bool graphics::renderable::DrawableSurface::CanBind() const
{
	return texture->CanBind();
}

// drawing methods
void graphics::renderable::DrawableSurface::Begin()
{
	texture->BeginDraw();
}

void graphics::renderable::DrawableSurface::Clear(float red, float green, float blue, float alpha)
{
	texture->Clear(red, green, blue, alpha);
}

void graphics::renderable::DrawableSurface::End()
{
	texture->EndDraw();
}

float graphics::renderable::DrawableSurface::GetWidth() const
{
	return static_cast<float>(texture->GetWidth());
}

float graphics::renderable::DrawableSurface::GetHeight() const
{
	return static_cast<float>(texture->GetHeight());
}

spatial::SizeF graphics::renderable::DrawableSurface::GetSize() const
{
	return spatial::SizeF
	{
		static_cast<float>(texture->GetWidth()),
		static_cast<float>(texture->GetHeight())
	};
}

void graphics::renderable::DrawableSurface::Reset()
{
	texture->Reset();
}
