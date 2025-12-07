#pragma once
#include <Graphics/Renderer/IRenderer.h>
#include <Graphics/Renderer/IRendererImpl.h>

namespace graphics::renderer
{
    class Renderer: public graphics::renderer::IRenderer
    {
    private:
        std::unique_ptr<graphics::renderer::IRendererImpl> impl;

    public:
        Renderer(std::unique_ptr<graphics::renderer::IRendererImpl> pImpl);
        virtual ~Renderer() = default;

        virtual std::string GetTypeName() const override final;

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        // Releases all sprite rendering resources
        virtual void ShutDown() override final;

        // Initializes the sprite renderer
        virtual bool Initialize() override final;

        // Begins a new sprite rendering batch
        virtual void Begin() override final;

        // Ends the current sprite rendering batch
        virtual void End() override final;

        // clipping region for rendering
		virtual void SetClipRegion(const math::geometry::RectF& region) override final;
		virtual void EnableClipping(const bool enable) override final;

        // Draws a colored quad at the specified position, size, and rotation
        virtual void Draw(
            const spatial::PositionF pos,                                 // Top-left screen position
            const spatial::SizeF size,                               // Sprite dimensions
            const graphics::ColorF color,                                   // RGBA color tint
            const float rotation                                                    // Rotation in radians
        ) override final;

        // Draws a string using a font atlas at the specified position and color
        virtual void DrawText(
            const graphics::renderable::IFontAtlas& font, // Font atlas
            const std::string& text,                    // Text to render
            const spatial::PositionF pos,                                 // Top-left screen position
            const graphics::ColorF color                                   // RGBA color tint
        )  override final;

        // Draws a single character using a font atlas with color and rotation
        virtual void DrawChar(
            const graphics::renderable::IFontAtlas& font, // Font atlas
            const unsigned char character,              // Character to render
            const spatial::PositionF pos,               // Top-left screen position
            const graphics::ColorF color,               // RGBA color tint
            const float rotation                        // Rotation in radians
        )  override final;

        // Draws a renderable quad with color tint and rotation
        virtual void DrawRenderable(
            const graphics::renderable::IRenderable& renderable,                    // renderable object
            const spatial::PositionF pos,                                 // Top-left screen position
            const spatial::SizeF size,                               // Sprite dimensions
            const graphics::ColorF color,                                   // RGBA color tint
            const float rotation                                                    // Rotation in radians
        )  override final;
    };


}


