#pragma once
#include <Containers/Dictionary.h>
#include <Containers/Grid.h>

namespace engine
{
	namespace spatial
	{
#pragma region // SpatialOccupancyGrid

		// -----------------------------------------------------------------------------
		// SpatialOccupancyGrid
		//
		// A bidirectional spatial index for objects occupying discrete grid cells.
		//
		// Core responsibilities:
		//  - Track which objects occupy which grid cells
		//  - Track which cells each object occupies
		//  - Provide fast lookup: cell → objects, object → cells
		//  - Store optional per-cell metadata (DATA)
		//
		// Design philosophy:
		//  - This is a *spatial association system*, not a gameplay system
		//  - It does NOT decide validity, overlap, or collision rules
		//  - It does NOT enforce game logic constraints
		//  - It only maintains consistent spatial mappings
		//
		// Typical use cases:
		//  - Tile footprint tracking
		//  - Selection systems (click picking)
		//  - Collision broad-phase indexing
		//  - Navigation constraint sources
		// -----------------------------------------------------------------------------
		template<typename T, typename DATA>
		class SpatialOccupancyGrid
		{
		private:
			// -----------------------------------------------------------------------------
			// Occupant
			//
			// Represents an object occupying a single grid cell, with optional per-cell
			// metadata.
			//
			// This is an internal structure used by SpatialOccupancyGrid.
			// It is NOT exposed outside the grid to avoid leaking implementation details.
			// -----------------------------------------------------------------------------
			template<typename T, typename DATA>
			struct Occupant
			{
				T* object = nullptr;
				DATA data;
			};

			// -------------------------------------------------------------------------
			// Object → Cells mapping
			//
			// Used for fast removal and reverse lookup:
			// "Where is this object located in the grid?"
			// -------------------------------------------------------------------------
			engine::container::Dictionary<T*, std::vector<engine::spatial::Coord>> m_objects;

			// -------------------------------------------------------------------------
			// Cell → Occupants mapping
			//
			// Each grid cell stores a list of objects occupying it.
			// -------------------------------------------------------------------------
			engine::container::Grid<std::vector<Occupant<T, DATA>>> m_grid;

		public:
			SpatialOccupancyGrid() = default;
			~SpatialOccupancyGrid() = default;


			// -------------------------------------------------------------------------
			// Initialize
			//
			// Creates a grid with fixed dimensions and clears all associations.
			// -------------------------------------------------------------------------
			void Initialize(size_t width, size_t height)
			{
				m_grid.Initialize(
					width,
					height,
					std::vector<Occupant<T, DATA>>());

				m_objects.Clear();
			}

			void Initialize(Size<size_t> size)
			{
				Initialize(size.width, size.height);
			}

			// -------------------------------------------------------------------------
			// size query
			// -------------------------------------------------------------------------
			Size<size_t> GetSize() const
			{
				return m_grid.GetSize();
			}

			size_t GetObjectCount() const
			{
				return m_objects.Size();
			}

			// -------------------------------------------------------------------------
			// Add
			//
			// Registers an object in a specific cell with associated metadata.
			//
			// Rules:
			//  - Does NOT resolve overlap
			//  - Does NOT validate gameplay rules
			//  - Does NOT evict existing occupants
			//  - Enforces uniqueness: one object per cell
			// -------------------------------------------------------------------------
			bool Add(
				T* object,
				const Coord& cell,
				const DATA& data)
			{
				if (!object)
				{
					throw std::runtime_error("SpatialOccupancyGrid::Add() - null object");
				}

				if (!m_grid.IsInBounds(cell))
				{
					return false;
				}

				auto& bucket = m_grid.Get(cell);

				// object already exists in this tile?
				auto it = std::find_if(
					bucket.begin(),
					bucket.end(),
					[&](const Occupant<T, DATA>& occupant)
					{
						return occupant.object == object;
					});

				// strict. the caller must be responsible to remove this object if it already exists in this cell
				if (it != bucket.end())
				{
					throw std::runtime_error("SpatialOccupancyGrid::Add() - object already exists in tile");
				}

				// store occupant
				bucket.push_back({ object, data });

				// remember object occupancy
				if (!m_objects.Has(object))
				{
					m_objects.Set(object, {});
				}

				auto& occupiedCells = m_objects.Get(object);

				// enforce unique coords
				auto coordIt = std::find(
					occupiedCells.begin(),
					occupiedCells.end(),
					cell);

				if (coordIt == occupiedCells.end())
				{
					occupiedCells.push_back(cell);
				}

				return true;
			}

			// ------------------------------------------------------------------------
			// Remove object from specific cell
			// ------------------------------------------------------------------------
			bool Remove(
				T* object,
				const Coord& cell)
			{
				if (!object)
				{
					throw std::runtime_error(
						"SpatialOccupancyGrid::Remove() - null object");
				}

				if (!m_grid.IsInBounds(cell))
				{
					return false;
				}

				auto& bucket = m_grid.Get(cell);

				auto it = std::remove_if(
					bucket.begin(),
					bucket.end(),
					[&](const Occupant<T, DATA>& occupant)
					{
						return occupant.object == object;
					});

				if (it == bucket.end())
				{
					return false;
				}

				auto removedCount =
					std::distance(it, bucket.end());

				if (removedCount != 1)
				{
					throw std::runtime_error(
						"SpatialOccupancyGrid::Remove() - duplicate occupants detected");
				}

				bucket.erase(it, bucket.end());

				// update reverse lookup
				if (!m_objects.Has(object))
				{
					throw std::runtime_error(
						"SpatialOccupancyGrid::Remove() - object missing from m_objects");
				}

				auto& occupiedCells = m_objects.Get(object);

				auto coordIt = std::remove(
					occupiedCells.begin(),
					occupiedCells.end(),
					cell);

				if (coordIt == occupiedCells.end())
				{
					throw std::runtime_error(
						"SpatialOccupancyGrid::Remove() - cell missing from object mapping");
				}

				occupiedCells.erase(coordIt, occupiedCells.end());

				// cleanup empty object entry
				if (occupiedCells.empty())
				{
					if (!m_objects.Unregister(object))
					{
						throw std::runtime_error(
							"SpatialOccupancyGrid::Remove() - failed to unregister object");
					}
				}

				return true;
			}

			// ------------------------------------------------------------------------
			// Remove object from all occupied cells
			// ------------------------------------------------------------------------
			void Remove(T* object)
			{
				if (!m_objects.Has(object))
				{
					throw std::runtime_error(
						"SpatialOccupancyGrid::Remove(T*) - object not found");
				}

				// copy because Remove(object, cell)
				// mutates m_objects
				auto cells = m_objects.Get(object);

				for (const auto& cell : cells)
				{
					if (!Remove(object, cell))
					{
						throw std::runtime_error("SpatialOccupancyGrid::Remove(T*) - failed removing object from cell");
					}
				}
			}

			bool Has(T* object) const
			{
				return m_objects.Has(object);
			}

			// ------------------------------------------------------------------------
			// Get data of a given object in a given cell
			// ------------------------------------------------------------------------
			const DATA& Get(T* object, const Coord& cell) const
			{
				if (!m_grid.IsInBounds(cell))
				{
					throw std::runtime_error("SpatialOccupancyGrid::Get() - invalid cell");
				}

				const auto& occupants = m_grid.Get(cell);

				// find the occupant that is our object
				for (const Occupant<T, DATA>& occupant : occupants)
				{
					if (occupant.object == object)
					{
						return occupant.data;
					}
				}
				// we're not sure if this method requires specified cell guarantees object exist. but for now, let's be strict and make it so to avoid silent failures
				throw std::runtime_error("the specified object does not exist in the given coord");
			}

			// ------------------------------------------------------------------------
			// Get occupied cells of object
			// ------------------------------------------------------------------------
			std::vector<Coord> GetOccupiedCells(T* object) const
			{
				if (!m_objects.Has(object))
				{
					return {};
				}

				return m_objects.Get(object);
			}

			// -------------------------------------------------------------------------
			// Iterate cells a given object occupies
			// -------------------------------------------------------------------------
			template<typename Func>
			void ForEachCell(T* object, Func func) const
			{
				if (!m_objects.Has(object))
				{
					throw std::runtime_error("SpatialOccupancyGrid::ForEachCell() - invalid cell");
				}

				// this returns reference to cells
				const std::vector<Coord>& cells = m_objects.Get(object);

				for (const Coord& coord : cells)
				{
					func(coord);
				}
			}

			// -------------------------------------------------------------------------
			// Iterate objects in a cell (object only)
			// -------------------------------------------------------------------------
			template<typename Func>
			void ForEachObject(const Coord& cell, Func func) const
			{
				if (!m_grid.IsInBounds(cell))
				{
					throw std::runtime_error("SpatialOccupancyGrid::ForEachObject() - invalid cell");
				}

				const auto& occupants = m_grid.Get(cell);

				for (const auto& occupant : occupants)
				{
					func(occupant.object);
				}
			}

			// -------------------------------------------------------------------------
			// Iterate objects + data in a cell
			// -------------------------------------------------------------------------
			template<typename Func>
			void ForEach(const Coord& cell, Func func) const
			{
				if (!m_grid.IsInBounds(cell))
				{
					throw std::runtime_error("SpatialOccupancyGrid::Get() - invalid cell");
				}

				const auto& occupants = m_grid.Get(cell);

				for (const auto& occupant : occupants)
				{
					func(occupant.object, occupant.data);
				}
			}

			// ------------------------------------------------------------------------
			// Validation
			// ------------------------------------------------------------------------
			void Validate() const
			{
				// OBJECT -> GRID
				for (const auto& [object, cells] : m_objects)
				{
					if (!object)
					{
						throw std::runtime_error(
							"Validate() - null object");
					}

					for (const auto& cell : cells)
					{
						if (!m_grid.IsInBounds(cell))
						{
							throw std::runtime_error(
								"Validate() - object has invalid cell");
						}

						const auto& bucket = m_grid.Get(cell);

						auto it = std::find_if(
							bucket.begin(),
							bucket.end(),
							[&](const Occupant<T, DATA>& occupant)
							{
								return occupant.object == object;
							});

						if (it == bucket.end())
						{
							throw std::runtime_error(
								"Validate() - object missing from tile");
						}
					}
				}

				// GRID -> OBJECT
				m_grid.ForEach(
					[&](
						size_t row,
						size_t col,
						const std::vector<Occupant<T, DATA>>& bucket)
					{
						Coord cell
						{
							(int)row,
							(int)col
						};

						for (const auto& occupant : bucket)
						{
							if (!occupant.object)
							{
								throw std::runtime_error(
									"Validate() - null occupant");
							}

							if (!m_objects.Has(occupant.object))
							{
								throw std::runtime_error(
									"Validate() - occupant missing from object map");
							}

							const auto& cells =
								m_objects.Get(occupant.object);

							auto it = std::find(
								cells.begin(),
								cells.end(),
								cell);

							if (it == cells.end())
							{
								throw std::runtime_error(
									"Validate() - tile missing from object mapping");
							}
						}
					});
			}
		};

#pragma endregion
	}
}