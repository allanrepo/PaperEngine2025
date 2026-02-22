#pragma once
#include <Graphics/Resource/DX11TextureImpl.h>
#include <Graphics/Resource/ITexture.h>
#include <Core/Factory.h>
#include <Cache/Registry.h>
#include <Containers/Dictionary.h>
#include <memory>

namespace graphics
{
    class TextureFactory
    {
    public:
        static std::unique_ptr<engine::graphics::resource::ITexture> Create()
        {
            // get environment config from cache
            std::string typeName =
                engine::cache::Registry<engine::container::Dictionary<>>::Instance().Has("EnvironmentConfig") ?             // do we have environment config?
                engine::cache::Registry<engine::container::Dictionary<>>::Instance().Get("EnvironmentConfig").Has("API") ?   // do we have API field in environment config?
                engine::cache::Registry<engine::container::Dictionary<>>::Instance().Get("EnvironmentConfig").Get("API") :  // yes we have API field. let's get it
                engine::graphics::dx11::resource::DX11TextureImpl::TypeName :                                    // no API field in environment config, fallback to DX11
                engine::graphics::dx11::resource::DX11TextureImpl::TypeName;                                    // no config, fallback to DX11

            static bool loaded = false;
            if (!loaded)
            {
                engine::core::Factory<std::string, engine::graphics::resource::ITexture>::Instance().Register(
                    engine::graphics::dx11::resource::DX11TextureImpl::TypeName, []()
                    {
                        return std::make_unique<engine::graphics::dx11::resource::DX11TextureImpl>();
                    });

                loaded = true;
            }
            return engine::core::Factory<std::string, engine::graphics::resource::ITexture>::Instance().Create(typeName);
        }
    };
}


