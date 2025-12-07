#pragma once
#include <Graphics/Core/DX11Core.h>
#include <wrl/client.h>
#include <d3d11.h>

namespace graphics::dx11::renderer
{
	class DX11RendererBase
	{
	protected:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pd3dVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pd3dVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pd3dPixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pd3dInputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pd3dConstantBufferProjection;
		D3D11_VIEWPORT m_D3DViewPort = {};
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pd3dConstantBufferUpdate;
		Microsoft::WRL::ComPtr<ID3D11BlendState> m_pd3dBlendState;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pd3dSamplerState;


	protected:
		// helper function to create resources. decided to implement helper function this way instead of internally creating resources so this function can be reused in future where a different resource is to be created
		HRESULT CreateVertexBuffer(const void* pVertices, size_t vertexCount, size_t vertexSize, Microsoft::WRL::ComPtr<ID3D11Buffer>& pd3dVertexBuffer);
		HRESULT CompileShader(const char* code, const char* sourceName, const char* entryPoint, const char* shaderModel, Microsoft::WRL::ComPtr<ID3DBlob>& pd3dShaderBlob);
		HRESULT CreateShadersAndInputLayout(
			const char* vsCode, const char* vsEntryPoint,
			const char* psCode, const char* psEntryPoint,
			const D3D11_INPUT_ELEMENT_DESC* ied, const unsigned int iedCount,
			Microsoft::WRL::ComPtr<ID3D11VertexShader>& pd3dVertexShader,
			Microsoft::WRL::ComPtr<ID3D11PixelShader>& pd3dPixelShader,
			Microsoft::WRL::ComPtr<ID3D11InputLayout>& pd3dInputLayout
		);

		HRESULT CreateConstantBuffer(
			const void* pData,
			unsigned int byteWidth,
			Microsoft::WRL::ComPtr<ID3D11Buffer>& pd3dConstantBuffer
		);

		HRESULT CreateBlendState(
			Microsoft::WRL::ComPtr<ID3D11BlendState>& pd3dBlendState, 
			D3D11_BLEND_DESC* pBlendDesc = nullptr
		);

		HRESULT CreateSamplerState(Microsoft::WRL::ComPtr<ID3D11SamplerState>& pd3dSamplerState);

	public:
		DX11RendererBase(const DX11RendererBase&) = delete;
		DX11RendererBase& operator=(const DX11RendererBase&) = delete;

		DX11RendererBase() = default;
		virtual ~DX11RendererBase() = default;

		virtual void ShutDown();
	};

}


