#pragma once
#include <Graphics/Resource/ITexture.h>
#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi.h>

namespace graphics::dx11::resource
{
	// TODO:
	// this implementation does not follow pimpl pattern. it directly inherits from ITexture
	// there is another version which is exactly similar to this but follows impl pattern. 
	// after some testing, this maybe removed 
	class DX11Texture : public graphics::resource::ITexture
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
		DX11Texture() = default;
		virtual ~DX11Texture() = default;

		// cannot be copied
		DX11Texture(const DX11Texture&) = delete;
		DX11Texture& operator=(const DX11Texture&) = delete;

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