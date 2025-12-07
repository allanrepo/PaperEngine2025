#pragma once
#include <d3d11.h>               // For ID3D11Device, ID3D11DeviceContext, ID3D11Texture2D, ID3D11ShaderResourceView
#include <wrl/client.h>          // Optional: for Microsoft::WRL::ComPtr if used
#include <string>                // For std::wstring
#include <filesystem>            // For std::filesystem::path
#include <algorithm>             // For std::transform
#include <cctype>                // For ::towlower
#include <wincodec.h>



namespace graphics::dx11::imageio
{
	namespace png
	{
		HRESULT LoadTextureFromFile(
			ID3D11Device* device,
			ID3D11DeviceContext* context,
			const wchar_t* filename,
			ID3D11ShaderResourceView** textureView);

		HRESULT SaveTextureToFile(
			ID3D11Device* device,
			ID3D11DeviceContext* context,
			ID3D11Texture2D* texture,
			const wchar_t* filename);
	}

	namespace bmp
	{
		HRESULT LoadTextureFromFile(
			ID3D11Device* device,
			ID3D11DeviceContext* context,
			const wchar_t* filename,
			ID3D11ShaderResourceView** textureView);

		HRESULT SaveTextureToFile(
			ID3D11Device* device,
			ID3D11DeviceContext* context,
			ID3D11Texture2D* texture,
			const wchar_t* filename);
	}

	HRESULT LoadTextureFromFile(
		ID3D11Device* device,
		ID3D11DeviceContext* context,
		const wchar_t* filename,
		ID3D11ShaderResourceView** textureView);

	HRESULT SaveTextureToFile(
		ID3D11Device* device,
		ID3D11DeviceContext* context,
		ID3D11Texture2D* texture,
		const wchar_t* filename);
}