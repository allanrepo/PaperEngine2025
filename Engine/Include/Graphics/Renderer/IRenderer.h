#pragma once
#include <Graphics/Resource/IFontAtlas.h>
#include <Math/Rect.h>
#include <Spatial/Position.h>
#include <Spatial/Size.h>
#include <Graphics/Core/Color.h>
#include <Graphics/Core/Sprite.h>
#include <memory>
#include <string>

namespace engine::graphics::renderer
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
		virtual void SetClipRegion(const engine::math::geometry::RectF& region) = 0;
		virtual void EnableClipping(const bool enable) = 0;
        virtual engine::math::geometry::RectF GetClipRegion() const = 0;

        // Draws a colored quad at the specified position, size, and rotation
        virtual void Draw(
            const engine::spatial::PositionF& pos,                                 // Top-left screen position
            const spatial::SizeF& size,                               // Sprite dimensions
            const engine::graphics::ColorF& color,                                   // RGBA color tint
            const float rotation                                                    // Rotation in radians
        ) = 0;

        // Draws a string using a font atlas at the specified position and color
        virtual void Draw(
            const engine::graphics::resource::IFontAtlas& font, // Font atlas
            const std::string& text,                    // Text to render
            const engine::spatial::PositionF& pos,                                 // Top-left screen position
            const engine::graphics::ColorF& color                                   // RGBA color tint
        ) = 0;

        // Draws a renderable quad with color tint and rotation
        virtual void Draw(
            const engine::graphics::Sprite& sprite,                    // sprite object
            const engine::spatial::PositionF& pos,                                 // Top-left screen position
            const spatial::SizeF& size,                               // Sprite dimensions
            const engine::graphics::ColorF& color,                                   // RGBA color tint
            const float rotation                                                    // Rotation in radians
        ) = 0;
    };
}
