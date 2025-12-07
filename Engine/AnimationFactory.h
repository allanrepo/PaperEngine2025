#pragma once
#include "CSVFile.h"
#include "Animation.h"
#include "Sprite.h"
#include "SpriteFactory.h"

namespace graphics::factory
{
	template<typename T>
	class AnimationFactory
	{
		// helper function to load animation frames from CSV file
		static bool LoadAnimationFromCSVFile(graphics::animation::Animation<T>* animation, const std::string& filename)
		{
			// read the file and get all the files to read animation, etc...
			utilities::fileio::CSVFile csvFile(filename, ',');
			if (!csvFile.read())
			{
				LOG("Failed to read CSV file: " << filename);
				return false;
			}
			int rowCount = static_cast<int>(csvFile.GetRowCount());

			for (int row = 0; row < rowCount; row++)
			{
				int numCols = static_cast<int>(csvFile.GetColCount(row));
				if (!numCols) continue;

				std::string key = csvFile.GetValue<std::string>(row, 0);

				if (key == "image_file" && numCols == 4)
				{
					std::string spriteKey = csvFile.GetValue<std::string>(row, 1);
					std::string atlasFileName = csvFile.GetValue<std::string>(row, 2);
					std::string uvFileName = csvFile.GetValue<std::string>(row, 3);

					cache::Repository<cache::Dictionary<>>::Instance().Get("SpriteToAtlasMap").Register(
						spriteKey, // sprite name
						atlasFileName  // atlas name
					);
					cache::Repository<cache::Dictionary<>>::Instance().Get("AtlasToUVRectsMap").Register(
						atlasFileName, // atlas name
						uvFileName  // uv csv file name
					);
				}
				else if (key == "frame" && numCols == 10)
				{
					std::string spriteKey = csvFile.GetValue<std::string>(row, 1);
					int spriteIndex = csvFile.GetValue<int>(row, 2);
					float duration = csvFile.GetValue<float>(row, 3);

					// define render offset (this value is expected to be normalized)
					math::VecF normalizedRenderOffset = { csvFile.GetValue<float>(row, 4),  csvFile.GetValue<float>(row, 5) };

					// create the sprite object
					graphics::resource::Sprite sprite = graphics::factory::SpriteFactory::Get(
						spriteKey, // sprite name
						spriteIndex, // sprite index
						normalizedRenderOffset // render offset
					);

					// define contact region. data from source is normalized so we convert it to absolute
					graphics::animation::Region region
					{
						math::geometry::RectF
						{
							csvFile.GetValue<float>(row, 6) * sprite.GetWidth(),
							csvFile.GetValue<float>(row, 7) * sprite.GetHeight(),
							csvFile.GetValue<float>(row, 8) * sprite.GetWidth(),
							csvFile.GetValue<float>(row, 9) * sprite.GetHeight()
						}
					};

					// create the frame and add into animation
					animation->frames.push_back(
						graphics::animation::Frame<graphics::resource::Sprite>{
							sprite,
							duration, // duration
							region
					});
				}
				else if (key == "loop" && numCols == 2)
				{
					animation->loop = csvFile.GetValue<bool>(row, 1);
				}
			}

			return true;
		}
	public:
		static std::unique_ptr<graphics::animation::Animation<T>> Create(const std::string& key, const std::string& filename)
		{
			// make animation object
			std::unique_ptr<graphics::animation::Animation<T>> animation = std::make_unique<graphics::animation::Animation<T>>();
			animation->name = key;

			// load animation frames into animation using the animation file
			if (!LoadAnimationFromCSVFile(animation.get(), filename))
			{
				LOG("Failed to load animation from file: " << filename);
				return nullptr;
			}

			return animation;
		}
	};
}