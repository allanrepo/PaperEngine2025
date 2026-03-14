#pragma once
#include <Spatial/ISizeable.h>
#include <Graphics/Core/Sprite.h>

namespace engine::graphics::renderable
{
	class IRenderSurface: public spatial::ISizeable<float>
	{
	protected:
	public:
		virtual ~IRenderSurface() = default;
		virtual bool Initialize(unsigned int width, unsigned int height) = 0;

		virtual void Reset() = 0;

		// drawing methods
		virtual void Begin() = 0;
		virtual void Clear(float red, float green, float blue, float alpha) = 0;
		virtual void End() = 0;

		virtual const engine::graphics::Sprite GetSprite() const = 0;
		virtual bool IsValid() const = 0;
	};
}


