#pragma once
#include <Components/Tile.h>
#include <Containers/Dictionary.h>
#include <Spatial/Coord.h>

namespace engine
{
	namespace tile
	{

		enum class TileVariant : unsigned int
		{
			// Base tiles
			Empty,// = 4, // water tile so not walkable. doesn't matter. this is background map
			Island,// = 30, // single land tile surrounded by water
			Full,// = 10,   // land surrounded on all sides

			// single-edge tiles (land on one side, water on three)
			NorthEdge,// = 21, // land south, water north+east+west
			SouthEdge,// = 3,  // land north, water south+east+west
			EastEdge,// = 29,  // land west, water north+south+east
			WestEdge,// = 27,  // land east, water north+south+west

			// corner tiles (land on two adjacent sides)
			NECorner,// = 0, // land south+west, water north+east
			NWCorner,// = 2, // land south+east, water north+west
			SECorner,// = 18, // land north+west, water south+east
			SWCorner,// = 20, // land north+east, water south+west

			// strips (land on opposite sides)
			Vertical,// = 12, // land north+south, water east+west
			Horizontal,// = 28,  // land east+west, water north+south

			// junctions
			TNorth,// = 1, // land south+east+west, water north 
			TSouth,// = 19, // land north+east+west, water south
			TEast,// = 9,  // land north+south+west, water east
			TWest,// = 11  // land north+south+east, water west
		};

		// purpose:
		// AutoTileResolver is a generic engine component that automatically selects the correct tile variant based on its neighbors. 
		// it ensures that when you place or remove a tile, the surrounding tiles update to visually connect seamlessly (edges, corners, strips, junctions, etc.).
		//
		// mapping System:
		// the mapping is handled by two dictionaries : 
		//	-	m_indexToVariant: Maps a tile index(from the tileset) to a TileVariant.
		//	-	m_variantToIndex : Maps a TileVariant back to the tileset index.
		// this ensures bidirectional lookup :
		//	-	when placing a tile, you can resolve its variant from its index.
		//  -	when computing a variant, you can resolve the correct index to instantiate.
		// 
		// mask Computation 
		// neighbors are encoded into a 4‑bit mask :
		//	-	North = 8
		//	-	South = 2
		//	-	East = 4
		//	-	West = 1
		// example:
		// surrounded on all sides → mask = 15 → TileVariant::Full
		// land north + south only → mask = 10 → TileVariant::Vertical
		// land east + west only → mask = 5 → TileVariant::Horizontal
		// this mask is passed to ResolveTileVariant(mask) which returns the correct variant.
		//
		// tile Placement 
		// when you call PlaceTile(region, tileset, coord, variant) 
		//	-	the tile at coord is set to the correct variant. 
		//	-	its 4 cardinal neighbors are re‑evaluated(N, S, E, W). 
		//	-	each neighbor’s mask is recomputed, and its variant updated if necessary. 
		//	-	this ensures seamless transitions when tiles are added or removed.
		// 
		// example flow 
		//	-	you place a land tile at(5, 5). 
		//	-	its mask is computed → surrounded by water → Island. 
		//	-	neighbors(4, 5), (6, 5), (5, 4), (5, 6) are checked. 
		//	-	if any are land, their masks change(e.g., (5, 4) becomes WestEdge). 
		//	-	the visual map updates automatically.
		template<typename T>
		class AutoTileResolver
		{
		private:
			engine::container::Dictionary<int, TileVariant> m_indexToVariant;
			engine::container::Dictionary<TileVariant, int> m_variantToIndex;

		public:
			AutoTileResolver()
			{
			}

			void Register(int index, TileVariant variant)
			{
				m_indexToVariant[index] = variant;
				m_variantToIndex[variant] = index;
			}

			void Set(engine::component::tile::TileRegion<T>& region, engine::component::tile::Tileset<T>& tileset, const engine::spatial::Coord& coord)
			{
				if (!region.IsInBounds(coord)) return;

				unsigned int mask = ComputeMask(region, coord.row, coord.col);

				TileVariant variant = ResolveTileVariant(mask);

				PlaceTile(region, tileset, coord, variant);
			}

			void Remove(engine::component::tile::TileRegion<T>& region, const engine::component::tile::Tileset<T>& tileset, const Coord& coord)
			{
				if (!region.IsInBounds(coord)) return;

				// if we don't have an empty tile registered, we can't remove. just return early.
				if (!m_variantToIndex.Has(TileVariant::Empty)) return;

				PlaceTile(region, tileset, coord, TileVariant::Empty);
			}

			void UpdateMask(engine::component::tile::TileRegion<T>& region, int row, int col, unsigned int& mask, unsigned int bit)
			{
				if (region.IsInBounds(row, col))
				{
					// defensive check to ensure tile exists at this location before accessing its index. if tile doesn't exist, treat it as empty for autotiling purposes and skip it.
					if (!region.Get(row, col).isValid()) return;

					int index = region.Get(row, col)->GetIndex();
					if (m_indexToVariant.Has(index) && m_indexToVariant[index] != TileVariant::Empty)
					{
						mask |= bit;
					}
				}
			}

			unsigned int ComputeMask(engine::component::tile::TileRegion<T>& region, int row, int col)
			{
				unsigned int mask = 0;

				UpdateMask(region, row - 1, col, mask, 8);	// N
				UpdateMask(region, row + 1, col, mask, 2);	// S
				UpdateMask(region, row, col + 1, mask, 4);	// E
				UpdateMask(region, row, col - 1, mask, 1);	// W
				//UpdateMask(region, row - 1, col - 1, mask);	// NW
				//UpdateMask(region, row - 1, col + 1, mask);	// NE
				//UpdateMask(region, row + 1, col - 1, mask);	// SW
				//UpdateMask(region, row + 1, col + 1, mask);	// SE

				return mask;
			}

			TileVariant ResolveTileVariant(int mask)
			{
				switch (mask)
				{
				case 0:   return TileVariant::Island;		// surrounded by nothing
				case 15:  return TileVariant::Full;			// surrounded by same tile type on all 4 sides

				case 8:   return TileVariant::NorthEdge;	// same tile type on north only. nothing on south, east, west
				case 2:   return TileVariant::SouthEdge;	// same tile type on south only. nothing on north, east, west
				case 1:   return TileVariant::EastEdge;		// same tile type on east only. nothing on north, south, west
				case 4:   return TileVariant::WestEdge;		// same tile type on west only. nothing on north, south, east

				case 10:  return TileVariant::Vertical;		// same tile type on north+south. nothing on east, west
				case 5:   return TileVariant::Horizontal;	// same tile type on east+west. nothing on north, south

				case 7:   return TileVariant::TNorth;		// same tile type on south+east+west. nothing on north
				case 13:  return TileVariant::TSouth;		// same tile type on north+east+west. nothing on south
				case 14:  return TileVariant::TEast;		// same tile type on north+south+west. nothing on east	
				case 11:  return TileVariant::TWest;		// same tile type on north+south+east. nothing on west

				case 6: return TileVariant::NECorner;		// same tile type on north+east. nothing on south, west 
				case 3: return TileVariant::NWCorner;		// same tile type on north+west. nothing on south, east
				case 12: return TileVariant::SECorner;		// same tile type on south+east. nothing on north, west
				case 9: return TileVariant::SWCorner;		// same tile type on south+west. nothing on north, east

				default:  return TileVariant::Empty;		// default to empty tile if mask configuration not found. this should not happen if we cover all cases.
				}
			}

			void PlaceTile(engine::component::tile::TileRegion<T>& region, const engine::component::tile::Tileset<T>& tileset, const engine::spatial::Coord& coord, const TileVariant type)
			{
				// Set the selected tile
				region.Set(coord, tileset.MakeTile(m_variantToIndex[type]));

				// Update self + 4 neighbors (skip diagonals)
				for (int dr = -1; dr <= 1; ++dr)
				{
					for (int dc = -1; dc <= 1; ++dc)
					{
						// Skip corners (diagonals)
						if (std::abs(dr) + std::abs(dc) > 1) continue;

						// neighbor tile coords
						int nr = coord.row + dr;
						int nc = coord.col + dc;

						// if neighbor tile is out of bounds or not walkable, skip it.
						if (!region.IsInBounds(nr, nc)) continue;

						// this is the tile we just placed, so we already know its new variant. skip it since we don't need to recompute it.
						if (dr == 0 && dc == 0) continue;

						// defensive check to ensure tile exists at this location before accessing its index. if tile doesn't exist, treat it as empty for autotiling purposes and skip it.
						if (!region.Get(nr, nc).isValid()) continue;

						// if tile exists but is empty tile, skip it since empty tile is like "air" and doesn't affect autotiling of neighbors
						int index = region.Get(nr, nc)->GetIndex();
						if (m_indexToVariant.Has(index) && m_indexToVariant[index] == TileVariant::Empty) continue;

						// evaluate this neighbor if this it of same tile type as the one we just placed. if not, skip it since its tile variant won't be affected by the new tile.
						if (!m_indexToVariant.Has(index)) continue;

						unsigned int mask = ComputeMask(region, nr, nc);
						TileVariant variant = ResolveTileVariant(mask);

						region.Set(nr, nc, tileset.MakeTile(m_variantToIndex[variant]));
					}
				}
			}
		};
	}
}