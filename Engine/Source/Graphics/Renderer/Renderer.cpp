#include <Graphics/Renderer/Renderer.h>

engine::graphics::renderer::Renderer::Renderer(std::unique_ptr<engine::graphics::renderer::IRendererImpl> pImpl)
    : impl(std::move(pImpl))
{
}

std::string engine::graphics::renderer::Renderer::GetTypeName() const
{
    return impl->GetTypeName();
}

// Releases all sprite rendering resources
void engine::graphics::renderer::Renderer::ShutDown()
{
    impl->ShutDown();
}

// Initializes the sprite renderer
bool engine::graphics::renderer::Renderer::Initialize()
{
    return impl->Initialize();
}

// Begins a new sprite rendering batch
void engine::graphics::renderer::Renderer::Begin()
{
    impl->Begin();
}

// Ends the current sprite rendering batch
void engine::graphics::renderer::Renderer::End()
{
    impl->End();
}

void engine::graphics::renderer::Renderer::SetClipRegion(const engine::math::geometry::RectF& region)
{
	impl->SetClipRegion(region);
}

void engine::graphics::renderer::Renderer::EnableClipping(const bool enable)
{
	impl->EnableClipping(enable);
}

// Draws a colored quad at the specified position, size, and rotation
void engine::graphics::renderer::Renderer::Draw(
    const engine::spatial::PositionF& pos,
    const spatial::SizeF& size,
    const engine::graphics::ColorF& color,
    const float rotation
)
{
    impl->Draw(pos, size, color, rotation);
}

// Draws a string using a font atlas at the specified position and color
void engine::graphics::renderer::Renderer::DrawText(
    const engine::graphics::renderable::IFontAtlas& font, // Font atlas
    const std::string& text,                    // Text to render
    const engine::spatial::PositionF& pos,                                 // Top-left screen position
    const engine::graphics::ColorF& color
)
{
    impl->DrawText(font, text, pos, color);
}

// Draws a string using a font atlas at the specified position and color
void engine::graphics::renderer::Renderer::DrawText(
    const engine::graphics::resource::IFontAtlas& font, // Font atlas
    const std::string& text,                    // Text to render
    const engine::spatial::PositionF& pos,                                 // Top-left screen position
    const engine::graphics::ColorF& color
)
{
    impl->DrawText(font, text, pos, color);
}

// Draws a single character using a font atlas with color and rotation
void engine::graphics::renderer::Renderer::DrawChar(
    const engine::graphics::renderable::IFontAtlas& font, // Font atlas
    const unsigned char character,              // Character to render
    const engine::spatial::PositionF& pos,               // Top-left screen position
    const engine::graphics::ColorF& color,               // RGBA color tint
    const float rotation                        // Rotation in radians
)
{
    impl->DrawChar(font, character, pos, color, rotation);
}

void engine::graphics::renderer::Renderer::DrawRenderable(
    const engine::graphics::renderable::IRenderable& renderable,                    // renderable object
    const engine::spatial::PositionF& pos,                                 // Top-left screen position
    const spatial::SizeF& size,                               // Sprite dimensions
    const engine::graphics::ColorF& color,                                   // RGBA color tint
    const float rotation                                                    // Rotation in radians
)
{
    impl->DrawRenderable(renderable, pos, size, color, rotation);
}

