#include <Graphics/Core/Canvas.h>

graphics::Canvas::Canvas(std::unique_ptr<graphics::ICanvasImpl> pImpl)
    : impl(std::move(pImpl))
{
}

bool graphics::Canvas::Initialize(void* pWindowHandle)
{
    return impl->Initialize(pWindowHandle);
}

void graphics::Canvas::Resize(uint32_t uiWidth, uint32_t uiHeight)
{
    impl->Resize(uiWidth, uiHeight);
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

void graphics::Canvas::Clear(float fRed, float fGreen, float fBlue, float fAlpha)
{
    impl->Clear(fRed, fGreen, fBlue, fAlpha);
}

void graphics::Canvas::SetViewPort(float uiX, float uiY, float uiWidth, float uiHeight)
{
    impl->SetViewPort(uiX, uiY, uiWidth, uiHeight);
}

void graphics::Canvas::SetViewPort()
{
    impl->SetViewPort();
}

std::string graphics::Canvas::GetTypeName() const
{
    return impl->GetTypeName();
}
