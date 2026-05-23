#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <Spatial/ISizeable.h>
#include <Core/View.h>
#include <Containers/Container.h>
#include <Containers/Grid.h>

namespace engine
{
	namespace container
	{
		template<typename T>
		class Table;

		template<typename T>
		class TableView;
	}
}

namespace engine::container
{
	// tile layer represents a 2d Table of tile instances
	template<typename T>
	class Table : public IGrid<std::string> 
	{
	private:
		// flat array of tiles
		std::vector<T> m_data;
		size_t m_width;

	public:
		Table(size_t width = 0) :
			m_width(width)
		{
		}

		Table(const engine::math::Size<size_t> size, const T& t):
			m_width(size.width)
		{
			while (m_data.size() < size.width * size.height)
			{
				m_data.push_back(t);
			}
		}

		void Add(const T& data) override
		{
			m_data.push_back(data);
		}

		// data is not const reference, so we can move it. if it is set to const, code will still compile but data will be silently copied instead of moved
		void Take(T&& data) override
		{
			m_data.push_back(std::move(data));	
		}

		void AddRange(const std::vector<T>& data) override
		{
			m_data.insert(m_data.end(), data.begin(), data.end());
		}

		// data is not const reference, so we can move it. if it is set to const, code will still compile but data will be silently copied instead of moved
		void TakeRange(std::vector<T>&& data) override
		{
			m_data.insert(m_data.end(), std::make_move_iterator(data.begin()), std::make_move_iterator(data.end())); // move
		}

		void Pop() override
		{
			if (m_data.size())
			{
				m_data.pop_back();
			}
		}

		const T& Get(size_t index) const override
		{
			if (index >= m_data.size())
			{
				throw std::out_of_range("Table::Get - index out of bounds");
			}
			return m_data[index];
		}

		T& Get(size_t index) override
		{
			if (index >= m_data.size())
			{
				throw std::out_of_range("Table::Get - index out of bounds");
			}
			return m_data[index];
		}

		void Fill(const T& data) override
		{
			for (size_t i = 0; i < m_data.size(); i++)
			{
				m_data[i] = data;
			}
		}

		void Reserve(const math::Size<size_t>& size) override
		{
			m_data.reserve(size.width * size.height);
		}

		size_t GetElementCount() const override
		{
			return m_data.size();
		}

		bool IsEmpty() const override
		{
			return m_data.empty();
		}

		void Clear() override
		{
			m_data.clear();
			m_data.shrink_to_fit();
			m_width = 0;
		}

		bool IsInBounds(const size_t index) const override
		{
			return index < m_data.size();
		}

		T& Back() override
		{
			if (m_data.empty())
			{
				throw std::out_of_range("Table::Back - table is empty");
			}
			return m_data.back();
		}

		const T& Back() const override
		{
			if (m_data.empty())
			{
				throw std::out_of_range("Table::Back - table is empty");
			}
			return m_data.back();
		}

		// retrieves the tile at (row, col)
		const T& Get(int row, int col) const override
		{
			if (!IsInBounds(row, col))
			{
				throw std::out_of_range("Table::Get - index out of bounds");
			}
			return m_data[row * m_width + col];
		}

		T& Get(int row, int col) override
		{
			if (!IsInBounds(row, col))
			{
				throw std::out_of_range("Table::Get - index out of bounds");
			}
			return m_data[row * m_width + col];
		}

		void Set(int row, int col, const T& data) override
		{
			if (!IsInBounds(row, col))
			{
				throw std::out_of_range("Table::Set - index out of bounds");
			}
			m_data[row * m_width + col] = data;
		}

		// checks if (row, col) is within bounds
		bool IsInBounds(int row, int col) const override
		{
			return
				row >= 0 && col >= 0 &&					// make sure rows and columns are not negatives.
				col < m_width &&						// make sure column is within the Table's width
				row * m_width + col < m_data.size();	// make sure if you map the row and column, it is within the Table array's range
		}

		// difference in AddRange is that it sets width
		void AddRow(const std::vector<T>& data)
		{
			AddRange(data);
			if (!m_width)
			{
				m_width = data.size();
			}
		}
	
		// sets Table width only
		void SetWidth(const size_t width)
		{
			m_width = width;
		}

		// returns Table width
		size_t GetWidth() const override
		{
			return m_width;
		}

		// returns Table height
		// even if last row is incomplete, it counts it as row so if data size is 15 and with is 4, height is 4 with last row having only 3 data
		size_t GetHeight() const override
		{
			return m_width > 0 ? (m_data.size() + m_width - 1) / m_width : 0;
		}

		math::Size<size_t> GetSize() const override
		{
			return math::Size<size_t>
			{
				GetWidth(),
				GetHeight()
			};
		}

		bool IsInBounds(const engine::spatial::Coord& coord) const override final
		{
			return IsInBounds(coord.row, coord.col);
		}

		T& Get(const engine::spatial::Coord& coord) override final
		{
			return Get(coord.row, coord.col);
		}

		const T& Get(const engine::spatial::Coord& coord) const override final
		{
			return Get(coord.row, coord.col);
		}

		void Set(const engine::spatial::Coord& coord, const T& data) override final
		{
			Set(coord.row, coord.col, data);
		}
	};

	template<typename T>
	class TableView : public spatial::ISizeable<size_t>, public core::View<Table<T>>
	{
	private:
		friend class Table<T>;
		int m_row;
		int m_col;
		size_t m_width;
		size_t m_height;

	protected:
		TableView(const Table<T>& table, int row, int col, size_t width, size_t height) :
			core::View<Table<T>>(&table),
			m_row(row),
			m_col(col),
			m_width(width),
			m_height(height)
		{
		}

	public:

		// ISizeable implementations
		size_t GetHeight() const override
		{
			return m_width;
		}
		size_t GetWidth() const override
		{
			return m_height;
		}

		math::Size<size_t> GetSize() const override
		{
			return { m_width, m_height };
		}

		const T& Get(int row, int col) const 
		{ 
			if (row < 0 || col < 0 || row >= (int)m_height || col >= (int)m_width)
			{
				throw std::out_of_range("TableView::Get - index out of bounds");
			}
			
			// need to be explicitly qualified to avoid ambiguity with core::View<T>::Get()
			return core::View<Table<T>>::m_data->Get(m_row + row, m_col + col);
		}

		const T& Get(size_t index) const 
		{
			size_t row = index / m_width;
			size_t col = index % m_width;

			return core::View<Table<T>>::m_data->Get(m_row + row, m_col + col);
		}

		size_t GetElementCount() const 
		{
			return m_width * m_height;
		}

	};
}
