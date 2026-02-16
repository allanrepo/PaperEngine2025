#pragma once
#include <memory>
#include <string>
//#include <Graphics/Renderer/IRenderer.h>
#include <Graphics/Renderer/DX11RendererBatchImpl.h>
#include <Graphics/Renderer/DX11RendererImmediateImpl.h>
#include <Cache/Registry.h>
#include <Containers/Dictionary.h>
#include <Core/Factory.h>
#include <Graphics/Renderer/Renderer.h>

using namespace engine;

namespace engine::graphics::factory
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
        static std::unique_ptr<engine::graphics::renderer::IRenderer> Create()
        {
            // get environment config from cache
            std::string typeName =
                cache::Registry<container::Dictionary<>>::Instance().Has("EnvironmentConfig") ?                     // do we have environment config?
                cache::Registry<container::Dictionary<>>::Instance().Get("EnvironmentConfig").Has("API") ?          // do we have API field in environment config?
                cache::Registry<container::Dictionary<>>::Instance().Get("EnvironmentConfig").Get("API") :          // yes we have API field. let's get it
                engine::graphics::dx11::renderer::DX11RendererBatchImpl::TypeName :                                     // no API field in environment config, fallback to DX11
                engine::graphics::dx11::renderer::DX11RendererBatchImpl::TypeName;                                      // no config, fallback to DX11

            std::string renderMode =
                cache::Registry<container::Dictionary<>>::Instance().Has("EnvironmentConfig") ?                     // do we have environment config?
                cache::Registry<container::Dictionary<>>::Instance().Get("EnvironmentConfig").Has("RenderMode") ?   // do we have RenderMode field in environment config?
                cache::Registry<container::Dictionary<>>::Instance().Get("EnvironmentConfig").Get("RenderMode") :   // yes we have RenderMode field. let's get it
                "Batch" :                                                                                       // no RenderMode field in environment config, fallback to "Batch"
                "Batch";                                                                                        // no config, fallback to "Batch"

            // using static flag, register all known implementation once
            static bool loaded = false;
            if (!loaded)
            {
                std::pair<std::string, std::string> key;

                // register type for renderer = directx 11, batch
                key.first = engine::graphics::dx11::renderer::DX11RendererBatchImpl::TypeName;  // directx 11 renderer
                key.second = "Batch";                                                   // batch sprite renderer
                engine::core::Factory<std::pair<std::string, std::string>, engine::graphics::renderer::IRenderer, PairHasher>::Instance().Register(
                    key, []()
                    {
                        return std::make_unique<engine::graphics::renderer::Renderer>(std::make_unique<engine::graphics::dx11::renderer::DX11RendererBatchImpl>());
                    });

                // register type for renderer = directx 11, immediate
                key.first = engine::graphics::dx11::renderer::DX11RendererImmediateImpl::TypeName;          // directx 11 renderer
                key.second = "Immediate";                                                           // immediate sprite renderer
                engine::core::Factory<std::pair<std::string, std::string>, engine::graphics::renderer::IRenderer, PairHasher>::Instance().Register(
                    key, []()
                    {
                        return std::make_unique<engine::graphics::renderer::Renderer>(std::make_unique<engine::graphics::dx11::renderer::DX11RendererImmediateImpl>());
                    });

                // set to true so we never load again
                loaded = true;
            }

            // set the key with the given renderer and Renderer type
            std::pair<std::string, std::string> key;
            key.first = typeName;    // renderer mode
            key.second = renderMode; // sprite renderer mode

            std::unique_ptr<engine::graphics::renderer::IRenderer> Renderer = engine::core::Factory<std::pair<std::string, std::string>, engine::graphics::renderer::IRenderer, PairHasher>::Instance().Create(key);
            if (Renderer == nullptr)
            {
                LOGERROR("Failed to create Renderer. Renderer type is invalid. Renderer Type: " << renderMode);
				return nullptr;
            }

            return Renderer;
        }
    };
}