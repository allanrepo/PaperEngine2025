#pragma once

#include <Graphics/Core/ICanvas.h>

namespace engine::graphics
{
    // Interface for platform-specific renderer implementations
    class ICanvasImpl: public ICanvas
    {
    public:
        // Virtual destructor for safe polymorphic cleanup
        virtual ~ICanvasImpl() = default;

    };
}
