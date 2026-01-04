// NOTE: 
// called sprite loader instead of factory because it does not create a new object
// the sprite object is actually a view into the sprite atlas
// the method is called Load because it loads the sprite atlas if not already loaded
// TODO:
// DEPRECATED: SpriteAtlas now has method to make Sprite. please DELETE THIS

#pragma once
#include <Graphics/Renderable/ISpriteAtlas.h>
#include <Engine/Factory/SpriteAtlasFactory.h>
#include <Cache/Dictionary.h>
#include <Cache/Registry.h>
#include <Utilities/Logger.h>   
#include <Utilities/CSVFile.h>
#include <Graphics/Resource/DX11TextureImpl.h>
#include <Core/Factory.h>
#include <Graphics/Renderable/Sprite.h>

namespace graphics::loader
{
    class SpriteLoader
    {
    public:
        static graphics::renderable::Sprite Load(const std::string& key, int index)
        {
			// check lookup table for sprite key to atlas file pair. can't proceed if not found
            // TODO: consider returning a fallback sprite instead of throwing exception. this will throw exception if the dictionary does not exist
            cache::Dictionary<>& spriteToAtlasLookup = cache::Registry<cache::Dictionary<>>::Instance().Get("SpriteToAtlasMap");

			// get atlas file name using sprite key. can't proceed if not found. this will throw exception if not found.
            // TODO: consider returning a fallback atlas instead of throwing exception. this will throw exception if the dictionary does not exist
            std::string& atlasName = spriteToAtlasLookup.Get(key);

            // now we know the atlas file name. check if it is already registered in the sprite atlas Repository
            if (!cache::Registry<graphics::renderable::ISpriteAtlas>::Instance().Has(atlasName))
            {
				// TODO: we should wrap this into a method in sprite atlas Repository. this is too long
                
                // we don't have the atlas registered. let's create it now and register to our cache 
                cache::Registry<graphics::renderable::ISpriteAtlas>::Instance().Register(atlasName, graphics::factory::SpriteAtlasFactory::Create());

				// get it from Repository and initialize it with the atlas file name
                // TODO: consider having a fallback atlas rather than let this throw exception
                graphics::renderable::ISpriteAtlas& spriteAtlas = cache::Registry<graphics::renderable::ISpriteAtlas>::Instance().Get(atlasName);

                if (!spriteAtlas.Initialize(std::wstring(atlasName.begin(), atlasName.end()).c_str()))
                {
                    // TODO: consider returning a fallback sprite instead of throwing exception
                    LOG("Failed to initialize sprite atlas with image file: " << atlasName);
					throw std::exception("Failed to initialize sprite atlas with image file.");
                }

                // check first if we have the AtlasToUVRectsMap lookup table. we need this to find the csv file that contains the UVs for the atlas
                // TODO: this is gonna be tricky, but if AtlasToUVRectsMap, we're kinda fucked. figure out how to have fallback here, or this will throw exception
                cache::Dictionary<>& atlasToUVsLookup = cache::Registry<cache::Dictionary<>>::Instance().Get("AtlasToUVRectsMap");

                // get UVs csv file name. if we don't have it, we cannot proceed. this will throw an exception
                // TODO: again, a tricky one, because we need UV information. if this fails, it throws exception. how can we have fallback instead? is it even possible?
                std::string& UVsName = atlasToUVsLookup.Get(atlasName);

                // read CSV file and add sprites to atlas
                utilities::fileio::CSVFile csvFile(UVsName, ',');
                if (!csvFile.read())
                {
                    throw std::runtime_error("Failed to read CSV file");
                }
                int rowCount = static_cast<int>(csvFile.GetRowCount());
                for (int row = 0; row < rowCount; row++)
                {
                    math::geometry::RectF rect
                    {
                        csvFile.GetValue<float>(row, 0), // left
                        csvFile.GetValue<float>(row, 1), // top
                        csvFile.GetValue<float>(row, 2), // right
                        csvFile.GetValue<float>(row, 3) // bottom
                    };
                    spriteAtlas.AddUVRect(rect);
                }

				// at this point, we should have the sprite atlas initialized and populated with sprites. we are ready to get the sprite using the index and return it
            }

			// if sprite atlas is already registered, get it. make sure we got it. can't proceed if not found
            // TODO: consider having a fallback atlas rather than let this throw exception
            graphics::renderable::ISpriteAtlas& spriteAtlas = cache::Registry<graphics::renderable::ISpriteAtlas>::Instance().Get(atlasName);

			// finally, get the sprite from the sprite atlas using the index and return it
			return graphics::renderable::Sprite(&spriteAtlas, spriteAtlas.GetUVRect(index));
        }
    };
}

