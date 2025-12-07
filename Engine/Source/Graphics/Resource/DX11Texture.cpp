#include <Graphics/Resource/DX11Texture.h>
#include <Graphics/Core/DX11Core.h>
#include <Graphics/IO/DX11ImageFileHelper.h>
#include <Cache/BindCache.h>

#pragma region // DX11Texture

// helper function to create DX11 texture
HRESULT graphics::dx11::resource::DX11Texture::CreateTexture(
	unsigned int width,
	unsigned int height,
	DXGI_FORMAT format,
	Microsoft::WRL::ComPtr<ID3D11Texture2D>& tex,
	D3D11_TEXTURE2D_DESC& desc
)
{
	desc = {};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	return graphics::dx11::DX11Core::Instance().GetDevice()->CreateTexture2D(&desc, nullptr, tex.GetAddressOf());
}

// helper function to create DX11 shader resource view
HRESULT graphics::dx11::resource::DX11Texture::CreateShaderResourceView(
	Microsoft::WRL::ComPtr<ID3D11Texture2D>& tex,
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv)
{
	D3D11_TEXTURE2D_DESC td = {};
	tex->GetDesc(&td);
	D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format = td.Format;
	desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.MipLevels = 1;

	return graphics::dx11::DX11Core::Instance().GetDevice()->CreateShaderResourceView(tex.Get(), &desc, srv.GetAddressOf());
}

// helper function to load data into DX11 texture
void graphics::dx11::resource::DX11Texture::SetTextureData(
	const void* pSrcData,
	unsigned int bytesPerRow,
	Microsoft::WRL::ComPtr<ID3D11Texture2D>& tex
)
{
	graphics::dx11::DX11Core::Instance().GetContext()->UpdateSubresource(tex.Get(), 0, nullptr, pSrcData, bytesPerRow, 0);
}

// helper function to create DX11 texture but also load data at the same time
HRESULT graphics::dx11::resource::DX11Texture::CreateTexture(
	const void* pSrcData,
	unsigned int bytesPerRow,
	unsigned int width, unsigned int height,
	DXGI_FORMAT format,
	Microsoft::WRL::ComPtr<ID3D11Texture2D>& tex,
	D3D11_TEXTURE2D_DESC& desc
)
{
	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = pSrcData;
	data.SysMemPitch = bytesPerRow;

	desc = {};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	return graphics::dx11::DX11Core::Instance().GetDevice()->CreateTexture2D(&desc, &data, tex.GetAddressOf());
}

std::string graphics::dx11::resource::DX11Texture::GetTypeName() const
{
	return TypeName;
}

bool graphics::dx11::resource::DX11Texture::Initialize(unsigned int width, unsigned int height, const void* srcData, unsigned int bytesPerRow)
{
	if (FAILED(CreateTexture(srcData, bytesPerRow, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, texture, textureDesc)))
	{
		LOGERROR("Failed to create texture.");
		return false;
	}

	if (FAILED(CreateShaderResourceView(texture, shaderResourceView)))
	{
		LOGERROR("Failed to create shader resource view.");
		return false;
	}

	return true;
}

bool graphics::dx11::resource::DX11Texture::Initialize(unsigned int width, unsigned int height)
{
	if (FAILED(CreateTexture(width, height, DXGI_FORMAT_R8G8B8A8_UNORM, texture, textureDesc)))
	{
		LOGERROR("Failed to create texture.");
		return false;
	}

	if (FAILED(CreateShaderResourceView(texture, shaderResourceView)))
	{
		LOGERROR("Failed to create shader resource view.");
		return false;
	}

	// create render target view for the texture
	if FAILED(graphics::dx11::DX11Core::Instance().GetDevice()->CreateRenderTargetView(texture.Get(), nullptr, renderTargetView.GetAddressOf()))
	{
		LOGERROR("Failed to create render target view.");
		return false;
	}

	return true;
}

bool graphics::dx11::resource::DX11Texture::Initialize(const wchar_t* fileNamePath)
{
	// create shader resource view from file
	if (FAILED(graphics::dx11::imageio::LoadTextureFromFile(
		graphics::dx11::DX11Core::Instance().GetDevice(),
		graphics::dx11::DX11Core::Instance().GetContext(),
		fileNamePath, shaderResourceView.GetAddressOf())))
	{
		LOGERROR("Failed to create texture.");
		return false;
	}

	// query for the texture interface
	Microsoft::WRL::ComPtr<ID3D11Resource> resource;
	shaderResourceView->GetResource(resource.GetAddressOf());

	if (FAILED(resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)texture.GetAddressOf())))
	{
		LOGERROR("Failed to query texture.");
		return false;
	}

	// get texture desc
	texture->GetDesc(&textureDesc);

	return true;
}

bool graphics::dx11::resource::DX11Texture::CanBind()
{
	return cache::BindCache<graphics::resource::ITexture>::Instance().CanBind(this, false);
}

void graphics::dx11::resource::DX11Texture::Bind()
{
	cache::BindCache<graphics::resource::ITexture>::Instance().Bind(
		this, // pointer to texture to bind if needed
		[this](graphics::resource::ITexture* tex) // lambda to perform if bind happens
		{
			if (shaderResourceView)
			{
				DX11Core::Instance().GetContext()->PSSetShaderResources(0, 1, shaderResourceView.GetAddressOf());
			}
		},
		false); // force to bind if true
}

void graphics::dx11::resource::DX11Texture::BeginDraw()
{
	// Save current render targets
	currDepthStencilView.Reset();
	currRenderTargetView.Reset();
	graphics::dx11::DX11Core::Instance().GetContext()->OMGetRenderTargets(1, currRenderTargetView.GetAddressOf(), currDepthStencilView.GetAddressOf());

	// set the render target view
	graphics::dx11::DX11Core::Instance().GetContext()->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), nullptr);

	// save current viewport
	UINT numViewports = 1;
	graphics::dx11::DX11Core::Instance().GetContext()->RSGetViewports(&numViewports, &currViewport);

	// set viewport, must match texture size
	D3D11_VIEWPORT vp = {};
	vp.Width = static_cast<float>(GetWidth());
	vp.Height = static_cast<float>(GetHeight());
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	graphics::dx11::DX11Core::Instance().GetContext()->RSSetViewports(1, &vp);
}

void graphics::dx11::resource::DX11Texture::Clear(float red, float green, float blue, float alpha)
{
	FLOAT c[4] = { red, green, blue, alpha };
	DX11Core::Instance().GetContext()->ClearRenderTargetView(renderTargetView.Get(), c);
}

void graphics::dx11::resource::DX11Texture::EndDraw()
{
	// return to original render target
	graphics::dx11::DX11Core::Instance().GetContext()->OMSetRenderTargets(1, currRenderTargetView.GetAddressOf(), currDepthStencilView.Get());

	// return to original viewport
	graphics::dx11::DX11Core::Instance().GetContext()->RSSetViewports(1, &currViewport);
}

const unsigned int graphics::dx11::resource::DX11Texture::GetWidth() const
{
	return textureDesc.Width;
}

const unsigned int graphics::dx11::resource::DX11Texture::GetHeight() const
{
	return textureDesc.Height;
}

void graphics::dx11::resource::DX11Texture::Reset()
{
	texture.Reset();
	shaderResourceView.Reset();
	currDepthStencilView.Reset();
	currRenderTargetView.Reset();
	renderTargetView.Reset();
}
bool graphics::dx11::resource::DX11Texture::SaveToFile(const wchar_t* filename)
{
	// create shader resource view from file
	if (FAILED(graphics::dx11::imageio::SaveTextureToFile(
		graphics::dx11::DX11Core::Instance().GetDevice(), 
		graphics::dx11::DX11Core::Instance().GetContext(), 
		texture.Get(), 
		filename
	)))
	{
		LOGERROR("Failed to save texture.");
		return false;
	}
	return true;
}
#pragma endregion