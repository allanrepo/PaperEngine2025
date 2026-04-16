#pragma once
#include <Graphics/Core/IRenderable.h>

namespace engine
{
	namespace graphics
	{
		// Renderable is a small concrete implementation of IRenderable that wraps a single Sprite handle and exposes it to the renderer. 
		// It is a lightweight value‑semantic object intended for tiles and simple scene objects where the visual is a single static sprite or a sprite handle.
		class Renderable : public IRenderable
		{
		private:
			Sprite m_sprite;

		public:
			Renderable() = default;
			~Renderable() override = default;

			Renderable(const Renderable&) = default;
			Renderable& operator=(const Renderable&) = default;
			Renderable(Renderable&&) noexcept = default;
			Renderable& operator=(Renderable&&) noexcept = default;

			explicit Renderable(const Sprite& sprite) noexcept :
				m_sprite(sprite)
			{
			}

			Renderable(Sprite&& sprite) noexcept :
				m_sprite(std::move(sprite))
			{
			}

			Sprite GetSprite() const noexcept override final
			{
				return m_sprite;
			}
		};
	}
}