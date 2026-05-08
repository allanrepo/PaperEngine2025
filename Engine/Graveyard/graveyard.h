#pragma once

namespace graveyard
{
#pragma region // MultiOccupancyGrid [GRAVEYARD] used for Bounding Box Grid. but later used a more common class system. just keeping in case i need something like this in future
	// design considerations:
	// - cells to be occupied are filtered to be unique so duplicates are normalized
	// - 
	template<typename T>
	class MultiOccupancyGrid
	{
	private:
		Dictionary<T*, std::vector<Coord>> m_objects;
		Grid<std::vector<T*>> m_grid;

	public:
		MultiOccupancyGrid() = default;
		~MultiOccupancyGrid() = default;

		void Initialize(size_t width, size_t height)
		{
			m_grid.Initialize(width, height, std::vector<T*>());
			m_objects.Clear();
		}

		void Initialize(Size<size_t> size)
		{
			Initialize(size.width, size.height);
		}

		Size<size_t> GetSize() const
		{
			return m_grid.GetSize();
		}

		size_t GetObjectCount() const
		{
			return m_objects.Size();
		}

		// design consideration:
		// - validate cells first. make sure is in bounds and has no duplicates. bail out if no valid cells
		// - if object to occupy already exist in this grid, vacate it first. this ensures occupants are unique. 
		// - this is like "moving" the object from old to new location
		// - update each cell to contain this object
		bool Add(T* object, const std::vector<Coord>& cells)
		{
			// -------------------------------------------------------------------------------
			// 1. VALIDATE CELLS. MAKE SURE WE HAVE VALID CELL TO OCCUPY BEFORE MUTATING
			// -------------------------------------------------------------------------------
			// validate cells first. if all cells are invalid, we won't add this object to the grid and we won't store it in m_objects since it is not really occupying any cell in the grid.
			std::vector<Coord> validCells;
			std::unordered_set<Coord> uniqueCells;
			for (const auto& cell : cells)
			{
				// we can have invalid cells in the list of cells to occupy. we will just skip those invalid cells and only occupy valid cells.
				// in case selected area in grid is occupied by object is partially out of bounds, we will just occupy the valid portion 
				// of the area and ignore the out of bounds portion.
				if (!m_grid.IsInBounds(cell)) continue;

				// remember valid cells 
				// note that we're also storing the cell in a set to ensure we avoid having duplicate cells in our valid cells.
				// this is to handle scenario where cells contain duplicate coords e.g.  (1,1), (1,1), (2,2), (2,2)
				if (uniqueCells.insert(cell).second)
				{
					validCells.push_back(cell);
				}
			}

			// if no valid cells, object cannot be added. bail out
			if (validCells.empty()) return false;

			// -------------------------------------------------------------------------------
			// 2. REMOVE THIS OBJECT IF ALREADY EXIST TO ENSURE OCCUPANTS ARE UNIQUE
			// -------------------------------------------------------------------------------			
			// let's enforce design rule where objects are unique in this grid. if we are adding an object that already exists, 
			// we will treat this as updating the cells occupied by this object. so we will remove previous footprint of this object 
			// and add new footprint of this object.
			if (m_objects.Has(object))
			{
				Vacate(object);
			}

			// -------------------------------------------------------------------------------
			// 3. OBJECT TO OCCUPY VALID CELLS IN GRID
			// -------------------------------------------------------------------------------		
			// write into grid. since cells are valid, it guarantees the new object will occupy these cells
			for (const auto& cell : validCells)
			{
				// since this is Add() method, we allow multiple objects to occupy the same cell. 
				// but we don't want to have duplicate entry of the same object in the same cell, so we check if this object already exist 
				// in the cell before we add it. if it already exists, we throw error because this is likely a bug from caller side. 
				auto& bucket = m_grid.Get(cell);
				if (std::find(bucket.begin(), bucket.end(), object) != bucket.end())
				{
					throw std::runtime_error("Add(T*, const std::vector<Coord>&) - duplicate object in cell");
				}
				bucket.push_back(object);
			}

			// object occupies these cells. no need to check if valid cells are empty since we already check in the beginning
			return m_objects.Set(object, validCells);
		}

		//design consideration:
		//difference from Add() - this removes existing objects that overlaps this new object
		//- validate cells first. make sure is in bounds and has no duplicates. bail out if no valid cells. 
		//- identify existing objects that overlaps this new object
		//
		//- if object to occupy already exist in this grid, vacate it first. this ensures occupants are unique. 
		//- this is like "moving" the object from old to new location
		//- update each cell to contain this object
		bool Occupy(T* object, const std::vector<Coord>& cells)
		{
			// -------------------------------------------------------------------------------
			// 1. VALIDATE CELLS. IDENTIFY EXISTING OBJECTS THAT OVERLAPS. 
			// -------------------------------------------------------------------------------
			// let's validate first before mutating our containers
			std::vector<Coord> validCells;
			std::unordered_set<T*> toEvict;
			std::unordered_set<Coord> uniqueCells;
			for (const auto& cell : cells)
			{
				// we can have invalid cells in the list of cells to occupy. we will just skip those invalid cells and only occupy valid cells.
				// in case selected area in grid is occupied by object is partially out of bounds, we will just occupy the valid portion 
				// of the area and ignore the out of bounds portion.
				if (!m_grid.IsInBounds(cell)) continue;

				// is there existing object in this cell? if yes, we queue it for eviction. even if the object found is same as object that is 
				// trying to occupy, we queue it. this is like "moving" the object from old location to new location. 
				// so we are vacating the object from old position, then later we will add it back into new location
				for (T* existing : m_grid.Get(cell))
				{
					toEvict.insert(existing);
				}

				// remember valid cells 
				// note that we're also storing the cell in a set to ensure we avoid having duplicate cells in our valid cells.
				// this is to handle scenario where cells contain duplicate coords e.g.  (1,1), (1,1), (2,2), (2,2)
				if (uniqueCells.insert(cell).second)
				{
					validCells.push_back(cell);
				}
			}

			// if there are no valid cells to occupy, then this object cannot occupy. bail out
			if (validCells.empty()) return false;

			// -------------------------------------------------------------------------------
			// 2. REMOVE EXISTING OBJECTS THAT OVERLAPS NEW OBJECT. 
			// -------------------------------------------------------------------------------
			// evict objects (except the one that is occupying) found in cells we 're trying to occupy. 
			// we choose to vacate rather than throw error because if there is really discrepancy between m_objects and m_grid, 
			// Vacate() will likely throw error anyways
			for (T* obj : toEvict)
			{
				Vacate(obj);
			}

			// -------------------------------------------------------------------------------
			// 3. REMOVE THIS OBJECT IF ALREADY EXIST TO ENSURE OCCUPANTS ARE UNIQUE
			// -------------------------------------------------------------------------------
			// let's enforce design rule where objects are unique in this grid. if we are adding an object that already exists, 
			// we will treat this as updating the cells occupied by this object. so we will remove previous footprint of this object 
			// and add new footprint of this object.
			// note that if this object's current location is overlapped by the new location it is trying to occupy, then it is already 
			// vacated since it will be in toEvict list. but in case it is not, we vacate it here.
			// regardless, we are doing it safely by checking first if it exist before vacating. 
			if (m_objects.Has(object))
			{
				Vacate(object);
			}

			// -------------------------------------------------------------------------------
			// 4. OBJECT TO OCCUPY VALID CELLS IN GRID
			// -------------------------------------------------------------------------------	
			// new object occupies this cell. we also defer this because if we do this first then evict existing objects after, 
			// we will end up vacating the new object that we just set in the grid since it occupies the same cell as existing objects.
			for (const auto& cell : validCells)
			{
				auto& bucket = m_grid.Get(cell);
				if (!bucket.empty())
				{
					throw std::runtime_error("Occupy(T*, const std::vector<Coord>&) - cell not empty after eviction");
				}

				bucket.push_back(object);
			}

			// object occupies these cells. no need to check if valid cells are empty since we already check in the beginning
			return m_objects.Set(object, validCells);
		}

		void Vacate(T* object)
		{
			// let's be strict here. this method expects the object exist. 
			if (!m_objects.Has(object))
			{
				throw std::runtime_error("SpatialOccupancyGrid::Vacate(T*) - object to remove not found");
			}

			// get the cells occupied by this object. we know this object exist in m_objects, so it must have a valid set of cells.
			const auto& cells = m_objects.Get(object);

			// remove this object from all cells it occupies in the grid. since we allow multiple objects to occupy the same cell,
			// we need to find and remove this object from the list of objects in each cell it occupies.
			for (const auto& cell : cells)
			{
				if (!m_grid.IsInBounds(cell))
				{
					throw std::runtime_error("SpatialOccupancyGrid::Vacate(T*) - invalid cell");
				}

				// get all the objects that occupy this cell
				auto& bucket = m_grid.Get(cell);

				// shift non-matching elements forward and returns an iterator to the new logical end.
				auto it = std::remove(bucket.begin(), bucket.end(), object);
				if (it != bucket.end())
				{
					// checks how many elements were removed by calculating the distance between the new logical end and the actual end of the bucket.
					auto removedCount = std::distance(it, bucket.end());

					// since we expect only one instance of this object in the bucket, we can be strict and check if removedCount is exactly 1. 
					// if not, it means there is a data inconsistency between m_objects and m_grid.
					if (removedCount != 1)
					{
						throw std::runtime_error("SpatialOccupancyGrid::Vacate(T*) - found " + std::to_string(removedCount) + " instances of object. potential data inconsistency between m_objects and m_grid");
					}

					// found one match
					bucket.erase(it, bucket.end());
				}
				else
				{
					// let's be strict here. if we can't find this object in the cell that it's supposed to occupy,
					// it means there is a data inconsistency between m_objects and m_grid.
					throw std::runtime_error("SpatialOccupancyGrid::Vacate(T*) - failed to find object in the cell it occupies. potential data inconsistency between m_objects and m_grid");
				}
			}

			// be strict here. we already vacated cells for this object in grid, if we fail to unregister this object from m_objects, 
			// it means there is a data inconsistency between m_objects and m_grid.
			if (!m_objects.Unregister(object))
			{
				throw std::runtime_error("OccupancyGrid::Vacate(T*) - failed to unregister object from m_objects");
			}
		}

		bool Has(T* object) const
		{
			return m_objects.Has(object);
		}

		// get objects that occupies these cells
		std::vector<T*> Get(const std::vector<Coord>& cells) const
		{
			std::vector<T*> result;

			for (const auto& cell : cells)
			{
				// skip invalid coord
				if (!m_grid.IsInBounds(cell)) continue;

				// get the bucket of objects in this cell. these are the objects that occupy this cell. 
				// since we allow multiple objects to occupy the same cell, we need to iterate through this bucket 
				// and add all unique objects to our result.
				const auto& bucket = m_grid.Get(cell);
				for (T* obj : bucket)
				{
					if (std::find(result.begin(), result.end(), obj) == result.end())
					{
						result.push_back(obj);
					}
				}
			}

			// returns list of unique objects found in cells given
			return result;
		}

		// get objects that occupies this cell
		std::vector<T*> Get(const Coord& cell) const
		{
			if (m_grid.IsInBounds(cell))
			{
				return m_grid.Get(cell);
			}
			return std::vector<T*>();
		}

		// get cell coords that are occupied by this object
		std::vector<Coord> GetOccupiedTiles(T* object) const
		{
			std::vector<Coord> result;

			if (!m_objects.Has(object))
			{
				return result;
			}

			return m_objects.Get(object);
		}

		void Validate() const
		{
			// 1. OBJECT -> GRID CHECK
			for (const auto& [obj, cells] : m_objects)
			{
				if (!obj)
				{
					throw std::runtime_error("Validate: null object in m_objects");
				}

				for (const auto& cell : cells)
				{
					if (!m_grid.IsInBounds(cell))
					{
						throw std::runtime_error("Validate: object has out-of-bounds cell");
					}

					const auto& bucket = m_grid.Get(cell);

					auto it = std::find(bucket.begin(), bucket.end(), obj);
					if (it == bucket.end())
					{
						throw std::runtime_error("Validate: object missing from grid cell it owns");
					}
				}
			}

			// 2. GRID -> OBJECT CHECK
			m_grid.ForEach([&](size_t row, size_t col, const std::vector<T*>& bucket)
				{
					Coord cell{ (int)row, (int)col };

					for (T* obj : bucket)
					{
						if (!obj)
						{
							throw std::runtime_error("Validate: null object in grid cell");
						}

						if (!m_objects.Has(obj))
						{
							throw std::runtime_error("Validate: grid contains object not in m_objects");
						}

						const auto& cells = m_objects.Get(obj);

						auto it = std::find(cells.begin(), cells.end(), cell);
						if (it == cells.end())
						{
							throw std::runtime_error("Validate: grid cell not listed in object mapping");
						}
					}
				});
		}
	};
#pragma endregion

}
