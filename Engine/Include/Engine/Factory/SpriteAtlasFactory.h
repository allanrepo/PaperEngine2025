#pragma once
#include <memory>
#include <string>
#include <vector>
#include <Math/Rect.h>

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
    private:
        // helper function to calculate UV 
        static std::vector<math::geometry::RectF> CalcUV(size_t row, size_t col, float fileWidth, float fileHeight);

    public:
        // create and return an uninitialized sprite atlas object
        static std::unique_ptr<graphics::renderable::ISpriteAtlas> Create();

        // create and return an initialized sprite atlas object
        static std::unique_ptr<graphics::renderable::ISpriteAtlas> Create(
            const std::wstring& filepath,
            const std::vector<math::geometry::RectF>& uvs
        );

        // creates an initialized sprite atlas object but store in registry with the given key name
        static bool Create(
            const std::string& name,
            const std::wstring& filepath,
            const std::vector<math::geometry::RectF>& uvs
        );

        static bool Create(
            const std::string& name,
            const std::wstring& filepath,
            const size_t row, const size_t col
        );
    };
}


