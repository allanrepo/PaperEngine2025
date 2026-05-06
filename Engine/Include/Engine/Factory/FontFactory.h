#pragma once
#include <memory>
#include <string>
#include <vector>
#include <Math/Rect.h>

#include <Graphics/Resource/FontAtlas.h>
#include <Graphics/Resource/IFontAtlas.h>
#include <Cache/Registry.h>
#include <Graphics/Resource/DX11TextureImpl.h>
#include <Core/Factory.h>
#include <Graphics/Resource/SpriteAtlas.h>


// this is the right way to forward declare a class like ISpriteAtlas because in SpriteAtlasFactory, we are using it on std::unique_ptr
// it wants it like this
namespace engine::graphics::resource
{
    class IFontAtlas;
}

namespace engine::graphics::factory
{
    class FontFactory
    {
        using FontAtlas = engine::graphics::resource::FontAtlas;
        using IFontAtlas = engine::graphics::resource::IFontAtlas;
        using SpriteAtlas = engine::graphics::resource::SpriteAtlas;
        using DX11TextureImpl = engine::graphics::dx11::resource::DX11TextureImpl;
        using Registry = engine::cache::Registry<std::string>;

    private:

    public:
        // create and return an uninitialized font atlas object
        static std::unique_ptr<IFontAtlas> Create()
        {
            // get environment config from cache
            std::string typeName =
                Registry::Instance().Has("API") ?      // do we have API field?
                Registry::Instance().Get("API") :      // yes we have API field. let's get it
                DX11TextureImpl::TypeName;             // no API field, fallback to DX11

            static bool loaded = false;
            if (!loaded)
            {
                // return value of our lambda create is an IFontAtlas. it doesn't matter what the flavor is created e.g. dx11,
                // we will return a pointer to a IFontAtlas regardless           
                engine::core::Factory<std::string, IFontAtlas>::Instance().Register(
                    DX11TextureImpl::TypeName, []()
                    {
                        return std::make_unique<FontAtlas>(std::make_unique<SpriteAtlas>(std::make_unique<DX11TextureImpl>()));
                    });

                loaded = true;
            }
            return engine::core::Factory<std::string, IFontAtlas>::Instance().Create(typeName);
        }

        // create and return an initialized font atlas object
        static std::unique_ptr<engine::graphics::resource::IFontAtlas> Create(
            const std::string& font,
            const size_t size
        )
        {
            std::unique_ptr<IFontAtlas> fontAtlas = Create();
            fontAtlas->Initialize(font, size);
            return fontAtlas;
        }

        // creates an uninitialized font atlas object but store in registry with the given key name
        static bool Create(const std::string& name)
        {
            std::unique_ptr<IFontAtlas> atlas = Create();

            if (!atlas)
            {
                return false;
            }

            cache::Registry<IFontAtlas>::Instance().Register(name, std::move(atlas));
            return true;
        }

        // creates an initialized font atlas object but store in registry with the given key name
        static bool Create(
            const std::string& name,
            const std::string& font,
            const unsigned int size
        )
        {
            std::unique_ptr<IFontAtlas> atlas = Create(font, size);

            if (!atlas)
            {
                return false;
            }

            cache::Registry<IFontAtlas>::Instance().Register(name, std::move(atlas));
            return true;
        }
    };
}


