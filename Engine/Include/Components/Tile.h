#pragma once
#include <Spatial/IResizeable.h>
#include <spatial/Size.h>
#include <Cache/Dictionary.h>
#include <vector>
#include <memory>
#include <stdexcept>

namespace component::tile
{
	template<typename T>
	class Tileset;

	template<typename T>
	class Tile;

	template<typename T>
	class TileLayer;
};

namespace component::tile
{
	// represents a coordinate in a 2d tile grid
	struct TileCoord
	{
		int row; // row index (vertical position in the grid)
		int col; // column index (horizontal position in the grid)

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

	template<typename T>
	class Tile
	{
	private:
		T* m_data; // pointer to tile data

		// only Tileset can create Tile instances
		friend class Tileset<T>;

		friend class TileLayer<T>;

		// private constructor used by Tileset to create tile instances
		Tile(T* data = nullptr) :
			m_data(data)
		{
		}

	public:
		virtual ~Tile() = default;

		bool isValid() const
		{
			return m_data != nullptr;
		}

		// Pointer-like access
		const T* operator->() const {
			if (!isValid()) {
				throw std::runtime_error("Tile::operator-> - invalid tile");
			}
			return m_data;
		}

		// Dereference access
		const T& operator*() const {
			if (!isValid()) {
				throw std::runtime_error("Tile::operator* - invalid tile");
			}
			return *m_data;
		}
	};

	// tileset holds tile definitions (not instances)
	// each tile is a unique behavior object (e.g. walkable, obstacle, etc.)
	template<typename T>
	class Tileset 
	{
	private:
		cache::Dictionary<int, std::unique_ptr<T>> registry;

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
			return registry.Register(id, std::move(data));
		}
		bool IsValid(int id) const 
		{ 
			return registry.Has(id); 
		}
		
		const T& Get(int id) const 
		{ 
			return *registry.Get(id); 
		}

		Tile<T> MakeTile(int id) const 
		{ 
			return registry.Has(id) ? Tile<T>(registry.Get(id).get()) : Tile<T>();
		}
	};

	template<typename T>
	class TileLayer : public spatial::IResizeable<int>
	{
	private:
		std::vector<Tile<T>> m_map;	// flat array of tile instances
		spatial::Size<int> m_size;	// grid dimensions (width x height)

	public:
		TileLayer() :
			m_size({ 0, 0 })
		{
		}

		// sets a tile instance at (row, col)
		// passing by value because tileinstance is a lightweight pod
		void SetTile(int row, int col, Tile<T> tile)
		{
			if (!IsValidTile(row, col))
			{
				throw std::out_of_range("TileLayer::SetTile - index out of bounds");
			}
			m_map[row * m_size.width + col] = tile;
		}

		// retrieves the tile instance at (row, col)
		const Tile<T>& GetTile(int row, int col) const
		{
			if (!IsValidTile(row, col)) {
				throw std::out_of_range("TileLayer::GetTile - index out of bounds");
			}
			return m_map[row * m_size.width + col];
		}

		const Tile<T>& GetTile(const TileCoord& coord) const
		{
			return GetTile(coord.row, coord.col);
		}

		// checks if (row, col) is within bounds
		bool IsValidTile(int row, int col) const
		{
			return !(row < 0 || row >= m_size.height || col < 0 || col >= m_size.width);
		}

		// overload for tilecoord input
		bool IsValidTile(const component::tile::TileCoord& tileCoord) const
		{
			return IsValidTile(tileCoord.row, tileCoord.col);
		}

		// returns grid width
		virtual const int GetWidth() const
		{
			return m_size.width;
		}

		// returns grid height
		virtual const int GetHeight() const
		{
			return m_size.height;
		}

		// returns full size struct
		virtual const spatial::Size<int> GetSize() const
		{
			return m_size;
		}

		// sets grid size and resizes internal map but tries to preserve existing data layout if possible
		virtual void SetSize(const spatial::Size<int>& size)
		{
			// need to remap the existing data to into newly sized map

			// create new map and we will copy existing data into it
			std::vector<Tile<T>> map;
			map.reserve(size.width * size.height);

			//// loop through rows of existing map. but not exceeding row size of either old or new map, whichever is smaller
			//for (int row = 0; row < std::min<int>(size.height, m_size.height); row++)
			//{
			//	// loop through columns of existing map. but not exceeding column size of either old or new map, whichever is smaller
			//	for (int col = 0; col < std::min<int>(size.width, m_size.width); col++)
			//	{
			//		// copy existing data into new map
			//		// if new map is larger than old map, the extra cells will remain default initialized, but spatial layout is preserved
			//		// if new map is smaller than old map, excess data will be discarded
			//		map[row * size.width + col] = m_map[row * m_size.width + col];
			//	}
			//}

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

	//template<typename T>
	//class Tileset
	//{
	//private:
	//	// vector of tile definitions indexed by type id
	//	std::vector<std::unique_ptr<T>> m_tiles;

	//public:
	//	Tileset() = default;
	//	virtual ~Tileset() = default;
	//	
	//	// non-copyable
	//	Tileset(const Tileset&) = delete;
	//	Tileset& operator=(const Tileset&) = delete;
	//	Tileset(Tileset&&) = default;
	//	Tileset& operator=(Tileset&&) = default;

	//	// registers a tile definition at a given id
	//	// resizes the vector if needed to accommodate the id
	//	void Register(int id, std::unique_ptr<T> def)
	//	{
	//		if (id >= static_cast<int>(m_tiles.size()))
	//		{
	//			m_tiles.resize(id + 1);
	//		}
	//		m_tiles[id] = std::move(def);
	//	}

	//	// checks if a tile id is valid and has a definition
	//	bool IsValid(int id) const
	//	{
	//		return !(id < 0 || id >= static_cast<int>(m_tiles.size()) || !m_tiles[id]);
	//	}

	//	// retrieves the tile definition at a given id
	//	// throws if the id is out of bounds or unregistered
	//	const T& Get(int id) const
	//	{
	//		if (id < 0 || id >= static_cast<int>(m_tiles.size()) || !m_tiles[id])
	//		{
	//			throw std::runtime_error("Tileset::Get - invalid index");	
	//		}
	//		return *m_tiles[id];
	//	}

	//	Tile<T> MakeTile(int id) const
	//	{
	//		if (!IsValid(id))
	//		{
	//			throw std::runtime_error("Tileset::MakeTile - invalid index");
	//		}
	//		return Tile(m_tiles[id].get());
	//	}
	//};
}

