#pragma once

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
	}
}