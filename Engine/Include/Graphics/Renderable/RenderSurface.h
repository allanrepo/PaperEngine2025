#pragma once
#include <Graphics/Renderable/IRenderSurface.h>
#include <memory>

// forward declare
namespace engine
{
	namespace graphics
	{
		namespace resource
		{
			class ITexture;
		}
	}
}


namespace engine::graphics::renderable
{
	class RenderSurface: public engine::graphics::renderable::IRenderSurface
	{
	private:
		std::unique_ptr<engine::graphics::resource::ITexture> m_texture;

	public:
		RenderSurface(std::unique_ptr<engine::graphics::resource::ITexture> texture);
		virtual ~RenderSurface() = default;

		// cannot be copied
		RenderSurface(const RenderSurface&) = delete;
		RenderSurface& operator=(const RenderSurface&) = delete;

		// initialize methods
		virtual bool Initialize(
			unsigned int width, unsigned int height
		) override final;

		// drawing methods
		virtual void Begin() override final;
		virtual void Clear(float red, float green, float blue, float alpha) override final;
		virtual void End() override final;

		virtual void Reset() override final;

		// ISizeable methods implementation
		virtual float GetWidth() const override final;
		virtual float GetHeight() const override final;
		virtual math::SizeF GetSize() const override final;

		// IRenderable methods implementation
		const engine::graphics::Sprite GetSprite() const override final;
		bool IsValid() const override final;

	};
}


