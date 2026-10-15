#include <Graphics/IO/DX11ImageFileHelper.h>


using Microsoft::WRL::ComPtr;

namespace engine::graphics::dx11::imageio
{
    HRESULT LoadTextureFromFile(
        ID3D11Device* device,
        ID3D11DeviceContext* context,
        const wchar_t* filename,
        ID3D11ShaderResourceView** textureView)
    {
        std::wstring ext = std::filesystem::path(filename).extension().wstring();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);

        if (ext == L".png") {
            return png::LoadTextureFromFile(device, context, filename, textureView);
        }
        else if (ext == L".bmp") {
            return bmp::LoadTextureFromFile(device, context, filename, textureView);
        }
        else if (ext == L".jpg" || ext == L".jpeg")
        {
            return jpg::LoadTextureFromFile(device, context, filename, textureView);
        }
        else
        {
            return E_FAIL; // or a custom error code for unsupported format
        }
    }

    HRESULT SaveTextureToFile(
        ID3D11Device* device,
        ID3D11DeviceContext* context,
        ID3D11Texture2D* texture,
        const wchar_t* filename)
    {
        std::wstring ext = std::filesystem::path(filename).extension().wstring();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);

        if (ext == L".png") {
            return png::SaveTextureToFile(device, context, texture, filename);
        }
        else if (ext == L".bmp") {
            return bmp::SaveTextureToFile(device, context, texture, filename);
        }
        else if (ext == L".jpg" || ext == L".jpeg")
        {
            return jpg::SaveTextureToFile(device, context, texture, filename);
        }
        else 
        {
            // Optionally log or throw an error
            return E_FAIL; 
        }
    }

    namespace png
    {
        HRESULT LoadTextureFromFile(
            ID3D11Device* device,
            ID3D11DeviceContext* context,
            const wchar_t* filename,
            ID3D11ShaderResourceView** textureView)
        {
            if (!device || !filename || !textureView)
                return E_INVALIDARG;

            ComPtr<IWICImagingFactory> factory;
            HRESULT hr = CoCreateInstance(
                CLSID_WICImagingFactory,
                nullptr,
                CLSCTX_INPROC_SERVER,
                IID_PPV_ARGS(&factory));
            if (FAILED(hr)) return hr;

            ComPtr<IWICBitmapDecoder> decoder;
            hr = factory->CreateDecoderFromFilename(
                filename,
                nullptr,
                GENERIC_READ,
                WICDecodeMetadataCacheOnDemand,
                &decoder);
            if (FAILED(hr)) return hr;

            ComPtr<IWICBitmapFrameDecode> frame;
            hr = decoder->GetFrame(0, &frame);
            if (FAILED(hr)) return hr;

            ComPtr<IWICFormatConverter> converter;
            hr = factory->CreateFormatConverter(&converter);
            if (FAILED(hr)) return hr;

            hr = converter->Initialize(
                frame.Get(),
                GUID_WICPixelFormat32bppRGBA,
                WICBitmapDitherTypeNone,
                nullptr,
                0.0f,
                WICBitmapPaletteTypeCustom);
            if (FAILED(hr)) return hr;

            UINT width, height;
            hr = converter->GetSize(&width, &height);
            if (FAILED(hr)) return hr;

            std::vector<uint8_t> pixels(width * height * 4);
            hr = converter->CopyPixels(
                nullptr,
                width * 4,
                static_cast<UINT>(pixels.size()),
                pixels.data());
            if (FAILED(hr)) return hr;

            D3D11_TEXTURE2D_DESC texDesc = {};
            texDesc.Width = width;
            texDesc.Height = height;
            texDesc.MipLevels = 1;
            texDesc.ArraySize = 1;
            texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            texDesc.SampleDesc.Count = 1;
            texDesc.Usage = D3D11_USAGE_DEFAULT;
            texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            texDesc.CPUAccessFlags = 0;
            texDesc.MiscFlags = 0;

            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = pixels.data();
            initData.SysMemPitch = width * 4;

            ComPtr<ID3D11Texture2D> texture;
            hr = device->CreateTexture2D(&texDesc, &initData, &texture);
            if (FAILED(hr)) return hr;

            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = texDesc.Format;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;

            hr = device->CreateShaderResourceView(texture.Get(), &srvDesc, textureView);
            return hr;
        }

        HRESULT SaveTextureToFile(
            ID3D11Device* device,
            ID3D11DeviceContext* context,
            ID3D11Texture2D* texture,
            const wchar_t* filename)
        {
            if (!device || !context || !texture || !filename)
                return E_INVALIDARG;

            D3D11_TEXTURE2D_DESC desc = {};
            texture->GetDesc(&desc);

            // Create staging texture
            D3D11_TEXTURE2D_DESC stagingDesc = desc;
            stagingDesc.Usage = D3D11_USAGE_STAGING;
            stagingDesc.BindFlags = 0;
            stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            stagingDesc.MiscFlags = 0;

            ComPtr<ID3D11Texture2D> stagingTexture;
            HRESULT hr = device->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);
            if (FAILED(hr)) return hr;

            context->CopyResource(stagingTexture.Get(), texture);

            D3D11_MAPPED_SUBRESOURCE mapped = {};
            hr = context->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped);
            if (FAILED(hr)) return hr;

            ComPtr<IWICImagingFactory> factory;
            hr = CoCreateInstance(
                CLSID_WICImagingFactory,
                nullptr,
                CLSCTX_INPROC_SERVER,
                IID_PPV_ARGS(&factory));
            if (FAILED(hr)) {
                context->Unmap(stagingTexture.Get(), 0);
                return hr;
            }

            ComPtr<IWICBitmap> bitmap;
            hr = factory->CreateBitmapFromMemory(
                desc.Width,
                desc.Height,
                GUID_WICPixelFormat32bppRGBA,
                mapped.RowPitch,
                mapped.RowPitch * desc.Height,
                static_cast<BYTE*>(mapped.pData),
                &bitmap);
            context->Unmap(stagingTexture.Get(), 0);
            if (FAILED(hr)) return hr;

            ComPtr<IWICStream> stream;
            hr = factory->CreateStream(&stream);
            if (FAILED(hr)) return hr;

            hr = stream->InitializeFromFilename(filename, GENERIC_WRITE);
            if (FAILED(hr)) return hr;

            ComPtr<IWICBitmapEncoder> encoder;
            hr = factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder);
            if (FAILED(hr)) return hr;

            hr = encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache);
            if (FAILED(hr)) return hr;

            ComPtr<IWICBitmapFrameEncode> frame;
            ComPtr<IPropertyBag2> props;
            hr = encoder->CreateNewFrame(&frame, &props);
            if (FAILED(hr)) return hr;

            hr = frame->Initialize(props.Get());
            if (FAILED(hr)) return hr;

            hr = frame->SetSize(desc.Width, desc.Height);
            if (FAILED(hr)) return hr;

            WICPixelFormatGUID format = GUID_WICPixelFormat32bppRGBA;
            hr = frame->SetPixelFormat(&format);
            if (FAILED(hr)) return hr;

            hr = frame->WriteSource(bitmap.Get(), nullptr);
            if (FAILED(hr)) return hr;

            hr = frame->Commit();
            if (FAILED(hr)) return hr;

            hr = encoder->Commit();
            return hr;
        }
    }

    namespace bmp
    {
        // TODO: need to test
        HRESULT LoadTextureFromFile(
            ID3D11Device* device,
            ID3D11DeviceContext* context,
            const wchar_t* filename,
            ID3D11ShaderResourceView** textureView)
        {
            if (!device || !filename || !textureView)
                return E_INVALIDARG;

            ComPtr<IWICImagingFactory> factory;
            HRESULT hr = CoCreateInstance(
                CLSID_WICImagingFactory,
                nullptr,
                CLSCTX_INPROC_SERVER,
                IID_PPV_ARGS(&factory));
            if (FAILED(hr)) return hr;

            ComPtr<IWICBitmapDecoder> decoder;
            hr = factory->CreateDecoderFromFilename(
                filename,
                nullptr,
                GENERIC_READ,
                WICDecodeMetadataCacheOnDemand,
                &decoder);
            if (FAILED(hr)) return hr;

            ComPtr<IWICBitmapFrameDecode> frame;
            hr = decoder->GetFrame(0, &frame);
            if (FAILED(hr)) return hr;

            ComPtr<IWICFormatConverter> converter;
            hr = factory->CreateFormatConverter(&converter);
            if (FAILED(hr)) return hr;

            hr = converter->Initialize(
                frame.Get(),
                GUID_WICPixelFormat32bppBGRA, // BMP prefers BGRA
                WICBitmapDitherTypeNone,
                nullptr,
                0.0f,
                WICBitmapPaletteTypeCustom);
            if (FAILED(hr)) return hr;

            UINT width, height;
            hr = converter->GetSize(&width, &height);
            if (FAILED(hr)) return hr;

            std::vector<uint8_t> pixels(width * height * 4);
            hr = converter->CopyPixels(
                nullptr,
                width * 4,
                static_cast<UINT>(pixels.size()),
                pixels.data());
            if (FAILED(hr)) return hr;

            D3D11_TEXTURE2D_DESC texDesc = {};
            texDesc.Width = width;
            texDesc.Height = height;
            texDesc.MipLevels = 1;
            texDesc.ArraySize = 1;
            texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // Matches BGRA
            texDesc.SampleDesc.Count = 1;
            texDesc.Usage = D3D11_USAGE_DEFAULT;
            texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            texDesc.CPUAccessFlags = 0;
            texDesc.MiscFlags = 0;

            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = pixels.data();
            initData.SysMemPitch = width * 4;

            ComPtr<ID3D11Texture2D> texture;
            hr = device->CreateTexture2D(&texDesc, &initData, &texture);
            if (FAILED(hr)) return hr;

            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = texDesc.Format;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;

            hr = device->CreateShaderResourceView(texture.Get(), &srvDesc, textureView);
            return hr;
        }


        // TODO: need to test
        HRESULT SaveTextureToFile(
            ID3D11Device* device,
            ID3D11DeviceContext* context,
            ID3D11Texture2D* texture,
            const wchar_t* filename)
        {
            if (!device || !context || !texture || !filename)
                return E_INVALIDARG;

            D3D11_TEXTURE2D_DESC desc = {};
            texture->GetDesc(&desc);

            D3D11_TEXTURE2D_DESC stagingDesc = desc;
            stagingDesc.Usage = D3D11_USAGE_STAGING;
            stagingDesc.BindFlags = 0;
            stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            stagingDesc.MiscFlags = 0;

            ComPtr<ID3D11Texture2D> stagingTexture;
            HRESULT hr = device->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);
            if (FAILED(hr)) return hr;

            context->CopyResource(stagingTexture.Get(), texture);

            D3D11_MAPPED_SUBRESOURCE mapped = {};
            hr = context->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped);
            if (FAILED(hr)) return hr;

            ComPtr<IWICImagingFactory> factory;
            hr = CoCreateInstance(
                CLSID_WICImagingFactory,
                nullptr,
                CLSCTX_INPROC_SERVER,
                IID_PPV_ARGS(&factory));
            if (FAILED(hr)) {
                context->Unmap(stagingTexture.Get(), 0);
                return hr;
            }

            ComPtr<IWICBitmap> bitmap;
            hr = factory->CreateBitmapFromMemory(
                desc.Width,
                desc.Height,
                GUID_WICPixelFormat32bppBGRA, // BMP prefers BGRA
                mapped.RowPitch,
                mapped.RowPitch * desc.Height,
                static_cast<BYTE*>(mapped.pData),
                &bitmap);
            context->Unmap(stagingTexture.Get(), 0);
            if (FAILED(hr)) return hr;

            ComPtr<IWICStream> stream;
            hr = factory->CreateStream(&stream);
            if (FAILED(hr)) return hr;

            hr = stream->InitializeFromFilename(filename, GENERIC_WRITE);
            if (FAILED(hr)) return hr;

            ComPtr<IWICBitmapEncoder> encoder;
            hr = factory->CreateEncoder(GUID_ContainerFormatBmp, nullptr, &encoder);
            if (FAILED(hr)) return hr;

            hr = encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache);
            if (FAILED(hr)) return hr;

            ComPtr<IWICBitmapFrameEncode> frame;
            ComPtr<IPropertyBag2> props;
            hr = encoder->CreateNewFrame(&frame, &props);
            if (FAILED(hr)) return hr;

            hr = frame->Initialize(props.Get());
            if (FAILED(hr)) return hr;

            hr = frame->SetSize(desc.Width, desc.Height);
            if (FAILED(hr)) return hr;

            WICPixelFormatGUID format = GUID_WICPixelFormat32bppBGRA;
            hr = frame->SetPixelFormat(&format);
            if (FAILED(hr)) return hr;

            hr = frame->WriteSource(bitmap.Get(), nullptr);
            if (FAILED(hr)) return hr;

            hr = frame->Commit();
            if (FAILED(hr)) return hr;

            hr = encoder->Commit();
            return hr;
        }

    }

    namespace jpg
    {
        HRESULT LoadTextureFromFile(
            ID3D11Device* device,
            ID3D11DeviceContext* context,
            const wchar_t* filename,
            ID3D11ShaderResourceView** textureView)
        {
            if (!device || !filename || !textureView)
                return E_INVALIDARG;

            *textureView = nullptr;

            ComPtr<IWICImagingFactory> factory;
            HRESULT hr = CoCreateInstance(
                CLSID_WICImagingFactory,
                nullptr,
                CLSCTX_INPROC_SERVER,
                IID_PPV_ARGS(&factory));

            if (FAILED(hr)) return hr;

            ComPtr<IWICBitmapDecoder> decoder;
            hr = factory->CreateDecoderFromFilename(
                filename,
                nullptr,
                GENERIC_READ,
                WICDecodeMetadataCacheOnDemand,
                &decoder);

            if (FAILED(hr)) return hr;

            ComPtr<IWICBitmapFrameDecode> frame;
            hr = decoder->GetFrame(0, &frame);

            if (FAILED(hr)) return hr;

            ComPtr<IWICFormatConverter> converter;
            hr = factory->CreateFormatConverter(&converter);

            if (FAILED(hr)) return hr;

            // JPEG has no alpha channel. Convert to RGBA for a
            // consistent texture format with the PNG implementation.
            hr = converter->Initialize(
                frame.Get(),
                GUID_WICPixelFormat32bppRGBA,
                WICBitmapDitherTypeNone,
                nullptr,
                0.0,
                WICBitmapPaletteTypeCustom);

            if (FAILED(hr)) return hr;

            UINT width = 0;
            UINT height = 0;

            hr = converter->GetSize(&width, &height);

            if (FAILED(hr)) return hr;

            if (width == 0 || height == 0 ||
                width > UINT_MAX / 4 ||
                height > UINT_MAX / (width * 4))
            {
                return E_INVALIDARG;
            }

            const UINT stride = width * 4;
            const UINT bufferSize = stride * height;

            std::vector<uint8_t> pixels(bufferSize);

            hr = converter->CopyPixels(
                nullptr,
                stride,
                bufferSize,
                pixels.data());

            if (FAILED(hr)) return hr;

            D3D11_TEXTURE2D_DESC texDesc = {};
            texDesc.Width = width;
            texDesc.Height = height;
            texDesc.MipLevels = 1;
            texDesc.ArraySize = 1;
            texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            texDesc.SampleDesc.Count = 1;
            texDesc.Usage = D3D11_USAGE_DEFAULT;
            texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = pixels.data();
            initData.SysMemPitch = stride;

            ComPtr<ID3D11Texture2D> texture;
            hr = device->CreateTexture2D(
                &texDesc,
                &initData,
                &texture);

            if (FAILED(hr)) return hr;

            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = texDesc.Format;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;

            return device->CreateShaderResourceView(
                texture.Get(),
                &srvDesc,
                textureView);
        }


        HRESULT SaveTextureToFile(
            ID3D11Device* device,
            ID3D11DeviceContext* context,
            ID3D11Texture2D* texture,
            const wchar_t* filename)
        {
            if (!device || !context || !texture || !filename)
                return E_INVALIDARG;

            D3D11_TEXTURE2D_DESC desc = {};
            texture->GetDesc(&desc);

            // This implementation saves ordinary, single-sample
            // 2D textures. Copy the GPU texture to CPU-readable memory.
            if (desc.Width == 0 || desc.Height == 0 ||
                desc.SampleDesc.Count != 1 ||
                desc.ArraySize != 1 ||
                desc.MipLevels != 1 ||
                desc.Width > UINT_MAX / 4 ||
                desc.Height > UINT_MAX / (desc.Width * 4))
            {
                return E_INVALIDARG;
            }

            D3D11_TEXTURE2D_DESC stagingDesc = desc;
            stagingDesc.Usage = D3D11_USAGE_STAGING;
            stagingDesc.BindFlags = 0;
            stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            stagingDesc.MiscFlags = 0;

            ComPtr<ID3D11Texture2D> stagingTexture;
            HRESULT hr = device->CreateTexture2D(
                &stagingDesc,
                nullptr,
                &stagingTexture);

            if (FAILED(hr)) return hr;

            context->CopyResource(stagingTexture.Get(), texture);

            D3D11_MAPPED_SUBRESOURCE mapped = {};
            hr = context->Map(
                stagingTexture.Get(),
                0,
                D3D11_MAP_READ,
                0,
                &mapped);

            if (FAILED(hr)) return hr;

            ComPtr<IWICImagingFactory> factory;
            hr = CoCreateInstance(
                CLSID_WICImagingFactory,
                nullptr,
                CLSCTX_INPROC_SERVER,
                IID_PPV_ARGS(&factory));

            if (FAILED(hr))
            {
                context->Unmap(stagingTexture.Get(), 0);
                return hr;
            }

            // Match the texture's actual byte layout.
            WICPixelFormatGUID sourceFormat;

            if (desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM ||
                desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
            {
                sourceFormat = GUID_WICPixelFormat32bppRGBA;
            }
            else if (desc.Format == DXGI_FORMAT_B8G8R8A8_UNORM ||
                desc.Format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB)
            {
                sourceFormat = GUID_WICPixelFormat32bppBGRA;
            }
            else
            {
                context->Unmap(stagingTexture.Get(), 0);
                return DXGI_ERROR_UNSUPPORTED;
            }

            ComPtr<IWICBitmap> bitmap;
            hr = factory->CreateBitmapFromMemory(
                desc.Width,
                desc.Height,
                sourceFormat,
                mapped.RowPitch,
                mapped.RowPitch * desc.Height,
                static_cast<BYTE*>(mapped.pData),
                &bitmap);

            context->Unmap(stagingTexture.Get(), 0);

            if (FAILED(hr)) return hr;

            // JPEG requires a non-alpha pixel format.
            // Convert RGBA/BGRA to 24-bit BGR before encoding.
            ComPtr<IWICFormatConverter> converter;
            hr = factory->CreateFormatConverter(&converter);

            if (FAILED(hr)) return hr;

            hr = converter->Initialize(
                bitmap.Get(),
                GUID_WICPixelFormat24bppBGR,
                WICBitmapDitherTypeNone,
                nullptr,
                0.0,
                WICBitmapPaletteTypeCustom);

            if (FAILED(hr)) return hr;

            ComPtr<IWICStream> stream;
            hr = factory->CreateStream(&stream);

            if (FAILED(hr)) return hr;

            hr = stream->InitializeFromFilename(
                filename,
                GENERIC_WRITE);

            if (FAILED(hr)) return hr;

            ComPtr<IWICBitmapEncoder> encoder;
            hr = factory->CreateEncoder(
                GUID_ContainerFormatJpeg,
                nullptr,
                &encoder);

            if (FAILED(hr)) return hr;

            hr = encoder->Initialize(
                stream.Get(),
                WICBitmapEncoderNoCache);

            if (FAILED(hr)) return hr;

            ComPtr<IWICBitmapFrameEncode> frame;
            ComPtr<IPropertyBag2> props;

            hr = encoder->CreateNewFrame(&frame, &props);

            if (FAILED(hr)) return hr;

            // Set JPEG quality to 90%.
            if (props)
            {
                PROPBAG2 option = {};
                option.pstrName = const_cast<LPOLESTR>(L"ImageQuality");

                VARIANT quality;
                VariantInit(&quality);
                quality.vt = VT_R4;
                quality.fltVal = 0.9f;

                hr = props->Write(1, &option, &quality);
                VariantClear(&quality);

                if (FAILED(hr)) return hr;
            }

            hr = frame->Initialize(props.Get());

            if (FAILED(hr)) return hr;

            hr = frame->SetSize(desc.Width, desc.Height);

            if (FAILED(hr)) return hr;

            WICPixelFormatGUID format = GUID_WICPixelFormat24bppBGR;

            hr = frame->SetPixelFormat(&format);

            if (FAILED(hr)) return hr;

            // Ensure the encoder accepted the requested pixel format.
            if (!IsEqualGUID(format, GUID_WICPixelFormat24bppBGR))
                return WINCODEC_ERR_UNSUPPORTEDPIXELFORMAT;

            hr = frame->WriteSource(converter.Get(), nullptr);

            if (FAILED(hr)) return hr;

            hr = frame->Commit();

            if (FAILED(hr)) return hr;

            return encoder->Commit();
        }
    }

}
