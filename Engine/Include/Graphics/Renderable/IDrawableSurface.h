#pragma once
#include <Graphics/Renderable/IRenderable.h>
#include <Spatial/ISizeable.h>

namespace graphics::renderable
{
	class IDrawableSurface: public graphics::renderable::IRenderable, public spatial::ISizeable<float>
	{
	protected:
	public:
		virtual ~IDrawableSurface() = default;
		virtual bool Initialize(unsigned int width, unsigned int height) = 0;

		virtual void Reset() = 0;

		// drawing methods
		virtual void Begin() = 0;
		virtual void Clear(float red, float green, float blue, float alpha) = 0;
		virtual void End() = 0;
	};
}


