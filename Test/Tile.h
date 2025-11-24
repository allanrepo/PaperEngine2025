#pragma once
#include <vector>
#include "Sprite.h"
#include "ISizeable.h"
#include "IRenderer.h"
#include "Event.h"
#include "CSVFile.h"
#include <functional>
#include <memory>
#include <stdexcept>

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

	// represents a single tile placed in a layer. holds only instance-specific data like type id or per-instance flags
	struct TileInstance 
	{
		int index = -1;       // index into Tileset definitions
		// other per-instance flags can be added here (animation phase, fog, damage, etc.)
	};

	// abstract base class for tile definitions. defines behavior that all concrete tiles must implement
	class ITile 
	{
	public:
		virtual ~ITile() = default;
		virtual bool IsWalkable() const = 0;
		virtual int GetCost() const = 0;
		// Add more behavior hooks as needed
	};

	// concrete tile definition: walkable surface
	class WalkableTile : public ITile
	{
	public:
		bool IsWalkable() const override { return true; }
		int GetCost() const override { return 1; }
	};

	// concrete tile definition: impassable obstacle
	class ObstacleTile : public ITile
	{
	public:
		bool IsWalkable() const override { return false; }
		int GetCost() const override { return 999; }
	};

	// tileset holds tile definitions (not instances)
	// each tile is a unique behavior object (e.g. walkable, obstacle, etc.)
	class Tileset
	{
	private:
		// vector of tile definitions indexed by type id
		std::vector<std::unique_ptr<ITile>> m_tiles;

	public:
		// registers a tile definition at a given id
		// resizes the vector if needed to accommodate the id
		void Register(int id, std::unique_ptr<ITile> def)
		{
			if (id >= static_cast<int>(m_tiles.size()))
			{
				m_tiles.resize(id + 1);
			}
			m_tiles[id] = std::move(def);
		}

		// checks if a tile id is valid and has a definition
		bool IsValid(int id) const
		{
			return !(id < 0 || id >= static_cast<int>(m_tiles.size()) || !m_tiles[id]);
		}

		// retrieves the tile definition at a given id
		// throws if the id is out of bounds or unregistered
		const ITile& GetTile(int id) const
		{
			if (id < 0 || id >= static_cast<int>(m_tiles.size()) || !m_tiles[id])
			{
				throw std::runtime_error("Tileset::GetTile - invalid index");
			}
			return *m_tiles[id];
		}
	};

	// tilelayer holds a 2d grid of tile instances (type ids only)
	// it does not interpret behavior — that’s handled by tileset
	class TileLayer : public spatial::IResizeable<int>
	{
	private:
		std::vector<TileInstance> m_map; // flat array of tile instances
		spatial::Size<int> m_size;       // grid dimensions (width x height)

	public:
		// sets a tile instance at (row, col)
		// passing by value because tileinstance is a lightweight pod
		void SetTileInstance(int row, int col, TileInstance tileInst)
		{
			if (!IsValidTile(row, col))
			{
				throw std::out_of_range("TileLayer::SetTileInstance - index out of bounds");
			}
			m_map[row * m_size.width + col] = tileInst;
		}

		// retrieves the tile instance at (row, col)
		const TileInstance& GetTileInstance(int row, int col) const
		{
			if (!IsValidTile(row, col)) {
				throw std::out_of_range("TileLayer::GetTileInstance - index out of bounds");
			}
			return m_map[row * m_size.width + col];
		}

		const TileInstance& GetTileInstance(const TileCoord& coord) const 
		{
			return GetTileInstance(coord.row, coord.col);
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

		// sets grid size and resizes internal map
		virtual void SetSize(const spatial::Size<int>& size)
		{
			m_map.resize(size.width * size.height);
			m_size = size;
		}

		// sets grid height only
		virtual void SetHeight(const int height)
		{
			m_size.height = height;
		}

		// sets grid width only
		virtual void SetWidth(const int width)
		{
			m_size.width = width;
		}
	};

	// helper method to check if given tile coordinate is walkable
	inline bool IsWalkable(const TileLayer& layer, const Tileset& tileset, int row, int col)
	{
		if (!layer.IsValidTile(row, col)) return false;
		int id = layer.GetTileInstance(row, col).index;
		return tileset.IsValid(id) && tileset.GetTile(id).IsWalkable();
	}

} 
