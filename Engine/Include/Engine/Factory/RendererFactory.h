#pragma once
#include <memory>
#include <string>
//#include <Graphics/Renderer/IRenderer.h>
#include <Graphics/Renderer/DX11RendererBatchImpl.h>
#include <Graphics/Renderer/DX11RendererImmediateImpl.h>
#include <Cache/Registry.h>
#include <Cache/Dictionary.h>
#include <Core/Factory.h>
#include <Graphics/Renderer/Renderer.h>

namespace graphics::factory
{
    class RendererFactory
    {
    public:
        struct PairHasher 
        {
            std::size_t operator()(const std::pair<std::string, std::string>& key) const
            {
                std::size_t h1 = std::hash<std::string>{}(key.first);
                std::size_t h2 = std::hash<std::string>{}(key.second);
                return h1 ^ (h2 << 1);
            }
        };

    public:
        static std::unique_ptr<graphics::renderer::IRenderer> Create()
        {
            // get environment config from cache
            std::string typeName =
                cache::Registry<cache::Dictionary<>>::Instance().Has("EnvironmentConfig") ?                     // do we have environment config?
                cache::Registry<cache::Dictionary<>>::Instance().Get("EnvironmentConfig").Has("API") ?          // do we have API field in environment config?
                cache::Registry<cache::Dictionary<>>::Instance().Get("EnvironmentConfig").Get("API") :          // yes we have API field. let's get it
                graphics::dx11::renderer::DX11RendererBatchImpl::TypeName :                                     // no API field in environment config, fallback to DX11
                graphics::dx11::renderer::DX11RendererBatchImpl::TypeName;                                      // no config, fallback to DX11

            std::string renderMode =
                cache::Registry<cache::Dictionary<>>::Instance().Has("EnvironmentConfig") ?                     // do we have environment config?
                cache::Registry<cache::Dictionary<>>::Instance().Get("EnvironmentConfig").Has("RenderMode") ?   // do we have RenderMode field in environment config?
                cache::Registry<cache::Dictionary<>>::Instance().Get("EnvironmentConfig").Get("RenderMode") :   // yes we have RenderMode field. let's get it
                "Batch" :                                                                                       // no RenderMode field in environment config, fallback to "Batch"
                "Batch";                                                                                        // no config, fallback to "Batch"

            // using static flag, register all known implementation once
            static bool loaded = false;
            if (!loaded)
            {
                std::pair<std::string, std::string> key;

                // register type for renderer = directx 11, batch
                key.first = graphics::dx11::renderer::DX11RendererBatchImpl::TypeName;  // directx 11 renderer
                key.second = "Batch";                                                   // batch sprite renderer
                core::Factory<std::pair<std::string, std::string>, graphics::renderer::IRenderer, PairHasher>::Instance().Register(
                    key, []()
                    {
                        return std::make_unique<graphics::renderer::Renderer>(std::make_unique<graphics::dx11::renderer::DX11RendererBatchImpl>());
                    });

                // register type for renderer = directx 11, immediate
                key.first = graphics::dx11::renderer::DX11RendererImmediateImpl::TypeName;          // directx 11 renderer
                key.second = "Immediate";                                                           // immediate sprite renderer
                core::Factory<std::pair<std::string, std::string>, graphics::renderer::IRenderer, PairHasher>::Instance().Register(
                    key, []()
                    {
                        return std::make_unique<graphics::renderer::Renderer>(std::make_unique<graphics::dx11::renderer::DX11RendererImmediateImpl>());
                    });

                // set to true so we never load again
                loaded = true;
            }

            // set the key with the given renderer and Renderer type
            std::pair<std::string, std::string> key;
            key.first = typeName;    // renderer mode
            key.second = renderMode; // sprite renderer mode

            std::unique_ptr<graphics::renderer::IRenderer> Renderer = core::Factory<std::pair<std::string, std::string>, graphics::renderer::IRenderer, PairHasher>::Instance().Create(key);
            if (Renderer == nullptr)
            {
                LOGERROR("Failed to create Renderer. Renderer type is invalid. Renderer Type: " << renderMode);
				return nullptr;
            }

            return Renderer;
        }
    };
}