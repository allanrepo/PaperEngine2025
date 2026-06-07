#pragma once

#include <string>
#include <Math/Rect.h>
#include <Graphics/Core/Color.h>
#include <Math/Size.h>

namespace engine::graphics
{
    class ICanvas
    {
    public:
        virtual ~ICanvas() = default;

        virtual bool Initialize(void* pWindowHandle) = 0;
        virtual void Resize(const math::Size<uint32_t>& size) = 0;
        virtual void ShutDown() = 0;

        virtual void Begin() = 0;
        virtual void End() = 0;

        virtual void SetViewPort(const engine::math::RectF& rect) = 0;
        virtual void Clear(const engine::graphics::ColorF& color) = 0;
        virtual void SetViewPort() = 0;
		virtual engine::math::RectF GetViewPort() const = 0;

        virtual bool SetFullscreen(bool fullscreen) = 0;
        virtual bool IsFullScreen() const = 0;

        virtual std::string GetTypeName() const = 0;
    };
}

