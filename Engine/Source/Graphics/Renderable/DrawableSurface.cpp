#include <Graphics/Renderable/DrawableSurface.h>
#include <Graphics/Resource/ITexture.h>
#include <Utilities/Logger.h>

engine::graphics::renderable::DrawableSurface::DrawableSurface(std::unique_ptr<engine::graphics::resource::ITexture> tex)
	:texture(std::move(tex))
{
}

// initialize methods
bool engine::graphics::renderable::DrawableSurface::Initialize(unsigned int width, unsigned int height) 
{
	if (!texture->Initialize(width, height))
	{
		LOGERROR("Failed to initialize DrawableSurface' texture resource.");
		return false;
	}

	return true;
}

engine::math::geometry::RectF engine::graphics::renderable::DrawableSurface::GetUVRect() const
{
	return engine::math::geometry::RectF{ 0, 0, 1, 1 };
}

void engine::graphics::renderable::DrawableSurface::Bind() const
{
	texture->Bind();
}

bool engine::graphics::renderable::DrawableSurface::CanBind() const
{
	return texture->CanBind();
}

// drawing methods
void engine::graphics::renderable::DrawableSurface::Begin()
{
	texture->BeginDraw();
}

void engine::graphics::renderable::DrawableSurface::Clear(float red, float green, float blue, float alpha)
{
	texture->Clear(red, green, blue, alpha);
}

void engine::graphics::renderable::DrawableSurface::End()
{
	texture->EndDraw();
}

float engine::graphics::renderable::DrawableSurface::GetWidth() const
{
	return static_cast<float>(texture->GetWidth());
}

float engine::graphics::renderable::DrawableSurface::GetHeight() const
{
	return static_cast<float>(texture->GetHeight());
}

engine::spatial::SizeF engine::graphics::renderable::DrawableSurface::GetSize() const
{
	return spatial::SizeF
	{
		static_cast<float>(texture->GetWidth()),
		static_cast<float>(texture->GetHeight())
	};
}

void engine::graphics::renderable::DrawableSurface::Reset()
{
	texture->Reset();
}
