#pragma once
#include <Graphics/Core/ICanvas.h>
#include <Graphics/Core/ICanvasImpl.h>
#include <memory>

namespace graphics
{
    class Canvas : public graphics::ICanvas
    {
    public:
        explicit Canvas(std::unique_ptr<graphics::ICanvasImpl> pImpl);

        virtual bool Initialize(void* pWindowHandle) override final;
        virtual void Resize(const spatial::Size<uint32_t>& size) override final;
        virtual void ShutDown() override final;                  

        virtual void Begin() override final;
        virtual void End() override final;

        //virtual void Clear(float fRed, float fGreen, float fBlue, float fAlpha) override final;
        //virtual void SetViewPort(float uiX, float uiY, float uiWidth, float uiHeight) override final;
        virtual void SetViewPort() override final;
        virtual void SetViewPort(const math::geometry::RectF& rect) override final;
        virtual void Clear(const graphics::ColorF& color) override final;
        virtual math::geometry::RectF GetViewPort() const override final;

        virtual std::string GetTypeName() const override final;

    private:
        std::unique_ptr<graphics::ICanvasImpl> impl;
    };
}

