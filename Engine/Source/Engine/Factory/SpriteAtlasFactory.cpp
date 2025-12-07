#include <Engine/Factory/SpriteAtlasFactory.h>
#include <Graphics/Renderable/SpriteAtlas.h>
#include <Cache/Registry.h>
#include <Graphics/Resource/DX11TextureImpl.h>
#include <Core/Factory.h>

std::unique_ptr<graphics::renderable::ISpriteAtlas> graphics::factory::SpriteAtlasFactory::Create()
{
    // get environment config from cache
    std::string typeName =
        cache::Registry<cache::Dictionary<>>::Instance().Has("EnvironmentConfig") ?                       // do we have environment config?
        cache::Registry<cache::Dictionary<>>::Instance().Get("EnvironmentConfig").Has("API") ?             // do we have API field in environment config?
        cache::Registry<cache::Dictionary<>>::Instance().Get("EnvironmentConfig").Get("API") :            // yes we have API field. let's get it
        graphics::dx11::resource::DX11TextureImpl::TypeName :                                                 // no API field in environment config, fallback to DX11
        graphics::dx11::resource::DX11TextureImpl::TypeName;                                                 // no config, fallback to DX11

    static bool loaded = false;
    if (!loaded)
    {
        // return value of our lambda create is an ISpriteAtlas. it doesn't matter what the flavor is created e.g. dx11,
        // we will return a pointer to a ISpriteAtlas regardless           
        core::Factory<std::string, graphics::renderable::ISpriteAtlas>::Instance().Register(
            graphics::dx11::resource::DX11TextureImpl::TypeName, []()
            {
                // instead of using std::make_unique to create SpriteAtlas, we are doing it raw. then we just pass it on an instance of unique_ptr. it's the same
                // why we did this is because SpriteAtlas' constructor is private but SpriteAtlas is a friend to SpriteAtlasFactory. 
                // however, std::make_unique is not a friend. so SpriteAtlasFactory must call the SpriteAtlas' constructor. 
                graphics::renderable::SpriteAtlas* spriteAtlas = new graphics::renderable::SpriteAtlas(std::make_unique<graphics::dx11::resource::DX11TextureImpl>());
                return std::unique_ptr<graphics::renderable::SpriteAtlas>(spriteAtlas);
            });

        loaded = true;
    }
    return core::Factory <std::string, graphics::renderable::ISpriteAtlas>::Instance().Create(typeName);
}