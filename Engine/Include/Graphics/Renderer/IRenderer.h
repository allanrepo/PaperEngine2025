#pragma once
#include <Graphics/Renderable/IFontAtlas.h>
#include <Math/Rect.h>
#include <Spatial/Position.h>
#include <Spatial/Size.h>
#include <Graphics/Core/Color.h>
#include <Graphics/Renderable/IRenderable.h>
#include <memory>
#include <string>

namespace graphics::renderer
{
    class IRenderer
    {
    public:
        // Virtual destructor for safe polymorphic cleanup
        virtual ~IRenderer() = default;

        virtual std::string GetTypeName() const = 0;

        // Releases all sprite rendering resources
        virtual void ShutDown() = 0;

        // Initializes the sprite renderer
        virtual bool Initialize() = 0;

        // Begins a new sprite rendering batch
        virtual void Begin() = 0;

        // Ends the current sprite rendering batch
        virtual void End() = 0;

		// clipping region for rendering
		virtual void SetClipRegion(const math::geometry::RectF& region) = 0;
		virtual void EnableClipping(const bool enable) = 0;

        // Draws a colored quad at the specified position, size, and rotation
        virtual void Draw(
            const spatial::PositionF pos,                                 // Top-left screen position
            const spatial::SizeF size,                               // Sprite dimensions
            const graphics::ColorF color,                                   // RGBA color tint
            const float rotation                                                    // Rotation in radians
        ) = 0;

        // Draws a string using a font atlas at the specified position and color
        virtual void DrawText(
            const graphics::renderable::IFontAtlas& font, // Font atlas
            const std::string& text,                    // Text to render
            const spatial::PositionF pos,                                 // Top-left screen position
            const graphics::ColorF color                                   // RGBA color tint
        ) = 0;

        // Draws a single character using a font atlas with color and rotation
        virtual void DrawChar(
            const graphics::renderable::IFontAtlas& font, // Font atlas
            const unsigned char character,              // Character to render
            const spatial::PositionF pos,               // Top-left screen position
            const graphics::ColorF color,               // RGBA color tint
            const float rotation                        // Rotation in radians
        ) = 0;

        // Draws a renderable quad with color tint and rotation
        virtual void DrawRenderable(
            const graphics::renderable::IRenderable& renderable,                    // renderable object
            const spatial::PositionF pos,                                 // Top-left screen position
            const spatial::SizeF size,                               // Sprite dimensions
            const graphics::ColorF color,                                   // RGBA color tint
            const float rotation                                                    // Rotation in radians
        ) = 0;
    };
}
