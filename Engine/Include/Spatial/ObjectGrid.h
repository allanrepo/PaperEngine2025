#pragma once
#include <Containers/Dictionary.h>
#include <Spatial/Size.h>
#include <vector>


namespace engine 
{
	// ObjectGrid is placed under engine::spatial rather than container or grid
	// because its primary purpose is spatial partitioning, not generic storage.
	//
	// While it behaves like a container (grouping objects by key within each cell),
	// its core responsibility is to organize and query objects based on their
	// position in a 2D grid (row, col). The grid coordinates are fundamental to
	// how the data is accessed and used.
	//
	// In other words:
	// - container aspect: HOW data is stored (objects grouped by key)
	// - spatial aspect:   WHY it exists (fast lookup by world location)
	//
	// Since the class is primarily used to partition and query objects in space
	// (e.g. trees, walls, entities per cell), it is classified under spatial.
	// This aligns with common engine design where spatial partitioning structures
	// (uniform grids, spatial hashes, etc.) are treated as spatial systems rather
	// than generic containers.
	namespace spatial
	{
#pragma region // ObjectGrid - grid that stores instances of objects per cell
		template <typename K, typename T>
		class ObjectGrid
		{
		private:
#pragma region // Cell implementation
			class Cell
			{
			private:
#pragma region // parameters
				using Objects = std::vector<std::unique_ptr<T>>;
				engine::container::Dictionary<K, Objects> m_objects;
#pragma endregion

			public:
#pragma region // constructors, destructor, copy and assignment operators
				Cell() = default;
				~Cell() = default;

				// delete copy
				Cell(const Cell&) = delete;
				Cell& operator=(const Cell&) = delete;

				// allow move
				Cell(Cell&&) noexcept = default;
				Cell& operator=(Cell&&) noexcept = default;
#pragma endregion

#pragma region // content management
				void Add(std::unique_ptr<T> obj, const K& key)
				{
					m_objects[key].push_back(std::move(obj));
				}
#pragma endregion

#pragma region // accessors
				Objects* Get(const K& key)
				{
					return m_objects.Has(key) ? &m_objects[key] : nullptr;
				}

				const Objects* Get(const K& key) const
				{
					return m_objects.Has(key) ? &m_objects[key] : nullptr;
				}

				// Optional: direct safe access
				T* Get(const K& key, size_t index)
				{
					if (!m_objects.Has(key)) return nullptr;

					auto& vec = m_objects[key];
					return index < vec.size() ? vec[index].get() : nullptr;
				}

				// Optional: direct safe access
				const T* Get(const K& key, size_t index) const
				{
					if (!m_objects.Has(key)) return nullptr;

					auto& vec = m_objects[key];
					return index < vec.size() ? vec[index].get() : nullptr;
				}
#pragma endregion

#pragma region // iteration
				// execute a predicate on every object in the cell, regardless of key. useful for iterating through all objects
				// predicate exposes object and corresponding key for every entry
				template<typename Predicate>
				void ForEach(Predicate func)
				{
					for (auto& [key, list] : m_objects)
					{
						for (auto& obj : list)
						{
							func(obj.get(), key);
						}
					}
				}

				// execute a predicate on every object of a given key in the cell. useful for iterating through objects of a specified key
				// predicate exposes the object that belongs to the specified key
				template<typename Predicate>
				void ForEach(const K& key, Predicate func)
				{
					if (!m_objects.Has(key)) return;

					for (auto& obj : m_objects[key])
					{
						func(obj.get());
					}
				}
#pragma endregion

#pragma region // clear, removal
				// Remove specific index in a key group
				void RemoveAt(const K& key, size_t index)
				{
					if (!m_objects.Has(key)) return;

					auto& vec = m_objects[key];
					if (index >= vec.size()) return;

					// swap-remove
					size_t last = vec.size() - 1;
					if (index != last)
					{
						std::swap(vec[index], vec[last]);
					}

					vec.pop_back();

					// clean empty group
					if (vec.empty())
					{
						m_objects.Unregister(key);
					}
				}

				// Remove last object in key
				bool RemoveLast(const K& key)
				{
					if (!m_objects.Has(key)) return false;

					auto& vec = m_objects[key];
					if (vec.empty()) return false;

					vec.pop_back();

					if (vec.empty())
					{
						m_objects.Unregister(key);
					}

					return true;
				}

				// Remove by predicate inside a key
				template<typename Predicate>
				void RemoveIf(const K& key, Predicate pred)
				{
					if (!m_objects.Has(key)) return;

					auto& vec = m_objects[key];

					size_t i = 0;
					while (i < vec.size())
					{
						if (pred(*vec[i]))
						{
							RemoveAt(key, i);
						}
						else
						{
							++i;
						}
					}
				}

				// remove all cells, effectively emptying the whole cell
				void Clear()
				{
					m_objects.Clear();
				}

				// Remove all objects for key
				void Clear(const K& key)
				{
					if (!m_objects.Has(key)) return;
					m_objects.Unregister(key);
				}
#pragma endregion
			};
#pragma endregion

#pragma region // parameters
			engine::container::Grid<Cell> m_map;
#pragma endregion

		public:
#pragma region // constructor/destructor
			ObjectGrid() :
				m_map(0)
			{
			}
#pragma endregion

#pragma region // non copyable, non movable
			ObjectGrid(const ObjectGrid&) = delete;
			ObjectGrid& operator=(const ObjectGrid&) = delete;
			ObjectGrid(ObjectGrid&&) = delete;
			ObjectGrid& operator=(ObjectGrid&&) = delete;
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

			spatial::Size<size_t> GetSize() const
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
			// design consideration: 
			// -	no need for bounds check because Grid<T> already has. it is redundant
			// -	we're returning pointer because this class is generic. it does not really know what T is
			//		so we don't know how to provide default/safe object if the return value does not exist.
			//		if we inherit this class to be used specifically to store IRenderable as T, then that class
			//		can define a default value if necessary. then it can return a reference, or a view.
			// -	if key is invalid, Cell handles it by returning nullptr
			std::vector<std::unique_ptr<T>>* Get(int row, int col, const K& key)
			{
				return m_map.Get(row, col).Get(key);
			}

			const std::vector<std::unique_ptr<T>>* Get(int row, int col, const K& key) const
			{
				return m_map.Get(row, col).Get(key);
			}

			T* Get(int row, int col, const K& key, size_t index)
			{
				return m_map.Get(row, col).Get(key, index);
			}

			const T* Get(int row, int col, const K& key, size_t index) const
			{
				return m_map.Get(row, col).Get(key, index);
			}

#pragma endregion

#pragma region // replace value
			// set a key of a given cell with the given object. any existing object in that key is removed, effectively replaced by new object
			void Set(int row, int col, const K& key, std::unique_ptr<T> object)
			{
				Clear(row, col, key);
				Add(row, col, key, std::move(object));
			}

			// set a key of a given cell with the given object. any existing object in that key is removed, effectively replaced by new object
			void Set(const Coord& coord, const K& key, std::unique_ptr<T> object)
			{
				Set(coord.row, coord.col, key, std::move(object));
			}
#pragma endregion

#pragma region // content management

			// clears all cells of their content. keeps cells in grid
			void Clear()
			{
				Size<size_t> size = GetSize();
				for (size_t i = 0; i < size.width * size.height; ++i)
				{
					m_map.Get(i).Clear();
				}
			}

			// this empties the grid of all cell contents. must now initialize it again so it can be used
			void Reset()
			{
				m_map.Clear();
			}

			void Initialize(size_t width, size_t height)
			{
				Reset();
				m_map.SetWidth(width);
				m_map.Reserve({ width, height });

				for (size_t i = 0; i < width * height; ++i)
				{
					m_map.Add(Cell());
				}
			}

			void Initialize(Size<size_t> size)
			{
				Initialize(size.width, size.height);
			}

			void Add(int row, int col, const K& key, std::unique_ptr<T> object)
			{
				m_map.Get(row, col).Add(std::move(object), key);
			}

			void Add(const Coord& coord, const K& key, std::unique_ptr<T> object)
			{
				Add(coord.row, coord.col, key, std::move(object));
			}

			// clear the whole cell of a given coordinate
			void Clear(int row, int col)
			{
				m_map.Get(row, col).Clear();
			}

			// clear the whole cell of a given coordinate
			void Clear(const Coord& coord)
			{
				m_map.Get(coord).Clear();
			}

			// clear the key of a cell of a given coordinate
			void Clear(int row, int col, const K& key)
			{
				m_map.Get(row, col).Clear(key);
			}

			// clear the key of a cell of a given coordinate
			void Clear(const Coord& coord, const K& key)
			{
				m_map.Get(coord).Clear(key);
			}
#pragma endregion

#pragma region // iteration
			// execute a predicate for all the objects given cell coordinate regardless of their keys
			// the predicate exposes reference to object and corresponding key
			template<typename Predicate>
			void ForEach(int row, int col, Predicate func)
			{
				if (!IsInBounds(row, col)) return;
				m_map.Get(row, col).ForEach(func);
			}

			// execute a predicate for all objects of the specified key in a given cell coordinate.
			// the predicate exposes reference to object
			template<typename Predicate>
			void ForEach(int row, int col, const K& key, Predicate func)
			{
				if (!IsInBounds(row, col)) return;
				m_map.Get(row, col).ForEach(key, func);
			}
#pragma endregion
		};

#pragma endregion
	}
}