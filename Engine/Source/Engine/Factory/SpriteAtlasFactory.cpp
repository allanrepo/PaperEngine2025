#include <Engine/Factory/SpriteAtlasFactory.h>
#include <Graphics/Resource/SpriteAtlas.h>
#include <Cache/Registry.h>
#include <Graphics/Resource/DX11TextureImpl.h>
#include <Core/Factory.h>

// helper function to calculate a set of UV rects with the assumption that sprite atlas's extent is evenly divided 
// by given row and column
std::vector<engine::math::geometry::RectF> engine::graphics::factory::SpriteAtlasFactory::CalcUV(size_t row, size_t col, float fileWidth, float fileHeight)
{
    std::vector<engine::math::geometry::RectF> uvs;
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

            uvs.push_back(engine::math::geometry::RectF{ left, top, right, bottom });
        }
    }
    return uvs;
}

std::unique_ptr<engine::graphics::resource::ISpriteAtlas> engine::graphics::factory::SpriteAtlasFactory::Create()
{
    // get environment config from cache
    std::string typeName =
        cache::Registry<std::string>::Instance().Has("API") ?            // do we have API field?
        cache::Registry<std::string>::Instance().Get("API") :            // yes we have API field. let's get it
        engine::graphics::dx11::resource::DX11TextureImpl::TypeName;             // no API field, fallback to DX11

    static bool loaded = false;
    if (!loaded)
    {
        // return value of our lambda create is an ISpriteAtlas. it doesn't matter what the flavor is created e.g. dx11,
        // we will return a pointer to a ISpriteAtlas regardless           
        engine::core::Factory<std::string, engine::graphics::resource::ISpriteAtlas>::Instance().Register(
            engine::graphics::dx11::resource::DX11TextureImpl::TypeName, []()
            {
                // instead of using std::make_unique to create SpriteAtlas, we are doing it raw. then we just pass it on an instance of unique_ptr. it's the same
                // why we did this is because SpriteAtlas' constructor is private but SpriteAtlas is a friend to SpriteAtlasFactory. 
                // however, std::make_unique is not a friend. so SpriteAtlasFactory must call the SpriteAtlas' constructor. 
                engine::graphics::resource::SpriteAtlas* spriteAtlas = new engine::graphics::resource::SpriteAtlas(std::make_unique<engine::graphics::dx11::resource::DX11TextureImpl>());
                return std::unique_ptr<engine::graphics::resource::SpriteAtlas>(spriteAtlas);
            });

        loaded = true;
    }
    return engine::core::Factory <std::string, engine::graphics::resource::ISpriteAtlas>::Instance().Create(typeName);
}

bool engine::graphics::factory::SpriteAtlasFactory::Create(
    const std::string& name, 
    const std::wstring& filepath, 
    const std::vector<engine::math::geometry::RectF>& uvs
)
{
    std::unique_ptr<engine::graphics::resource::ISpriteAtlas> atlas = engine::graphics::factory::SpriteAtlasFactory::Create(filepath, uvs);

    if (!atlas)
    {
        return false;
    }
    
    cache::Registry<engine::graphics::resource::ISpriteAtlas>::Instance().Register(name, std::move(atlas));

    return true;
}

// create sprite atlas with the assumption that its sprites are evenly divided into given row and column
bool engine::graphics::factory::SpriteAtlasFactory::Create(
    const std::string& name,
    const std::wstring& filepath,
    const size_t row, const size_t col
)
{
    // create an uninitialized sprite atlas object
    std::unique_ptr<engine::graphics::resource::ISpriteAtlas> atlas = engine::graphics::factory::SpriteAtlasFactory::Create(filepath, row, col);

    // make sure it is created successfully
    if (!atlas)
    {
        return false;
    }

    // save into cache
    cache::Registry<engine::graphics::resource::ISpriteAtlas>::Instance().Register(name, std::move(atlas));

    return true;
}

std::unique_ptr<engine::graphics::resource::ISpriteAtlas> engine::graphics::factory::SpriteAtlasFactory::Create(const std::wstring& filepath, const size_t row, const size_t col)
{
    // create an uninitialized sprite atlas object
    std::unique_ptr<engine::graphics::resource::ISpriteAtlas> atlas = engine::graphics::factory::SpriteAtlasFactory::Create();

    // make sure it is created successfully
    if (!atlas)
    {
        return nullptr;
    }

    // load image source
    if (!atlas->Initialize(filepath.c_str()))
    {
        return nullptr;
    }

    // create UV list and load them to atlas 
    std::vector<engine::math::geometry::RectF> uvs = CalcUV(row, col, atlas->GetWidth(), atlas->GetHeight());
    if (!uvs.empty())
    {
        atlas->AddUVRects(uvs);
    }

    return atlas;
}

std::unique_ptr<engine::graphics::resource::ISpriteAtlas> engine::graphics::factory::SpriteAtlasFactory::Create(
    const std::wstring& filepath,
    const std::vector<engine::math::geometry::RectF>& uvs
)
{
    std::unique_ptr<engine::graphics::resource::ISpriteAtlas> atlas = engine::graphics::factory::SpriteAtlasFactory::Create();

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