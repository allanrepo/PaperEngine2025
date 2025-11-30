#include <Graphics/Core/DX11Core.h>
#include <Utilities/Logger.h>

#include <stdexcept>

graphics::dx11::DX11Core::DX11Core()
    : device(nullptr)
    , context(nullptr)
    , factory(nullptr)
{
    if (!Initialize())
    {
        LOGERROR("Failed to initialize DX11Core.");
        throw std::runtime_error("Failed to initialize DX11Core");
    }
}

graphics::dx11::DX11Core::~DX11Core()
{
    ShutDown();
}

bool graphics::dx11::DX11Core::Initialize()
{
    CoInitialize(NULL);

    HRESULT result;

    // Create DXGI factory for adapter enumeration and swap chain creation
    result = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(factory.GetAddressOf()));
    if (FAILED(result))
    {
        LOGERROR("Failed to create DXGI factory.");
        return false;
    }

    // Set device creation flags (enable debug layer in debug builds)
    UINT deviceFlags = 0;
#ifdef _DEBUG
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // Specify desired feature level (Direct3D 11.0)
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0
    };

    // Create the Direct3D 11 device and immediate context
    result = D3D11CreateDevice(
        nullptr,                  // Use default adapter
        D3D_DRIVER_TYPE_HARDWARE, // Prefer hardware acceleration
        nullptr,                  // No software rasterizer fallback
        deviceFlags,              // Creation flags
        featureLevels,            // Requested feature levels
        1,                        // Number of feature levels
        D3D11_SDK_VERSION,        // SDK version
        &device,                  // Output device pointer
        nullptr,                  // Actual feature level (unused)
        &context                  // Output device context pointer
    );

    if (FAILED(result))
    {
        LOGERROR("Failed to create D3D11 device and context.");
        ShutDown(); // Clean up any partially initialized resources
        return false;
    }

    return true;
}

void graphics::dx11::DX11Core::ShutDown()
{
    context.Reset();
    device.Reset();
    factory.Reset();
}
