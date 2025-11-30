#pragma once

#include <Core/Singleton.h>
#include <Utilities/Logger.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace graphics::dx11
{
    class DX11Core : public core::Singleton<DX11Core>
    {
        friend class core::Singleton<DX11Core>;

    private:
        Microsoft::WRL::ComPtr<ID3D11Device> device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
        Microsoft::WRL::ComPtr<IDXGIFactory> factory;

    private:
        // singleton so cannot be instanced
        DX11Core();
        ~DX11Core();

        // this method is private because this class is singleton. this method will be called in its constructor
        // so the first time it is instanced, this will be called.
        bool Initialize();

        // this is to reset the DX11 COM pointers
        void ShutDown();

    public:
        // cannot be copied
        DX11Core(const DX11Core&) = delete;
        DX11Core& operator=(const DX11Core&) = delete;

        // Getter for D3D11 device
        ID3D11Device* GetDevice() const 
        {
            return device.Get();
        }

        // Getter for D3D11 device context
        ID3D11DeviceContext* GetContext() const 
        {
            return context.Get();
        }

        // Getter for DXGI factory
        IDXGIFactory* GetFactory() const 
        {
            return factory.Get();
        }
    };


}

