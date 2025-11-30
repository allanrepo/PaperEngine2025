#include <Graphics/Core/DX11CanvasImpl.h>
#include <Graphics/Core/DX11Core.h>
#include <Windows.h>

// Constructor: no initialization needed here
graphics::dx11::DX11CanvasImpl::DX11CanvasImpl() {}

// Destructor: ensure resources are released
graphics::dx11::DX11CanvasImpl::~DX11CanvasImpl()
{
    ShutDown();
}

// Creates a render target view from the swap chain's back buffer
bool graphics::dx11::DX11CanvasImpl::createRenderTargetView()
{
    DX11Core& core = DX11Core::Instance();

    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;

    // Get the back buffer from the swap chain
    HRESULT result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
    if (FAILED(result))
    {
        LOGERROR("Failed to get back buffer from swap chain.");
        return false;
    }

    // Create a render target view using the back buffer
    result = core.GetDevice()->CreateRenderTargetView(backBuffer.Get(), nullptr, renderTargetView.GetAddressOf());
    if (FAILED(result))
    {
        LOGERROR("Failed to create render target view.");
        return false;
    }

    return true;
}

// Initializes the swap chain and render target view
bool graphics::dx11::DX11CanvasImpl::Initialize(void* windowHandle)
{
    graphics::dx11::DX11Core& core = graphics::dx11::DX11Core::Instance();

    // Describe swap chain parameters
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = reinterpret_cast<HWND>(windowHandle); // Cast to HWND
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;

    // Create the swap chain using the DXGI factory
    HRESULT result = core.GetFactory()->CreateSwapChain(
        static_cast<IUnknown*>(core.GetDevice()),
        &swapChainDesc,
        swapChain.GetAddressOf()
    );
    if (FAILED(result))
    {
        LOGERROR("Failed to create DXGI swap chain.");
        return false;
    }

    // Create the render target view from the swap chain
    if (!createRenderTargetView())
    {
        LOGERROR("Failed to create render target view.");
        ShutDown();
        return false;
    }

    return true;
}

// Resizes the swap chain buffers and recreates the render target view
void graphics::dx11::DX11CanvasImpl::Resize(uint32_t width, uint32_t height)
{
    if (!swapChain)
        return;

    // Release the old render target view before resizing
    renderTargetView.Reset();

    // Resize the swap chain buffers
    HRESULT result = swapChain->ResizeBuffers(
        0,                      // Preserve buffer count
        width,                  // New width
        height,                 // New height
        DXGI_FORMAT_UNKNOWN,    // Keep existing format
        0                       // No special flags
    );
    if (FAILED(result))
    {
        LOGERROR("Failed to resize DXGI swap chain buffers.");
        return;
    }

    // Recreate the render target view for the resized back buffer
    if (!createRenderTargetView())
    {
        LOGERROR("Failed to recreate render target view after resizing.");
        return;
    }
}

// Releases all rendering resources
void graphics::dx11::DX11CanvasImpl::ShutDown()
{
    renderTargetView.Reset(); // Release render target view
    swapChain.Reset();        // Release swap chain
}

// Begins a new frame by binding the render target view
void graphics::dx11::DX11CanvasImpl::Begin()
{
    DX11Core& core = DX11Core::Instance();

    // Bind the render target view to the output merger stage
    core.GetContext()->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), nullptr);
}

// Ends the frame by presenting the swap chain
void graphics::dx11::DX11CanvasImpl::End()
{
    if (!swapChain)
        return;

    // Present the back buffer to the screen
    swapChain->Present(0, 0);
}

// Clears the render target view with the specified color
void graphics::dx11::DX11CanvasImpl::Clear(float red, float green, float blue, float alpha)
{
    DX11Core& core = DX11Core::Instance();

    // Define the clear color
    float clearColor[] = { red, green, blue, alpha };

    // Clear the render target view
    core.GetContext()->ClearRenderTargetView(renderTargetView.Get(), clearColor);
}

// Sets a custom viewport rectangle
void graphics::dx11::DX11CanvasImpl::SetViewPort(float x, float y, float width, float height)
{
    DX11Core& core = DX11Core::Instance();

    // Define the viewport dimensions
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = x;
    viewport.TopLeftY = y;
    viewport.Width = width;
    viewport.Height = height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    // Apply the viewport to the rasterizer stage
    core.GetContext()->RSSetViewports(1, &viewport);
}

// Sets the viewport to match the full swap chain buffer size
void graphics::dx11::DX11CanvasImpl::SetViewPort()
{
    DXGI_SWAP_CHAIN_DESC desc = {};
    if (SUCCEEDED(swapChain->GetDesc(&desc)))
    {
        // Use full buffer dimensions for the viewport
        SetViewPort(0.0f, 0.0f, static_cast<float>(desc.BufferDesc.Width), static_cast<float>(desc.BufferDesc.Height));
    }
}
