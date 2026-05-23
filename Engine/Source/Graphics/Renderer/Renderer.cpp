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

void engine::graphics::renderer::Renderer::SetClipRegion(const engine::math::RectF& region)
{
	impl->SetClipRegion(region);
}

void engine::graphics::renderer::Renderer::EnableClipping(const bool enable)
{
	impl->EnableClipping(enable);
}

engine::math::RectF engine::graphics::renderer::Renderer::GetClipRegion() const
{
    return impl->GetClipRegion();
}

// Draws a colored quad at the specified position, size, and rotation
void engine::graphics::renderer::Renderer::Draw(
    const engine::spatial::PositionF& pos,
    const math::SizeF& size,
    const engine::graphics::ColorF& color,
    const float rotation
)
{
    impl->Draw(pos, size, color, rotation);
}

// Draws a string using a font atlas at the specified position and color
void engine::graphics::renderer::Renderer::Draw(
    const engine::graphics::resource::IFontAtlas& font, // Font atlas
    const std::string& text,                    // Text to render
    const engine::spatial::PositionF& pos,                                 // Top-left screen position
    const engine::graphics::ColorF& color
)
{
    impl->Draw(font, text, pos, color);
}

void engine::graphics::renderer::Renderer::Draw(
    const engine::graphics::Sprite& sprite,                          // sprite object
    const engine::spatial::PositionF& pos,                                 // Top-left screen position
    const math::SizeF& size,                                     // Sprite dimensions
    const engine::graphics::ColorF& color,                                   // RGBA color tint
    const float rotation                                                    // Rotation in radians
)
{
    impl->Draw(sprite, pos, size, color, rotation);
}

