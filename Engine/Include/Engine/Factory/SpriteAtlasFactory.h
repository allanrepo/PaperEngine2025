#pragma once
#include <memory>

// this is the right way to forward declare a class like ISpriteAtlas because in SpriteAtlasFactory, we are using it on std::unique_ptr
// it wants it like this
namespace graphics::renderable
{
    class ISpriteAtlas;
}

namespace graphics::factory
{
    class SpriteAtlasFactory
    {
    public:
        static std::unique_ptr<graphics::renderable::ISpriteAtlas> Create();
    };
}


