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
#include <Spatial/IResizeable.h>
#include <Spatial/Size.h>
#include <Cache/Dictionary.h>
#include <Core/View.h>
#include <vector>
#include <memory>
#include <stdexcept>

// forward declare
namespace component::tile
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

namespace component::tile
{
	// represents a coordinate in a 2d tile grid
	struct TileCoord
	{
		int row; 
		int col; 

		// equality operator: returns true if both row and col match
		bool operator==(const TileCoord& other) const
		{
			return row == other.row && col == other.col;
		}

		// inequality operator: returns true if either row or col differ
		bool operator!=(const TileCoord& other) const
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
		virtual ~Tile() = default;

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
		cache::Dictionary<int, std::unique_ptr<T>> m_registry;

	public:
		Tileset() = default;
		virtual ~Tileset() = default;

		// non-copyable
		Tileset(const Tileset&) = delete;
		Tileset& operator=(const Tileset&) = delete;
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
	class TileGrid : public spatial::IResizeable<int>
	{
	private:
		// flat array of tiles
		std::vector<Tile<T>> m_map;

		// grid dimensions (width x height)
		spatial::Size<int> m_size;	

	public:
		TileGrid() :
			m_size({ 0, 0 })
		{
		}

		// sets a tile at (row, col)
		// passing by value because tile is a lightweight pod
		void SetTile(int row, int col, Tile<T> tile)
		{
			if (!IsInBounds(row, col))
			{
				throw std::out_of_range("TileGrid::SetTile - index out of bounds");
			}
			m_map[row * m_size.width + col] = tile;
		}

		// retrieves the tile at (row, col)
		const Tile<T>& GetTile(int row, int col) const
		{
			if (!IsInBounds(row, col)) 
			{
				throw std::out_of_range("TileGrid::GetTile - index out of bounds");
			}
			return m_map[row * m_size.width + col];
		}

		// retrieves the tile at tilecoord
		const Tile<T>& GetTile(const TileCoord& coord) const
		{
			return GetTile(coord.row, coord.col);
		}

		// checks if (row, col) is within bounds
		bool IsInBounds(int row, int col) const
		{
			return !(row < 0 || row >= m_size.height || col < 0 || col >= m_size.width);
		}

		// overload for tilecoord input
		bool IsInBounds(const component::tile::TileCoord& tileCoord) const
		{
			return IsInBounds(tileCoord.row, tileCoord.col);
		}

		// returns grid width
		virtual int GetWidth() const
		{
			return m_size.width;
		}

		// returns grid height
		virtual int GetHeight() const
		{
			return m_size.height;
		}

		// returns full size struct
		virtual spatial::Size<int> GetSize() const
		{
			return m_size;
		}

		// sets grid size and resizes internal map but tries to preserve existing data layout if possible
		virtual void SetSize(const spatial::Size<int>& size)
		{
			// need to remap the existing data to into newly sized map

			// create new map and we will copy existing data into it
			//std::vector<Tile<T>> map;
			//map.reserve(size.width * size.height);
			std::vector<Tile<T>> map(size.width * size.height, Tile<T>());

			// loop through rows of existing map. but not exceeding row size of either old or new map, whichever is smaller
			for (int row = 0; row < std::min<int>(size.height, m_size.height); row++)
			{
				// loop through columns of existing map. but not exceeding column size of either old or new map, whichever is smaller
				for (int col = 0; col < std::min<int>(size.width, m_size.width); col++)
				{
					// copy existing data into new map
					// if new map is larger than old map, the extra cells will remain default initialized, but spatial layout is preserved
					// if new map is smaller than old map, excess data will be discarded
					map[row * size.width + col] = m_map[row * m_size.width + col];
				}
			}

// another way of doing this if the map is reserved but not filled initially with empty tiles
#if false 
			// loop through all cells of new map size
			for (int row = 0; row < size.height; ++row)
			{
				for (int col = 0; col < size.width; ++col)
				{
					// if row and col is within bounds of old map, copy existing tile
					if (row < m_size.height && col < m_size.width)
					{
						// copy old tile
						map.push_back(m_map[row * m_size.width + col]);
					}
					// if row or col exceeds old map bounds, fill with empty tile
					else
					{
						// fill new slot with empty tile
						map.push_back(Tile<T>());
					}
				}
			}
#endif
			// replace old map with new map
			m_map = std::move(map);

			// finally update size
			m_size = size;
		}

		// sets grid height only
		virtual void SetHeight(const int height)
		{
			m_size.height = height;
			SetSize(m_size);
		}

		// sets grid width only
		virtual void SetWidth(const int width)
		{
			m_size.width = width;
			SetSize(m_size);
		}
		
	};

	template<typename T>
	class TileRegion
	{
	private:
		TileGrid<T> m_tilegrid;

		friend class TileLayer<T>;

		TileRegion(spatial::Size<int> size)
		{
			m_tilegrid.SetSize(size);
		}

	public:
		inline void SetTile(int row, int col, Tile<T> tile)
		{
			m_tilegrid.SetTile(row, col, tile);
		}

		const Tile<T>& GetTile(int row, int col) const
		{
			return m_tilegrid.GetTile(row, col);
		}

		const Tile<T>& GetTile(const TileCoord& coord) const
		{
			return m_tilegrid.GetTile(coord);
		}

	};

	template<typename T>
	class TileLayer
	{
	private:
		std::vector<TileRegion<T>> m_regions;

		spatial::Size<int> m_size;

		spatial::Size<int> m_regionSize;

	public:
		TileLayer(int rows, int cols, spatial::Size<int> regionSize): 
			m_size({rows, cols}),
			m_regionSize(regionSize),
			m_regions(rows * cols, TileRegion<T>(regionSize)) 
		{
		}

		bool IsInBounds(int row, int col) const
		{
			return !(row < 0 || row >= m_size.height || col < 0 || col >= m_size.width);
		}

		const TileRegion<T>& GetRegion(int row, int col) const
		{
			if (!IsInBounds(row, col))
			{
				throw std::out_of_range("TileRegion::GetRegion - index out of bounds");
			}
			return m_regions[row * m_size.width + col];
		}

		const component::tile::Tile<T>& GetTile(int worldRow, int worldCol) 
		{
			int regionRow = worldRow / m_regionSize.height;
			int regionCol = worldCol / m_regionSize.width;
			int localRow = worldRow % m_regionSize.height;
			int localCol = worldCol % m_regionSize.width;

			return GetRegion(regionRow, regionCol).GetTile(localRow, localCol);
		}

		void SetTile(int worldRow, int worldCol, component::tile::Tile<T> tile) 
		{
			int regionRow = worldRow / m_regionSize.height;
			int regionCol = worldCol / m_regionSize.width;
			int localRow = worldRow % m_regionSize.height;
			int localCol = worldCol % m_regionSize.width;

			GetRegion(regionRow, regionCol).SetTile(localRow, localCol, tile);
		}
	};
}

