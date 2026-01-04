#pragma once

#include <string>
#include <Math/Rect.h>
#include <Graphics/Core/Color.h>
#include <Spatial/Size.h>

namespace graphics
{
    class ICanvas
    {
    public:
        virtual ~ICanvas() = default;

        virtual bool Initialize(void* pWindowHandle) = 0;
        virtual void Resize(const spatial::Size<uint32_t>& size) = 0;
        virtual void ShutDown() = 0;

        virtual void Begin() = 0;
        virtual void End() = 0;

        //virtual void Clear(float fRed, float fGreen, float fBlue, float fAlpha) = 0;
        //virtual void SetViewPort(float uiX, float uiY, float uiWidth, float uiHeight) = 0;
        virtual void SetViewPort(const math::geometry::RectF& rect) = 0;
        virtual void Clear(const graphics::ColorF& color) = 0;
        virtual void SetViewPort() = 0;
		virtual math::geometry::RectF GetViewPort() const = 0;

        virtual std::string GetTypeName() const = 0;
    };
}

