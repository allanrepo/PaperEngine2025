#pragma once
#include <Graphics/Core/DX11CanvasImpl.h>
#include <Graphics/Core/Canvas.h>
#include <Core/Factory.h>
#include <Cache/Registry.h>
#include <Cache/Dictionary.h>
#include <memory>

using namespace graphics::dx11;

namespace graphics
{
	class CanvasFactory
    {
    public:
        static std::unique_ptr<graphics::ICanvas> Create();
    };
}


