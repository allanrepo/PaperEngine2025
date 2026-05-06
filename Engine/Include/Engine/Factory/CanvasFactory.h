#pragma once
#include <Graphics/Core/DX11CanvasImpl.h>
#include <Graphics/Core/Canvas.h>
#include <Core/Factory.h>
#include <Cache/Registry.h>
#include <memory>

namespace engine::graphics
{
	class CanvasFactory
    {
    public:
        static std::unique_ptr<engine::graphics::ICanvas> Create();

        static bool Create(const std::string& name);
    };
}


