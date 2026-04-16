#pragma once
#include <Graphics/Core/Sprite.h>

namespace engine
{
	namespace graphics
	{
		// IRenderable is a minimal, single‑responsibility interface that exposes a renderable visual handle (a Sprite) 
		// for any object that can be drawn by the renderer. It decouples rendering from object logic so renderers, tile systems, 
		// and object managers can treat heterogeneous objects uniformly.
		class IRenderable
		{
		public:
			virtual ~IRenderable() = default;
			virtual Sprite GetSprite() const noexcept = 0;
		};
	}
}