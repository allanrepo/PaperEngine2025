#pragma once
#include <Graphics/Resource/DX11TextureImpl.h>
#include <Graphics/Resource/ITexture.h>
#include <Core/Factory.h>
#include <Cache/Registry.h>
#include <Cache/Dictionary.h>
#include <memory>

namespace graphics
{
    class TextureFactory
    {
    public:
        static std::unique_ptr<graphics::resource::ITexture> Create()
        {
            // get environment config from cache
            std::string typeName =
                cache::Registry<cache::Dictionary<>>::Instance().Has("EnvironmentConfig") ?             // do we have environment config?
                cache::Registry<cache::Dictionary<>>::Instance().Get("EnvironmentConfig").Has("API") ?   // do we have API field in environment config?
                cache::Registry<cache::Dictionary<>>::Instance().Get("EnvironmentConfig").Get("API") :  // yes we have API field. let's get it
                graphics::dx11::resource::DX11TextureImpl::TypeName :                                    // no API field in environment config, fallback to DX11
                graphics::dx11::resource::DX11TextureImpl::TypeName;                                    // no config, fallback to DX11

            static bool loaded = false;
            if (!loaded)
            {
                core::Factory<std::string, graphics::resource::ITexture>::Instance().Register(
                    graphics::dx11::resource::DX11TextureImpl::TypeName, []()
                    {
                        return std::make_unique<graphics::dx11::resource::DX11TextureImpl>();
                    });

                loaded = true;
            }
            return core::Factory<std::string, graphics::resource::ITexture>::Instance().Create(typeName);
        }
    };
}


