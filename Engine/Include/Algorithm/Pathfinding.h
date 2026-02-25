#pragma once
#include <Components/Tile.h>
#include <queue>
#include <functional>

namespace engine::navigation
{
	namespace tile
	{
		constexpr int CardinalCost = 10;
		constexpr int DiagonalCost = 14;

		struct Node
		{
			engine::component::tile::Coord pos;
			int g = 0;
			int h = 0;
			engine::component::tile::Coord parent;
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
			std::vector<engine::component::tile::Coord> m_openTiles;
			std::vector<engine::component::tile::Coord> m_closedTiles;
			int m_steps;

			bool m_diagonal;
			int m_maxSteps;
			bool m_cutCorners;
			std::function<bool(int, int)> m_isWalkable;
			navigation::tile::HeuristicType m_heuristicType;
			std::function<bool(const engine::component::tile::Coord&, const engine::component::tile::Coord&)> m_canMoveDiagonally;

			int Heuristic(const engine::component::tile::Coord& a, const engine::component::tile::Coord& b) const
			{
				switch (m_heuristicType)
				{
				case navigation::tile::HeuristicType::Manhattan:
					return HeuristicManhattan(a, b);
				case navigation::tile::HeuristicType::Euclidean:
					return HeuristicEuclidean(a, b);
				case navigation::tile::HeuristicType::Octile:
					return HeuristicOctile(a, b);
				default:
					throw std::invalid_argument("Unknown heuristic type");
				}
			}

			int HeuristicEuclidean(const engine::component::tile::Coord& a, const engine::component::tile::Coord& b) const
			{
				// get distance in rows and columns like in Manhattan heuristic
				int distanceRow = std::abs(a.row - b.row);
				int distanceCol = std::abs(a.col - b.col);

				return static_cast<int>(CardinalCost * std::sqrt(distanceCol * distanceCol + distanceRow * distanceRow));
			}

			int HeuristicManhattan(const engine::component::tile::Coord& a, const engine::component::tile::Coord& b) const
			{
				// get distance in rows and columns like in Manhattan heuristic
				int distanceRow = std::abs(a.row - b.row);
				int distanceCol = std::abs(a.col - b.col);

				return (distanceCol + distanceRow) * CardinalCost;
			}

			// Octile heuristic
			int HeuristicOctile(const engine::component::tile::Coord& a, const engine::component::tile::Coord& b) const
			{
				// get distance in rows and columns like in Manhattan heuristic
				int distanceRow = std::abs(a.row - b.row);
				int distanceCol = std::abs(a.col - b.col);

				// the diagonal distance is the minimum of the two distances
				int diagonalDistance = std::min<int>(distanceRow, distanceCol);

				// the cardinal distance is the difference between the two distances
				int cardinalDistance = std::abs(distanceRow - distanceCol);

				// cost is 14 for diagonal movement and 10 for cardinal movement. total cost is sum of both
				return diagonalDistance * DiagonalCost + cardinalDistance * CardinalCost;
			}

			std::vector<engine::component::tile::Coord> GetNeighbors(
				const engine::component::tile::Coord& pos,
				const int width, const int height
			) const
			{
				std::vector<engine::component::tile::Coord> neighbors;

				// iterate through all adjacent tiles of the given tile coord, including those from its diagonals			
				for (int dr = -1; dr <= 1; ++dr)
				{
					for (int dc = -1; dc <= 1; ++dc)
					{
						// skip the tile itself
						if (dr == 0 && dc == 0)
						{
							continue;
						}

						// if diagonal movement is not allowed, skip diagonal neighbors
						if (!m_diagonal && dr != 0 && dc != 0)
						{
							continue;
						}

						int row = pos.row + dr;
						int col = pos.col + dc;

						// the width and height are supposed to be the size of the grid/map. any tile coord outside this range is invalid
						if (row < 0 || row >= height ||
							col < 0 || col >= width)
						{
							continue;
						}

						// add to neighbors list as local coordinates to the region
						neighbors.push_back({ row, col });
					}
				}

				return neighbors;
			}

		public:
			PathFinder(
				std::function<bool(int, int)> isWalkable,
				std::function<bool(const engine::component::tile::Coord&, const engine::component::tile::Coord&)> canMoveDiagonally = nullptr,
				bool diagonal = false,
				bool cutCorners = false,
				int maxSteps = 1000,
				navigation::tile::HeuristicType heuristicType = navigation::tile::HeuristicType::Octile
			) :
				m_isWalkable(isWalkable),
				m_maxSteps(maxSteps),
				m_diagonal(diagonal),
				m_cutCorners(cutCorners),
				m_heuristicType(heuristicType),
				m_canMoveDiagonally(canMoveDiagonally)
			{
			}

			void SetWalkableFunc(std::function<bool(int, int)> isWalkable)
			{
				m_isWalkable = isWalkable;
			}

			void SetCanMoveDiagonallyFunc(std::function<bool(const engine::component::tile::Coord&, const engine::component::tile::Coord&)> canMoveDiagonally)
			{
				m_canMoveDiagonally = canMoveDiagonally;
			}

			const std::vector<engine::component::tile::Coord>& GetOpenTiles() const
			{
				return m_openTiles;
			}

			const std::vector<engine::component::tile::Coord>& GetClosedTiles() const
			{
				return m_closedTiles;
			}

			const std::vector<std::vector<Node>>& GetNodes() const
			{
				return m_nodes;
			}

			void EnableCutCorners(bool enabled)
			{
				m_cutCorners = enabled;
			}

			bool IsCutCornersEnabled() const
			{
				return m_cutCorners;
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
				const engine::component::tile::Coord& start,
				const engine::component::tile::Coord& goal,
				std::vector<engine::component::tile::Coord>& outPath
			)
			{
				// clear previous data
				m_openTiles.clear();
				m_closedTiles.clear();
				outPath.clear();

				// set size of the region and initialize nodes
				int width = region.right - region.left;
				int height = region.bottom - region.top;
				m_nodes.assign(height, std::vector<Node>(width));

				// translate start and goal to region coordinates
				engine::component::tile::Coord regionStart = { start.row - region.top, start.col - region.left };
				engine::component::tile::Coord regionGoal = { goal.row - region.top, goal.col - region.left };

				// initialize start node	
				m_nodes[regionStart.row][regionStart.col].pos = regionStart;						// tile coordinate
				m_nodes[regionStart.row][regionStart.col].g = 0;									// cost from start
				m_nodes[regionStart.row][regionStart.col].h = Heuristic(regionStart, regionGoal);	// heuristic cost to goal
				m_nodes[regionStart.row][regionStart.col].open = true;								// mark as in open list

				// add start node's tile coordinate to open list
				m_openTiles.push_back(regionStart);

				m_steps = m_maxSteps;
				while (!m_openTiles.empty() && m_steps-- > 0)
				{
					// find node in open list with lowest f = g + h
					auto bestIt = m_openTiles.begin();
					for (auto it = m_openTiles.begin(); it != m_openTiles.end(); ++it)
					{
						// this is the node of the current tile coordinate
						const Node& a = m_nodes[it->row][it->col];

						// this is the node of the best tile coordinate found so far
						const Node& b = m_nodes[bestIt->row][bestIt->col];

						// if f is the same, prefer node with lower h
						int af = a.g + a.h;
						int bf = b.g + b.h;
						if (af < bf || (af == bf && a.h < b.h)) bestIt = it;
					}

					// copy tile coordinate of the node with the lowest f from open list
					engine::component::tile::Coord currentTile = *bestIt;

					// get reference to the node with the lowest f 
					Node& currentNode = m_nodes[currentTile.row][currentTile.col];

					// since we now have a copy of the tile coordinate of the node with with the lowest f, we can remove it from open list
					m_openTiles.erase(bestIt);

					// since this node is now being processed, mark it as closed
					currentNode.open = false;
					currentNode.closed = true;
					m_closedTiles.push_back(currentTile);

					// did we reach the goal?
					if (currentTile == regionGoal)
					{
						// for now, we store the path in reverse order (from goal to start)
						engine::component::tile::Coord tc = regionGoal;
						while (tc != regionStart)
						{
							// now we translate back to world coordinates
							outPath.push_back({ tc.row + region.top, tc.col + region.left });

							// move to parent
							tc = m_nodes[tc.row][tc.col].parent;
						}
						// finally, add the start tile in world coordinates
						outPath.push_back(start);

						// reverse the path to be from start to goal
						std::reverse(outPath.begin(), outPath.end());
						return true;
					}

					// iterate over neighbor tiles of the current tile
					for (const engine::component::tile::Coord& neighborTile : GetNeighbors(currentTile, width, height))
					{
						// get the tile coordinates of this neighbor tile
						int neighborTileRow = region.top + neighborTile.row;
						int neighborTileCol = region.left + neighborTile.col;

						// skip non-walkable tiles. note that we check walkability in world coordinates
						if (!m_isWalkable(neighborTileRow, neighborTileCol)) continue;

						// if cutting corners is not allowed, skip diagonal neighbors that would require cutting corners
						if (!m_cutCorners &&
							neighborTile.row != currentTile.row &&
							neighborTile.col != currentTile.col
							)
						{
							// do we have a special handler for checking diagonal movement exclusively?
							if (m_canMoveDiagonally)
							{
								if (!m_canMoveDiagonally(currentTile, neighborTile))
								{
									continue;
								}
							}
							// if none, we just check if adjacent tiles between current and neighbor tiles are both blocked. 
							// if any of them is blocked, then we cannot move diagonally
							else
							{
								// if current tile and neighbor tile are diagonal to each other, then check if both adjacent orthogonal tiles are walkable
								if (!m_isWalkable(region.top + currentTile.row, region.left + neighborTile.col) ||
									!m_isWalkable(region.top + neighborTile.row, region.left + currentTile.col))
								{
									continue;
								}
							}

						}

						// get the node of the neighbor tile. if this tile is already closed, skip it
						Node& neighborNode = m_nodes[neighborTile.row][neighborTile.col];
						if (neighborNode.closed)
						{
							continue;
						}

						// calculate tentative g cost considering diagonal movement
						int tentativeG = neighborTile.row != currentTile.row && neighborTile.col != currentTile.col ?
							currentNode.g + DiagonalCost :	// diagonal movement cost is 14
							currentNode.g + CardinalCost;	// orthogonal movement cost is 10

						// if this neighbor node is not in open list yet
						if (!neighborNode.open)
						{
							// initialize neighbor node
							neighborNode.pos = neighborTile;
							neighborNode.parent = currentTile;

							// g cost is cost from start tile to this neighbor tile via current tile
							neighborNode.g = tentativeG;

							// h cost is heuristic cost from this neighbor tile to goal tile
							// both nighborTile and regionGoal are in region coordinates
							neighborNode.h = Heuristic(neighborTile, regionGoal);

							// add it to open list
							m_openTiles.push_back(neighborTile);
							neighborNode.open = true;
						}

						// else if this neighbor node is already in open list, check if this path to neighbor tile is better (lower g cost)
						else if (tentativeG < neighborNode.g)
						{
							// update parent to current tile
							neighborNode.parent = currentTile;

							// update g cost to the lower tentative g cost
							neighborNode.g = tentativeG;
						}
					}
				}

				return true;
			}
		
			//virtual bool FindPath1(
			//	const math::geometry::Rect<int>& region,
			//	const engine::component::tile::Coord& start,
			//	const engine::component::tile::Coord& goal,
			//	std::vector<engine::component::tile::Coord>& outPath
			//)
			//{
			//	// clear previous data
			//	m_openTiles.clear();
			//	m_closedTiles.clear();
			//	outPath.clear();

			//	// set size of the region and initialize nodes
			//	int width = region.right - region.left;
			//	int height = region.bottom - region.top;
			//	m_nodes.assign(height, std::vector<Node>(width));

			//	// translate start and goal to region coordinates
			//	engine::component::tile::Coord regionStart = { start.row - region.top, start.col - region.left };
			//	engine::component::tile::Coord regionGoal = { goal.row - region.top, goal.col - region.left };

			//	// initialize start node	
			//	m_nodes[regionStart.row][regionStart.col].pos = regionStart;						// tile coordinate
			//	m_nodes[regionStart.row][regionStart.col].g = 0;									// cost from start
			//	m_nodes[regionStart.row][regionStart.col].h = Heuristic(regionStart, regionGoal);	// heuristic cost to goal
			//	m_nodes[regionStart.row][regionStart.col].open = true;								// mark as in open list

			//	// add start node's tile coordinate to open list
			//	m_openTiles.push_back(regionStart);

			//	m_steps = m_maxSteps;
			//	while (!m_openTiles.empty() && m_steps-- > 0)
			//	{
			//		// find node in open list with lowest f = g + h
			//		auto bestIt = m_openTiles.begin();
			//		for (auto it = m_openTiles.begin(); it != m_openTiles.end(); ++it)
			//		{
			//			// this is the node of the current tile coordinate
			//			const Node& a = m_nodes[it->row][it->col];

			//			// this is the node of the best tile coordinate found so far
			//			const Node& b = m_nodes[bestIt->row][bestIt->col];

			//			// if f is the same, prefer node with lower h
			//			int af = a.g + a.h;
			//			int bf = b.g + b.h;
			//			if (af < bf || (af == bf && a.h < b.h)) bestIt = it;
			//		}

			//		// copy tile coordinate of the node with the lowest f from open list
			//		engine::component::tile::Coord currentTile = *bestIt;

			//		// get reference to the node with the lowest f 
			//		Node& currentNode = m_nodes[currentTile.row][currentTile.col];

			//		// since we now have a copy of the tile coordinate of the node with with the lowest f, we can remove it from open list
			//		m_openTiles.erase(bestIt);

			//		// since this node is now being processed, mark it as closed
			//		currentNode.open = false;
			//		currentNode.closed = true;
			//		m_closedTiles.push_back(currentTile);

			//		// did we reach the goal?
			//		if (currentTile == regionGoal)
			//		{
			//			// for now, we store the path in reverse order (from goal to start)
			//			engine::component::tile::Coord tc = regionGoal;
			//			while (tc != regionStart)
			//			{
			//				// now we translate back to world coordinates
			//				outPath.push_back({ tc.row + region.top, tc.col + region.left });

			//				// move to parent
			//				tc = m_nodes[tc.row][tc.col].parent;
			//			}
			//			// finally, add the start tile in world coordinates
			//			outPath.push_back(start);

			//			// reverse the path to be from start to goal
			//			std::reverse(outPath.begin(), outPath.end());
			//			return true;
			//		}

			//		// iterate over neighbor tiles of the current tile
			//		for (const engine::component::tile::Coord& neighborTile : GetNeighbors(currentTile, width, height))
			//		{
			//			// get the tile coordinates of this neighbor tile
			//			int neighborTileRow = region.top + neighborTile.row;
			//			int neighborTileCol = region.left + neighborTile.col;

			//			// skip non-walkable tiles. note that we check walkability in world coordinates
			//			if (!m_isWalkable(neighborTileRow, neighborTileCol)) continue;

			//			// if cutting corners is not allowed, skip diagonal neighbors that would require cutting corners
			//			if (!m_cutCorners &&
			//				neighborTile.row != currentTile.row &&
			//				neighborTile.col != currentTile.col
			//				)
			//			{
			//				if (!m_canMoveDiagonally(currentTile, neighborTile))
			//				{
			//					continue;
			//				}
			//				//// if current tile and neighbor tile are diagonal to each other, then check if both adjacent orthogonal tiles are walkable
			//				//if (!m_isWalkable(region.top + currentTile.row, region.left + neighborTile.col) ||
			//				//	!m_isWalkable(region.top + neighborTile.row, region.left + currentTile.col))
			//				//{
			//				//	continue;
			//				//}
			//			}

			//			// get the node of the neighbor tile. if this tile is already closed, skip it
			//			Node& neighborNode = m_nodes[neighborTile.row][neighborTile.col];
			//			if (neighborNode.closed)
			//			{
			//				continue;
			//			}

			//			// calculate tentative g cost considering diagonal movement
			//			int tentativeG = neighborTile.row != currentTile.row && neighborTile.col != currentTile.col ?
			//				currentNode.g + DiagonalCost :	// diagonal movement cost is 14
			//				currentNode.g + CardinalCost;	// orthogonal movement cost is 10

			//			// if this neighbor node is not in open list yet
			//			if (!neighborNode.open)
			//			{
			//				// initialize neighbor node
			//				neighborNode.pos = neighborTile;
			//				neighborNode.parent = currentTile;

			//				// g cost is cost from start tile to this neighbor tile via current tile
			//				neighborNode.g = tentativeG;

			//				// h cost is heuristic cost from this neighbor tile to goal tile
			//				// both nighborTile and regionGoal are in region coordinates
			//				neighborNode.h = Heuristic(neighborTile, regionGoal);

			//				// add it to open list
			//				m_openTiles.push_back(neighborTile);
			//				neighborNode.open = true;
			//			}

			//			// else if this neighbor node is already in open list, check if this path to neighbor tile is better (lower g cost)
			//			else if (tentativeG < neighborNode.g)
			//			{
			//				// update parent to current tile
			//				neighborNode.parent = currentTile;

			//				// update g cost to the lower tentative g cost
			//				neighborNode.g = tentativeG;
			//			}
			//		}
			//	}

			//	return true;
			//}
		};

		class PathFinderUsingPriorityQueue : public PathFinder
		{
		private:

			struct NodeComparator
			{
				const std::vector<std::vector<Node>>* nodes;

				NodeComparator(const std::vector<std::vector<Node>>* n) : nodes(n) {}

				bool operator()(const engine::component::tile::Coord& a,
					const engine::component::tile::Coord& b) const
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

			std::priority_queue<engine::component::tile::Coord, std::vector<engine::component::tile::Coord>, NodeComparator> openTiles;

		public:
			PathFinderUsingPriorityQueue(
				std::function<bool(int, int)> isWalkable,
				std::function<bool(const engine::component::tile::Coord&, const engine::component::tile::Coord&)> canMoveDiagonally = nullptr,
				int maxSteps = 1000,
				bool diagonal = false,
				bool cutCorners = false,
				navigation::tile::HeuristicType heuristicType = navigation::tile::HeuristicType::Octile
			)
				:PathFinder(
					isWalkable,
					canMoveDiagonally,
					maxSteps,
					diagonal,
					cutCorners,
					heuristicType
				),
				openTiles(NodeComparator(nullptr))
			{
			}

			virtual const std::vector<engine::component::tile::Coord> GetOpenTiles() const
			{
				// copy the queue (since priority_queue has no iterators)
				auto temp = openTiles;
				std::vector<engine::component::tile::Coord> result;
				while (!temp.empty()) 
				{
					result.push_back(temp.top());
					temp.pop();
				}
				return result;
			}

			virtual bool FindPath(
				const math::geometry::Rect<int>& region,
				const engine::component::tile::Coord& start,
				const engine::component::tile::Coord& goal,
				std::vector<engine::component::tile::Coord>& outPath
			)
			{
				// clear previous data
				m_openTiles.clear();
				m_closedTiles.clear();
				outPath.clear();

				// set size of the region and initialize nodes
				int width = region.right - region.left;
				int height = region.bottom - region.top;
				m_nodes.assign(height, std::vector<Node>(width));

				// translate start and goal to region coordinates
				engine::component::tile::Coord regionStart = { start.row - region.top, start.col - region.left };
				engine::component::tile::Coord regionGoal = { goal.row - region.top, goal.col - region.left };

				// initialize start node	
				m_nodes[regionStart.row][regionStart.col].pos = regionStart;						// tile coordinate
				m_nodes[regionStart.row][regionStart.col].g = 0;									// cost from start
				m_nodes[regionStart.row][regionStart.col].h = Heuristic(regionStart, regionGoal);	// heuristic cost to goal
				m_nodes[regionStart.row][regionStart.col].open = true;								// mark as in open list

				// priority queue for open list
				NodeComparator cmp(&m_nodes);
				//std::priority_queue<engine::component::tile::Coord, std::vector<engine::component::tile::Coord>, NodeComparator> openTiles(cmp);
				openTiles = std::priority_queue<engine::component::tile::Coord, std::vector<engine::component::tile::Coord>, NodeComparator>(cmp);

				// add start node's tile coordinate to open list
				openTiles.push(regionStart);

				int steps = m_maxSteps;
				while (!openTiles.empty() && steps-- > 0)
				{
					// pop the best candidate by lowest f(then h)
					engine::component::tile::Coord currentTile = openTiles.top();
					openTiles.pop();

					// get reference to the node with the lowest f 
					Node& currentNode = m_nodes[currentTile.row][currentTile.col];

					// If this node was already processed (closed), skip it.
					 // This can happen if the node re-entered the queue after a cost update
					 // and an earlier instance was already popped and closed.
					if (currentNode.closed) continue;

					// since this node is now being processed, mark it as closed. we are expanding it now.
					currentNode.open = false;
					currentNode.closed = true;
					m_closedTiles.push_back(currentTile);

					// did we reach the goal?
					if (currentTile == regionGoal)
					{
						// for now, we store the path in reverse order (from goal to start)
						engine::component::tile::Coord tc = regionGoal;
						while (tc != regionStart)
						{
							// now we translate back to world coordinates
							outPath.push_back({ tc.row + region.top, tc.col + region.left });

							// move to parent
							tc = m_nodes[tc.row][tc.col].parent;
						}
						// finally, add the start tile in world coordinates
						outPath.push_back(start);

						// reverse the path to be from start to goal
						std::reverse(outPath.begin(), outPath.end());
						return true;
					}

					// iterate over neighbor tiles of the current tile
					for (const engine::component::tile::Coord& neighborTile : GetNeighbors(currentTile, width, height))
					{
						// get the tile coordinates of this neighbor tile
						int neighborTileRow = region.top + neighborTile.row;
						int neighborTileCol = region.left + neighborTile.col;

						// skip non-walkable tiles. note that we check walkability in world coordinates
						if (!m_isWalkable(neighborTileRow, neighborTileCol)) continue;

						// if cutting corners is not allowed, skip diagonal neighbors that would require cutting corners
						if (!m_cutCorners &&
							neighborTile.row != currentTile.row &&
							neighborTile.col != currentTile.col
							)
						{
							// if current tile and neighbor tile are diagonal to each other, then check if both adjacent orthogonal tiles are walkable
							if (!m_isWalkable(region.top + currentTile.row, region.left + neighborTile.col) ||
								!m_isWalkable(region.top + neighborTile.row, region.left + currentTile.col))
							{
								continue;
							}
						}

						// get the node of the neighbor tile. if this tile is already closed, skip it
						Node& neighborNode = m_nodes[neighborTile.row][neighborTile.col];
						if (neighborNode.closed)
						{
							continue;
						}

						// calculate tentative g cost considering diagonal movement
						int tentativeG = neighborTile.row != currentTile.row && neighborTile.col != currentTile.col ?
							currentNode.g + DiagonalCost :	// diagonal movement cost is 14
							currentNode.g + CardinalCost;		// orthogonal movement cost is 10

						// if this neighbor node is not in open list yet OR we found a cheaper path, update its state
						if (!neighborNode.open || tentativeG < neighborNode.g)
						{
							// initialize neighbor node
							neighborNode.pos = neighborTile;
							neighborNode.parent = currentTile;

							// g cost is cost from start tile to this neighbor tile via current tile
							neighborNode.g = tentativeG;

							// h cost is heuristic cost from this neighbor tile to goal tile
							// both nighborTile and regionGoal are in region coordinates
							neighborNode.h = Heuristic(neighborTile, regionGoal);

							// mark as in open set
							neighborNode.open = true;

							// push into open list; comparator will read latest g/h from m_nodes.
							// If this node was already in the queue, this push acts like an "update":
							// the older entry becomes stale and will be ignored when popped (closed check).
							openTiles.push(neighborTile);
						}
					}
				}


				return true;
			}
		};

		static std::vector<engine::component::tile::Coord> GetWayPoints(const std::vector<engine::component::tile::Coord>& path)
		{
			std::vector<engine::component::tile::Coord> wp;

			if (path.size() < 2)
			{
				return wp;
			}

			wp.push_back(path[0]);

			engine::math::VecF currDir
			{
				(float)(path[1].col - path[0].col),
				(float)(path[1].row - path[0].row)
			};

			for (size_t i = 2; i < path.size(); i++)
			{
				engine::math::VecF dir
				{
					(float)(path[i].col - path[i - 1].col),
					(float)(path[i].row - path[i - 1].row)
				};

				if (dir.x != currDir.x || dir.y != currDir.y)
				{
					wp.push_back(path[i - 1]);

					currDir = dir;
				}
			}

			wp.push_back(path[path.size() - 1]);

			return wp;
		}

		// this helper function checks if any tiles in the region of the map coordinates a and b occupy is walkable or not
		// it uses predicate so caller can define a lambda to evaluate if tile is walkable or not
		template<typename Predicate>
		bool IsRegionClear(
			const engine::component::tile::Coord& a,
			const engine::component::tile::Coord& b,
			Predicate&& isWalkable
		)
		{
			int mincol = (int)std::floor(std::min<int>(a.col, b.col));
			int maxcol = (int)std::floor(std::max<int>(a.col, b.col));
			int minrow = (int)std::floor(std::min<int>(a.row, b.row));
			int maxrow = (int)std::floor(std::max<int>(a.row, b.row));

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
		std::vector<engine::component::tile::Coord> SmoothWayPoints(
			const std::vector<engine::component::tile::Coord>& waypoints,
			Predicate&& isWalkable
		)
		{
			std::vector<engine::component::tile::Coord> smoothed;

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


	}

}

