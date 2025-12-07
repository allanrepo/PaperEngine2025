#pragma once
#include <Graphics/Resource/ITextureImpl.h>
#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi.h>

namespace graphics::dx11::resource
{
	class DX11TextureImpl : public graphics::resource::ITextureImpl
	{
	protected:
		// DX11 resources
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
		D3D11_TEXTURE2D_DESC textureDesc = {};

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> currRenderTargetView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> currDepthStencilView;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
		D3D11_VIEWPORT currViewport = {};

		// helper function to create DX11 texture
		HRESULT CreateTexture(
			unsigned int width,
			unsigned int height,
			DXGI_FORMAT format,
			Microsoft::WRL::ComPtr<ID3D11Texture2D>& tex,
			D3D11_TEXTURE2D_DESC& desc
		);

		// helper function to create DX11 shader resource view
		HRESULT CreateShaderResourceView(
			Microsoft::WRL::ComPtr<ID3D11Texture2D>& tex,
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv);

		// helper function to load data into DX11 texture
		void SetTextureData(
			const void* pSrcData,
			unsigned int bytesPerRow,
			Microsoft::WRL::ComPtr<ID3D11Texture2D>& tex
		);

		// helper function to create DX11 texture but also load data at the same time
		HRESULT CreateTexture(
			const void* pSrcData,
			unsigned int bytesPerRow,
			unsigned int width, unsigned int height,
			DXGI_FORMAT format,
			Microsoft::WRL::ComPtr<ID3D11Texture2D>& tex,
			D3D11_TEXTURE2D_DESC& desc
		);

	public:
		DX11TextureImpl() = default;
		virtual ~DX11TextureImpl() = default;

		// TODO: these are commented. but why are they here in the first place? should textures be non copyable?
		//// cannot be copied
		//DX11TextureImpl(const DX11TextureImpl&) = delete;
		//DX11TextureImpl& operator=(const DX11TextureImpl&) = delete;

		static constexpr const char* TypeName = "DirectX11";
		std::string GetTypeName() const override final;

		// initialize methods
		virtual bool Initialize(
			unsigned int width, unsigned int height,
			const void* srcData,
			unsigned int bytesPerRow
		) override final;

		virtual bool Initialize(
			unsigned int width, unsigned int height
		) override final;

		virtual bool Initialize(const wchar_t* fileNamePath) override final;

		virtual bool CanBind() override final;
		virtual void Bind() override final;

		// drawing methods
		virtual void BeginDraw() override final;
		virtual void Clear(float red, float green, float blue, float alpha) override final;
		virtual void EndDraw() override final;

		virtual const unsigned int GetWidth() const override final;
		virtual const unsigned int GetHeight() const override final;

		virtual void Reset() override final;

		virtual bool SaveToFile(const wchar_t* filename) override final;

	};
}