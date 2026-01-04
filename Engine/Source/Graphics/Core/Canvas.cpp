#include <Graphics/Core/Canvas.h>

graphics::Canvas::Canvas(std::unique_ptr<graphics::ICanvasImpl> pImpl)
    : impl(std::move(pImpl))
{
}

bool graphics::Canvas::Initialize(void* pWindowHandle)
{
    return impl->Initialize(pWindowHandle);
}

void graphics::Canvas::Resize(const spatial::Size<uint32_t>& size)
{
    impl->Resize(size);
}

void graphics::Canvas::ShutDown()
{
    impl->ShutDown();
}

void graphics::Canvas::Begin()
{
    impl->Begin();
}

void graphics::Canvas::End()
{
    impl->End();
}

//void graphics::Canvas::Clear(float fRed, float fGreen, float fBlue, float fAlpha)
//{
//    impl->Clear(fRed, fGreen, fBlue, fAlpha);
//}
//
//void graphics::Canvas::SetViewPort(float uiX, float uiY, float uiWidth, float uiHeight)
//{
//    impl->SetViewPort(uiX, uiY, uiWidth, uiHeight);
//}

void graphics::Canvas::SetViewPort()
{
    impl->SetViewPort();
}

void graphics::Canvas::SetViewPort(const math::geometry::RectF& rect)
{
	impl->SetViewPort(rect);
}

void graphics::Canvas::Clear(const graphics::ColorF& color)
{
	impl->Clear(color);
}

math::geometry::RectF graphics::Canvas::GetViewPort() const
{
	return impl->GetViewPort();
}

std::string graphics::Canvas::GetTypeName() const
{
    return impl->GetTypeName();
}
