#include <Graphics/Resource/Texture.h>

engine::graphics::resource::Texture::Texture(std::unique_ptr<engine::graphics::resource::ITextureImpl> pImpl):
    m_impl(std::move(pImpl))
{
}

std::string engine::graphics::resource::Texture::GetTypeName() const
{
    return m_impl->GetTypeName();
}

bool engine::graphics::resource::Texture::Initialize(unsigned int width, unsigned int height, const void* srcData, unsigned int bytesPerRow)
{
    return m_impl->Initialize(width, height, srcData, bytesPerRow);
}

bool engine::graphics::resource::Texture::Initialize(unsigned int width, unsigned int height)
{
    return m_impl->Initialize(width, height);
}

bool engine::graphics::resource::Texture::Initialize(const wchar_t* fileNamePath)
{
    return m_impl->Initialize(fileNamePath);
}

void engine::graphics::resource::Texture::BeginDraw()
{
    m_impl->BeginDraw();
}

void engine::graphics::resource::Texture::Clear(float red, float green, float blue, float alpha)
{
    m_impl->Clear(red, green, blue, alpha);
}

void engine::graphics::resource::Texture::EndDraw()
{
    m_impl->EndDraw();
}

void engine::graphics::resource::Texture::Bind() const 
{
    m_impl->Bind();
}

bool engine::graphics::resource::Texture::CanBind() const
{
    return m_impl->CanBind();
    return false;
}

const unsigned int engine::graphics::resource::Texture::GetWidth() const
{
    return m_impl->GetWidth();
}

const unsigned int engine::graphics::resource::Texture::GetHeight() const
{
    return m_impl->GetHeight();
}

void engine::graphics::resource::Texture::Reset()
{
    m_impl->Reset();
}

bool engine::graphics::resource::Texture::SaveToFile(const wchar_t* filename)
{
    return m_impl->SaveToFile(filename);
}
