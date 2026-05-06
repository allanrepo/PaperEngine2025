#pragma once
#include <Spatial/Position.h>
#include <Spatial/Size.h>
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

		inline const Coord PositionToCoord(const engine::spatial::PositionF& position, const spatial::SizeF& cellsize)
		{
			return engine::spatial::Coord(
				static_cast<int>(position.y / cellsize.height),
				static_cast<int>(position.x / cellsize.width)
			);
		}

		inline const engine::spatial::PositionF CoordToPosition(const engine::spatial::Coord& coord, const spatial::SizeF& cellsize)
		{
			return engine::spatial::PositionF(
				coord.col * cellsize.width,
				coord.row * cellsize.height
			);
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