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

#include <Graphics/Renderable/Sprite.h>

// forward declare
namespace engine::component::tile
{
	template<typename T>
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

namespace engine::component::tile
{
	// represents a 2d coordinate in tilemap. this can be grid or region
	struct Coord
	{
		int row;
		int col;

		// equality operator: returns true if both row and col match
		bool operator==(const Coord& other) const
		{
			return row == other.row && col == other.col;
		}

		// inequality operator: returns true if either row or col differ
		bool operator!=(const Coord& other) const
		{
			return !(*this == other);
		}
	};

	// tile instance holds a reference to tile data from tileset
	// lightweight view into tile data 
	template<typename T>
	class Tile : public core::View<T>
	{
	private:
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
	template<typename T>
	class Tileset 
	{
	protected:
		container::Dictionary<int, std::unique_ptr<T>> m_registry;

	public:
		Tileset() = default;
		~Tileset() = default;

		Tileset(const Tileset&) = default;
		Tileset& operator=(const Tileset&) = default;
		Tileset(Tileset&&) = default;
		Tileset& operator=(Tileset&&) = default;

		bool Register(int id, std::unique_ptr<T> data) 
		{ 
			return m_registry.Register(id, std::move(data));
		}
		bool IsValid(int id) const 
		{ 
			return m_registry.Has(id);
		}
		
		const T& Get(int id) const 
		{ 
			return *m_registry.Get(id);
		}

		// creates a tile instance for the given id. returns invalid tile if id not found
		Tile<T> MakeTile(int id) const 
		{ 
			return m_registry.Has(id) ? Tile<T>(m_registry.Get(id).get()) : Tile<T>();
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
		bool IsInBounds(const component::tile::Coord& Coord) const
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
		const Tile<T>& Get(const Coord& coord) const
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
	class TileRegion: public container::IGrid<Tile<T>>//public spatial::ISizeable<size_t>
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

		spatial::Size<size_t> GetSize() const override
		{
			return m_tilegrid.GetSize();
		}

		// overload for Coord input
		bool IsInBounds(const component::tile::Coord& Coord) const
		{
			return m_tilegrid.IsInBounds(Coord);
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

		const  Tile<T>& Get(int row, int col) const override
		{
			return m_tilegrid.Get(row, col);
		}

		// retrieves the region at coord
		Tile<T>& Get(const Coord& coord)
		{
			return Get(coord.row, coord.col);
		}

		void Set(int row, int col, const Tile<T>& data) override
		{
			m_tilegrid.Set(row, col, data);
		}

		bool IsInBounds(int row, int col) const override
		{
			return m_tilegrid.IsInBounds(row, col);
		}

		const TileMap<T> MakeTileMap()
		{
			return m_tilegrid.MakeTileMap();
		}

		TileMap<T> MakeTileMap() const
		{
			return m_tilegrid.MakeTileMap();
		}
	};

	template<typename T>
	class TileLayer: public container::IGrid<TileRegion<T>>// public spatial::ISizeable<size_t>
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

		//TileRegion<T>& CreateAndAddRegion(const spatial::Size<size_t>& size)
		//{
		//	TileRegion<T> region; 
		//	region.SetWidth(size.width); 
		//	
		//	m_regions.emplace_back(std::move(region)); 

		//	// return reference to the newly created region
		//	return m_regions.back(); 
		//}

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
		TileRegion<T>& Get(const Coord& coord)
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
		
		bool IsInBounds(const component::tile::Coord& coord) const
		{
			return IsInBounds(coord.row, coord.col);
		}
		
		void SetWidth(const size_t width)
		{
			m_width = width;
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

		void Set(int row, int col, const T& data) 
		{
			m_view->Set(row, col, data);
		}

		bool IsInBounds(int row, int col) const
		{
			return m_view->IsInBounds(row, col);
		}

		inline bool isValid() const
		{
			return m_view.isValid();
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

