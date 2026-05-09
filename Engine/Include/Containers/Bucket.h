#pragma once
#include <vector>
#include <memory>
#include <Containers/Grid.h>
#include <Spatial/Size.h>
#include <Spatial/Coord.h>
#include <Containers/Dictionary.h>

namespace engine
{
	namespace container
	{
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

#pragma region // BucketGrid
		template <typename T>
		class BucketGrid
		{
		private:
#pragma region // parameters
			engine::container::Grid<Bucket<T>> m_map;
			engine::container::Dictionary<T*, spatial::Coord> m_objectToCoord;
#pragma endregion

		public:
#pragma region // constructor/destructor
			BucketGrid() :
				m_map(0)
			{
			}
#pragma endregion

#pragma region // non copyable, non movable
			BucketGrid(const BucketGrid&) = delete;
			BucketGrid& operator=(const BucketGrid&) = delete;
			BucketGrid(BucketGrid&&) = delete;
			BucketGrid& operator=(BucketGrid&&) = delete;
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

			void Initialize(engine::spatial::Size<size_t> size)
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
					throw std::runtime_error("BucketGrid::Add(int, int, std::unique_ptr<T>)	 - object already exists");
				}

				m_map.Get(row, col).Add(std::move(object));
				m_objectToCoord.Set(obj, spatial::Coord{ row, col });
			}

			void Add(const engine::spatial::Coord& coord, std::unique_ptr<T> object)
			{
				Add(coord.row, coord.col, std::move(object));
			}

			// clears all cells of their content. keeps cells in grid
			void Clear()
			{
				engine::spatial::Size<size_t> size = GetSize();
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
			void Clear(int row, int col)
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
			void Clear(const engine::spatial::Coord& coord)
			{
				Clear(coord.row, coord.col);
			}

			// this is a strict method. it assumes the object to remove exists and throws an error if not. 
			// it also assumes valid coordinates. full strict. let application check for valid coordinate
			// this will likely crash the application but doing this will catch incorrect usage rather than fail silently
			bool Remove(int row, int col, T* object)
			{
				if (!IsInBounds(row, col))
				{
					throw std::runtime_error("BucketGrid::Remove(int, row, T*) - invalid coordinates");
				}

				if (!m_map.Get(row, col).Remove(object))
				{
					throw std::runtime_error("BucketGrid::Remove(int, row, T*) - failed to remove object from grid");
				}

				if (!m_objectToCoord.Unregister(object))
				{
					throw std::runtime_error("BucketGrid::Remove(int, row, T*) - failed to remove object from dictionary");
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
					throw std::runtime_error("BucketGrid::Remove(T*) - object does not exist in dictionary");
				}

				return Remove(coord, object);
			}
#pragma endregion

#pragma region // iteration
			// execute a predicate for all the objects given cell coordinate regardless of their keys
			// useful for iterating through all objects in a cell specified by row and col
			// the predicate exposes reference to object and corresponding key
			template<typename Predicate>
			void ForEach(int row, int col, Predicate func)
			{
				if (!IsInBounds(row, col)) return;
				m_map.Get(row, col).ForEach(func);
			}

			template<typename Predicate>
			void ForEach(Predicate func) 
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

			template<typename Predicate>
			void ForEach(Predicate func) const
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
