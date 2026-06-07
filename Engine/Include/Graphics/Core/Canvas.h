#pragma once
#include <Graphics/Core/ICanvas.h>
#include <Graphics/Core/ICanvasImpl.h>
#include <memory>

namespace engine::graphics
{
    class Canvas : public engine::graphics::ICanvas
    {
    public:
        explicit Canvas(std::unique_ptr<engine::graphics::ICanvasImpl> pImpl);

        virtual bool Initialize(void* pWindowHandle) override final;
        virtual void Resize(const math::Size<uint32_t>& size) override final;
        virtual void ShutDown() override final;                  

        virtual void Begin() override final;
        virtual void End() override final;

        virtual void SetViewPort() override final;
        virtual void SetViewPort(const engine::math::RectF& rect) override final;
        virtual void Clear(const engine::graphics::ColorF& color) override final;
        virtual engine::math::RectF GetViewPort() const override final;

        bool SetFullscreen(bool fullscreen) override final;
        bool IsFullScreen() const override final;

        virtual std::string GetTypeName() const override final;

    private:
        std::unique_ptr<engine::graphics::ICanvasImpl> impl;
    };
}

