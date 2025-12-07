#include <Graphics/Renderable/ImageSurface.h>
#include <Utilities/Logger.h>
#include <Graphics/Resource/ITexture.h>

graphics::renderable::ImageSurface::ImageSurface(std::unique_ptr<graphics::resource::ITexture> tex)
	:texture(std::move(tex))
{
}

// initialize methods
bool graphics::renderable::ImageSurface::Initialize(const wchar_t* fileNamePath)
{
	if (!texture->Initialize(fileNamePath))
	{
		LOGERROR("Failed to initialize ImageSurface' texture resource.");
		return false;
	}

	return true;
}

math::geometry::RectF graphics::renderable::ImageSurface::GetUVRect() const
{
	return math::geometry::RectF{ 0, 0, 1, 1 };
}

void graphics::renderable::ImageSurface::Bind() const
{
	texture->Bind();
}

bool graphics::renderable::ImageSurface::CanBind() const
{
	return texture->CanBind();
}

const float graphics::renderable::ImageSurface::GetWidth() const
{
	return static_cast<float>(texture->GetWidth());
}

const float graphics::renderable::ImageSurface::GetHeight() const
{
	return static_cast<float>(texture->GetHeight());
}

const spatial::SizeF graphics::renderable::ImageSurface::GetSize() const
{
	return spatial::SizeF
	{
		 static_cast<float>(texture->GetWidth()),
		  static_cast<float>(texture->GetHeight())
	};
}

void graphics::renderable::ImageSurface::Reset()
{
	texture->Reset();
}
