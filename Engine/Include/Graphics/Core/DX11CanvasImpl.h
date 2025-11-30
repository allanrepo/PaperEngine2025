#pragma once

// COM smart pointers for DX interfaces
#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi.h>

// Abstract renderer interface
#include <Graphics/Core/ICanvasImpl.h>

namespace graphics::dx11
{
    // DX11-specific implementation of the ICanvasImpl interface
    class DX11CanvasImpl : public graphics::ICanvasImpl
    {
    private:
        // Swap chain for presenting rendered frames to the window
        Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;

        // Render target view for the back buffer
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;

        // Creates the render target view from the swap chain's back buffer
        bool createRenderTargetView();

    public:
        // Constructor: prepares internal state
        DX11CanvasImpl();

        // Destructor: releases resources
        ~DX11CanvasImpl() override;

        // prevent copying
        DX11CanvasImpl(const DX11CanvasImpl&) = delete;
        DX11CanvasImpl& operator=(const DX11CanvasImpl&) = delete;

        // Initializes swap chain and render target view using the window handle
        bool Initialize(void* windowHandle) override;

        // Handles resizing of the swap chain and render target view
        void Resize(uint32_t width, uint32_t height) override;

        // Cleans up all DX11 resources
        void ShutDown() override;

        // Begins a new frame (e.g., clears buffers, sets render targets)
        void Begin() override;

        // Ends the frame (e.g., presents the swap chain)
        void End() override;

        // Clears the render target with the specified color
        void Clear(float red, float green, float blue, float alpha) override;

        // Sets a custom viewport rectangle
        void SetViewPort(float x, float y, float width, float height) override;

        // Sets the default viewport to cover the entire render target
        void SetViewPort() override;

        // this class means it is implemented with directx 11
        static constexpr const char* TypeName = "DirectX11";
        virtual std::string GetTypeName() const override
        {
            return TypeName;
        }
    };
}
