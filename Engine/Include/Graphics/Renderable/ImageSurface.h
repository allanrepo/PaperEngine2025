#pragma once
#include <memory>
#include <Graphics/Renderable/IImageSurface.h>

// forward declare
namespace graphics
{
	namespace resource
	{
		class ITexture;
	}
}

namespace graphics::renderable
{
	class ImageSurface : public graphics::renderable::IImageSurface
	{
	private:
		std::shared_ptr<graphics::resource::ITexture> texture;

	public:
		ImageSurface(std::unique_ptr<graphics::resource::ITexture> tex);
		virtual ~ImageSurface() = default;

		// cannot be copied
		ImageSurface(const ImageSurface&) = delete;
		ImageSurface& operator=(const ImageSurface&) = delete;

		// initialize methods
		virtual bool Initialize(const wchar_t* fileNamePath) override final;
		virtual void Reset();

		// ISizeable methods implementation
		virtual const float GetWidth() const override final;
		virtual const float GetHeight() const override final;
		const spatial::SizeF GetSize() const override final;

		// IRenderable methods implementation
		virtual math::geometry::RectF GetUVRect() const override final;
		virtual void Bind() const override final;
		virtual bool CanBind() const override final;
	};
}


