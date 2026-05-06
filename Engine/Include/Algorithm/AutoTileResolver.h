#pragma once
#include <Components/Tile.h>
#include <Containers/Dictionary.h>
#include <Spatial/Coord.h>
#include <Core/Event.h>

namespace engine
{
	namespace tile2
	{
		// UML diagram :
		//  +--------------------------+       fires event      +-----------------------+
		//  | AutoTileResolver         | ---------------------> | LookupTileResolver    |
		//	|--------------------------|                        |-----------------------|
		//	| +TileVariantChangedEvent |                        | +OnTileVariantChanged |
		//	+--------------------------+                        +-----------------------+
		//	           |
		//	           | delegates variant info
		//	           v
		//	+ -------------------------+
		//	| Event<Coord&, Variant>   |
		//	|--------------------------|
		//	| +operator()              |
		//	| +operator+=              |
		//	| +operator-=              |
		//	+--------------------------+
		//
		// Flow:
		// AutoTileResolver computes adjacency masks and determines the correct TileVariant for a floor tile.
		// It places the tile in the TileRegion using the Tileset.
		// If a LookupTileResolver is plugged in, AutoTileResolver passes the TileVariant and coordinate to it.
		// LookupTileResolver uses its own dictionary(variant → index) and places the corresponding wall(or decoration) tile in its own region / tileset.

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

		template<typename T>
		class LookupTileResolver
		{
		private:
			engine::container::Dictionary<TileVariant, int> m_variantToIndex;
			engine::component::tile1::TileRegion<T>& m_region;
			engine::component::tile1::Tileset<T>& m_tileset;

		public:
			LookupTileResolver(
				engine::component::tile1::TileRegion<T>& region,
				engine::component::tile1::Tileset<T>& tileset)
				: m_region(region), m_tileset(tileset)
			{
			}

			void Register(TileVariant variant, int index)
			{
				m_variantToIndex[variant] = index;
			}

			void Set(const engine::spatial::Coord& coord, TileVariant variant)
			{
				if (!m_region.IsInBounds(coord)) return;
				if (!m_variantToIndex.Has(variant)) return;

				int index = m_variantToIndex[variant];
				m_region.Set(coord, m_tileset.MakeTile(index));
			}
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
			engine::component::tile1::TileRegion<T>& m_region;
			engine::component::tile1::Tileset<T>& m_tileset;

		public:
			AutoTileResolver(
				engine::component::tile1::TileRegion<T>& region,
				engine::component::tile1::Tileset<T>& tileset)
				: m_region(region), m_tileset(tileset)
			{
			}

			virtual ~AutoTileResolver()
			{
				TileVariantChangedEvent.Clear();	
			}

			void Register(int index, TileVariant variant)
			{
				m_indexToVariant[index] = variant;
				m_variantToIndex[variant] = index;
			}

			void ResolveNeighbors(const engine::spatial::Coord& coord)	
			{
				// Update self + 4 neighbors (skip diagonals)
				for (int dr = -1; dr <= 1; ++dr)
				{
					for (int dc = -1; dc <= 1; ++dc)
					{
						// Skip corners (diagonals)
						if (std::abs(dr) + std::abs(dc) > 1) continue;

						// neighbor tile coords
						engine::spatial::Coord neighborCoord = { coord.row + dr, coord.col + dc };

						// if neighbor tile is out of bounds or not walkable, skip it.
						if (!m_region.IsInBounds(neighborCoord)) continue;

						// this is the tile we just placed, so we already know its new variant. skip it since we don't need to recompute it.
						if (dr == 0 && dc == 0) continue;

						// defensive check to ensure tile exists at this location before accessing its index. if tile doesn't exist, treat it as empty for autotiling purposes and skip it.
						if (!m_region.Get(neighborCoord).IsValid()) continue;

						// if tile exists but is empty tile, skip it since empty tile is like "air" and doesn't affect autotiling of neighbors
						int index = m_region.Get(neighborCoord)->GetIndex();
						if (m_indexToVariant.Has(index) && m_indexToVariant[index] == TileVariant::Empty) continue;

						// evaluate this neighbor if this it of same tile type as the one we just placed. if not, skip it since its tile variant won't be affected by the new tile.
						if (!m_indexToVariant.Has(index)) continue;

						unsigned int mask = ComputeMask(neighborCoord);
						TileVariant variant = ResolveTileVariant(mask);

						PlaceTile(neighborCoord, variant);
					}
				}
			}

			void Set(const engine::spatial::Coord& coord)
			{
				if (!m_region.IsInBounds(coord)) return;

				unsigned int mask = ComputeMask(coord);

				TileVariant variant = ResolveTileVariant(mask);

				// Set the selected tile
				PlaceTile(coord, variant);

				// update neighbors to ensure seamless transitions
				ResolveNeighbors(coord);
			}

			void Set()
			{
				engine::spatial::Size<size_t> size = m_region.GetSize();

				for (int row = 0; row < size.height; row++)
				{
					for (int col = 0; col < size.width; col++)
					{
						Set({ row, col });
					}
				}
			}

			void Remove()
			{
				engine::spatial::Size<size_t> size = m_region.GetSize();

				for (int row = 0; row < size.height; row++)
				{
					for (int col = 0; col < size.width; col++)
					{
						Remove({ row, col });
					}
				}
			}

			void Remove(const engine::spatial::Coord& coord)
			{
				if (!m_region.IsInBounds(coord)) return;

				// if we don't have an empty tile registered, we can't remove. just return early.
				if (!m_variantToIndex.Has(TileVariant::Empty)) return;

				// remove the selected tile
				PlaceTile(coord, TileVariant::Empty);


				// update neighbors to ensure seamless transitions
				ResolveNeighbors(coord);
			}

			void UpdateMask(const engine::spatial::Coord& coord, unsigned int& mask, unsigned int bit)
			{
				if (m_region.IsInBounds(coord))
				{
					// defensive check to ensure tile exists at this location before accessing its index. if tile doesn't exist, treat it as empty for autotiling purposes and skip it.
					if (!m_region.Get(coord).IsValid()) return;

					int index = m_region.Get(coord)->GetIndex();
					if (m_indexToVariant.Has(index) && m_indexToVariant[index] != TileVariant::Empty)
					{
						mask |= bit;
					}
				}
			}

			unsigned int ComputeMask(const engine::spatial::Coord& coord)
			{
				unsigned int mask = 0;

				UpdateMask({ coord.row - 1, coord.col }, mask, 8);	// N
				UpdateMask({ coord.row + 1, coord.col }, mask, 2);	// S
				UpdateMask({ coord.row, coord.col + 1 }, mask, 4);	// E
				UpdateMask({ coord.row, coord.col - 1 }, mask, 1);	// W

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

			void PlaceTile(const engine::spatial::Coord& coord, const TileVariant type)
			{
				// Set the selected tile
				m_region.Set(coord, m_tileset.MakeTile(m_variantToIndex[type]));

				// Notify listeners about the tile variant change
				TileVariantChangedEvent(coord, type);
			}

			engine::event::Event<const engine::spatial::Coord&, TileVariant> TileVariantChangedEvent;
		};

	}
}

namespace engine
{
	namespace tile
	{
		// UML diagram :
		//  +--------------------------+       fires event      +-----------------------+
		//  | AutoTileResolver         | ---------------------> | LookupTileResolver    |
		//	|--------------------------|                        |-----------------------|
		//	| +TileVariantChangedEvent |                        | +OnTileVariantChanged |
		//	+--------------------------+                        +-----------------------+
		//	           |
		//	           | delegates variant info
		//	           v
		//	+ -------------------------+
		//	| Event<Coord&, Variant>   |
		//	|--------------------------|
		//	| +operator()              |
		//	| +operator+=              |
		//	| +operator-=              |
		//	+--------------------------+
		//
		// Flow:
		// AutoTileResolver computes adjacency masks and determines the correct TileVariant for a floor tile.
		// It places the tile in the TileRegion using the Tileset.
		// If a LookupTileResolver is plugged in, AutoTileResolver passes the TileVariant and coordinate to it.
		// LookupTileResolver uses its own dictionary(variant → index) and places the corresponding wall(or decoration) tile in its own region / tileset.

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
		class AutoTileResolver
		{
		public:
			struct AutoTileConfig
			{
				engine::container::Dictionary<int, TileVariant> indexToVariant;
				engine::container::Dictionary<TileVariant, int> variantToIndex;

				void Register(int index, TileVariant variant)
				{
					indexToVariant[index] = variant;
					variantToIndex[variant] = index;
				}

				bool HasIndex(int index) const
				{
					return indexToVariant.Has(index);
				}

				bool HasVariant(TileVariant variant) const
				{
					return variantToIndex.Has(variant);
				}

				TileVariant ToVariant(int index) const
				{
					return indexToVariant[index];
				}

				int ToIndex(TileVariant variant) const
				{
					return variantToIndex[variant];
				}
			};

		private:
			AutoTileConfig* m_autoTileConfig;

			std::function<int(const Coord&)> m_getIndex;
			std::function<bool(const Coord&)> m_isValid;
			std::function<void(const Coord&, int)> m_applyVariant;

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

			void UpdateMask(const engine::spatial::Coord& coord, unsigned int& mask, unsigned int bit)
			{
				// these are neighbor tiles. we only check main tile for bounds, so we check this neighbor here
				if (!m_isValid(coord)) return;

				// get index. we know it's a valid coord by now
				int index = m_getIndex(coord);

				if (m_autoTileConfig->indexToVariant.Has(index) && m_autoTileConfig->indexToVariant[index] != TileVariant::Empty)
				{
					mask |= bit;
				}
			}

			unsigned int ComputeMask(const engine::spatial::Coord& coord)
			{
				unsigned int mask = 0;

				UpdateMask({ coord.row - 1, coord.col }, mask, 8);	// N
				UpdateMask({ coord.row + 1, coord.col }, mask, 2);	// S
				UpdateMask({ coord.row, coord.col + 1 }, mask, 4);	// E
				UpdateMask({ coord.row, coord.col - 1 }, mask, 1);	// W

				return mask;
			}

			void PlaceTile(const engine::spatial::Coord& coord, const TileVariant type)
			{
				// Set the selected tile
				m_applyVariant(coord, m_autoTileConfig->variantToIndex[type]);

				// Notify listeners about the tile variant change
				TileVariantChangedEvent(coord, type);
			}

			void ResolveNeighbors(const engine::spatial::Coord& coord)
			{
				// Update self + 4 neighbors (skip diagonals)
				for (int dr = -1; dr <= 1; ++dr)
				{
					for (int dc = -1; dc <= 1; ++dc)
					{
						// Skip corners (diagonals)
						if (std::abs(dr) + std::abs(dc) > 1) continue;

						// neighbor tile coords
						engine::spatial::Coord neighborCoord = { coord.row + dr, coord.col + dc };

						// if neighbor tile is out of bounds or not walkable, skip it.
						if (!m_isValid(neighborCoord)) continue;

						// if this is the tile we just placed, so we already know its new variant. skip it since we don't need to recompute it.
						if (dr == 0 && dc == 0) continue;

						// if tile exists but is empty tile, skip it since empty tile is like "air" and doesn't affect autotiling of neighbors
						int index = m_getIndex(neighborCoord);
						if (m_autoTileConfig->indexToVariant.Has(index) && m_autoTileConfig->indexToVariant[index] == TileVariant::Empty) continue;

						// evaluate this neighbor if this it of same tile type as the one we just placed. if not, skip it since its tile variant won't be affected by the new tile.
						if (!m_autoTileConfig->indexToVariant.Has(index)) continue;

						unsigned int mask = ComputeMask(neighborCoord);
						TileVariant variant = ResolveTileVariant(mask);

						TileVariant currVariant = m_autoTileConfig->indexToVariant[index];
						if (currVariant != variant)
						{
							// Set the selected tile
							PlaceTile(neighborCoord, variant);
						}

						//PlaceTile(neighborCoord, variant);
					}
				}
			}

		public:
			AutoTileResolver(
				std::function<bool(const Coord&)> isValid,
				std::function<int(const Coord&)> getIndex,
				std::function<void(const Coord&, int)> applyVariant,
				AutoTileConfig& autoTileConfig
			):
				m_isValid(isValid),
				m_getIndex(getIndex),
				m_applyVariant(applyVariant),
				m_autoTileConfig(&autoTileConfig)
			{
			}

			AutoTileResolver(const AutoTileResolver&) = delete;
			AutoTileResolver& operator=(const AutoTileResolver&) = delete;
			AutoTileResolver(AutoTileResolver&&) = delete;
			AutoTileResolver& operator=(AutoTileResolver&&) = delete;

			int Get(TileVariant variant)
			{
				return m_autoTileConfig->variantToIndex[variant];
			}

			TileVariant Get(int index)
			{
				return m_autoTileConfig->indexToVariant[index];
			}

			bool Has(TileVariant variant)
			{
				return m_autoTileConfig->variantToIndex.Has(variant);
			}

			bool Has(int index)
			{
				return m_autoTileConfig->indexToVariant.Has(index);
			}

			void Register(int index, TileVariant variant)
			{
				m_autoTileConfig->indexToVariant[index] = variant;
				m_autoTileConfig->variantToIndex[variant] = index;
			}

			virtual ~AutoTileResolver()
			{
				TileVariantChangedEvent.Clear();
			}

			void Set(const engine::spatial::Coord& coord)
			{
				// quick check if coord is within bounds
				if (!m_isValid(coord)) return;

				// evaluates cardinal neighboors of this coord to check if they are set as tiles
				unsigned int mask = ComputeMask(coord);

				// get the variant of the current tile in the coord
				int currIndex = m_getIndex(coord);
				if (!m_autoTileConfig->indexToVariant.Has(currIndex)) return;
				TileVariant currVariant = m_autoTileConfig->indexToVariant[currIndex];

				// decide which variant of tile this coord is going to be
				TileVariant variant = ResolveTileVariant(mask);

				// if current variant is same from replacement variant, don't bother setting it.
				if (currVariant != variant)
				{
					// Set the selected tile
					PlaceTile(coord, variant);
				}

				// (TODO: always, but find out why) update neighbors to ensure seamless transitions
				ResolveNeighbors(coord);
			}

			void Remove(const engine::spatial::Coord& coord)
			{
				// quick check if coord is within bounds
				if (!m_isValid(coord)) return;

				// if we don't have an empty tile registered, we can't remove. just return early.
				if (!m_autoTileConfig->variantToIndex.Has(TileVariant::Empty)) return;

				// get the variant of the current tile in the coord
				int currIndex = m_getIndex(coord);
				if (!m_autoTileConfig->indexToVariant.Has(currIndex)) return;
				TileVariant currVariant = m_autoTileConfig->indexToVariant[currIndex];

				// if tile is already empty, no need to remove anymore.
				if (currVariant != TileVariant::Empty)
				{
					// remove the selected tile
					PlaceTile(coord, TileVariant::Empty);
				}

				// update neighbors to ensure seamless transitions
				ResolveNeighbors(coord);
			}

			void Set(engine::spatial::Size<size_t> size)
			{
				for (int row = 0; row < size.height; row++)
				{
					for (int col = 0; col < size.width; col++)
					{
						Set(engine::spatial::Coord(row, col));
					}
				}
			}

			void Remove(engine::spatial::Size<size_t> size)
			{
				for (int row = 0; row < size.height; row++)
				{
					for (int col = 0; col < size.width; col++)
					{
						Remove(engine::spatial::Coord(row, col));
					}
				}
			}

			engine::event::Event<const engine::spatial::Coord&, TileVariant> TileVariantChangedEvent;
		};


	}

}