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

using namespace engine;

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
		friend class TileGrid<T>;

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
	private:
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
	};

	// tile layer represents a 2d grid of tile instances
	template<typename T>
	class TileGrid: public spatial::ISizeable<size_t>
	{
	private:
		// flat array of tiles
		std::vector<Tile<T>> m_map;
		size_t m_width;

	public:
		TileGrid() :
			m_width(0)
		{
		}

		void Clear()
		{
			m_map.clear();
			// optional: doing this releases memory back to system immediately
			m_map.shrink_to_fit();
			m_width = 0;
		}

		void Append(Tile<T> tile)
		{
			m_map.push_back(tile);
		}

		void Pop()
		{
			if (m_map.size())
			{
				m_map.pop_back();
			}
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

		// retrieves the tile at (row, col)
		const Tile<T>& GetTile(int row, int col) const
		{
			if (!IsInBounds(row, col))
			{
				throw std::out_of_range("TileGrid::GetTile - index out of bounds");
			}
			return m_map[row * m_width + col];
		}

		// checks if (row, col) is within bounds
		bool IsInBounds(int row, int col) const 
		{
			return 
				row >= 0 && col >= 0 &&				// make sure rows and columns are not negatives.
				col < m_width &&					// make sure column is within the grid's width
				row * m_width + col < m_map.size();	// make sure if you map the row and column, it is within the grid array's range
		}

		// overload for tilecoord input
		bool IsInBounds(const component::tile::Coord& tileCoord) const
		{
			return IsInBounds(tileCoord.row, tileCoord.col);
		}

		void Reserve(const spatial::Size<size_t>& size)
		{
			m_map.reserve(size.width * size.height);
		}
		
		// retrieves the tile at tilecoord
		const Tile<T>& GetTile(const Coord& coord) const
		{
			return GetTile(coord.row, coord.col);
		}

		size_t GetTileCount() const
		{
			return m_map.size();
		}
	};

	template<typename T>
	class TileRegion: public spatial::ISizeable<size_t>
	{
	private:
		TileGrid<T> m_tilegrid;

		friend class TileLayer<T>;

	public:
		TileRegion()
		{
		}

		TileGrid<T>& Get()
		{
			return m_tilegrid;
		}

		void Append(Tile<T> tile)
		{
			m_tilegrid.Append(tile);
		}

		void Pop()
		{
			m_tilegrid.Pop();
		}

		const Tile<T>& GetTile(int row, int col) const 
		{
			return m_tilegrid.GetTile(row, col);
		}

		const Tile<T>& GetTile(const Coord& coord) const 
		{
			return m_tilegrid.GetTile(coord);
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

		// checks if (row, col) is within bounds
		bool IsInBounds(int row, int col) const
		{
			return m_tilegrid.IsInBounds(row, col);
		}

		// overload for tilecoord input
		bool IsInBounds(const component::tile::Coord& tileCoord) const
		{
			return m_tilegrid.IsInBounds(tileCoord);
		}

		size_t GetTileCount() const
		{
			return m_tilegrid.GetTileCount();
		}
	};

	template<typename T>
	class TileLayer: public spatial::ISizeable<size_t>
	{
	private:
		std::vector<TileRegion<T>> m_regions;
		size_t m_width;

	public:
		TileLayer():
			m_width(0)
		{
		}

		TileRegion<T>& CreateAndAddRegion(const spatial::Size<size_t>& size)
		{
			TileRegion<T> region; 
			region.SetWidth(size.width); 
			
			m_regions.emplace_back(std::move(region)); 

			// return reference to the newly created region
			return m_regions.back(); 
		}

		void Pop()
		{
			if (m_regions.size())
			{
				m_regions.pop_back();
			}
		}

		void SetWidth(const size_t width)
		{
			m_width = width;
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

		void Reserve(const spatial::Size<size_t>& size)
		{
			m_regions.reserve(size.width * size.height);
		}

		// checks if (row, col) is within bounds
		bool IsInBounds(int row, int col) const
		{
			return
				row >= 0 && col >= 0 &&					// make sure rows and columns are not negatives.
				col < m_width &&						// make sure column is within the grid's width
				row * m_width + col < m_regions.size();	// make sure if you map the row and column, it is within the grid array's range
		}

		// overload for coord input
		bool IsInBounds(const component::tile::Coord& coord) const
		{
			return IsInBounds(coord.row, coord.col);
		}

		// retrieves the tile at (row, col)
		TileRegion<T>& GetRegion(int row, int col) 
		{
			if (!IsInBounds(row, col))
			{
				throw std::out_of_range("TileGrid::GetTile - index out of bounds");
			}
			return m_regions[row * m_width + col];
		}

		// retrieves the tile at tilecoord
		TileRegion<T>& GetRegion(const Coord& coord) 
		{
			return GetRegion(coord.row, coord.col);
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
	};
}

