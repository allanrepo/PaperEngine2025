#pragma once
#include <Components/Tile.h>
#include <Utilities/CSVFile.h>
#include <Math/Rect.h>
#include <functional>

namespace app::utilities
{
	namespace io
	{
		template<typename T, typename U>
		class TileGridLoader {
		public:
			static engine::component::tile::TileGrid<T> LoadFromCSV(
				const std::string& filename,
				const engine::component::tile::Tileset<T>& tileset,
				std::function<engine::component::tile::Tile<T>(int, int, const U&, const engine::component::tile::Tileset<T>&)> tileLoader
			)
			{
				engine::utilities::fileio::CSVFile csvFile(filename, ',');
				if (!csvFile.read())
				{
					throw std::runtime_error("Failed to read tile layer CSV file.");
				}

				int height = static_cast<int>(csvFile.GetRowCount());
				int width = static_cast<int>(csvFile.GetColCount(0)); // assume uniform width

				engine::component::tile::TileGrid<T> layer;
				//layer.SetSize({ width, height });

				for (int row = 0; row < height; ++row)
				{
					for (int col = 0; col < width; ++col)
					{
						// skip rows with inconsistent column count
						if (static_cast<int>(csvFile.GetColCount(row)) != width)
						{
							continue;
						}

						U cell = csvFile.GetValue<U>(row, col);

						engine::component::tile::Tile<T> tile = tileLoader(row, col, cell, tileset);

						//layer.SetTile(row, col, tile);
					}
				}

				return layer;
			}
		};

		template<typename T, typename U>
		class TileLayerLoader
		{
		public:
			static engine::component::tile::TileLayer<T> LoadFromCSV(
				const std::string& filename,
				const engine::component::tile::Tileset<T>& tileset,
				std::function<engine::component::tile::Tile<T>(int, int, const U&, const engine::component::tile::Tileset<T>&)> tileLoader
			)
			{

			}
		};
	}

	namespace graphics
	{
		static std::vector<engine::math::geometry::RectF> CalcUV(int row, int col, int fileWidth, int fileHeight)
		{
			std::vector<engine::math::geometry::RectF> uvs;
			float width = static_cast<float>(fileWidth / col);
			float height = static_cast<float>(fileHeight / row);
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

					//LOG(std::to_string(left) << ", " << std::to_string(top) << ", " << std::to_string(right) << ", " << std::to_string(bottom));
				}
			}
			return uvs;
		}
	}
}
