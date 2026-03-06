#pragma once
#include <Spatial/ISizeable.h>
#include <Containers/Container.h>
#include <Spatial/Coord.h>

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
	class Grid : public container::IGrid<T>
	{
	private:
		std::vector<T> m_data;
		size_t m_width;

	public:
		Grid(size_t width = 0) :
			m_width(width)
		{
		}

		// sets grid width only
		void SetWidth(const size_t width)
		{
			m_width = width;
		}

		// returns grid width
		size_t GetWidth() const override
		{
			return m_width;
		}

		// returns grid height
		// if last row is incomplete (number of tiles < width), it does not count in height
		size_t GetHeight() const override
		{
			return m_width > 0 ? (m_data.size() / m_width) : 0;
		}

		spatial::Size<size_t> GetSize() const override
		{
			return spatial::Size<size_t>
			{
				GetWidth(),
				GetHeight()
			};
		}

		bool IsInBounds(int row, int col) const override
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

		size_t GetElementCount() const
		{
			return m_data.size();
		}

		void Add(const T& data) override
		{
			m_data.push_back(data);

		}

		void Take(T&& data) override
		{
			m_data.push_back(std::move(data));
		}

		void AddRange(const std::vector<T>& data) override
		{
			m_data.insert(m_data.end(), data.begin(), data.end());
		};

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
				throw std::out_of_range("TileLayer::Get - index out of bounds");
			}
			return m_data[index];
		}

		T& Get(size_t index) override
		{
			if (index >= m_data.size())
			{
				throw std::out_of_range("TileLayer::Get - index out of bounds");
			}
			return m_data[index];
		}

		void Reserve(const spatial::Size<size_t>& size) override
		{
			m_data.reserve(size.width * size.height);
		}

		bool IsEmpty() const override
		{
			return m_data.empty();
		}

		void Clear() override
		{
			m_data.clear();
			// optional: doing this releases memory back to system immediately
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
				throw std::out_of_range("no elements");
			}
			return m_data.back();
		}

		const T& Back() const override
		{
			if (m_data.empty())
			{
				throw std::out_of_range("no elements");
			}
			return m_data.back();
		}

		T& Get(int row, int col) override
		{
			if (!IsInBounds(row, col))
			{
				throw std::out_of_range("index out of bounds");
			}
			return m_data[row * m_width + col];
		}

		const T& Get(int row, int col) const override
		{
			if (!IsInBounds(row, col))
			{
				throw std::out_of_range("index out of bounds");
			}
			return m_data[row * m_width + col];
		}

		// retrieves the data at Coord
		T& Get(const engine::spatial::Coord& coord) override final
		{
			return Get(coord.row, coord.col);
		}

		// retrieves the data at Coord
		const T& Get(const engine::spatial::Coord& coord) const override final
		{
			return Get(coord.row, coord.col);
		}

		void Set(int row, int col, const T& data) override
		{
			if (!IsInBounds(row, col))
			{
				throw std::out_of_range("index out of bounds");
			}
			m_data[row * m_width + col] = data;
		}

		void Set(const engine::spatial::Coord& coord, const T& data) override
		{
			if (!IsInBounds(coord))
			{
				throw std::out_of_range("index out of bounds");
			}
			m_data[coord.row * m_width + coord.col] = data;
		}

		void Fill(const T& data) override
		{
			for (size_t i = 0; i < m_data.size(); i++)
			{
				m_data[i] = data;
			}
		}


	};
}
