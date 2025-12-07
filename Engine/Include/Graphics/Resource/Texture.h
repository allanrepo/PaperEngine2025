#pragma once
#include <Graphics/Resource/ITextureImpl.h>
#include <Graphics/Resource/ITexture.h>
#include <string>
#include <memory>

namespace graphics::resource
{
	class Texture: public graphics::resource::ITexture
	{

	public:
		explicit Texture(std::unique_ptr<graphics::resource::ITextureImpl> pImpl);

		virtual ~Texture() = default;
		virtual std::string GetTypeName() const override final;

		// TODO: these are commented. but why are they here in the first place? should textures be non copyable?
		//// cannot be copied
		//Texture(const Texture&) = delete;
		//Texture& operator=(const Texture&) = delete;

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

		virtual void Bind() override final;
		virtual bool CanBind() override final;

		virtual const unsigned int GetWidth() const override final;
		virtual const unsigned int GetHeight() const override final;

		virtual void Reset() override final;

		virtual bool SaveToFile(const wchar_t* filename) override final;

	private:
		std::unique_ptr<graphics::resource::ITextureImpl> m_impl;
	};
}










