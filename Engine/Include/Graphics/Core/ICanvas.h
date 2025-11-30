#pragma once

#include <string>

namespace graphics
{
    class ICanvas
    {
    public:
        virtual ~ICanvas() = default;

        virtual bool Initialize(void* pWindowHandle) = 0;
        virtual void Resize(unsigned int uiWidth, unsigned int uiHeight) = 0;
        virtual void ShutDown() = 0;

        virtual void Begin() = 0;
        virtual void End() = 0;

        virtual void Clear(float fRed, float fGreen, float fBlue, float fAlpha) = 0;
        virtual void SetViewPort(float uiX, float uiY, float uiWidth, float uiHeight) = 0;
        virtual void SetViewPort() = 0;

        virtual std::string GetTypeName() const = 0;
    };
}

