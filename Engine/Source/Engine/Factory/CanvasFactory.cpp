#include <Engine/Factory/CanvasFactory.h>

//using namespace engine;

std::unique_ptr<engine::graphics::ICanvas> engine::graphics::CanvasFactory::Create()
{
    // get environment config from cache
    std::string typeName =
        cache::Registry<engine::container::Dictionary<>>::Instance().Has("EnvironmentConfig") ?             // do we have environment config?
        cache::Registry<engine::container::Dictionary<>>::Instance().Get("EnvironmentConfig").Has("API") ?  // do we have API field in environment config?
        cache::Registry<engine::container::Dictionary<>>::Instance().Get("EnvironmentConfig").Get("API") :  // yes we have API field. let's get it
        engine::graphics::dx11::DX11CanvasImpl::TypeName :                                                          // no API field in environment config, fallback to DX11
        engine::graphics::dx11::DX11CanvasImpl::TypeName;                                                           // no config, fallback to DX11

    static bool loaded = false;
	// TODO: we can improve this by registering all canvas types during engine initialization instead of doing it here
    if (!loaded)
    {
        engine::core::Factory<std::string, engine::graphics::Canvas>::Instance().Register(
            engine::graphics::dx11::DX11CanvasImpl::TypeName, []()
            {
                return std::make_unique<engine::graphics::Canvas>(std::make_unique<engine::graphics::dx11::DX11CanvasImpl>());
            });

        loaded = true;
    }
    return engine::core::Factory <std::string, engine::graphics::Canvas> ::Instance().Create(typeName);
}