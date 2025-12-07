#include <Graphics/Core/DX11Core.h>
#include <Graphics/Renderer/DX11RendererBase.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

#pragma region // DX11SpriteBase
void graphics::dx11::renderer::DX11RendererBase::ShutDown()
{
	m_pd3dConstantBufferUpdate.Reset();
	m_pd3dConstantBufferProjection.Reset();
	m_pd3dInputLayout.Reset();
	m_pd3dPixelShader.Reset();
	m_pd3dVertexShader.Reset();
	m_pd3dVertexBuffer.Reset();
	m_pd3dBlendState.Reset();
	m_pd3dSamplerState.Reset();
}

HRESULT graphics::dx11::renderer::DX11RendererBase::CreateVertexBuffer(
	const void* pVertices,
	size_t vertexCount, 
	size_t vertexSize, 
	Microsoft::WRL::ComPtr<ID3D11Buffer>& pd3dVertexBuffer
)
{
	D3D11_SUBRESOURCE_DATA srd = {};
	srd.pSysMem = pVertices;
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = static_cast<unsigned int>(vertexCount * vertexSize);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = static_cast<UINT>(vertexSize);
	return graphics::dx11::DX11Core::Instance().GetDevice()->CreateBuffer(&bd, &srd, pd3dVertexBuffer.GetAddressOf());
}

HRESULT graphics::dx11::renderer::DX11RendererBase::CompileShader(
	const char* code, 
	const char* sourceName, 
	const char* entryPoint, 
	const char* shaderModel, 
	Microsoft::WRL::ComPtr<ID3DBlob>& pd3dShaderBlob
)
{
	Microsoft::WRL::ComPtr<ID3DBlob> pd3dErrorBlob;
	return D3DCompile(code, strlen(code), sourceName, nullptr, nullptr, entryPoint, shaderModel, 0, 0, pd3dShaderBlob.GetAddressOf(), pd3dErrorBlob.GetAddressOf());
}
HRESULT graphics::dx11::renderer::DX11RendererBase::CreateShadersAndInputLayout(
	const char* vsCode, const char* vsEntryPoint,
	const char* psCode, const char* psEntryPoint,
	const D3D11_INPUT_ELEMENT_DESC* ied, const unsigned int iedCount,
	Microsoft::WRL::ComPtr<ID3D11VertexShader>& pd3dVertexShader,
	Microsoft::WRL::ComPtr<ID3D11PixelShader>& pd3dPixelShader,
	Microsoft::WRL::ComPtr<ID3D11InputLayout>& pd3dInputLayout
)
{
	graphics::dx11::DX11Core& rCore = graphics::dx11::DX11Core::Instance();

	Microsoft::WRL::ComPtr<ID3DBlob> pd3dVSBlob2D;
	HRESULT hr = CompileShader(vsCode, "VSQuadBatch", vsEntryPoint, "vs_5_0", pd3dVSBlob2D);
	if (FAILED(hr))
	{
		LOGERROR("Failed to compile vertex shader.");
		return hr;
	}

	Microsoft::WRL::ComPtr<ID3DBlob> pd3dPSBlob2D;
	hr = CompileShader(psCode, "PSQuadBatch", psEntryPoint, "ps_5_0", pd3dPSBlob2D);
	if (FAILED(hr))
	{
		LOGERROR("Failed to compile pixel shader.");
		return hr;
	}

	hr = rCore.GetDevice()->CreateVertexShader(pd3dVSBlob2D->GetBufferPointer(), pd3dVSBlob2D->GetBufferSize(), nullptr, pd3dVertexShader.GetAddressOf());
	if (FAILED(hr))
	{
		LOGERROR("Failed to create vertex shader.");
		return hr;
	}

	hr = rCore.GetDevice()->CreatePixelShader(pd3dPSBlob2D->GetBufferPointer(), pd3dPSBlob2D->GetBufferSize(), nullptr, pd3dPixelShader.GetAddressOf());
	if (FAILED(hr))
	{
		LOGERROR("Failed to create pixel shader.");
		return hr;
	}

	hr = rCore.GetDevice()->CreateInputLayout(ied, iedCount, pd3dVSBlob2D->GetBufferPointer(), pd3dVSBlob2D->GetBufferSize(), pd3dInputLayout.GetAddressOf());
	if (FAILED(hr))
	{
		LOGERROR("Failed to create input layout.");
		return hr;
	}

	return hr;
}

HRESULT graphics::dx11::renderer::DX11RendererBase::CreateBlendState(
	Microsoft::WRL::ComPtr<ID3D11BlendState>& pd3dBlendState, 
	D3D11_BLEND_DESC* pBlendDesc
)
{
	graphics::dx11::DX11Core& rCore = graphics::dx11::DX11Core::Instance();

	D3D11_BLEND_DESC blendDesc = {};
	if (!pBlendDesc)
	{
		// <DEBUG>
		// this is original blend state setup that works fine when rendering in canvas, but broken when rendering in texture
		if (false)
		{
			blendDesc.RenderTarget[0].BlendEnable = TRUE;

			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

			blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		}		
		// <DEBUG>
		// this one looks like it fix the rendering to texture bug where transparent pixel turns pixel into transparent
		// we will use this from now on but observe if we encounter any issue. i don't fully understand blending math yet.
		else
		{
			blendDesc.RenderTarget[0].BlendEnable = TRUE;

			// Standard alpha blending for RGB
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			//blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

			// Disable alpha blending (preserve dest alpha)
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

			// Write only RGB channels
			//blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;
			blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		}

		pBlendDesc = &blendDesc;
	}

	return rCore.GetDevice()->CreateBlendState(pBlendDesc, pd3dBlendState.GetAddressOf());
}

HRESULT graphics::dx11::renderer::DX11RendererBase::CreateConstantBuffer(
	const void* pData,
	unsigned int byteWidth,
	Microsoft::WRL::ComPtr<ID3D11Buffer>& pd3dConstantBuffer
)
{
	D3D11_BUFFER_DESC cbd = {};
	cbd.Usage = D3D11_USAGE_DEFAULT;
	cbd.ByteWidth = byteWidth;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = 0;
	cbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA cbsd = {};
	cbsd.pSysMem = pData;

	return graphics::dx11::DX11Core::Instance().GetDevice()->CreateBuffer(&cbd, &cbsd, pd3dConstantBuffer.GetAddressOf());
}

HRESULT graphics::dx11::renderer::DX11RendererBase::CreateSamplerState(Microsoft::WRL::ComPtr<ID3D11SamplerState>& pd3dSamplerState)
{
	D3D11_SAMPLER_DESC sd = {};
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;
	return DX11Core::Instance().GetDevice()->CreateSamplerState(&sd, pd3dSamplerState.GetAddressOf());
}
#pragma endregion

