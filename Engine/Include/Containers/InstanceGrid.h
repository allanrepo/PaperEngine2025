#pragma once
#include <vector>
#include <memory>
#include <Containers/Grid.h>
#include <Math/Size.h>
#include <Spatial/Coord.h>
#include <Containers/Dictionary.h>

namespace engine
{
	namespace container
	{

#pragma region // InstanceGrid

#pragma region // documentation
// ----------------------------------------------------------------------------------------------------
// Overview:
// ----------------------------------------------------------------------------------------------------
// InstanceGrid<T> is a spatially - indexed container that stores runtime instances of objects 
// organized in a 2D grid structure.
// Each grid cell acts as a bucket of owned objects, allowing multiple instances per spatial 
// coordinate.
// It is designed for systems where spatial partitioning and fast localized iteration 
// are required(e.g.tilemaps, world objects, collision systems, entity grouping).
//
// ----------------------------------------------------------------------------------------------------
// Core Concept
// ----------------------------------------------------------------------------------------------------
// At its core, InstanceGrid combines three responsibilities :
//	- Spatial Partitioning - Objects are grouped by(row, col) grid coordinates.
//	- Ownership Management - The grid owns all stored instances via std::unique_ptr<T>.
//	- Bidirectional Lookup - Supports T* → Coord mapping for reverse spatial queries.

// ----------------------------------------------------------------------------------------------------
// Design Philosophy
// ----------------------------------------------------------------------------------------------------
// InstanceGrid is intentionally not a generic STL - like container.
// Instead, it is :
//	A domain - specific spatial data structure with enforced ownership and indexing semantics.
// It prioritizes :
//	- correctness over flexibility
//	- explicit ownership over ambiguity
//	- spatial locality over general - purpose usage

// ----------------------------------------------------------------------------------------------------
// Internal Structure
// ----------------------------------------------------------------------------------------------------
// Grid Layout implemented as 
// 
//		Grid<Bucket<T>> m_map
// 
// Each cell contains a Bucket<T> :
//	- a dynamic list of std::unique_ptr<T> representing multiple instances in a single spatial tile
//
// Ownership Model
//	- InstanceGrid owns all objects via std::unique_ptr<T>
// 	- Objects cannot exist outside the grid once inserted 
//  - Removal transfers ownership out of the grid and destroys the object
// 
// Reverse Lookup
//	- A dictionary maintains object pointer to coord mapping
// 	- enables fast existence checks, object location queries, safe removal without scanning the grid
//
// ----------------------------------------------------------------------------------------------------
// Key Characteristics
// ----------------------------------------------------------------------------------------------------
// Spatial Indexing
//	- objects are grouped strictly by row/col index
// 	- alows spatial query, cache friendly traversal per cell
// 
// Multi-Occupancy Cells
//	- each cell can contain, many, one, or no objects
// 	- there is no uniqueness constraint per cell
//
// Ownership Semantics
//	- strong ownership (unique_ptr)
//
// Bidirectional Consistency
//  - all operations maintain consistency between grid and reverse lookup map
// 
// ----------------------------------------------------------------------------------------------------
// API Semantics
// ----------------------------------------------------------------------------------------------------
// Insertion
//	- transfers ownership to grid
// 
// Removal
//	- object is removed. it is released and destroyed
// 
// Querying
//	- Has(T*) → existence check
//	- GetCoord(T*) → spatial location
//	- TryGetCoord(...) → safe lookup
// 
// Iteration
//	- Single cell iteration
//	- Full grid iteration
//	- Const - safe iteration
// 
// ----------------------------------------------------------------------------------------------------
// Performance Characteristics
// ----------------------------------------------------------------------------------------------------
//	- Cell access: O(1)
//	- Object lookup : O(1) average(dictionary)
//	- Full grid iteration : O(n) 
//	- Cell iteration : O(k) where k = objects in cell
// 
// ----------------------------------------------------------------------------------------------------
// Intended Use Cases
// ----------------------------------------------------------------------------------------------------
//	- Tile-based world systems 
//	- Spatial partitioning of entities
//	- Collision grouping 
//	- Visibility / interaction filtering
//	- Editor - time world organization
// 
// ----------------------------------------------------------------------------------------------------
// Summary
// ----------------------------------------------------------------------------------------------------
// InstanceGrid<T> is a spatial ownership container, not a generic data structure.
// It is designed around the principle :
//		“Objects live in space, and space defines their grouping.”
//

#pragma endregion
		template <typename T>
		class InstanceGrid
		{
		private:
#pragma region // Bucket<T>
			template<typename T>
			class Bucket
			{
			private:
#pragma region // parameters
				std::vector<std::unique_ptr<T>> m_objects;
#pragma endregion

			public:
#pragma region // constructors, destructor, copy and assignment operators
				Bucket() = default;
				~Bucket() = default;

				// delete copy
				Bucket(const Bucket&) = delete;
				Bucket& operator=(const Bucket&) = delete;

				// allow move
				Bucket(Bucket&&) noexcept = default;
				Bucket& operator=(Bucket&&) noexcept = default;
#pragma endregion

#pragma region // content management
				void Add(std::unique_ptr<T> obj)
				{
					m_objects.push_back(std::move(obj));
				}
#pragma endregion

#pragma region // accessors
#pragma endregion

#pragma region // iteration
				// execute a predicate on every object in the cell, regardless of key. useful for iterating through all objects
				// predicate exposes object and corresponding key for every entry
				template<typename Predicate>
				void ForEach(Predicate func)
				{
					for (std::unique_ptr<T>& object : m_objects)
					{
						func(object.get());
					}
				}

				template<typename Predicate>
				void ForEach(Predicate func) const
				{
					for (const std::unique_ptr<T>& object : m_objects)
					{
						func(object.get());
					}
				}
#pragma endregion

#pragma region // clear, removal
				void RemoveAt(size_t index)
				{
					if (index >= m_objects.size()) return;
					m_objects.erase(m_objects.begin() + index);
				}

				// Remove last object in key
				void RemoveLast()
				{
					if (m_objects.empty()) return;
					m_objects.pop_back();
				}

				bool Remove(T* object)
				{
					// moves the unique_ptr<object> to the end of the container (function name is misleading, it does not remove the object)
					auto it = std::remove_if(m_objects.begin(), m_objects.end(),
						[&](const std::unique_ptr<T>& ptr)
						{
							return ptr.get() == object;
						});

					// this is where the actual remove happens. this removes object from the iterator which is now at end of the container
					if (it != m_objects.end())
					{
						m_objects.erase(it, m_objects.end());
						return true;
					}

					return false;
				}

				// empty the cell
				void Clear()
				{
					m_objects.clear();
				}
#pragma endregion
			};
#pragma endregion

		private:
#pragma region // parameters
			engine::container::Grid<Bucket<T>> m_map;
			engine::container::Dictionary<T*, spatial::Coord> m_objectToCoord;
#pragma endregion

		public:
#pragma region // constructor/destructor
			InstanceGrid() :
				m_map(0)
			{
			}
#pragma endregion

#pragma region // non copyable, non movable
			InstanceGrid(const InstanceGrid&) = delete;
			InstanceGrid& operator=(const InstanceGrid&) = delete;
			InstanceGrid(InstanceGrid&&) = delete;
			InstanceGrid& operator=(InstanceGrid&&) = delete;
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

			math::Size<size_t> GetSize() const
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

#pragma region // bound checks
			bool IsInBounds(int row, int col) const
			{
				return m_map.IsInBounds(row, col);
			}

			// overload for Coord input
			bool IsInBounds(const engine::spatial::Coord& Coord) const
			{
				return m_map.IsInBounds(Coord.row, Coord.col);
			}
#pragma endregion

#pragma region // accessors
			bool Has(T* object) const
			{
				return m_objectToCoord.Has(object);
			}

			spatial::Coord GetCoord(T* object) const
			{
				return m_objectToCoord.Get(object);
			}

			bool TryGetCoord(T* object, spatial::Coord& outCoord) const
			{
				return m_objectToCoord.TryGetValue(object, outCoord);
			}

			size_t GetObjectCount() const
			{
				return m_objectToCoord.Size();
			}

#pragma endregion

#pragma region // initialization
			void Initialize(size_t width, size_t height)
			{
				m_objectToCoord.Clear();

				m_map.Clear();
				m_map.SetWidth(width);
				m_map.Reserve({ width, height });

				for (size_t i = 0; i < width * height; ++i)
				{
					m_map.Add(Bucket<T>());
				}
			}

			void Initialize(engine::math::Size<size_t> size)
			{
				Initialize(size.width, size.height);
			}
#pragma endregion

#pragma region // content management
			void Add(int row, int col, std::unique_ptr<T> object)
			{
				// we do this so we can move the actual object into the grid and keep the pointer for dictionary. 
				T* obj = object.get();

				// this shouldn't happen. object is unique_ptr. but in case it does, let's catch it rather than fail silently and cause hard to track bugs. 
				if (m_objectToCoord.Has(obj))
				{
					throw std::runtime_error("InstanceGrid::Add(int, int, std::unique_ptr<T>)	 - object already exists");
				}

				m_map.Get(row, col).Add(std::move(object));
				m_objectToCoord.Set(obj, spatial::Coord{ row, col });
			}

			void Add(const engine::spatial::Coord& coord, std::unique_ptr<T> object)
			{
				Add(coord.row, coord.col, std::move(object));
			}

			// clears all cells of their content. keeps cells in grid
			void Reset()
			{
				engine::math::Size<size_t> size = GetSize();
				for (size_t i = 0; i < size.width * size.height; ++i)
				{
					m_map.Get(i).Clear();
				}
				m_objectToCoord.Clear();
			}

			// clear the whole cell of a given coordinate
			// design note: 
			// we could have just get the bucket and clear it then clear the dictionary.
			// but we want to call Remove() here enforce strictness
			void Reset(int row, int col)
			{
				// defer removal for safety
				std::vector<T*> toRemove;
				ForEach(row, col, [&](T* object)
					{
						toRemove.push_back(object);
					}
				);

				// safely remove
				for (T* object : toRemove)
				{
					Remove(object);
				}
			}

			// clear the whole cell of a given coordinate
			void Reset(const engine::spatial::Coord& coord)
			{
				Reset(coord.row, coord.col);
			}

			// this is a strict method. it assumes the object to remove exists and throws an error if not. 
			// it also assumes valid coordinates. full strict. let application check for valid coordinate
			// this will likely crash the application but doing this will catch incorrect usage rather than fail silently
			bool Remove(int row, int col, T* object)
			{
				if (!IsInBounds(row, col))
				{
					throw std::runtime_error("InstanceGrid::Remove(int, row, T*) - invalid coordinates");
				}

				if (!m_map.Get(row, col).Remove(object))
				{
					throw std::runtime_error("InstanceGrid::Remove(int, row, T*) - failed to remove object from grid");
				}

				if (!m_objectToCoord.Unregister(object))
				{
					throw std::runtime_error("InstanceGrid::Remove(int, row, T*) - failed to remove object from dictionary");
				}

				return true;
			}

			bool Remove(const engine::spatial::Coord& coord, T* object)
			{
				return Remove(coord.row, coord.col, object);
			}

			// like other Remover overloads, also strict. expects objects to remove must exist. this is to prevent silent failure.
			bool Remove(T* object)
			{
				engine::spatial::Coord coord;
				if (!m_objectToCoord.TryGetValue(object, coord))
				{
					throw std::runtime_error("InstanceGrid::Remove(T*) - object does not exist in dictionary");
				}

				return Remove(coord, object);
			}
#pragma endregion

#pragma region // iteration
			// execute a predicate for all the objects given cell coordinate regardless of their keys
			// useful for iterating through all objects in a cell specified by row and col
			// the predicate exposes reference to object and corresponding key
			template<typename Func>
			void ForEach(int row, int col, const Func& func)
			{
				if (!IsInBounds(row, col))
				{
					throw std::runtime_error("out of bounds");
				}

				m_map.Get(row, col).ForEach(func);
			}

			template<typename Func>
			void ForEach(int row, int col, const Func& func) const
			{
				if (!IsInBounds(row, col))
				{
					throw std::runtime_error("out of bounds");
				}
				m_map.Get(row, col).ForEach(func);
			}

			// execute a predicate for all objects on all cells
			template<typename Func>
			void ForEach(const Func& func)
			{
				m_map.ForEach([&](size_t row, size_t col, Bucket<T>& cell)
					{
						cell.ForEach([&func, row, col](T* object)
							{
								func(row, col, object);
							}
						);
					}
				);
			}

			// execute a predicate for all objects on all cells
			template<typename Func>
			void ForEach(const Func& func) const
			{
				m_map.ForEach([&](size_t row, size_t col, const Bucket<T>& cell)
					{
						cell.ForEach([&func, row, col](const T* object)
							{
								func(row, col, object);
							}
						);
					}
				);
			}

#pragma endregion
		};

#pragma endregion


	}
}
