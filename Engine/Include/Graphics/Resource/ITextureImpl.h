#pragma once

#include <Graphics/Resource/ITexture.h>

namespace graphics::resource
{
    // Interface for platform-specific renderer implementations
    class ITextureImpl: public graphics::resource::ITexture
    {
    public:
        // Virtual destructor for safe polymorphic cleanup
        virtual ~ITextureImpl() = default;

    };
}
