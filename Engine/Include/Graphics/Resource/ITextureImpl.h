#pragma once

#include <Graphics/Resource/ITexture.h>

namespace engine::graphics::resource
{
    // Interface for platform-specific renderer implementations
    class ITextureImpl: public engine::graphics::resource::ITexture
    {
    public:
        // Virtual destructor for safe polymorphic cleanup
        virtual ~ITextureImpl() = default;

    };
}
