#pragma once
#include <string>

namespace graphics::resource
{
	class ITexture
	{
	protected:
		// NOTE: need to do this so derived class like Texture can call its constructor. this is because we delete copy constructor
		//ITexture() = default;   

	public:
		virtual ~ITexture() = default;
		virtual std::string GetTypeName() const = 0;

		// TODO: these are commented. but why are they here in the first place? should textures be non copyable?
		//// cannot be copied
		//ITexture(const ITexture&) = delete;
		//ITexture& operator=(const ITexture&) = delete;

		// initialize with data
		virtual bool Initialize(
			unsigned int width, unsigned int height,
			const void* srcData,
			unsigned int bytesPerRow
		) = 0;

		// initialize empty texture
		virtual bool Initialize(
			unsigned int width, unsigned int height
		) = 0;

		// initialize with file
		virtual bool Initialize(const wchar_t* fileNamePath) = 0;

		// drawing methods
		virtual void BeginDraw() = 0;
		virtual void Clear(float red, float green, float blue, float alpha) = 0;
		virtual void EndDraw() = 0;

		virtual void Bind() = 0;
		virtual bool CanBind() = 0;

		virtual const unsigned int GetWidth() const = 0;
		virtual const unsigned int GetHeight() const = 0;

		virtual void Reset() = 0;

		virtual bool SaveToFile(const wchar_t* filename) = 0;
	};
}










