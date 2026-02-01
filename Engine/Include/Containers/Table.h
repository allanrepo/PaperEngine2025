#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <Spatial/ISizeable.h>
#include <Core/View.h>

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
	class Table : public spatial::ISizeable<size_t>
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

		void Clear()
		{
			m_data.clear();
			m_data.shrink_to_fit();
			m_width = 0;
		}

		void Add(const T& data)
		{
			m_data.push_back(data);
		}

		void AddRange(const std::vector<T>& data)
		{
			m_data.insert(m_data.end(), data.begin(), data.end());
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

		void Pop()
		{
			if (m_data.size())
			{
				m_data.pop_back();
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

		spatial::Size<size_t> GetSize() const override
		{
			return spatial::Size<size_t>
			{
				GetWidth(),
				GetHeight()
			};
		}

		const T& Get(size_t index) const
		{
			if (index >= m_data.size())
			{
				throw std::out_of_range("Table::Get - index out of bounds");
			}
			return m_data[index];
		}

		// retrieves the tile at (row, col)
		const T& Get(int row, int col) const
		{
			if (!IsInBounds(row, col))
			{
				throw std::out_of_range("Table::Get - index out of bounds");
			}
			return m_data[row * m_width + col];
		}

		void Set(int row, int col, const T& data)
		{
			if (!IsInBounds(row, col))
			{
				throw std::out_of_range("Table::Set - index out of bounds");
			}
			m_data[row * m_width + col] = data;
		}

		// checks if (row, col) is within bounds
		bool IsInBounds(int row, int col) const
		{
			return
				row >= 0 && col >= 0 &&					// make sure rows and columns are not negatives.
				col < m_width &&						// make sure column is within the Table's width
				row * m_width + col < m_data.size();	// make sure if you map the row and column, it is within the Table array's range
		}

		void Reserve(const spatial::Size<size_t>& size)
		{
			m_data.reserve(size.width * size.height);
		}

		size_t GetElementCount() const
		{
			return m_data.size();
		}

		TableView<T> MakeTableView(int row, int col, size_t width, size_t height) const
		{
			return TableView<T>(*this, row, col, width, height);
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

		spatial::Size<size_t> GetSize() const override
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
