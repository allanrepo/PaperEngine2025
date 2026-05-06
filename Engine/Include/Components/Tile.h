// tilemap is composed of a list of tilelayers
// tilelayer represents the renderables like floors, buildings, trees, characters 
// tilelayer also represents logic tiles such as walkable, obstacle, etc...
// tilelayer is composed of a list of tileregions, which are chunks of the map
// tileregion is a chunk of a map and is composed of tilegrid
// tilegrid contains 2d array of tiles
//
// tilelayer loader
// - reads the tilemap source and stores the map data into chunks of tile regions, which will be stored into tilelayer
// - the chunk size (tileregion size) is provided by caller. 
// - if chunk size is larger than the tilemap source, only 1 chunk will be created and the size will be the size of the tilemap source
// - if tilemap source size is not divisible by given chunk size...
//		- loader will create enough chunks that fits in the tilemap source. 
//		- the remainder of the tilemap source will be stored in chunk but the chunk size is the size of the remainder
//		- the remainder chunks will be at the right and bottom side of the map
// - tilemap source data is stored in file.
//		- supports CSV file format for now
// - map can be extremely large so it can take a while to load it. it loads the map in per frame so it does not stall the application
//		- it first reads the data from the file and will read a chunk (size in byte
// 

#pragma once
#include <Spatial/ISizeable.h>
#include <Spatial/Size.h>
#include <Containers/Dictionary.h>
#include <Core/View.h>
#include <vector>
#include <memory>
#include <stdexcept>
#include <Containers/Table.h>
#include <Spatial/Coord.h>

#include <Graphics/Core/Sprite.h>

// forward declare
namespace engine::component::tile1
{
	template<typename T, typename K = int>
	class Tileset;

	template<typename T>
	class Tile;

	template<typename T>
	class TileGrid;

	template<typename T>
	class TileRegion;

	template<typename T>
	class TileLayer;

	template<typename T>
	class TileMap;
};

namespace engine::component::tile1
{
	// tile instance holds a reference to tile data from tileset
	// lightweight view into tile data 
	template<typename T>
	class Tile : public core::View<T>
	{
	protected:
		// only tileset and TileGrid can create tile instances
		friend class Tileset<T>;

		// private constructor used by Tileset to create tile instances. defaults to invalid tile if no data provided
		Tile(T* data = nullptr) :
			core::View<T>(data)
		{
		}

	public:
		~Tile() override = default;

		Tile(const Tile&) = default;
		Tile& operator=(const Tile&) = default;
		Tile(Tile&&) = default;
		Tile& operator=(Tile&&) = default;
	};

	// manages registration and retrieval of tile data by ID
	template<typename T, typename K>
	class Tileset 
	{
	protected:
		container::Dictionary<K, std::unique_ptr<T>> m_registry;

	public:
		Tileset() = default;
		~Tileset() = default;

		Tileset(const Tileset&) = default;
		Tileset& operator=(const Tileset&) = default;
		Tileset(Tileset&&) = default;
		Tileset& operator=(Tileset&&) = default;

		bool Register(K id, std::unique_ptr<T> data)
		{ 
			return m_registry.Register(id, std::move(data));
		}
		bool IsValid(K id) const
		{ 
			return m_registry.Has(id);
		}
		
		const T& Get(K id) const 
		{ 
			return *m_registry.Get(id);
		}

		// creates a tile instance for the given id. returns invalid tile if id not found
		Tile<T> MakeTile(K id) const 
		{ 
			return m_registry.Has(id) ? Tile<T>(m_registry.Get(id).get()) : Tile<T>();
		}

		// define iterator for our container
		using iterator = typename container::Dictionary<K, std::unique_ptr<T>>::iterator;
		using const_iterator = typename container::Dictionary<K, std::unique_ptr<T>>::const_iterator;

		// iterator access
		iterator begin() { return m_registry.begin(); }
		iterator end() { return m_registry.end(); }
		const_iterator begin() const { return m_registry.begin(); }
		const_iterator end() const { return m_registry.end(); }
		const_iterator cbegin() const { return m_registry.cbegin(); }
		const_iterator cend() const { return m_registry.cend(); }
	};

	// tile layer represents a 2d grid of tile instances
	template<typename T>
	class TileGrid: public container::IGrid<Tile<T>>//public spatial::ISizeable<size_t>
	{
	private:
		// flat array of tiles
		std::vector<Tile<T>> m_map;
		size_t m_width;

	public:
		TileGrid(size_t width = 0) :
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
			return m_width > 0? (m_map.size() / m_width) : 0;
		}

		spatial::Size<size_t> GetSize() const override
		{
			return spatial::Size<size_t>
			{
				GetWidth(),
				GetHeight()
			};
		}

		// overload for Coord input
		bool IsInBounds(const engine::spatial::Coord& Coord) const
		{
			return IsInBounds(Coord.row, Coord.col);
		}

		size_t GetTileCount() const
		{
			return m_map.size();
		}

		void Add(const Tile<T>& tile) override
		{
			m_map.push_back(tile);

		}

		void Take(Tile<T>&& tile) override
		{
			m_map.push_back(std::move(tile));
		}

		void AddRange(const std::vector<Tile<T>>& data) override
		{
			m_map.insert(m_map.end(), data.begin(), data.end());
		};

		void TakeRange(std::vector<Tile<T>>&& data) override
		{
			m_map.insert(m_map.end(), std::make_move_iterator(data.begin()), std::make_move_iterator(data.end())); // move 
		}

		void Pop() override
		{
			if (m_map.size())
			{
				m_map.pop_back();
			}
		}

		const Tile<T>& Get(size_t index) const override
		{
			if (index >= m_map.size())
			{
				throw std::out_of_range("TileLayer::Get - index out of bounds");
			}
			return m_map[index];
		}

		Tile<T>& Get(size_t index) override
		{
			if (index >= m_map.size())
			{
				throw std::out_of_range("TileLayer::Get - index out of bounds");
			}
			return m_map[index];
		}

		void Reserve(const spatial::Size<size_t>& size) override
		{
			m_map.reserve(size.width * size.height);
		}

		size_t GetElementCount() const override
		{
			return m_map.size();
		}

		bool IsEmpty() const override
		{
			return m_map.empty();
		}

		void Clear() override
		{
			m_map.clear();
			// optional: doing this releases memory back to system immediately
			m_map.shrink_to_fit();
			m_width = 0;
		}

		bool IsInBounds(const size_t index) const override
		{
			return index < m_map.size();
		}

		Tile<T>& Back() override
		{
			if (m_map.empty())
			{
				throw std::out_of_range("TileLayer::Back - no elements");
			}
			return m_map.back();
		}

		const Tile<T>& Back() const override
		{
			if (m_map.empty())
			{
				throw std::out_of_range("TileLayer::Back - no elements");
			}
			return m_map.back();
		}

		Tile<T>& Get(int row, int col) override
		{
			if (!IsInBounds(row, col))
			{
				throw std::out_of_range("TileGrid::GetTile - index out of bounds");
			}
			return m_map[row * m_width + col];
		}

		const Tile<T>& Get(int row, int col) const override
		{
			if (!IsInBounds(row, col))
			{
				throw std::out_of_range("TileGrid::GetTile - index out of bounds");
			}
			return m_map[row * m_width + col];
		}

		// retrieves the tile at Coord
		Tile<T>& Get(const engine::spatial::Coord& coord) override final
		{
			return Get(coord.row, coord.col);
		}

		// retrieves the tile at Coord
		const Tile<T>& Get(const engine::spatial::Coord& coord) const override final
		{
			return Get(coord.row, coord.col);
		}

		void Set(int row, int col, const Tile<T>& data) override
		{
			if (!IsInBounds(row, col))
			{
				throw std::out_of_range("TileGrid::GetTile - index out of bounds");
			}
			m_map[row * m_width + col] = data;
		}

		void Set(const engine::spatial::Coord& coord, const Tile<T>& data) override
		{
			if (!IsInBounds(coord))
			{
				throw std::out_of_range("TileGrid::GetTile - index out of bounds");
			}
			m_map[coord.row * m_width + coord.col] = data;
		}

		void Fill(const Tile<T>& data) override
		{
			for (size_t i = 0; i < m_map.size(); i++)
			{
				m_map[i] = data;
			}
		}

		bool IsInBounds(int row, int col) const override
		{
			return
				row >= 0 && col >= 0 &&				// make sure rows and columns are not negatives.
				col < m_width &&					// make sure column is within the grid's width
				row * m_width + col < m_map.size();	// make sure if you map the row and column, it is within the grid array's range
		}

		const TileMap<T> MakeTileMap() const
		{
			return TileMap<T>(this);
		}

		TileMap<T> MakeTileMap()
		{
			return TileMap<T>(this);
		}
	};

	template<typename T>
	class TileRegion: public container::IGrid<Tile<T>>
	{
	private:
		TileGrid<T> m_tilegrid;

		friend class TileLayer<T>;

	public:
		TileRegion(size_t width = 0) :
			m_tilegrid(width)
		{
		}

		TileGrid<T>& Get()
		{
			return m_tilegrid;
		}

		void SetWidth(const size_t width)
		{
			m_tilegrid.SetWidth(width);
		}

		size_t GetHeight() const override
		{
			return m_tilegrid.GetHeight();
		}

		size_t GetWidth() const override
		{
			return m_tilegrid.GetWidth();
		}

		spatial::Size<size_t> GetSize() const override final
		{
			return m_tilegrid.GetSize();
		}

		// overload for Coord input
		bool IsInBounds(const engine::spatial::Coord& Coord) const override final
		{
			return m_tilegrid.IsInBounds(Coord);
		}

		// overload for Coord input
		bool IsInBounds(int row, int col) const override final
		{
			return m_tilegrid.IsInBounds(row, col);
		}

		size_t GetTileCount() const
		{
			return m_tilegrid.GetTileCount();
		}

		void Add(const Tile<T>& tile) override
		{
			m_tilegrid.Add(tile);
		}

		void Take(Tile<T>&& data) override
		{
			m_tilegrid.Take(std::move(data));
		}

		void AddRange(const std::vector<Tile<T>>& data) override
		{
			m_tilegrid.AddRange(data);
		}

		void TakeRange(std::vector<Tile<T>>&& data) override
		{
			m_tilegrid.TakeRange(std::move(data));
		}

		void Pop() override
		{
			m_tilegrid.Pop();
		}

		const Tile<T>& Get(size_t index) const override
		{
			return m_tilegrid.Get(index);
		}

		Tile<T>& Get(size_t index) override
		{
			return m_tilegrid.Get(index);
		}

		void Reserve(const spatial::Size<size_t>& size) override
		{
			m_tilegrid.Reserve(size);
		}

		size_t GetElementCount() const override
		{
			return m_tilegrid.GetTileCount();
		}

		bool IsEmpty() const override
		{
			return m_tilegrid.GetTileCount() == 0;
		}

		void Clear() override
		{
			m_tilegrid.Clear();
		}

		bool IsInBounds(const size_t index) const override
		{
			return index < m_tilegrid.GetTileCount();
		}

		Tile<T>& Back() override
		{
			return m_tilegrid.Back();
		}

		const Tile<T>& Back() const override
		{
			return m_tilegrid.Back();
		}

		Tile<T>& Get(int row, int col) override
		{
			return m_tilegrid.Get(row, col);
		}

		const Tile<T>& Get(int row, int col) const override
		{
			return m_tilegrid.Get(row, col);
		}

		// retrieves the region at coord
		Tile<T>& Get(const engine::spatial::Coord& coord) override final
		{
			return Get(coord.row, coord.col);
		}

		// retrieves the region at coord
		const Tile<T>& Get(const engine::spatial::Coord& coord) const override final
		{
			return Get(coord.row, coord.col);
		}

		void Set(int row, int col, const Tile<T>& data) override
		{
			m_tilegrid.Set(row, col, data);
		}

		void Set(const engine::spatial::Coord& coord, const Tile<T>& data) override
		{
			Set(coord.row, coord.col, data);
		}

		void Fill(const Tile<T>& data) override
		{
			m_tilegrid.Fill(data);
		}

		TileMap<T> MakeTileMap()
		{
			return m_tilegrid.MakeTileMap();
		}

		const TileMap<T> MakeTileMap() const
		{
			return m_tilegrid.MakeTileMap();
		}
	};

	template<typename T>
	class TileLayer: public container::IGrid<TileRegion<T>>
	{
	private:
		std::vector<TileRegion<T>> m_regions;
		size_t m_width;

	public:
		TileLayer():
			m_width(0)
		{
		}

		void Add(const TileRegion<T>& data) override
		{
			m_regions.push_back(data);
		}

		// data is not const reference, so we can move it. if it is set to const, code will still compile but data will be silently copied instead of moved
		void Take(TileRegion<T>&& data) override
		{
			m_regions.push_back(std::move(data));
		}

		void AddRange(const std::vector<TileRegion<T>>& data) override
		{
			m_regions.insert(m_regions.end(), data.begin(), data.end());
		}

		// data is not const reference, so we can move it. if it is set to const, code will still compile but data will be silently copied instead of moved
		void TakeRange(std::vector<TileRegion<T>>&& data) override
		{
			m_regions.insert(m_regions.end(), std::make_move_iterator(data.begin()), std::make_move_iterator(data.end())); // move 
		}

		// this could be a very expensive method. it is best to set tile by tile and do it asynchronously
		void Fill(const TileRegion<T>& data) override
		{
			for (size_t i = 0; i < m_regions.size(); i++)
			{
				m_regions[i] = data;
			}
		}

		void Pop() override
		{
			if (m_regions.size())
			{
				m_regions.pop_back();
			}
		}

		// ISizeable implementations
		size_t GetHeight() const override
		{
			return m_width > 0 ? (m_regions.size() / m_width) : 0;
		}

		size_t GetWidth() const override
		{
			return m_width;
		}

		spatial::Size<size_t> GetSize() const override
		{
			return spatial::Size<size_t>
			{
				GetWidth(),
				GetHeight()
			};
		}

		// checks if (row, col) is within bounds
		bool IsInBounds(int row, int col) const override
		{
			return
				row >= 0 && col >= 0 &&					// make sure rows and columns are not negatives.
				col < m_width &&						// make sure column is within the grid's width
				row * m_width + col < m_regions.size();	// make sure if you map the row and column, it is within the grid array's range
		}

		bool IsInBounds(const size_t index) const override
		{
			return index < m_regions.size();
		}

		// retrieves the region at (row, col)
		TileRegion<T>& Get(int row, int col) override
		{
			if (!IsInBounds(row, col))
			{
				throw std::out_of_range("TileGrid::GetTile - index out of bounds");
			}
			return m_regions[row * m_width + col];
		}

		const TileRegion<T>& Get(int row, int col) const override
		{
			if (!IsInBounds(row, col))
			{
				throw std::out_of_range("TileGrid::GetTile - index out of bounds");
			}
			return m_regions[row * m_width + col];
		}
		
		void Set(int row, int col, const TileRegion<T>& data) override
		{
			if (!IsInBounds(row, col))
			{
				throw std::out_of_range("TileLayer::Set - index out of bounds");
			}
			m_regions[row * m_width + col] = data;
		}

		void Clear() override
		{
			m_regions.clear();
			// optional: doing this releases memory back to system immediately
			m_regions.shrink_to_fit();
			m_width = 0;
		}

		size_t GetElementCount() const override
		{
			return m_regions.size();
		}

		bool IsEmpty() const override
		{
			return m_regions.empty();
		}

		const TileRegion<T>& Get(size_t index) const override
		{
			if (index >= m_regions.size())
			{
				throw std::out_of_range("TileLayer::Get - index out of bounds");
			}
			return m_regions[index];
		}

		TileRegion<T>& Get(size_t index) override
		{
			if (index >= m_regions.size())
			{
				throw std::out_of_range("TileLayer::Get - index out of bounds");
			}
			return m_regions[index];
		}

		void Reserve(const spatial::Size<size_t>& size) override
		{
			m_regions.reserve(size.width * size.height);
		}
	
		// retrieves the region at coord
		TileRegion<T>& Get(const engine::spatial::Coord& coord) override final
		{
			return Get(coord.row, coord.col);
		}

		size_t GetTileCount() const
		{
			size_t tileCount = 0;
			for (const TileRegion<T>& region : m_regions)
			{
				tileCount += region.GetTileCount();
			}
			return tileCount;
		}
		
		bool IsInBounds(const engine::spatial::Coord& coord) const override final
		{
			return IsInBounds(coord.row, coord.col);
		}
		
		void SetWidth(const size_t width)
		{
			m_width = width;
		}

		const TileRegion<T>& Get(const engine::spatial::Coord& coord) const override final
		{
			return Get(coord.row, coord.col);
		}

		void Set(const engine::spatial::Coord& coord, const TileRegion<T>& data) override final
		{
			Set(coord.row, coord.col, data);
		}
	
		// Returns a const reference to the last region
		const TileRegion<T>& Back() const override
		{
			if (m_regions.empty())
			{
				throw std::out_of_range("TileLayer::Back - no elements");
			}
			return m_regions.back();
		}

		// Optionally, a non-const version if modification is allowed
		TileRegion<T>& Back() override
		{
			if (m_regions.empty())
			{
				throw std::out_of_range("TileLayer::Back - no elements");
			}
			return m_regions.back();
		}
	};

	template<typename T>
	class TileMap: public spatial::ISizeable<size_t>
	{
	private:
		core::View<TileGrid<T>> m_view;

		friend class TileGrid<T>;

	protected:
		TileMap(TileGrid<T>* tileGrid) :
			m_view(tileGrid)
		{
		}

	public:
		virtual ~TileMap() = default;

		Tile<T> Get(int row, int col)
		{
			return m_view->Get(row, col);
		}

		const Tile<T> Get(int row, int col) const
		{
			return m_view->Get(row, col);
		}

		bool IsInBounds(int row, int col) const
		{
			return m_view->IsInBounds(row, col);
		}

		inline bool IsValid() const
		{
			return m_view.IsValid();
		}

		virtual size_t GetWidth() const override final
		{
			return m_view->GetWidth();
		}
		virtual size_t GetHeight() const override final
		{
			return m_view->GetHeight();

		}
		virtual spatial::Size<size_t> GetSize() const override final
		{
			return m_view->GetSize();
		}
	};


}

namespace engine
{
	namespace tile
	{
		// Lightweight view that holds a pointer to tile data and an integer index. 
		// Intended to be cheap to copy and to outlive callers only while the Tileset remains alive.
		// Provides GetIndex() to retrieve the tileset key for the view.
		template<typename T>
		class Tile : public core::View<T>
		{
			template<typename T>
			friend class Tileset;
			int m_index;

		protected:
			Tile(int index, T* data = nullptr) :
				core::View<T>(data),
				m_index(index)
			{
			}

		public:
			~Tile() = default;

			Tile(const Tile&) = default;
			Tile& operator=(const Tile&) = default;
			Tile(Tile&&) = default;
			Tile& operator=(Tile&&) = default;

			const int GetIndex() const
			{
				return m_index;
			}

			int GetIndex()
			{
				return m_index;
			}
		};
		
		// Description:
		// Owns tile data in a dictionary keyed by int. 
		// Provides registration, lookup, iteration, and a factory MakeTile(int) that returns a Tile<T> view.
		// Uses m_invalidIndex to represent an invalid tile index.
		// 
		// Design consideration:
		// index is set to int. use resolver or lookup for indices other than int
		// Tileset is designed to outlive those who gets Tile reference from it.
		template<typename T>
		class Tileset
		{
		protected:
			container::Dictionary<int, std::unique_ptr<T>> m_registry;

			int m_invalidIndex;

		public:
			Tileset(int invalidIndex = -1) :
				m_invalidIndex(invalidIndex)
			{
			}

			~Tileset() = default;

			// non copyable, non movable
			Tileset(const Tileset&) = delete;
			Tileset& operator=(const Tileset&) = delete;
			Tileset(Tileset&&) = delete;
			Tileset& operator=(Tileset&&) = delete;

			bool Register(int id, std::unique_ptr<T> data)
			{
				if (id == m_invalidIndex)
				{
					throw std::out_of_range("Tileset::Register - index is invalid");
				}
				return m_registry.Register(id, std::move(data));
			}

			bool IsValid(int id) const
			{
				return m_registry.Has(id);
			}

			const T* Get(int id) const
			{
				return m_registry.Has(id) ? m_registry.Get(id).get() : nullptr;
			}

			// Find the key for a given Tile<T>
			int GetIndex(const Tile<T>& tile) const
			{
				if (!tile.IsValid())
				{
					return m_invalidIndex;
				}

				for (const auto& [id, data] : m_registry)
				{
					if (tile.m_data == data.get())
					{
						return id;
					}
				}
				return m_invalidIndex; // not found
			}

			// creates a tile instance for the given id. returns invalid tile if id not found
			Tile<T> MakeTile(int id) const
			{
				return m_registry.Has(id) ? Tile<T>(id, m_registry.Get(id).get()) : Tile<T>(m_invalidIndex);
			}

			Tile<T> MakeInvalidTile() const
			{
				return Tile<T>(m_invalidIndex);
			}

			// define iterator for our container
			using iterator = typename container::Dictionary<int, std::unique_ptr<T>>::iterator;
			using const_iterator = typename container::Dictionary<int, std::unique_ptr<T>>::const_iterator;

			// iterator access
			iterator begin() { return m_registry.begin(); }
			iterator end() { return m_registry.end(); }
			const_iterator begin() const { return m_registry.begin(); }
			const_iterator end() const { return m_registry.end(); }
			const_iterator cbegin() const { return m_registry.cbegin(); }
			const_iterator cend() const { return m_registry.cend(); }
		};

#pragma region // TileGrid - tile grid represents a 2d grid of tiles
		template<typename T>
		class TileGrid
		{
		private:
#pragma region // parameters
			engine::container::Grid<Tile<T>> m_map;
#pragma endregion

		public:
#pragma region // constructor/destructor
			TileGrid() :
				m_map(0)
			{
			}
#pragma endregion

#pragma region // non copyable, non movable
			TileGrid(const TileGrid&) = delete;
			TileGrid& operator=(const TileGrid&) = delete;
			TileGrid(TileGrid&&) = delete;
			TileGrid& operator=(TileGrid&&) = delete;
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
			Tile<T>& Get(int row, int col)
			{
				return m_map.Get(row, col);
			}

			const Tile<T>& Get(int row, int col) const
			{
				return m_map.Get(row, col);
			}

			// retrieves the data at Coord
			Tile<T>& Get(const engine::spatial::Coord& coord)
			{
				return m_map.Get(coord.row, coord.col);
			}

			// retrieves the data at Coord
			const Tile<T>& Get(const engine::spatial::Coord& coord) const
			{
				return m_map.Get(coord.row, coord.col);
			}
#pragma endregion

#pragma region // replace value
			void Set(int row, int col, const Tile<T>& data)
			{
				m_map.Set(row, col, data);
			}

			void Set(int row, int col, Tile<T>&& data)
			{
				m_map.Set(row, col, std::move(data));
			}

			void Set(const engine::spatial::Coord& coord, const Tile<T>& data)
			{
				m_map.Set(coord, data);
			}

			void Set(const engine::spatial::Coord& coord, Tile<T>&& data)
			{
				m_map.Set(coord, std::move(data));
			}
#pragma endregion

#pragma region // iterator support
			typename std::vector<Tile<T>>::iterator begin() { return m_map.begin(); }
			typename std::vector<Tile<T>>::iterator end() { return m_map.end(); }
			typename std::vector<Tile<T>>::const_iterator begin() const { return m_map.begin(); }
			typename std::vector<Tile<T>>::const_iterator end() const { return m_map.end(); }
			typename std::vector<Tile<T>>::const_iterator cbegin() const { return m_map.cbegin(); }
			typename std::vector<Tile<T>>::const_iterator cend() const { return m_map.cend(); }
			typename std::vector<Tile<T>>::reverse_iterator rbegin() { return m_map.rbegin(); }
			typename std::vector<Tile<T>>::reverse_iterator rend() { return m_map.rend(); }
			typename std::vector<Tile<T>>::const_reverse_iterator rbegin() const { return m_map.rbegin(); }
			typename std::vector<Tile<T>>::const_reverse_iterator rend() const { return m_map.rend(); }
#pragma endregion

#pragma region // content management
			void Reserve(const spatial::Size<size_t>& size)
			{
				m_map.Reserve(size);
			}

			void Clear()
			{
				m_map.Clear();
			}

			// copy only, no move option. expects T to be copyable or else
			void Initialize(size_t width, size_t height, const Tile<T>& data)
			{
				m_map.Clear();
				m_map.SetWidth(width);
				m_map.Reserve({ width, height });

				for (size_t i = 0; i < width * height; ++i)
				{
					m_map.Add(data);
				}
			}

			void Initialize(engine::spatial::Size<size_t> size, const Tile<T>& data)
			{
				Initialize(size.width, size.height, data);
			}
#pragma endregion

		};
#pragma endregion
	}
}