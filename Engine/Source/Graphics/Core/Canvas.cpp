#include <Graphics/Core/Canvas.h>

engine::graphics::Canvas::Canvas(std::unique_ptr<engine::graphics::ICanvasImpl> pImpl)
    : impl(std::move(pImpl))
{
}

bool engine::graphics::Canvas::Initialize(void* pWindowHandle)
{
    return impl->Initialize(pWindowHandle);
}

void engine::graphics::Canvas::Resize(const math::Size<uint32_t>& size)
{
    impl->Resize(size);
}

void engine::graphics::Canvas::ShutDown()
{
    impl->ShutDown();
}

void engine::graphics::Canvas::Begin()
{
    impl->Begin();
}

void engine::graphics::Canvas::End()
{
    impl->End();
}

void engine::graphics::Canvas::SetViewPort()
{
    impl->SetViewPort();
}

void engine::graphics::Canvas::SetViewPort(const engine::math::RectF& rect)
{
	impl->SetViewPort(rect);
}

void engine::graphics::Canvas::Clear(const engine::graphics::ColorF& color)
{
	impl->Clear(color);
}

engine::math::RectF engine::graphics::Canvas::GetViewPort() const
{
	return impl->GetViewPort();
}

bool engine::graphics::Canvas::SetFullscreen(bool fullscreen)
{
    return impl->SetFullscreen(fullscreen);
}

bool engine::graphics::Canvas::IsFullScreen() const
{
    return impl->IsFullScreen();
}

std::string engine::graphics::Canvas::GetTypeName() const
{
    return impl->GetTypeName();
}
