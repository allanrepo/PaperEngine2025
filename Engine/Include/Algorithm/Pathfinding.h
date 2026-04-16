#pragma once
#include <Spatial/Coord.h>
#include <Spatial/Size.h>
#include <Math/Rect.h>
#include <Containers/Grid.h>
#include <functional>
#include <memory>
#include <queue>

namespace engine::navigation
{
	namespace tile
	{
		// the ITileNavigationResolver interface defines how movement between tiles is validated.
		// it decouples pathfinding mechanics(handled by PathFinder) from tile semantics(rules about walls, diagonals, blocked centers, etc.)
		// application can inherit from this interface to implement their own tile rules
		class ITileNavigationResolver
		{
		public:
			virtual bool CanMove(const engine::spatial::Coord& curr, const engine::spatial::Coord& next) = 0;
		};

		// TileConstraint is a bitmask enum representing walls, corners, and blocked regions of a tile.
		// Basic edges : N, E, S, W
		// Corners : NE, NW, SE, SW
		// Center : CENTER(fully blocked tile)
		// Combinations:
		// 		BLOCKED → all bits set(tile is fully blocked).
		// 		N_WALL, E_WALL, S_WALL, W_WALL → walls along edges including adjacent corners.
		//		Half‑triangles(NE_HALFTRI, etc.) → partial blocking shapes.
		// 
		// Imagine each tile as a square with 8 edges/corners plus a center:
		//   NW   N   NE
		//    +---+---+
		//    |       |
		// 	W |   C   | E
		//    |       |
		//    +---+---+
		//   SW   S   SE
		// Edges (N, E, S, W) → block movement across that edge.
		// Corners(NE, NW, SE, SW) → block diagonal entry.
		// CENTER → tile is fully blocked.
		// Walls(N_WALL, etc.) → edge + adjacent corners blocked.
		// Half‑triangles → partial blocking shapes(e.g., NE_HALFTRI blocks north / east half of tile).
		enum class TileConstraint : unsigned int
		{
			NONE = 0b000000000,	// walkable
			N = 0b000000001,  // bit 0
			E = 0b000000010,  // bit 1
			S = 0b000000100,  // bit 2
			W = 0b000001000,  // bit 3
			NE = 0b000010000,  // bit 4
			NW = 0b000100000,  // bit 5
			SE = 0b001000000,  // bit 6
			SW = 0b010000000,  // bit 7
			CENTER = 0b100000000,  // bit 8

			// fully blocked 
			BLOCKED = N | E | S | W | NE | NW | SE | SW | CENTER,

			// Half-block triangles
			NE_HALFTRI = N | E | NE | SE | NW | CENTER,
			NW_HALFTRI = N | W | NW | NE | SW | CENTER,
			SE_HALFTRI = S | E | SE | NE | SW | CENTER,
			SW_HALFTRI = S | W | SW | NW | SE | CENTER,

			// Walls (edge + adjacent corners)
			N_WALL = N | NE | NW,
			E_WALL = E | NE | SE,
			S_WALL = S | SE | SW,
			W_WALL = W | NW | SW
		};

		constexpr TileConstraint operator | (TileConstraint lhs, TileConstraint rhs)
		{
			return static_cast<TileConstraint>(static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs));
		}

		constexpr TileConstraint operator & (TileConstraint lhs, TileConstraint rhs) 
		{
			return static_cast<TileConstraint>(static_cast<unsigned int>(lhs) & static_cast<unsigned int>(rhs));
		}

		constexpr TileConstraint operator ^ (TileConstraint lhs, TileConstraint rhs) 
		{
			return static_cast<TileConstraint>(static_cast<unsigned int>(lhs) ^ static_cast<unsigned int>(rhs));
		}

		constexpr TileConstraint operator ~ (TileConstraint val) 
		{
			return static_cast<TileConstraint>(~static_cast<unsigned int>(val));
		}

		constexpr TileConstraint& operator |= (TileConstraint& lhs, TileConstraint rhs) 
		{
			lhs = lhs | rhs;
			return lhs;
		}

		constexpr TileConstraint& operator &= (TileConstraint& lhs, TileConstraint rhs) 
		{
			lhs = lhs & rhs;
			return lhs;
		}

		constexpr TileConstraint& operator ^= (TileConstraint& lhs, TileConstraint rhs) 
		{
			lhs = lhs ^ rhs;
			return lhs;
		}

		// a more sophisticated resolver for maps with complex tile constraints (walls, corners, half‑triangles, blocked centers).
		// how it works:
		//	uses a std::function<TileConstraint(int row, int col)> to retrieve a tile’s constraint bitmask.
		//  each tile can encode multiple blocking features : edges, corners, center, or composite shapes.
		//  cardinal moves :
		//		denied if the current tile has a wall on the moving edge, or if the destination tile has a wall on the opposite edge.
		//  diagonal moves : 
		//		denied if adjacent corner tiles or walls block the diagonal path.
		//		checks both the current and destination tile’s constraints, plus their adjacent neighbors. 
		//		encodes nuanced rules like half‑triangles and wall composites, ensuring realistic movement restrictions.
		// use case: 
		//	ideal for maps with detailed geometry(e.g., tiles representing slopes, partial walls, or irregular obstacles). 
		//	provides fine‑grained control over movement semantics. 
		//	slightly heavier than the binary resolver, but necessary for complex environments.
		// 
		// half triangle block are tiles where 2 adjacent sides are blocked as well as corners connected to either of them,
		// plus center, making the tile not eligible as next tile.
		// the terms crnc, nrcc are neighbor tiles of both current and next tile that are diagonal to each other.
		// SE_HALFTRI (south-east triangle blocked):
		// 	+-------++-------+
		//	|       ||\#CRNC#| # = blocked
		//	|   C   || \#####| CURR = source tile
		//	|   U   ||  \####| NEXT = destination tile
		//	|   R   ||   \###| CRNC and NRCC 
		//	|   R   ||    \##|	- neighbor tiles adjacent to both CURR and NEXT tiles
		//	|       ||     \#|	- CR stands for CURR.ROW and NC for NEXT.COL
		//	+-------++-------+  - NR stands for NEXT.ROW and CC for CURR.COL
		// 	+-------++-------+
		//	|\      ||       |
		//	|#\     ||   N   |
		//	|##\    ||   E   |
		//	|###\   ||   X   |
		//	|####\  ||   T   |
		//	|NRCC#\ ||       |
		//	+-------++-------+
		//
		// NW_HALFTRI (north-west triangle blocked):
		// 	+-------++-------+
		//	|       ||\#NRCC#| # = blocked
		//	|   N   || \#####| CURR = source tile
		//	|   E   ||  \####| NEXT = destination tile
		//	|   X   ||   \###| CRNC and NRCC 
		//	|   T   ||    \##|	- neighbor tiles adjacent to both CURR and NEXT tiles
		//	|       ||     \#|	- CR stands for CURR.ROW and NC for NEXT.COL
		//	+-------++-------+  - NR stands for NEXT.ROW and CC for CURR.COL
		// 	+-------++-------+
		//	|\      ||       |
		//	|#\     ||   C   |
		//	|##\    ||   U   |
		//	|###\   ||   R   |
		//	|####\  ||   R   |
		//	|CRNC#\ ||       |
		//	+-------++-------+	
		// 	
		// SW_HALFTRI (south-west triangle blocked):
		//  +-------++-------+
		//  |#CRNC#/||       |  
		//  |#####/ ||   C   |  
		//  |####/  ||   U   |  
		//  |###/   ||   R   |  
		//  |##/    ||   R   |  
		//  |#/     ||       |  
		//  +-------++-------+  
		//  +-------++-------+
		//  |       ||      /|
		//  |   N   ||     /#|
		//  |   E   ||    /##|
		//  |   X   ||   /###|
		//  |   T   ||  /####|
		//  |       || /NRCC#|
		//  +-------++-------+
		// 
		// NE_HALFTRI (south-west triangle blocked):
		//  +-------++-------+
		//  |#NRCC#/||       |  
		//  |#####/ ||   N   |  
		//  |####/  ||   E   |  
		//  |###/   ||   X   |  
		//  |##/    ||   T   |  
		//  |#/     ||       |  
		//  +-------++-------+  
		//  +-------++-------+
		//  |       ||      /|
		//  |   C   ||     /#|
		//  |   U   ||    /##|
		//  |   R   ||   /###|
		//  |   R   ||  /####|
		//  |       || /CRNC#|
		//  +-------++-------+
		class TileNavigationResolver: public ITileNavigationResolver
		{
		private:
			enum class Direction
			{
				NE,
				SE,
				NW,
				SW,
				N,
				S,
				W,
				E,
				NONE
			};

			std::function<TileConstraint(int row, int col)> m_getConstraint;

			bool HasAnyBits(const TileConstraint tile, const TileConstraint mask) const 
			{
				return (tile & mask) != TileConstraint::NONE;
			}

			Direction GetDirection(const engine::spatial::Coord& curr, const engine::spatial::Coord& next)
			{
				int dr = next.row - curr.row;
				int dc = next.col - curr.col;

				if (dr == -1 && dc == 0) return Direction::N;
				if (dr == 1 && dc == 0) return Direction::S;
				if (dr == 0 && dc == -1) return Direction::W;
				if (dr == 0 && dc == 1) return Direction::E;
				if (dr == -1 && dc == 1) return Direction::NE;
				if (dr == 1 && dc == 1) return Direction::SE;
				if (dr == -1 && dc == -1) return Direction::NW;
				if (dr == 1 && dc == -1) return Direction::SW;
				return Direction::NONE;
			}

			bool IsDiagonal(const engine::spatial::Coord& curr, const engine::spatial::Coord& next)
			{
				return (curr.row != next.row) && (curr.col != next.col);
			}

			bool IsCardinal(const engine::spatial::Coord& curr, const engine::spatial::Coord& next)
			{
				return (curr.row == next.row) ^ (curr.col == next.col);
			}

			bool CanMoveDiagonally(const TileConstraint crcc, const TileConstraint nrnc, const TileConstraint crnc, const TileConstraint nrcc, Direction dir);
			bool CanMoveCardinally(const TileConstraint crcc, const TileConstraint nrnc, Direction dir);

		public:
			TileNavigationResolver(
				std::function<TileConstraint(int row, int col)> getConstraint
			):
				m_getConstraint(getConstraint)
			{
			}

			bool CanMove(const engine::spatial::Coord& curr, const engine::spatial::Coord& next) override final;
		};

		// a lightweight resolver for simple maps where tiles are either walkable or blocked.
		// 
		// how it works:
		//	uses a std::function<bool(int row, int col)> to determine if a tile is walkable.
		//	if the target tile is blocked, movement is denied.
		//	for cardinal moves(N, S, E, W), it only checks the destination tile.
		//	for diagonal moves, it additionally checks the two adjacent cardinal neighbors(to prevent corner‑cutting through blocked tiles).
		//
		// use case:
		//	ideal for grids with binary states(e.g., walls vs.open floor).
		//	fast and simple, minimal overhead.
		//	good for performance‑critical pathfinding where complex tile semantics aren’t needed.
		class BinaryNavigationResolver : public ITileNavigationResolver
		{
		private:
			std::function<bool(int row, int col)> m_isWalkable;
			bool IsDiagonal(const engine::spatial::Coord& curr, const engine::spatial::Coord& next)
			{
				return (curr.row != next.row) && (curr.col != next.col);
			}

		public:
			BinaryNavigationResolver(
				std::function<bool(int row, int col)> isWalkable
			) :
				m_isWalkable(isWalkable)
			{
			}

			bool CanMove(const engine::spatial::Coord& curr, const engine::spatial::Coord& next) override final;
		};

		constexpr int CardinalCost = 10;
		constexpr int DiagonalCost = 14;

		struct Node
		{
			engine::spatial::Coord pos;
			int g = 0;
			int h = 0;
			engine::spatial::Coord parent;
			bool closed = false;
			bool open = false;

			int f() const
			{
				return g + h;
			}
		};

		enum class HeuristicType
		{
			Manhattan,
			Euclidean,
			Octile
		};

		class PathFinder
		{
		protected:
			// for debugging purposes, we keep track of all nodes, open tiles, and closed tiles
			std::vector<std::vector<Node>> m_nodes;
			std::vector<engine::spatial::Coord> m_openTiles;
			std::vector<engine::spatial::Coord> m_closedTiles;
			int m_steps;

			bool m_diagonal;
			int m_maxSteps;
			navigation::tile::HeuristicType m_heuristicType;

			std::unique_ptr<ITileNavigationResolver> m_tileNavigationResolver;

			int Heuristic(const engine::spatial::Coord& a, const engine::spatial::Coord& b) const;

			int HeuristicEuclidean(const engine::spatial::Coord& a, const engine::spatial::Coord& b) const;

			int HeuristicManhattan(const engine::spatial::Coord& a, const engine::spatial::Coord& b) const;

			// Octile heuristic
			int HeuristicOctile(const engine::spatial::Coord& a, const engine::spatial::Coord& b) const;

			std::vector<engine::spatial::Coord> GetNeighbors(
				const engine::spatial::Coord& pos,
				const int width, const int height
			) const;

		public:
			PathFinder(
				std::unique_ptr<ITileNavigationResolver> tileNavigationResolver,
				bool diagonal = false,
				int maxSteps = 1000,
				navigation::tile::HeuristicType heuristicType = navigation::tile::HeuristicType::Octile
			) :
				m_maxSteps(maxSteps),
				m_diagonal(diagonal),
				m_heuristicType(heuristicType),
				m_tileNavigationResolver(std::move(tileNavigationResolver)),
				m_steps(0)
			{
			}

			const std::vector<engine::spatial::Coord>& GetOpenTiles() const
			{
				return m_openTiles;
			}

			const std::vector<engine::spatial::Coord>& GetClosedTiles() const
			{
				return m_closedTiles;
			}

			const std::vector<std::vector<Node>>& GetNodes() const
			{
				return m_nodes;
			}

			void EnableDiagonal(bool enabled)
			{
				m_diagonal = enabled;
			}

			bool IsDiagonalEnabled() const
			{
				return m_diagonal;
			}

			void SetMaxSteps(int steps)
			{
				m_maxSteps = steps;
			}

			virtual bool FindPath(
				const math::geometry::Rect<int>& region,
				const engine::spatial::Coord& start,
				const engine::spatial::Coord& goal,
				std::vector<engine::spatial::Coord>& outPath
			);
		};

		class PathFinderUsingPriorityQueue : public PathFinder
		{
		private:
			struct NodeComparator
			{
				const std::vector<std::vector<Node>>* nodes;

				NodeComparator(const std::vector<std::vector<Node>>* n) : nodes(n) {}

				bool operator()(const engine::spatial::Coord& a,
					const engine::spatial::Coord& b) const
				{
					const Node& na = (*nodes)[a.row][a.col];
					const Node& nb = (*nodes)[b.row][b.col];

					int fa = na.g + na.h;
					int fb = nb.g + nb.h;

					if (fa == fb)
						return na.h > nb.h; // prefer lower h
					return fa > fb;         // prefer lower f
				}
			};
			std::priority_queue<engine::spatial::Coord, std::vector<engine::spatial::Coord>, NodeComparator> openTiles;

		public:
			PathFinderUsingPriorityQueue(
				std::unique_ptr<ITileNavigationResolver> tileNavigationResolver,
				bool diagonal = false,
				int maxSteps = 1000,
				navigation::tile::HeuristicType heuristicType = navigation::tile::HeuristicType::Octile
			):
				PathFinder(
					std::move(tileNavigationResolver),
					diagonal,
					maxSteps,
					heuristicType
				),
				openTiles(NodeComparator(nullptr))
			{
			}

			const std::vector<engine::spatial::Coord> GetOpenTiles() const;

			virtual bool FindPath(
				const engine::math::geometry::Rect<int>& region,
				const engine::spatial::Coord& start,
				const engine::spatial::Coord& goal,
				std::vector<engine::spatial::Coord>& outPath
			);
		};

		std::vector<engine::spatial::Coord> GetWayPoints(const std::vector<engine::spatial::Coord>& path);

		// this helper function checks if any tiles in the region of the map coordinates a and b occupy is walkable or not
		// it uses predicate so caller can define a lambda to evaluate if tile is walkable or not
		template<typename Predicate>
		bool IsRegionClear(
			const engine::spatial::Coord& a,
			const engine::spatial::Coord& b,
			Predicate&& isWalkable
		)
		{
			int mincol = static_cast<int>(std::floor(std::min<int>(a.col, b.col)));
			int maxcol = static_cast<int>(std::floor(std::max<int>(a.col, b.col)));
			int minrow = static_cast<int>(std::floor(std::min<int>(a.row, b.row)));
			int maxrow = static_cast<int>(std::floor(std::max<int>(a.row, b.row)));

			for (int row = minrow; row <= maxrow; ++row)
			{
				for (int col = mincol; col <= maxcol; ++col)
				{
					if (!isWalkable(row, col))
					{
						return false;
					}
				}
			}
			return true;
		}

		template<typename Predicate>
		std::vector<engine::spatial::Coord> SmoothWayPoints(
			const std::vector<engine::spatial::Coord>& waypoints,
			Predicate&& isWalkable
		)
		{
			std::vector<engine::spatial::Coord> smoothed;

			if (waypoints.empty())
			{
				return smoothed;
			}

			// Always keep the first waypoint
			smoothed.push_back(waypoints.front());

			size_t i = 0;
			while (i < waypoints.size() - 1)
			{
				size_t j = waypoints.size() - 1;

				// Try to jump as far ahead as possible
				for (; j > i + 1; --j)
				{
					if (IsRegionClear(waypoints[i], waypoints[j], isWalkable))
					{
						break; // found a clear jump
					}
				}

				// Keep the farthest reachable waypoint
				smoothed.push_back(waypoints[j]);
				i = j;
			}

			return smoothed;
		}


#pragma region // ConstraintGrid - grid that stores constraint value of each cell. it also has pathfinding feature
		class ConstraintGrid
		{
			using Coord = engine::spatial::Coord;
			using Grid = engine::container::Grid<TileConstraint>;
			using Rect = engine::math::geometry::Rect<int>;
			using Size = engine::spatial::Size<size_t>;

		private:
#pragma region // parameters
			Grid m_map;
			PathFinder m_pathFinder;
			TileConstraint m_default;

#pragma endregion

		public:
#pragma region // constructor, destructors, copy, move assignment operators
			ConstraintGrid() :
				m_pathFinder(
					std::make_unique<TileNavigationResolver>(
						[this](int row, int col) -> TileConstraint
						{
							return m_map.Get(row, col);
						}),
					true
				),
				m_default(TileConstraint::NONE)
			{
			}
#pragma endregion

#pragma region // accessors
			TileConstraint Get(int row, int col) const
			{
				return m_map.Get(row, col);
			}

			TileConstraint Get(const Coord& coord) const
			{
				return m_map.Get(coord);
			}
#pragma endregion

#pragma region // path finding
			bool FindPath(const Coord& start, const Coord& end, std::vector<Coord>& path)
			{
				Rect map = { 0, 0, (int)m_map.GetWidth(), (int)m_map.GetHeight() };

				return m_pathFinder.FindPath(
					map,
					start,
					end,
					path
				);
			}
#pragma endregion

#pragma region // content management
			void Reset()
			{
				m_map.Clear();
			}

			void Clear()
			{
				Fill(m_default);
			}

			void Initialize(size_t width, size_t height, TileConstraint constraint)
			{
				Reset();
				m_map.SetWidth(width);
				m_map.Reserve({ width, height });

				for (size_t i = 0; i < width * height; ++i)
				{
					m_map.Add(constraint);
				}
			}

			void Initialize(Size size, TileConstraint constraint)
			{
				Initialize(size.width, size.height, constraint);
			}

			void Fill(TileConstraint constraint)
			{
				for (int row = 0; row < m_map.GetHeight(); row++)
				{
					for (int col = 0; col < m_map.GetWidth(); col++)
					{
						m_map.Set(row, col, constraint);
					}
				}
			}

			void Set(int row, int col, TileConstraint constraint)
			{
				m_map.Set(row, col, constraint);
			}

			void Set(const Coord& coord, TileConstraint constraint)
			{
				m_map.Set(coord, constraint);
			}

			void SetDefault(TileConstraint constraint)
			{
				m_default = constraint;
			}

			void Replace(TileConstraint oldValue, TileConstraint newValue)
			{
				for (int row = 0; row < m_map.GetHeight(); row++)
				{
					for (int col = 0; col < m_map.GetWidth(); col++)
					{
						if (m_map.Get(row, col) == oldValue)  // exact match
						{
							m_map.Set(row, col, newValue);
						}
					}
				}
			}

			void AddFlag(int row, int col, TileConstraint constraint)
			{
				m_map.Set(row, col, m_map.Get(row, col) | constraint);
			}

			void AddFlag(const Coord& coord, TileConstraint constraint)
			{
				m_map.Set(coord, m_map.Get(coord) | constraint);
			}

			bool HasFlag(int row, int col, TileConstraint constraint) const
			{
				return (m_map.Get(row, col) & constraint) != TileConstraint::NONE;
			}

			// Clear the given flag(s) at row/col. No return value; no check whether anything changed.
			void RemoveFlag(int row, int col, TileConstraint constraint) noexcept
			{
				TileConstraint current = m_map.Get(row, col);
				TileConstraint updated = static_cast<TileConstraint>(current & ~constraint);
				m_map.Set(row, col, updated);
			}

			// Clear the given flag(s) at Coord
			void RemoveFlag(const Coord& coord, TileConstraint constraint) noexcept
			{
				RemoveFlag(coord.row, coord.col, constraint);
			}



#pragma endregion

#pragma region // bound checks
			bool IsInBounds(int row, int col) const
			{
				return m_map.IsInBounds(row, col);
			}

			bool IsInBounds(const engine::spatial::Coord& coord) const
			{
				return m_map.IsInBounds(coord);
			}
#pragma endregion

#pragma region // size query
			// returns grid width
			size_t GetWidth() const
			{
				return m_map.GetWidth();
			}

			// returns grid height. includes last row even if it is incomplete
			size_t GetHeight() const
			{
				return m_map.GetHeight();
			}

			Size GetSize() const
			{
				return m_map.GetSize();
			}

			size_t GetElementCount() const
			{
				return m_map.GetElementCount();
			}

			bool IsEmpty() const
			{
				return m_map.IsEmpty();
			}
#pragma endregion
		};

#pragma endregion

	}
}

