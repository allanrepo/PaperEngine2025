#pragma once
#include <Spatial/ISizeable.h>
#include <Containers/Container.h>
#include <Spatial/Coord.h>
#include <stdexcept>

namespace engine::container
{
	template<typename T>
	class IGrid : public IContainer<T>, public spatial::ISizeable<size_t>
	{
		virtual T& Get(int row, int col) = 0;

		virtual const T& Get(int row, int col) const = 0;

		virtual void Set(int row, int col, const T& data) = 0;

		virtual void Fill(const T& data) = 0;

		virtual bool IsInBounds(int row, int col) const = 0;

		virtual bool IsInBounds(const engine::spatial::Coord& coord) const = 0;

		virtual T& Get(const engine::spatial::Coord& coord) = 0;

		virtual const T& Get(const engine::spatial::Coord& coord) const = 0;

		virtual void Set(const engine::spatial::Coord& coord, const T& data) = 0;
	};

	template<typename T>
	class Grid
	{
	private:
		std::vector<T> m_data;
		size_t m_width;

	public:
#pragma region // constructor/destructor
		Grid(size_t width = 0) :
			m_width(width)
		{
		}
#pragma endregion

#pragma region // non copyable, non movable
		Grid(const Grid&) = delete;
		Grid& operator=(const Grid&) = delete;
		Grid(Grid&&) = delete;
		Grid& operator=(Grid&&) = delete;
#pragma endregion

#pragma region // size query
		// returns grid width
		size_t GetWidth() const
		{
			return m_width;
		}

		// returns grid height. includes last row even if it is incomplete
		size_t GetHeight() const
		{
			if (m_width == 0) return 0;

			// divide size by width, round up if there is a remainder
			return (m_data.size() + m_width - 1) / m_width;
		}

		spatial::Size<size_t> GetSize() const
		{
			return spatial::Size<size_t>{ GetWidth(), GetHeight() };
		}

		size_t GetElementCount() const
		{
			return m_data.size();
		}

		bool IsEmpty() const
		{
			return m_data.empty();
		}
#pragma endregion

#pragma region // bound checks
		bool IsInBounds(int row, int col) const
		{
			return
				row >= 0 && col >= 0 &&					// make sure rows and columns are not negatives.
				col < m_width &&						// make sure column is within the grid's width
				row * m_width + col < m_data.size();	// make sure if you map the row and column, it is within the grid array's range
		}

		// overload for Coord input
		bool IsInBounds(const engine::spatial::Coord& Coord) const
		{
			return IsInBounds(Coord.row, Coord.col);
		}

		// check for bounds via index
		bool IsInBounds(const size_t index) const
		{
			return index < m_data.size();
		}
#pragma endregion

#pragma region // accessors
		const T& Get(size_t index) const
		{
			if (index >= m_data.size())
			{
				throw std::out_of_range("TileLayer::Get - index out of bounds");
			}
			return m_data[index];
		}

		T& Get(size_t index)
		{
			if (index >= m_data.size())
			{
				throw std::out_of_range("TileLayer::Get - index out of bounds");
			}
			return m_data[index];
		}

		T& Back()
		{
			if (m_data.empty())
			{
				throw std::out_of_range("no elements");
			}
			return m_data.back();
		}

		const T& Back() const
		{
			if (m_data.empty())
			{
				throw std::out_of_range("no elements");
			}
			return m_data.back();
		}

		T& Get(int row, int col)
		{
			if (!IsInBounds(row, col))
			{
				throw std::out_of_range("index out of bounds");
			}
			return m_data[row * m_width + col];
		}

		const T& Get(int row, int col) const
		{
			if (!IsInBounds(row, col))
			{
				throw std::out_of_range("index out of bounds");
			}
			return m_data[row * m_width + col];
		}

		// retrieves the data at Coord
		T& Get(const engine::spatial::Coord& coord)
		{
			return Get(coord.row, coord.col);
		}

		// retrieves the data at Coord
		const T& Get(const engine::spatial::Coord& coord) const
		{
			return Get(coord.row, coord.col);
		}
#pragma endregion

#pragma region // replace value
		void Set(int row, int col, const T& data)
		{
			if (!IsInBounds(row, col))
			{
				throw std::out_of_range("index out of bounds");
			}
			m_data[row * m_width + col] = data;
		}

		void Set(int row, int col, T&& data)
		{
			if (!IsInBounds(row, col))
			{
				throw std::out_of_range("index out of bounds");
			}
			m_data[row * m_width + col] = std::move(data);
		}

		void Set(const engine::spatial::Coord& coord, const T& data)
		{
			if (!IsInBounds(coord))
			{
				throw std::out_of_range("index out of bounds");
			}
			m_data[coord.row * m_width + coord.col] = data;
		}

		void Set(const engine::spatial::Coord& coord, T&& data)
		{
			if (!IsInBounds(coord))
			{
				throw std::out_of_range("index out of bounds");
			}
			m_data[coord.row * m_width + coord.col] = std::move(data);
		}
#pragma endregion

#pragma region // iterator support
		typename std::vector<T>::iterator begin() { return m_data.begin(); }
		typename std::vector<T>::iterator end() { return m_data.end(); }
		typename std::vector<T>::const_iterator begin() const { return m_data.begin(); }
		typename std::vector<T>::const_iterator end() const { return m_data.end(); }
		typename std::vector<T>::const_iterator cbegin() const { return m_data.cbegin(); }
		typename std::vector<T>::const_iterator cend() const { return m_data.cend(); }
		typename std::vector<T>::reverse_iterator rbegin() { return m_data.rbegin(); }
		typename std::vector<T>::reverse_iterator rend() { return m_data.rend(); }
		typename std::vector<T>::const_reverse_iterator rbegin() const { return m_data.rbegin(); }
		typename std::vector<T>::const_reverse_iterator rend() const { return m_data.rend(); }
#pragma endregion

#pragma region // content management
		void Reserve(const spatial::Size<size_t>& size)
		{
			m_data.reserve(size.width * size.height);
		}

		void Clear()
		{
			m_data.clear();
			m_width = 0;
		}

		// sets grid width only
		void SetWidth(const size_t width)
		{
			m_width = width;
		}

		void Add(const T& data)
		{
			m_data.push_back(data);
		}

		void Add(T&& data)
		{
			m_data.push_back(std::move(data));
		}

		void AddRange(const std::vector<T>& data)
		{
			m_data.insert(m_data.end(), data.begin(), data.end());
		};

		void AddRange(std::vector<T>&& data)
		{
			m_data.insert(m_data.end(), std::make_move_iterator(data.begin()), std::make_move_iterator(data.end())); // move 
		}

		void Pop()
		{
			if (m_data.size())
			{
				m_data.pop_back();
			}
		}

#pragma endregion

	};
}
