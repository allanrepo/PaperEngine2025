#include <Engine/Factory/CanvasFactory.h>

std::unique_ptr<graphics::ICanvas> graphics::CanvasFactory::Create()
{
    // get environment config from cache
    std::string typeName =
        cache::Registry<cache::Dictionary<>>::Instance().Has("EnvironmentConfig") ?             // do we have environment config?
        cache::Registry<cache::Dictionary<>>::Instance().Get("EnvironmentConfig").Has("API") ?   // do we have API field in environment config?
        cache::Registry<cache::Dictionary<>>::Instance().Get("EnvironmentConfig").Get("API") :  // yes we have API field. let's get it
        graphics::dx11::DX11CanvasImpl::TypeName :                                               // no API field in environment config, fallback to DX11
        graphics::dx11::DX11CanvasImpl::TypeName;                                               // no config, fallback to DX11

    static bool loaded = false;
    if (!loaded)
    {
        core::Factory<std::string, graphics::Canvas>::Instance().Register(
            graphics::dx11::DX11CanvasImpl::TypeName, []()
            {
                return std::make_unique<graphics::Canvas>(std::make_unique<graphics::dx11::DX11CanvasImpl>());
            });

        loaded = true;
    }
    return core::Factory <std::string, graphics::Canvas> ::Instance().Create(typeName);
}