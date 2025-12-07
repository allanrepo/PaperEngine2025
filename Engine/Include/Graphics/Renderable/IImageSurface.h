#pragma once
#include <Graphics/Renderable/IRenderable.h>
#include <Spatial/ISizeable.h>

namespace graphics::renderable
{
	class IImageSurface : public graphics::renderable::IRenderable, public spatial::ISizeable<float>
	{
	public:
		virtual bool Initialize(const wchar_t* fileNamePath) = 0;
	};
}


