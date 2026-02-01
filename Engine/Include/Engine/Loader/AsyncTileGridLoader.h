#pragma once
#include <Containers/Table.h>
#include <Components/Tile.h>
#include <Timer/StopWatch.h>
#include <Spatial/Position.h>
#include <queue>

namespace engine
{
	namespace component
	{
		namespace tile
		{
			namespace loader
			{
				// ------------------------------------------------------------------------------------------------------------------
				// Design considerations
				// - Purpose:
				//     Utility class that incrementally or synchronously loads a TileGrid<T> from a Table of string data.
				//     Each cell is converted to type U, then mapped into a Tile<T> using a user-provided tileLoader function.
				// 
				// - Features:
				//     - Non-owning references: m_table, m_tileset, and m_grid are borrowed; caller manages their lifetime.
				//     - Supports both blocking (SyncLoadAll) and time-sliced (Begin + Update) loading.
				//     - Iterates serially over the table’s flat array of cells (row-major order).
				//     - Decouples parsing from tile construction via std::function tileLoader callback.
				//     - Maintains internal state via a flat index (m_currTile) and completion flag (m_isDone).
				// 
				// - Highlights:
				//     - Async loading: Update() consumes tiles within a time budget (maxTimeToReadMS).
				//     - Sync loading: SyncLoadAll() drives Update() until completion.
				//     - Progress reporting: GetLoadedTilesCount() and GetProgress() provide insight into load status.
				//     - Error handling: throws if table has no rows or columns.
				// 
				// - Limitations:
				//     - Assumes rectangular grid; does not validate row width consistency.
				//     - No parallelism: Update() runs on caller thread; external scheduling required.
				//     - No built-in progress events; client must poll GetProgress() or IsDone().
				//     - Loader does not own the TileGrid<T>; caller must manage its lifetime.
				// 
				// - Usage:
				//     // Blocking load
				//     auto grid = AsyncTileGridLoader<int,std::string>::SyncLoadAll(
				//         table,
				//         tileset,
				//         [](const std::string& cell, const Tileset<int>& ts) {
				//             return ts.CreateTile(cell);
				//         });
				//
				//     // Async load
				//     AsyncTileGridLoader<int,std::string> loader;
				//     loader.Begin(grid, table, tileset, tileLoader);
				//     while (!loader.IsDone()) {
				//         loader.Update(1.0); // process tiles within 1ms budget
				//     }
				// ------------------------------------------------------------------------------------------------------------------
				template<typename T, typename U>
				class AsyncTileGridLoader
				{
				private:
					const container::Table<std::string>* m_table;
					const component::tile::Tileset<T>* m_tileset;
					std::function<component::tile::Tile<T>(const U&, const component::tile::Tileset<T>&)> m_tileLoader;
					component::tile::TileGrid<T>* m_grid;
					bool m_isDone;
					size_t m_currTile;
					size_t m_totalTiles;

				public:
					AsyncTileGridLoader() :
						m_isDone(false),
						m_table(nullptr),
						m_tileset(nullptr),
						m_grid(nullptr),
						m_currTile(0),
						m_totalTiles(0)
					{
					}

					// blocking load: consumes the entire table in one shot 
					// internally calls Begin() and repeatedly Update() until all tiles are loaded.
					component::tile::TileGrid<T>& SyncLoadAll(
						component::tile::TileGrid<T>& grid,
						const container::Table<std::string>& table,
						const component::tile::Tileset<T>& tileset,
						std::function<component::tile::Tile<T>(const U&, const component::tile::Tileset<T>&)> tileLoader,
						size_t maxTilesPerUpdate = 0xFF,
						double maxTimePerUpdateMS = 1.0
					)
					{
						// Initialize 
						Begin(grid, table, tileset, tileLoader);

						// Loop until done 
						while (!IsDone())
						{
							Update(maxTimePerUpdateMS);
						}
						// Return the fully loaded grid 
						return grid;
					}

					size_t GetLoadedTilesCount() const
					{
						return m_currTile;
					}

					size_t GetTotalTilesCount() const
					{
						return m_totalTiles;
					}

					double GetProgress() const
					{
						return static_cast<double>(GetLoadedTilesCount()) / m_totalTiles;
					}

					// initializes the loader with a CSV table, tileset, and tileLoader function.
					// resets internal state and allocates a new TileGrid<T>.
					// throws if table has no rows or columns.
					void Begin(
						component::tile::TileGrid<T>& grid,
						const container::Table<std::string>& table,
						const component::tile::Tileset<T>& tileset,
						std::function<component::tile::Tile<T>(const U&, const component::tile::Tileset<T>&)> tileLoader
					)
					{
						// handle error if csv table has no rows
						if (!table.GetHeight())
						{
							throw std::out_of_range("csv table has no rows");
						}

						// handle error if csv's 1st row has no column
						if (!table.GetWidth())
						{
							throw std::out_of_range("csv table has no columns");
						}

						m_grid = &grid;
						m_table = &table;
						m_tileset = &tileset;
						m_tileLoader = tileLoader;
						m_isDone = false;
						m_currTile = 0;

						m_grid->SetWidth(m_table->GetWidth());

						m_totalTiles = m_table->GetElementCount();//  m_table->GetHeight()* m_table->GetWidth();
					}

					// incrementally loads tiles within a given time budget.
					// advances row / col indices, skipping malformed rows.
					// marks loader as done when all rows are processed.
					void Update(double maxTimeToReadMS = 1)
					{
						timer::StopWatch sw;
						sw.Start();
						while (!m_isDone)
						{
							// if we already loaded the last tile from map, we done. bail out.
							if (m_currTile >= m_table->GetElementCount())
							{
								m_isDone = true;
								break;
							}

							// at this point we now have valid row and col tile. get the tile object and load it
							std::string cell = m_table->Get(m_currTile);
							component::tile::Tile<T> tile = m_tileLoader(std::stoi(cell), *m_tileset);
							m_grid->Append(tile);

							// move to next col tile
							m_currTile++;

							// if we reached time budget, bail out for this frame
							if (sw.Peek<timer::milliseconds>() >= maxTimeToReadMS)
							{
								break;
							}
						}
					}

					bool IsDone() const
					{
						return m_isDone;
					}
				};

				template<typename T, typename U>
				class AsyncTileRegionLoader
				{
				private:
					AsyncTileGridLoader<T, U> m_asyncTileGridLoader;

				public:
					AsyncTileRegionLoader() = default;

					// blocking load: consumes the entire table in one shot 
					// internally calls Begin() and repeatedly Update() until all tiles are loaded.
					component::tile::TileRegion<T>& SyncLoadAll(
						component::tile::TileRegion<T>& region,
						const container::Table<std::string>& table,
						const component::tile::Tileset<T>& tileset,
						std::function<component::tile::Tile<T>(const U&, const component::tile::Tileset<T>&)> tileLoader,
						size_t maxTilesPerUpdate = 0xFF,
						double maxTimePerUpdateMS = 1.0
					)
					{
						// Initialize 
						Begin(region, table, tileset, tileLoader);

						// Loop until done 
						while (!IsDone())
						{
							Update(maxTimePerUpdateMS);
						}

						// Return the fully loaded grid 
						return region;
					}

					size_t GetLoadedTilesCount() const
					{
						return m_asyncTileGridLoader.GetLoadedTilesCount();
					}

					size_t GetTotalTilesCount() const
					{
						return m_asyncTileGridLoader.GetTotalTilesCount();
					}

					double GetProgress() const
					{
						return m_asyncTileGridLoader.GetProgress();
					}

					// initializes the loader with a CSV table, tileset, and tileLoader function.
					// resets internal state and allocates a new TileGrid<T>.
					// throws if table has no rows or columns.
					void Begin(
						component::tile::TileRegion<T>& region,
						const container::Table<std::string>& table,
						const component::tile::Tileset<T>& tileset,
						std::function<component::tile::Tile<T>(const U&, const component::tile::Tileset<T>&)> tileLoader
					)
					{
						m_asyncTileGridLoader.Begin(region.Get(), table, tileset, tileLoader);
					}

					// incrementally loads tiles within a given time budget.
					// advances row / col indices, skipping malformed rows.
					// marks loader as done when all rows are processed.
					void Update(double maxTimeToReadMS = 1)
					{
						m_asyncTileGridLoader.Update(maxTimeToReadMS);
					}

					bool IsDone() const
					{
						return m_asyncTileGridLoader.IsDone();
					}
				};

				// ------------------------------------------------------------------------------------------------------------------
				// Design considerations
				// - Purpose:
				//     Utility class that incrementally or synchronously loads a TileLayer<T> from a Table of string data.
				//     The table is partitioned into regions of configurable size; each cell is converted to type U,
				//     then mapped into a Tile<T> using a user-provided tileLoader function.
				//
				// - Features:
				//     - Non-owning references: m_table, m_tileset, and m_layer are borrowed; caller manages their lifetime.
				//     - Supports both blocking (SyncLoadAll) and time-sliced (Begin + Update) loading.
				//     - Partitions the map into regions of size regionSize, with edge regions clamped to fit non-divisible maps.
				//     - Decouples parsing from tile construction via std::function tileLoader callback.
				//     - Maintains internal state (current region, current tile, completion flag) for incremental progress.
				//
				// - Highlights:
				//     - Async loading: Update() consumes tiles within a time budget (maxTimeToReadMS).
				//     - Sync loading: SyncLoadAll() drives Update() until completion.
				//     - Region-aware: Handles non-divisible maps by creating smaller edge regions automatically.
				//     - Progress reporting: GetLoadedTilesCount() and GetProgress() provide insight into load status.
				//     - Error handling: throws if table has no rows or columns.
				//     - Encapsulated region creation logic via CreateAndAddNewRegion() helper.
				//
				// - Limitations:
				//     - Assumes rectangular table; does not validate row width consistency beyond basic bounds.
				//     - No parallelism: Update() runs on caller thread; external scheduling required.
				//     - No built-in progress events; client must poll GetProgress() or IsDone().
				//     - Loader does not own the TileLayer<T>; caller must manage its lifetime.
				//
				// - Usage:
				//     // Blocking load
				//     AsyncTileLayerLoader<int,std::string> loader;
				//     loader.SyncLoadAll(layer, table, tileset, {32,32},
				//         [](const std::string& cell, const Tileset<int>& ts) {
				//             return ts.CreateTile(cell);
				//         });
				//
				//     // Async load
				//     AsyncTileLayerLoader<int,std::string> loader;
				//     loader.Begin(layer, table, tileset, {32,32}, tileLoader);
				//     while (!loader.IsDone()) {
				//         loader.Update(1.0); // process tiles within 1ms budget
				//     }
				// ------------------------------------------------------------------------------------------------------------------
				template<typename T, typename U>
				class AsyncTileLayerLoader
				{
				private:
					const container::Table<std::string>* m_table;
					const component::tile::Tileset<T>* m_tileset;
					std::function<component::tile::Tile<T>(const U&, const component::tile::Tileset<T>&)> m_tileLoader;
					component::tile::TileLayer<T>* m_layer;
					bool m_isDone;
					spatial::Size<size_t> m_regionSize;
					spatial::Size<size_t> m_layerSize;
					Coord m_currRegion;
					Coord m_currTile;
					size_t m_loadedTiles;
					size_t m_totalTiles;
					spatial::Size<size_t> m_currRegionSize;

					// helper method to create and add new region
					void CreateAndAddNewRegion()
					{
						// calculate the actual size of the region to create (may be smaller than regionSize if at edge)
						m_currRegionSize.width = std::min<size_t>(m_regionSize.width, m_table->GetWidth() - m_currRegion.col * m_regionSize.width);
						m_currRegionSize.height = std::min<size_t>(m_regionSize.height, m_table->GetHeight() - m_currRegion.row * m_regionSize.height);

						// create and add the new region
						m_layer->CreateAndAddRegion(m_currRegionSize);
					}

				public:
					AsyncTileLayerLoader() :
						m_isDone(false),
						m_table(nullptr),
						m_tileset(nullptr),
						m_layer(nullptr),
						m_regionSize({ 0,0 }),
						m_layerSize({ 0,0 }),
						m_currRegion({ 0,0 }),
						m_currTile({ 0,0 }),
						m_loadedTiles(0),
						m_totalTiles(0),
						m_currRegionSize({ 0,0 })
					{
					}

					size_t GetLoadedTilesCount() const
					{
						return m_loadedTiles;
					}

					size_t GetTotalTilesCount() const
					{
						return m_totalTiles;
					}

					double GetProgress() const
					{
						return static_cast<double>(m_loadedTiles) / m_totalTiles;
					}

					void Begin(
						component::tile::TileLayer<T>& layer,
						const container::Table<std::string>& table,
						const component::tile::Tileset<T>& tileset,
						const spatial::Size<size_t>& regionSize,
						std::function<component::tile::Tile<T>(const U&, const component::tile::Tileset<T>&)> tileLoader
					)
					{
						// handle error if csv table has no rows
						if (!table.GetHeight())
						{
							throw std::out_of_range("csv table has no rows");
						}

						// handle error if csv's 1st row has no column
						if (!table.GetWidth())
						{
							throw std::out_of_range("csv table has no columns");
						}

						// now we know table is valid, let's initialize our loader state
						m_table = &table;
						m_tileset = &tileset;
						m_tileLoader = tileLoader;
						m_isDone = false;
						m_layer = &layer;
						m_regionSize = regionSize;
						m_currRegion = { 0, 0 };
						m_currTile = { 0, 0 };
						m_totalTiles = 0;
						m_loadedTiles = 0;

						// size of the layer in terms of regions. we round up here to account for any remainder tiles that dont fill an entire region
						m_layerSize.width = (m_table->GetWidth() + m_regionSize.width - 1) / m_regionSize.width;
						m_layerSize.height = (m_table->GetHeight() + m_regionSize.height - 1) / m_regionSize.height;

						// set the width of layer in region. this is the number of region across the layer's column
						m_layer->SetWidth(m_layerSize.width);

						// remember the actual number of cells 
						m_totalTiles = m_table->GetElementCount();					

						// at this point, we know we need to add the first region because table has data (width and height > 0)
						CreateAndAddNewRegion();
					}

					void Update(double maxTimeToReadMS = 1)
					{
						bool addNewRegion = false;
						timer::StopWatch sw;
						sw.Start();
						while (!m_isDone)
						{
							// if current region row > layer region rows, we done. bail out
							if (m_currRegion.row >= m_layerSize.height)
							{
								m_isDone = true;
								break;
							}

							// if we already done all regions in current row...
							if (m_currRegion.col >= m_layerSize.width)
							{
								// move to next row
								m_currRegion.col = 0;
								m_currRegion.row++;

								addNewRegion = true;

								// since we now moving to next region, reset our tile counters
								m_currTile.col = 0;
								m_currTile.row = 0;

								// we immediately continue so we can check if new row is still valid or we reached end
								continue;
							}

							// if current tile row > region row, we done with this region.
							if (m_currTile.row >= m_currRegionSize.height)// m_regionSize.height)
							{
								// move to next region on the current row
								m_currRegion.col++;

								addNewRegion = true;

								// since we now moving to next region, reset our tile counters
								m_currTile.col = 0;
								m_currTile.row = 0;

								// we immediately continue so we can check if we need to move to next region row
								continue;
							}

							// if current tile is beyond the region's tile width...
							if (m_currTile.col >= m_currRegionSize.width)//	 m_regionSize.width)
							{
								// move to next tile row
								m_currTile.col = 0;
								m_currTile.row++;

								// we immediately continue so we can check if new tile row is still within the region's tile height
								continue;
							}

							if (addNewRegion)
							{
								CreateAndAddNewRegion();
								addNewRegion = false;
							}

							// if we reached this point, our current layer region coord and region tile coord are valid
							int mapRow = m_currRegion.row * (int)m_regionSize.height + m_currTile.row;
							int mapCol = m_currRegion.col * (int)m_regionSize.width + m_currTile.col;

							std::string cell = m_table->Get(mapRow, mapCol);

							component::tile::Tile<T> tile = m_tileLoader(std::stoi(cell), *m_tileset);

							component::tile::TileRegion<T>& region = m_layer->GetRegion(m_currRegion);
							region.Append(tile);
							m_currTile.col++;
							m_loadedTiles++;

							if (sw.Peek<timer::milliseconds>() >= maxTimeToReadMS)
							{
								break;
							}
						}
					}				

					bool IsDone() const
					{
						return m_isDone;
					}
				
					// Blocking load: consumes the entire table in one shot.
					// Internally calls Begin() and repeatedly Update() until all tiles are loaded.
					component::tile::TileLayer<T>& SyncLoadAll(
						component::tile::TileLayer<T>& layer,
						const container::Table<std::string>& table,
						const component::tile::Tileset<T>& tileset,
						const spatial::Size<size_t>& regionSize,
						std::function<component::tile::Tile<T>(const U&, const component::tile::Tileset<T>&)> tileLoader,
						double maxTimePerUpdateMS = 1.0
					)
					{
						// Initialize
						Begin(layer, table, tileset, regionSize, tileLoader);

						// Loop until done
						while (!IsDone())
						{
							Update(maxTimePerUpdateMS);
						}

						// Return the fully loaded layer
						return layer;
					}
				};
			}
		}
	}
}
