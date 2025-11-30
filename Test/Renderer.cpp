#include "IActor.h"
#include "IFontAtlas.h"
#include "Renderer.h"

graphics::renderer::Renderer::Renderer(std::unique_ptr<graphics::renderer::IRendererImpl> pImpl)
    : impl(std::move(pImpl))
{
}

std::string graphics::renderer::Renderer::GetTypeName() const
{
    return impl->GetTypeName();
}

// Releases all sprite rendering resources
void graphics::renderer::Renderer::ShutDown()
{
    impl->ShutDown();
}

// Initializes the sprite renderer
bool graphics::renderer::Renderer::Initialize()
{
    return impl->Initialize();
}

// Begins a new sprite rendering batch
void graphics::renderer::Renderer::Begin()
{
    impl->Begin();
}

// Ends the current sprite rendering batch
void graphics::renderer::Renderer::End()
{
    impl->End();
}

void graphics::renderer::Renderer::SetClipRegion(const math::geometry::RectF& region)
{
	impl->SetClipRegion(region);
}

void graphics::renderer::Renderer::EnableClipping(const bool enable)
{
	impl->EnableClipping(enable);
}

// Draws a colored quad at the specified position, size, and rotation
void graphics::renderer::Renderer::Draw(
    const float x, const float y,               // Top-left screen position
    const float width, const float height,      // Sprite dimensions
    const float red, const float green,         // RGBA color components
    const float blue, const float alpha,
    const float rotation                        // Rotation in radians
) 
{
    impl->Draw(x, y, width, height, red, green, blue, alpha, rotation);
}

void graphics::renderer::Renderer::Draw(
    const spatial::PosF pos, 
    const spatial::SizeF size, 
    const graphics::ColorF color, 
    const float rotation
)
{
    impl->Draw(pos, size, color, rotation);
}

// Draws a string using a font atlas at the specified position and color
void graphics::renderer::Renderer::DrawText(
    const std::shared_ptr<graphics::resource::IFontAtlas>& font, // Font atlas
    const std::string& text,                    // Text to render
    const float x, const float y,               // Top-left screen position
    const float red, const float green,         // RGBA color
    const float blue, const float alpha
)
{
    impl->DrawText(font, text, x, y, red, green, blue, alpha);
}

// Draws a single character using a font atlas with color and rotation
void graphics::renderer::Renderer::DrawChar(
    const std::shared_ptr<graphics::resource::IFontAtlas>& font, // Font atlas
    const unsigned char character,            // Character to render
    const float x, const float y,             // Top-left screen position
    const float red, const float green,       // RGBA color
    const float blue, const float alpha,
    const float rotation                      // Rotation in radians
)
{
    impl->DrawChar(font, character, x, y, red, green, blue, alpha, rotation);
}

void graphics::renderer::Renderer::DrawRenderable(
    const std::shared_ptr<graphics::renderable::IRenderable>& renderable,   // renderable object
    const spatial::PosF pos,                                 // Top-left screen position
    const spatial::SizeF size,                               // Sprite dimensions
    const graphics::ColorF color,                                   // RGBA color tint
    const float rotation                                                    // Rotation in radians
)
{
    impl->DrawRenderable(renderable, pos, size, color, rotation);
}

void graphics::renderer::Renderer::Draw(
    const component::IActor& actor, 
    const graphics::ColorF tint, 
    const float rotation
)
{
    impl->Draw(actor, tint, rotation);
}


void graphics::renderer::Renderer::DrawRenderable(
    const graphics::renderable::IRenderable& renderable, 
    const spatial::PosF pos, 
    const spatial::SizeF size, 
    const graphics::ColorF color, 
    const float rotation
)
{
    impl->DrawRenderable(renderable, pos, size, color, rotation);
}
