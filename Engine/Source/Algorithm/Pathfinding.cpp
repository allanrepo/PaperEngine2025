#include <Algorithm/Pathfinding.h>
#include <type_traits>
#include <stdexcept>

namespace engine
{
	namespace navigation
	{
		namespace tile
		{
#pragma region TileNavigationResolver
			bool TileNavigationResolver::CanMoveDiagonally(const TileConstraint crcc, const TileConstraint nrnc, const TileConstraint crnc, const TileConstraint nrcc, Direction dir)
			{
				// let us deal with 
				switch (dir)
				{
				case Direction::SE:
					if (HasAnyBits(crnc, TileConstraint::SW | TileConstraint::W | TileConstraint::S)) return false;
					if (HasAnyBits(nrcc, TileConstraint::NE | TileConstraint::E | TileConstraint::N)) return false;

					if (HasAnyBits(crcc, TileConstraint::SE | TileConstraint::S_WALL | TileConstraint::E_WALL)) return false;
					if (HasAnyBits(nrnc, TileConstraint::NW | TileConstraint::N_WALL | TileConstraint::W_WALL)) return false;
					break;
				case Direction::NW:
					if (HasAnyBits(crnc, TileConstraint::NE | TileConstraint::E | TileConstraint::N)) return false;
					if (HasAnyBits(nrcc, TileConstraint::SW | TileConstraint::W | TileConstraint::S)) return false;

					if (HasAnyBits(crcc, TileConstraint::NW | TileConstraint::N_WALL | TileConstraint::W_WALL)) return false;
					if (HasAnyBits(nrnc, TileConstraint::SE | TileConstraint::S_WALL | TileConstraint::E_WALL)) return false;
					break;
				case Direction::NE:
					if (HasAnyBits(crnc, TileConstraint::NW | TileConstraint::W | TileConstraint::N)) return false;
					if (HasAnyBits(nrcc, TileConstraint::SE | TileConstraint::E | TileConstraint::S)) return false;

					if (HasAnyBits(crcc, TileConstraint::NE | TileConstraint::N_WALL | TileConstraint::E_WALL)) return false;
					if (HasAnyBits(nrnc, TileConstraint::SW | TileConstraint::S_WALL | TileConstraint::W_WALL)) return false;
					break;
				case Direction::SW:
					if (HasAnyBits(crnc, TileConstraint::SE | TileConstraint::E | TileConstraint::S)) return false;
					if (HasAnyBits(nrcc, TileConstraint::NW | TileConstraint::W | TileConstraint::N)) return false;

					if (HasAnyBits(crcc, TileConstraint::SW | TileConstraint::S_WALL | TileConstraint::W_WALL)) return false;
					if (HasAnyBits(nrnc, TileConstraint::NE | TileConstraint::N_WALL | TileConstraint::E_WALL)) return false;
					break;
				default:
					// invalid direction so we don't know where we're going. don't let curr move to tile.
					return false;
				}
				// if we reached this point, neighbor tiles are not blocking path from curr to next tile. so we can move
				return true;
			}

			bool TileNavigationResolver::CanMoveCardinally(const TileConstraint crcc, const TileConstraint nrnc, Direction dir)
			{
				// let us deal with 
				switch (dir)
				{
				case Direction::N:
					// if curr tile has wall on north edge, we can't move to next tile
					if (HasAnyBits(crcc, TileConstraint::N)) return false;

					// if next tile has wall on south edge, we can't move to next tile
					if (HasAnyBits(nrnc, TileConstraint::S)) return false;

					break;
				case Direction::S:
					// if curr tile has wall on south edge, we can't move to next tile
					if (HasAnyBits(crcc, TileConstraint::S)) return false;

					// if next tile has wall on north edge, we can't move to next tile
					if (HasAnyBits(nrnc, TileConstraint::N)) return false;

					break;
				case Direction::E:
					// if curr tile has wall on east edge, we can't move to next tile
					if (HasAnyBits(crcc, TileConstraint::E)) return false;

					// if next tile has wall on west edge, we can't move to next tile
					if (HasAnyBits(nrnc, TileConstraint::W)) return false;

					break;
				case Direction::W:
					// if curr tile has wall on west edge, we can't move to next tile
					if (HasAnyBits(crcc, TileConstraint::W)) return false;

					// if next tile has wall on east edge, we can't move to next tile
					if (HasAnyBits(nrnc, TileConstraint::E)) return false;
					break;
				default:
					// invalid direction so we don't know where we're going. don't let curr move to tile.
					return false;
				}

				// if we reached this point, nothing is blocking movement from curr to next tile. move.
				return true;
			}

			bool TileNavigationResolver::CanMove(const engine::spatial::Coord& curr, const engine::spatial::Coord& next)
			{
				// get constraints of curr and next tiles
				TileConstraint crcc = m_getConstraint(curr.row, curr.col);
				TileConstraint nrnc = m_getConstraint(next.row, next.col);

				// if next tile is blocked at center (possibly a half tri or fully blocked), we can't move to next tile
				if (HasAnyBits(nrnc, TileConstraint::CENTER)) return false;

				// if movement is cardinal
				if (IsCardinal(curr, next))
				{
					// get direction
					Direction dir = GetDirection(curr, next);

					return CanMoveCardinally(crcc, nrnc, dir);
				}
				// if movement is diagonal
				else
				{
					// get direction
					Direction dir = GetDirection(curr, next);

					// get TileConstraint values of curr and next tile's adjacent neighbors
					TileConstraint crnc = m_getConstraint(curr.row, next.col);
					TileConstraint nrcc = m_getConstraint(next.row, curr.col);

					// check if we can move diagonally
					return CanMoveDiagonally(crcc, nrnc, crnc, nrcc, dir);
				}

				return false;
			}
#pragma endregion

#pragma region BinaryNavigationResolver

			bool BinaryNavigationResolver::CanMove(const engine::spatial::Coord& curr, const engine::spatial::Coord& next) 
			{
				// if next tile is blocked, return false
				if (!m_isWalkable(next.row, next.col)) return false;

				// if direction is diagonal, get adjacent neighbors shared by curr and next tile
				if (!IsDiagonal(curr, next)) return true;
				engine::spatial::Coord crnc(curr.row, next.col);
				engine::spatial::Coord nrcc(next.row, curr.col);

				// if any of the adjacent neighbors are blocked, return false
				if (!m_isWalkable(crnc.row, crnc.col)) return false;
				if (!m_isWalkable(nrcc.row, nrcc.col)) return false;

				// if we reached this point, we can move to next tile
				return true;
			}

#pragma endregion

#pragma region PathFinder

			int PathFinder::Heuristic(const engine::spatial::Coord& a, const engine::spatial::Coord& b) const
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

			int PathFinder::HeuristicEuclidean(const engine::spatial::Coord& a, const engine::spatial::Coord& b) const
			{
				// get distance in rows and columns like in Manhattan heuristic
				int distanceRow = std::abs(a.row - b.row);
				int distanceCol = std::abs(a.col - b.col);

				return static_cast<int>(CardinalCost * std::sqrt(distanceCol * distanceCol + distanceRow * distanceRow));
			}

			int PathFinder::HeuristicManhattan(const engine::spatial::Coord& a, const engine::spatial::Coord& b) const
			{
				// get distance in rows and columns like in Manhattan heuristic
				int distanceRow = std::abs(a.row - b.row);
				int distanceCol = std::abs(a.col - b.col);

				return (distanceCol + distanceRow) * CardinalCost;
			}

			// Octile heuristic
			int PathFinder::HeuristicOctile(const engine::spatial::Coord& a, const engine::spatial::Coord& b) const
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

			std::vector<engine::spatial::Coord> PathFinder::GetNeighbors(
				const engine::spatial::Coord& pos,
				const int width, const int height
			) const
			{
				std::vector<engine::spatial::Coord> neighbors;

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


			bool PathFinder::FindPath(
				const math::Rect<int>& region,
				const engine::spatial::Coord& start,
				const engine::spatial::Coord& goal,
				std::vector<engine::spatial::Coord>& outPath
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
				engine::spatial::Coord regionStart = { start.row - region.top, start.col - region.left };
				engine::spatial::Coord regionGoal = { goal.row - region.top, goal.col - region.left };

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
					engine::spatial::Coord currentTile = *bestIt;

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
						engine::spatial::Coord tc = regionGoal;
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
					for (const engine::spatial::Coord& neighborTile : GetNeighbors(currentTile, width, height))
					{
						// get the tile coordinates of this neighbor tile
						int neighborTileRow = region.top + neighborTile.row;
						int neighborTileCol = region.left + neighborTile.col;

						// skip non-walkable tiles. note that we check walkability in world coordinates
						engine::spatial::Coord curr = { region.top + currentTile.row, region.left + currentTile.col };
						engine::spatial::Coord next = { region.top + neighborTile.row, region.left + neighborTile.col };
						if (!m_tileNavigationResolver->CanMove(curr, next)) continue;

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

#pragma endregion

#pragma region PathFinderUsingPriorityQueue
			const std::vector<engine::spatial::Coord> PathFinderUsingPriorityQueue::GetOpenTiles() const
			{
				// copy the queue (since priority_queue has no iterators)
				auto temp = openTiles;
				std::vector<engine::spatial::Coord> result;
				while (!temp.empty())
				{
					result.push_back(temp.top());
					temp.pop();
				}
				return result;
			}

			bool PathFinderUsingPriorityQueue::FindPath(
				const engine::math::Rect<int>& region,
				const engine::spatial::Coord& start,
				const engine::spatial::Coord& goal,
				std::vector<engine::spatial::Coord>& outPath
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
				engine::spatial::Coord regionStart = { start.row - region.top, start.col - region.left };
				engine::spatial::Coord regionGoal = { goal.row - region.top, goal.col - region.left };

				// initialize start node	
				m_nodes[regionStart.row][regionStart.col].pos = regionStart;						// tile coordinate
				m_nodes[regionStart.row][regionStart.col].g = 0;									// cost from start
				m_nodes[regionStart.row][regionStart.col].h = Heuristic(regionStart, regionGoal);	// heuristic cost to goal
				m_nodes[regionStart.row][regionStart.col].open = true;								// mark as in open list

				// priority queue for open list
				NodeComparator cmp(&m_nodes);
				openTiles = std::priority_queue<engine::spatial::Coord, std::vector<engine::spatial::Coord>, NodeComparator>(cmp);

				// add start node's tile coordinate to open list
				openTiles.push(regionStart);

				int steps = m_maxSteps;
				while (!openTiles.empty() && steps-- > 0)
				{
					// pop the best candidate by lowest f(then h)
					engine::spatial::Coord currentTile = openTiles.top();
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
						engine::spatial::Coord tc = regionGoal;
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
					for (const engine::spatial::Coord& neighborTile : GetNeighbors(currentTile, width, height))
					{
						// get the tile coordinates of this neighbor tile
						int neighborTileRow = region.top + neighborTile.row;
						int neighborTileCol = region.left + neighborTile.col;

						// skip non-walkable tiles. note that we check walkability in world coordinates
						engine::spatial::Coord curr = { region.top + currentTile.row, region.left + currentTile.col };
						engine::spatial::Coord next = { region.top + neighborTile.row, region.left + neighborTile.col };
						if (!m_tileNavigationResolver->CanMove(curr, next)) continue;

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
#pragma endregion

#pragma region helper methods
			std::vector<engine::spatial::Coord> GetWayPoints(const std::vector<engine::spatial::Coord>& path)
			{
				std::vector<engine::spatial::Coord> wp;

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

#pragma endregion
		}
	}
}


