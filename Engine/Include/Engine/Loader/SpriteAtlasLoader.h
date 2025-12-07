// Utility class for hydrating a SpriteAtlas with image and UV data.
//
// The SpriteAtlasLoader is responsible for loading external asset data
// into a SpriteAtlas instance. It performs two steps:
// 1. Initializes the atlas with an image file (via ISpriteAtlas::Initialize).
// 2. Reads UV rectangle definitions from a CSV file and populates the atlas
//     using ISpriteAtlas::AddUVRect.
//
// This class separates the concerns of instancing (handled by SpriteAtlasFactory)
// and hydration (loading metadata and textures). It allows SpriteAtlas to remain
// a lightweight runtime container while loaders handle external asset formats.
//     
//Typical usage:
//  - Create an atlas instance using SpriteAtlasFactory.
//  - Call SpriteAtlasLoader::Load() with the atlas, image file path, and UV CSV file path.
//  - After loading, the atlas is ready to provide UV rects for sprite creation.
 
#pragma once
#include <Graphics/Renderable/ISpriteAtlas.h>
#include <Utilities/CSVFile.h>
#include <string>
#include <memory>

namespace graphics::loader
{
    class SpriteAtlasLoader
    {
    public:
        // Load atlas image + UV rects from metadata
        static bool Load(
            graphics::renderable::ISpriteAtlas& atlas,
            const std::string& imageFilePath,
            const std::string& uvCsvFilePath
        )
        {
            // Step 1: initialize atlas with image file
			if (!atlas.Initialize(std::wstring(imageFilePath.begin(), imageFilePath.end()).c_str()))
            {
                // log error or fallback
                return false;
            }

            // Step 2: read UV rects from CSV
            utilities::fileio::CSVFile csvFile(uvCsvFilePath, ',');
            if (!csvFile.read())
            {
                // log error or fallback
                return false;
            }

            int rowCount = static_cast<int>(csvFile.GetRowCount());
            for (int row = 0; row < rowCount; ++row)
            {
                math::geometry::RectF rect
                {
                    csvFile.GetValue<float>(row, 0), // left
                    csvFile.GetValue<float>(row, 1), // top
                    csvFile.GetValue<float>(row, 2), // right
                    csvFile.GetValue<float>(row, 3)  // bottom
                };
                atlas.AddUVRect(rect);
            }

            return true;
        }
    };
}