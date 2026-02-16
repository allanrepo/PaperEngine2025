#include <Graphics/Renderable/ImageSurface.h>
#include <Utilities/Logger.h>
#include <Graphics/Resource/ITexture.h>

engine::graphics::renderable::ImageSurface::ImageSurface(std::unique_ptr<engine::graphics::resource::ITexture> tex)
	:texture(std::move(tex))
{
}

// initialize methods
bool engine::graphics::renderable::ImageSurface::Initialize(const wchar_t* fileNamePath)
{
	if (!texture->Initialize(fileNamePath))
	{
		LOGERROR("Failed to initialize ImageSurface' texture resource.");
		return false;
	}

	return true;
}

engine::math::geometry::RectF engine::graphics::renderable::ImageSurface::GetUVRect() const
{
	return engine::math::geometry::RectF{ 0, 0, 1, 1 };
}

void engine::graphics::renderable::ImageSurface::Bind() const
{
	texture->Bind();
}

bool engine::graphics::renderable::ImageSurface::CanBind() const
{
	return texture->CanBind();
}

float engine::graphics::renderable::ImageSurface::GetWidth() const
{
	return static_cast<float>(texture->GetWidth());
}

float engine::graphics::renderable::ImageSurface::GetHeight() const
{
	return static_cast<float>(texture->GetHeight());
}

engine::spatial::SizeF engine::graphics::renderable::ImageSurface::GetSize() const
{
	return spatial::SizeF
	{
		 static_cast<float>(texture->GetWidth()),
		  static_cast<float>(texture->GetHeight())
	};
}

void engine::graphics::renderable::ImageSurface::Reset()
{
	texture->Reset();
}
