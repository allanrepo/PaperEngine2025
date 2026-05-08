#pragma once
#include <Graphics/Resource/ISpriteAtlas.h>
#include <Cache/Registry.h>
#include <Engine/Factory/SpriteAtlasFactory.h>

namespace engine
{
	namespace graphics 
	{
		namespace loader
		{
#pragma region // SpriteAtlasLoader - factory only creates the object and creates its UV rects. loader "loads data" e.g. image file into the object
			class SpriteAtlasLoader
			{
			public:
				static engine::graphics::resource::ISpriteAtlas& Load(
					const std::string& name,
					const std::wstring& filepath,
					const size_t row, const size_t col
				)
				{
					auto& registry = engine::cache::Registry<engine::graphics::resource::ISpriteAtlas>::Instance();

					// if we don't have sprite atlas with this key in our registry, create one
					if (!registry.Has(name))
					{
						// using factory, create sprite atlas
						std::unique_ptr<engine::graphics::resource::ISpriteAtlas> atlas = engine::graphics::factory::SpriteAtlasFactory::Create(filepath, row, col);

						// register into cache
						registry.Register(name, std::move(atlas));
					}

					// return its reference
					return registry.Get(name);
				}

				static engine::graphics::resource::ISpriteAtlas& Load(
					const std::string& name,
					const std::wstring& filepath,
					const std::vector<engine::math::geometry::RectF>& uvs
				)
				{
					auto& registry = engine::cache::Registry<engine::graphics::resource::ISpriteAtlas>::Instance();

					// if we don't have sprite atlas with this key in our registry, create one
					if (!registry.Has(name))
					{
						// using factory, create sprite atlas
						std::unique_ptr<engine::graphics::resource::ISpriteAtlas> atlas = engine::graphics::factory::SpriteAtlasFactory::Create(filepath, uvs);

						// register into cache
						registry.Register(name, std::move(atlas));
					}

					// return its reference
					return registry.Get(name);
				}
			};
#pragma endregion
		}
	}
}