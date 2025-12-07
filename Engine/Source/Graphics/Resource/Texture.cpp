#include <Graphics/Resource/Texture.h>

graphics::resource::Texture::Texture(std::unique_ptr<graphics::resource::ITextureImpl> pImpl):
    m_impl(std::move(pImpl))
{
}

std::string graphics::resource::Texture::GetTypeName() const
{
    return m_impl->GetTypeName();
}

bool graphics::resource::Texture::Initialize(unsigned int width, unsigned int height, const void* srcData, unsigned int bytesPerRow)
{
    return m_impl->Initialize(width, height, srcData, bytesPerRow);
}

bool graphics::resource::Texture::Initialize(unsigned int width, unsigned int height)
{
    return m_impl->Initialize(width, height);
}

bool graphics::resource::Texture::Initialize(const wchar_t* fileNamePath)
{
    return m_impl->Initialize(fileNamePath);
}

void graphics::resource::Texture::BeginDraw()
{
    m_impl->BeginDraw();
}

void graphics::resource::Texture::Clear(float red, float green, float blue, float alpha)
{
    m_impl->Clear(red, green, blue, alpha);
}

void graphics::resource::Texture::EndDraw()
{
    m_impl->EndDraw();
}

void graphics::resource::Texture::Bind()
{
    m_impl->Bind();
}

bool graphics::resource::Texture::CanBind()
{
    return m_impl->CanBind();
    return false;
}

const unsigned int graphics::resource::Texture::GetWidth() const
{
    return m_impl->GetWidth();
}

const unsigned int graphics::resource::Texture::GetHeight() const
{
    return m_impl->GetHeight();
}

void graphics::resource::Texture::Reset()
{
    m_impl->Reset();
}

bool graphics::resource::Texture::SaveToFile(const wchar_t* filename)
{
    return m_impl->SaveToFile(filename);
}
