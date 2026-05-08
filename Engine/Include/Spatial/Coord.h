#pragma once
#include <Spatial/Position.h>
#include <Spatial/Size.h>
#include <Math/Rect.h>
#include <unordered_map>
#include <vector>
#include <functional>

namespace engine
{
	namespace spatial
	{
		// represents a 2d coordinate in a map or grid.
		struct Coord
		{
			int row;
			int col;

			// equality operator: returns true if both row and col match
			bool operator==(const Coord& other) const
			{
				return row == other.row && col == other.col;
			}

			// inequality operator: returns true if either row or col differ
			bool operator!=(const Coord& other) const
			{
				return !(*this == other);
			}

			Coord(int r, int c) : row(r), col(c) {}
			Coord() : row(0), col(0) {}
		};

		inline Coord PositionToCoord(const engine::spatial::PositionF& position, const spatial::SizeF& cellsize) noexcept
		{
			return Coord(
				static_cast<int>(std::floor(position.y / cellsize.height)),
				static_cast<int>(std::floor(position.x / cellsize.width))
			);
		}

		inline engine::spatial::PositionF CoordToPosition(const engine::spatial::Coord& coord, const spatial::SizeF& cellsize) noexcept
		{
			return engine::spatial::PositionF(
				coord.col * cellsize.width,
				coord.row * cellsize.height
			);
		}

		inline std::vector<Coord> QueryCoords(
			const engine::math::geometry::RectF& boundingbox,
			const SizeF& cellsize)
		{
			// ------------------------------------------------------------
			// 1. Normalize AABB (safety against flipped rectangles)
			// ------------------------------------------------------------
			const float left = std::min<float>(boundingbox.left, boundingbox.right);
			const float right = std::max<float>(boundingbox.left, boundingbox.right);
			const float top = std::min<float>(boundingbox.top, boundingbox.bottom);
			const float bottom = std::max<float>(boundingbox.top, boundingbox.bottom);

			// ------------------------------------------------------------
			// 2. Convert world bounds -> cell coordinates
			// ------------------------------------------------------------
			Coord minCell = PositionToCoord({ left, top }, cellsize);
			Coord maxCell = PositionToCoord({ right, bottom }, cellsize);

			// ------------------------------------------------------------
			// 3. Normalize cell ordering
			// ------------------------------------------------------------
			int startRow = std::min<int>(minCell.row, maxCell.row);
			int endRow = std::max<int>(minCell.row, maxCell.row);

			int startCol = std::min<int>(minCell.col, maxCell.col);
			int endCol = std::max<int>(minCell.col, maxCell.col);

			// ------------------------------------------------------------
			// 5. Early exit if no overlap
			// ------------------------------------------------------------
			if (startRow > endRow || startCol > endCol) return {};

			// ------------------------------------------------------------
			// 6. Collect cells
			// ------------------------------------------------------------
			std::vector<Coord> result;
			result.reserve((endRow - startRow + 1) * (endCol - startCol + 1));

			for (int row = startRow; row <= endRow; ++row)
			{
				for (int col = startCol; col <= endCol; ++col)
				{
					result.push_back({ row, col });
				}
			}

			return result;
		}

		// the "aabb" or boundingbox already implies overlap. the cellsize and gridsize implies we're querying a grid/map
		static std::vector<Coord> QueryCoords(
			const  engine::math::geometry::RectF& boundingbox,
			const SizeF& cellsize,
			const Size<size_t> gridsize)
		{
			// ------------------------------------------------------------
			// 1. Normalize AABB (safety against flipped rectangles)
			// ------------------------------------------------------------
			const float left = std::min<float>(boundingbox.left, boundingbox.right);
			const float right = std::max<float>(boundingbox.left, boundingbox.right);
			const float top = std::min<float>(boundingbox.top, boundingbox.bottom);
			const float bottom = std::max<float>(boundingbox.top, boundingbox.bottom);

			// ------------------------------------------------------------
			// 2. Convert world bounds -> cell coordinates
			// ------------------------------------------------------------
			Coord minCell = PositionToCoord({ left, top }, cellsize);
			Coord maxCell = PositionToCoord({ right, bottom }, cellsize);

			// ------------------------------------------------------------
			// 3. Normalize cell ordering
			// ------------------------------------------------------------
			int startRow = std::min<int>(minCell.row, maxCell.row);
			int endRow = std::max<int>(minCell.row, maxCell.row);

			int startCol = std::min<int>(minCell.col, maxCell.col);
			int endCol = std::max<int>(minCell.col, maxCell.col);

			// ------------------------------------------------------------
			// 4. Clamp to grid bounds (avoid negative / overflow access)
			// ------------------------------------------------------------
			startRow = std::max<int>(0, startRow);
			startCol = std::max<int>(0, startCol);

			endRow = std::min<int>(static_cast<int>(gridsize.height) - 1, endRow);
			endCol = std::min<int>(static_cast<int>(gridsize.width) - 1, endCol);

			// ------------------------------------------------------------
			// 5. Early exit if no overlap
			// ------------------------------------------------------------
			if (startRow > endRow || startCol > endCol)
				return {};

			// ------------------------------------------------------------
			// 6. Collect cells
			// ------------------------------------------------------------
			std::vector<Coord> result;
			result.reserve((endRow - startRow + 1) * (endCol - startCol + 1));

			for (int row = startRow; row <= endRow; ++row)
			{
				for (int col = startCol; col <= endCol; ++col)
				{
					result.push_back({ row, col });
				}
			}

			return result;
		}
	}
}

namespace std 
{
	template<> struct hash<engine::spatial::Coord> 
	{
		size_t operator()(engine::spatial::Coord const& c) const noexcept 
		{
			return (static_cast<size_t>(c.row) << 32) ^ static_cast<size_t>(static_cast<unsigned>(c.col));
		}
	};
}