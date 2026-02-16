#include <Engine/Factory/SpriteAtlasFactory.h>
#include <Graphics/Renderable/SpriteAtlas.h>
#include <Cache/Registry.h>
#include <Graphics/Resource/DX11TextureImpl.h>
#include <Core/Factory.h>

// helper function to calculate a set of UV rects with the assumption that sprite atlas's extent is evenly divided 
// by given row and column
std::vector<math::geometry::RectF> graphics::factory::SpriteAtlasFactory::CalcUV(size_t row, size_t col, float fileWidth, float fileHeight)
{
    std::vector<math::geometry::RectF> uvs;
    float width = fileWidth / col;
    float height = fileHeight / row;
    float left = 0;
    float top = 0;
    float right = left + width;
    float bottom = top + height;

    for (int r = 0; r < row; r++)
    {
        for (int c = 0; c < col; c++)
        {
            left = width * c;
            top = height * r;
            right = left + width;
            bottom = top + height;

            left /= fileWidth;
            top /= fileHeight;
            right /= fileWidth;
            bottom /= fileHeight;

            uvs.push_back(math::geometry::RectF{ left, top, right, bottom });
        }
    }
    return uvs;
}

std::unique_ptr<graphics::renderable::ISpriteAtlas> graphics::factory::SpriteAtlasFactory::Create()
{
    // get environment config from cache
    std::string typeName =
        cache::Registry<std::string>::Instance().Has("API") ?            // do we have API field?
        cache::Registry<std::string>::Instance().Get("API") :            // yes we have API field. let's get it
        graphics::dx11::resource::DX11TextureImpl::TypeName;             // no API field, fallback to DX11

    static bool loaded = false;
    if (!loaded)
    {
        // return value of our lambda create is an ISpriteAtlas. it doesn't matter what the flavor is created e.g. dx11,
        // we will return a pointer to a ISpriteAtlas regardless           
        core::Factory<std::string, graphics::renderable::ISpriteAtlas>::Instance().Register(
            graphics::dx11::resource::DX11TextureImpl::TypeName, []()
            {
                // instead of using std::make_unique to create SpriteAtlas, we are doing it raw. then we just pass it on an instance of unique_ptr. it's the same
                // why we did this is because SpriteAtlas' constructor is private but SpriteAtlas is a friend to SpriteAtlasFactory. 
                // however, std::make_unique is not a friend. so SpriteAtlasFactory must call the SpriteAtlas' constructor. 
                graphics::renderable::SpriteAtlas* spriteAtlas = new graphics::renderable::SpriteAtlas(std::make_unique<graphics::dx11::resource::DX11TextureImpl>());
                return std::unique_ptr<graphics::renderable::SpriteAtlas>(spriteAtlas);
            });

        loaded = true;
    }
    return core::Factory <std::string, graphics::renderable::ISpriteAtlas>::Instance().Create(typeName);
}

bool graphics::factory::SpriteAtlasFactory::Create(
    const std::string& name, 
    const std::wstring& filepath, 
    const std::vector<math::geometry::RectF>& uvs
)
{
    std::unique_ptr<graphics::renderable::ISpriteAtlas> atlas = graphics::factory::SpriteAtlasFactory::Create(filepath, uvs);

    if (!atlas)
    {
        return false;
    }
    
    cache::Registry<graphics::renderable::ISpriteAtlas>::Instance().Register(name, std::move(atlas));

    return true;
}

// create sprite atlas with the assumption that its sprites are evenly divided into given row and column
bool graphics::factory::SpriteAtlasFactory::Create(
    const std::string& name,
    const std::wstring& filepath,
    const size_t row, const size_t col
)
{
    // create an uninitialized sprite atlas object
    std::unique_ptr<graphics::renderable::ISpriteAtlas> atlas = graphics::factory::SpriteAtlasFactory::Create();

    // make sure it is created successfully
    if (!atlas)
    {
        return false;
    }

    // load image source
    if (!atlas->Initialize(filepath.c_str()))
    {
        return false;
    }

    // create UV list and load them to atlas 
    std::vector<math::geometry::RectF> uvs = CalcUV(row, col, atlas->GetWidth(), atlas->GetHeight());
    if (!uvs.empty())
    {
        atlas->AddUVRects(uvs);
    }

    // save into cache
    cache::Registry<graphics::renderable::ISpriteAtlas>::Instance().Register(name, std::move(atlas));

    return true;
}




std::unique_ptr<graphics::renderable::ISpriteAtlas> graphics::factory::SpriteAtlasFactory::Create(
    const std::wstring& filepath,
    const std::vector<math::geometry::RectF>& uvs
)
{
    std::unique_ptr<graphics::renderable::ISpriteAtlas> atlas = graphics::factory::SpriteAtlasFactory::Create();

    if (!atlas)
    {
        return nullptr;
    }

    // load image source
    if (!atlas->Initialize(filepath.c_str()))
    {
        return nullptr;
    }

    // load uv 
    if (!uvs.empty())
    {
        atlas->AddUVRects(uvs);
    }

    return atlas;
}