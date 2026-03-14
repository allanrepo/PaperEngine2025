#pragma once
#include <Graphics/Resource/ITextureImpl.h>
#include <Graphics/Resource/ITexture.h>
#include <string>
#include <memory>

namespace engine::graphics::resource
{
	class Texture: public engine::graphics::resource::ITexture
	{

	public:
		explicit Texture(std::unique_ptr<engine::graphics::resource::ITextureImpl> pImpl);

		virtual ~Texture() = default;
		virtual std::string GetTypeName() const override final;

		// cannot be copied, but movable
		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;
		Texture(Texture&&) noexcept = default;
		Texture& operator=(Texture&&) noexcept = default;

		// initialize with data
		virtual bool Initialize(
			unsigned int width, unsigned int height,
			const void* srcData,
			unsigned int bytesPerRow
		) override final;

		// initialize empty texture
		virtual bool Initialize(
			unsigned int width, unsigned int height
		) override final;

		// initialize with file
		virtual bool Initialize(const wchar_t* fileNamePath) override final;

		// drawing methods
		virtual void BeginDraw() override final;
		virtual void Clear(float red, float green, float blue, float alpha) override final;
		virtual void EndDraw() override final;

		virtual void Bind() const override final;
		virtual bool CanBind() const override final;

		virtual const unsigned int GetWidth() const override final;
		virtual const unsigned int GetHeight() const override final;

		virtual void Reset() override final;

		virtual bool SaveToFile(const wchar_t* filename) override final;

		virtual bool IsValid() override final;


	private:
		std::unique_ptr<engine::graphics::resource::ITextureImpl> m_impl;
	};
}










