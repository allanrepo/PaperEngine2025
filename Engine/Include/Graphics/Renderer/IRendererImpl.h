#pragma once

#include <Graphics/Renderer/IRenderer.h>

namespace engine::graphics::renderer
{
    class IRendererImpl: public engine::graphics::renderer::IRenderer
    {
    public:
        // Virtual destructor for safe polymorphic cleanup
        virtual ~IRendererImpl() = default;
    };
}
