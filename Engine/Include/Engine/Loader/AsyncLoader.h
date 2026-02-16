#pragma once
#include <Containers/Table.h>
#include <Components/Tile.h>
#include <Timer/StopWatch.h>
#include <Spatial/Position.h>
#include <Core/Loader.h>
#include <IO/ASyncFileReader.h>
#include <Utilities/CSVParser.h>
#include <Math/Rect.h>
#include <queue>

//using namespace engine;

// forward declare
namespace engine
{
	namespace loader
	{
		namespace tile
		{
		}
		namespace container
		{
			template<typename T>
			class AsyncContainerClearer;
		}
	}
}

namespace engine
{
	namespace loader
	{
		namespace tile
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
			class AsyncTileGridLoader : public engine::loader::IAsyncLoader
			{
			private:
				const engine::container::Table<std::string>* m_table;
				std::function<component::tile::Tile<T>(const U&)> m_tileLoader;
				component::tile::TileGrid<T>* m_grid;
				bool m_isDone;
				size_t m_currTile;
				size_t m_totalTiles;
				std::string m_label;

			public:
				AsyncTileGridLoader() :
					m_isDone(false),
					m_table(nullptr),
					m_grid(nullptr),
					m_currTile(0),
					m_totalTiles(0),
					m_label("")
				{
				}

				std::string GetLabel() const override
				{
					return m_label;
				}

				// blocking load: consumes the entire table in one shot 
				// internally calls Begin() and repeatedly Update() until all tiles are loaded.
				component::tile::TileGrid<T>& SyncLoadAll(
					component::tile::TileGrid<T>& grid,
					const engine::container::Table<std::string>& table,
					std::function<component::tile::Tile<T>(const U&)> tileLoader,
					size_t maxTilesPerUpdate = 0xFF,
					double maxTimePerUpdateMS = 1.0
				)
				{
					// Initialize 
					Begin("TileGrid", grid, table, tileLoader);

					// Loop until done 
					while (!IsDone())
					{
						Update(maxTimePerUpdateMS);
					}
					// Return the fully loaded grid 
					return grid;
				}

				size_t GetCurrent() const override
				{
					return m_currTile;
				}

				size_t GetTotal() const override
				{
					return m_totalTiles;
				}

				double GetProgress() const override
				{
					return static_cast<double>(GetCurrent()) / m_totalTiles;
				}

				// initializes the loader with a CSV table, tileset, and tileLoader function.
				// resets internal state and allocates a new TileGrid<T>.
				// throws if table has no rows or columns.
				void Begin(
					const std::string& label,
					component::tile::TileGrid<T>& grid,
					const engine::container::Table<std::string>& table,
					std::function<component::tile::Tile<T>(const U&)> tileLoader
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

					m_label = label;	
					m_grid = &grid;
					m_table = &table;
					m_tileLoader = tileLoader;
					m_isDone = false;
					m_currTile = 0;

					m_grid->SetWidth(m_table->GetWidth());

					m_totalTiles = m_table->GetElementCount();
				}

				// incrementally loads tiles within a given time budget.
				// advances row / col indices, skipping malformed rows.
				// marks loader as done when all rows are processed.
				void Update(double maxTimeToReadMS = 1) override
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
						component::tile::Tile<T> tile = m_tileLoader(std::stoi(cell));
						m_grid->Add(tile);

						// move to next col tile
						m_currTile++;

						// if we reached time budget, bail out for this frame
						if (sw.Peek<timer::milliseconds>() >= maxTimeToReadMS)
						{
							break;
						}
					}
				}

				bool IsDone() const override
				{
					return m_isDone;
				}
			};

			// ------------------------------------------------------------------------------------------------------------------
			// Design considerations
			//	- this is a wrapper to TileGridLoader that loads into TileRegion instead of TileGrid.
			// ------------------------------------------------------------------------------------------------------------------
			template<typename T, typename U>
			class AsyncTileRegionLoader : public engine::loader::IAsyncLoader
			{
			private:
				AsyncTileGridLoader<T, U> m_asyncTileGridLoader;

			public:
				AsyncTileRegionLoader() = default;

				std::string GetLabel() const override
				{
					return m_asyncTileGridLoader.GetLabel();
				}

				// blocking load: consumes the entire table in one shot 
				// internally calls Begin() and repeatedly Update() until all tiles are loaded.
				component::tile::TileRegion<T>& SyncLoadAll(
					component::tile::TileRegion<T>& region,
					const engine::container::Table<std::string>& table,
					std::function<engine::component::tile::Tile<T>(const U&)> tileLoader,
					size_t maxTilesPerUpdate = 0xFF,
					double maxTimePerUpdateMS = 1.0
				)
				{
					// Initialize 
					Begin(region, table, tileLoader);

					// Loop until done 
					while (!IsDone())
					{
						Update(maxTimePerUpdateMS);
					}

					// Return the fully loaded grid 
					return region;
				}

				size_t GetTotal() const override
				{
					return m_asyncTileGridLoader.GetTotal();
				}

				size_t GetCurrent() const override
				{
					return m_asyncTileGridLoader.GetCurrent();
				}

				double GetProgress() const override
				{
					return m_asyncTileGridLoader.GetProgress();
				}

				// initializes the loader with a CSV table, tileset, and tileLoader function.
				// resets internal state and allocates a new TileGrid<T>.
				// throws if table has no rows or columns.
				void Begin(
					std::string label,
					component::tile::TileRegion<T>& region,
					const engine::container::Table<std::string>& table,
					std::function<engine::component::tile::Tile<T>(const U&)> tileLoader
				)
				{
					m_asyncTileGridLoader.Begin(label, region.Get(), table, tileLoader);
				}

				// incrementally loads tiles within a given time budget.
				// advances row / col indices, skipping malformed rows.
				// marks loader as done when all rows are processed.
				void Update(double maxTimeToReadMS = 1) override
				{
					m_asyncTileGridLoader.Update(maxTimeToReadMS);
				}

				bool IsDone() const override
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
			class AsyncTileLayerLoader : public engine::loader::IAsyncLoader
			{
			private:
				const engine::container::Table<std::string>* m_table;
				std::function<engine::component::tile::Tile<T>(const U&)> m_tileLoader;
				engine::component::tile::TileLayer<T>* m_layer;
				bool m_isDone;
				spatial::Size<size_t> m_regionSize;
				spatial::Size<size_t> m_layerSize;
				engine::component::tile::Coord m_currRegion;
				engine::component::tile::Coord m_currTile;
				size_t m_loadedTiles;
				size_t m_totalTiles;
				spatial::Size<size_t> m_currRegionSize;
				std::string m_label;

				// helper method to create and add new region
				void CreateAndAddNewRegion()
				{
					// calculate the actual size of the region to create (may be smaller than regionSize if at edge)
					m_currRegionSize.width = std::min<size_t>(m_regionSize.width, m_table->GetWidth() - m_currRegion.col * m_regionSize.width);
					m_currRegionSize.height = std::min<size_t>(m_regionSize.height, m_table->GetHeight() - m_currRegion.row * m_regionSize.height);

					component::tile::TileRegion<T> region(m_currRegionSize.width);
					m_layer->Take(std::move(region));
				}

			public:
				AsyncTileLayerLoader() :
					m_isDone(false),
					m_table(nullptr),
					m_layer(nullptr),
					m_regionSize({ 0,0 }),
					m_layerSize({ 0,0 }),
					m_currRegion({ 0,0 }),
					m_currTile({ 0,0 }),
					m_loadedTiles(0),
					m_totalTiles(0),
					m_currRegionSize({ 0,0 }),
					m_label("")
				{
				}

				double GetProgress() const override
				{
					return static_cast<double>(m_loadedTiles) / m_totalTiles;
				}

				void Begin(
					const std::string& label,
					engine::component::tile::TileLayer<T>& layer,
					const engine::container::Table<std::string>& table,
					const spatial::Size<size_t>& regionSize,
					std::function<engine::component::tile::Tile<T>(const U&)> tileLoader
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
					m_label = label;
					m_table = &table;
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

				void Update(double maxTimeToReadMS = 1) override
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

						component::tile::Tile<T> tile = m_tileLoader(std::stoi(cell));

						component::tile::TileRegion<T>& region = m_layer->Get(m_currRegion);
						region.Add(tile);
						m_currTile.col++;
						m_loadedTiles++;

						if (sw.Peek<timer::milliseconds>() >= maxTimeToReadMS)
						{
							break;
						}
					}
				}				

				bool IsDone() const override
				{
					return m_isDone;
				}
				
				// Blocking load: consumes the entire table in one shot.
				// Internally calls Begin() and repeatedly Update() until all tiles are loaded.
				component::tile::TileLayer<T>& SyncLoadAll(
					component::tile::TileLayer<T>& layer,
					const engine::container::Table<std::string>& table,
					const spatial::Size<size_t>& regionSize,
					std::function<component::tile::Tile<T>(const U&)> tileLoader,
					double maxTimePerUpdateMS = 1.0
				)
				{
					// Initialize
					Begin(layer, table, regionSize, tileLoader);

					// Loop until done
					while (!IsDone())
					{
						Update(maxTimePerUpdateMS);
					}

					// Return the fully loaded layer
					return layer;
				}
				
				virtual size_t GetCurrent() const override
				{
					return m_loadedTiles;
				}

				virtual size_t GetTotal() const override
				{
					return m_totalTiles;
				}

				virtual std::string GetLabel() const override
				{
					return m_label;
				}
			};
			
			// ------------------------------------------------------------------------------------------------------------------
			// Design considerations
			// - Purpose:
			//     Utility class that orchestrates the asynchronous loading of a TileLayer<T> from a CSV file.
			//     Manages a pipeline of loaders: clears existing regions and table data, reads file contents,
			//     parses CSV rows into a table, and constructs tiles into regions via a user-provided callback.
			//
			// - Features:
			//     - Non-owning references: m_tileset, m_layer, and m_tileLoader are borrowed; caller manages their lifetime.
			//     - Integrates multiple loaders: AsyncFileReader, CSVParser, AsyncContainerClearer, AsyncTileLayerLoader.
			//     - Supports incremental (Update with time budget) and blocking (loop until IsDone()) operation.
			//     - Handles staged transitions automatically: region clearing → table clearing → file reading → tile layer loading.
			//     - Provides progress reporting via GetCurrent(), GetTotal(), GetProgress(), and GetLabel().
			//     - Maintains internal state (current loader, completion flag) for predictable pipeline execution.
			//
			// - Highlights:
			//     - Async orchestration: Update() advances the active loader until completion, then transitions to the next stage.
			//     - Safe clearing: regions are cleared incrementally, with Pop() deferred until after a region is fully processed.
			//     - Event chaining: CSVParser is wired to AsyncFileReader events, and Table is wired to CSVParser events.
			//     - Flexible tile construction: caller supplies a std::function tileLoader to map parsed data into Tile<T> objects.
			//     - Robust lifecycle: constructor chains events, destructor unchains them to prevent leaks.
			//
			// - Limitations:
			//     - Assumes rectangular CSV input; minimal validation of row consistency.
			//     - No parallelism: Update() runs on caller thread; external scheduling required.
			//     - No built-in progress events; client must poll IsDone() or GetProgress().
			//     - Loader does not own TileLayer<T> or Tileset<T>; caller must ensure they outlive the loader.
			//     - Progress resets per stage; cumulative progress reporting must be implemented externally if desired.
			//
			// - Usage:
			//     // Blocking load
			//     AsyncTileMapLoader<int,std::string> loader;
			//     loader.Open("map.csv", tileset, {32,32},
			//         [](const std::string& cell, const Tileset<int>& ts) {
			//             return ts.CreateTile(cell);
			//         },
			//         layer);
			//     while (!loader.IsDone()) {
			//         loader.Update(1.0); // process within 1ms budget
			//     }
			//
			//     // Async load (time-sliced)
			//     AsyncTileMapLoader<int,std::string> loader;
			//     loader.Open("map.csv", tileset, {32,32}, tileLoader, layer);
			//     while (!loader.IsDone()) {
			//         loader.Update(0.5); // incrementally advance pipeline within 0.5ms
			//     }
			// ------------------------------------------------------------------------------------------------------------------
			template<typename T, typename U>
			class AsyncTileMapLoader : public engine::loader::IAsyncLoader
			{
			private:
				// loaders
				io::AsyncFileReader m_fileReader;
				engine::loader::tile::AsyncTileLayerLoader<T, U> m_tileLayerLoader;
				engine::loader::container::AsyncContainerClearer<engine::component::tile::Tile<T>> m_tileRegionClearer;
				engine::loader::container::AsyncContainerClearer<std::string> m_tableClearer;

				// current loader pointer
				engine::loader::IAsyncLoader* m_currentLoader;

				// parser 
				engine::utilities::parser::CSVParser m_csvParser;

				// data holder
				engine::container::Table<std::string> m_table;

				// references
				std::function<component::tile::Tile<T>(const U&)> m_tileLoader;
				spatial::Size<size_t> m_regionSize;

				// target layer
				component::tile::TileLayer<T>* m_layer;

				// states
				bool m_isFinished;

			public:
				AsyncTileMapLoader(size_t maxBytesPerRead = 0x1) :
					m_isFinished(false),
					m_fileReader(maxBytesPerRead),
					m_currentLoader(nullptr),
					m_layer(nullptr),
					m_regionSize({ 0,0 })
				{
					// chain our events where CSV parser listens to file reader when it extract chunk of data from file
					m_fileReader.ProcessChunkEvent += event::Handler(&m_csvParser, &engine::utilities::parser::CSVParser::ParseChunk);
					m_fileReader.EndOfFileFoundEvent += event::Handler(&m_csvParser, &engine::utilities::parser::CSVParser::ParseRemaining);

					// chain CSV table to CSV parser to acquire row of data from CSV Parser when it parse chunk of data and extracts rows of CSV data
					m_csvParser.ParseRowEvent += event::Handler(&m_table, &engine::container::Table<std::string>::AddRow);
					m_csvParser.ParseRemainingEvent += event::Handler(&m_table, &engine::container::Table<std::string>::AddRange);
				}

				~AsyncTileMapLoader()
				{
					// unchain events
					m_fileReader.ProcessChunkEvent -= event::Handler(&m_csvParser, &engine::utilities::parser::CSVParser::ParseChunk);
					m_fileReader.EndOfFileFoundEvent -= event::Handler(&m_csvParser, &engine::utilities::parser::CSVParser::ParseRemaining);
					m_csvParser.ParseRowEvent -= event::Handler(&m_table, &engine::container::Table<std::string>::AddRow);
					m_csvParser.ParseRemainingEvent -= event::Handler(&m_table, &engine::container::Table<std::string>::AddRange);
				}

				bool Open(
					const std::string& filename,
					const spatial::Size<size_t>& regionSize,
					std::function<component::tile::Tile<T>(const U&)> tileLoader,
					component::tile::TileLayer<T>& layer
				)
				{
					m_tileLoader = tileLoader;
					m_regionSize = regionSize;
					m_layer = &layer;
					m_isFinished = false;

					// is there existing data in layer?
					if (m_layer->GetElementCount())
					{
						m_currentLoader = &m_tileRegionClearer;
						m_tileRegionClearer.Begin("Clearing TileRegion", m_layer->Back(), 0.1);
					}
					// is there existing data in table?
					else
					{
						m_currentLoader = &m_tableClearer;
						m_tableClearer.Begin("Clearing Table", m_table, 0.1);
					}

					return m_fileReader.Open(filename);
				}

				void Update(double delta)
				{
					// do nothing if no current loader
					if (!m_currentLoader)
					{
						return;
					}

					// if we have a loader, update it
					m_currentLoader->Update(delta);

					// if current loader is not done yet, return. let it finish first
					if (!m_currentLoader->IsDone())
					{
						return;
					}

					// if we reach this point, current loader is done. setup next loader

					// if we have more regions to clear, setup next region clearer
					if (m_currentLoader == &m_tileRegionClearer)
					{
						// pop the region we just cleared
						m_layer->Pop();

						// if no more regions to clear, setup table clearer
						if (m_layer->IsEmpty())
						{
							m_currentLoader = &m_tableClearer;
							m_tableClearer.Begin("Clearing Table", m_table, 0.1);
						}
						// more regions to clear...
						else
						{
							m_currentLoader = &m_tileRegionClearer;
							m_tileRegionClearer.Begin("Clearing TileRegion", m_layer->Back(), 0.1);
						}
					}
					// if we just cleared the table, setup file reader
					else if (m_currentLoader == &m_tableClearer)
					{
						m_currentLoader = &m_fileReader;
					}
					// if we just finished reading the file, setup tilelayer loader
					else if (m_currentLoader == &m_fileReader)
					{
						// all clearing done, setup tilelayer loader
						m_tileLayerLoader.Begin(
							m_fileReader.GetLabel(),
							*m_layer,
							m_table,
							m_regionSize,
							m_tileLoader
						);
						// switch to tilelayer loader
						m_currentLoader = &m_tileLayerLoader;
						return;
					}
					else
					{
						m_isFinished = true;
					}
				}

				virtual size_t GetCurrent() const
				{
					return m_currentLoader ? m_currentLoader->GetCurrent() : 0;
				}

				virtual size_t GetTotal() const
				{
					return m_currentLoader ? m_currentLoader->GetTotal() : 0;
				}

				virtual std::string GetLabel() const
				{
					return m_currentLoader ? m_currentLoader->GetLabel() : "";
				}

				virtual double GetProgress() const
				{
					return m_currentLoader ? m_currentLoader->GetProgress() : 0.0;
				}

				virtual bool IsDone() const
				{
					return m_isFinished;
				}
			};
		
			template<typename T>
			class AsyncTileLayerClearer : public engine::loader::IAsyncLoader
			{
			private:
				engine::component::tile::TileLayer<T>* m_layer;
				size_t m_total;
				std::string m_label;
				bool m_isDone;
				engine::loader::container::AsyncContainerClearer<engine::component::tile::Tile<T>> m_tileRegionClearer;

				// for debug purposes only, we can add a delay to simulate longer processing time
				double m_delayMS = 0.0;

			public:
				AsyncTileLayerClearer():
					m_layer(nullptr),
					m_total(0),
					m_isDone(false),
					m_label("")
				{

				}

				~AsyncTileLayerClearer() = default;

				void Begin(
					const std::string& label,
					engine::component::tile::TileLayer<T>& layer,
					double delayMS = 0.0
				)
				{
					m_layer = &layer;
					m_label = label;
					m_isDone = false;
					m_delayMS = delayMS;

					m_total = m_layer ? layer.GetElementCount() : 0;

					if (m_layer && m_layer->GetElementCount() > 0)
					{
						m_tileRegionClearer.Begin("Clearing TileLayer", m_layer->Back(), m_delayMS);
					}
				}
				void Update(double maxTimeToReadMS) override
				{
					// if no layer, we are done
					if (!m_layer)
					{
						m_isDone = true;
						return;
					}

					timer::StopWatch sw;
					sw.Start();
					while (!m_isDone)
					{
						if (m_layer->IsEmpty())
						{
							m_isDone = true;
							break;
						}

						m_tileRegionClearer.Update(maxTimeToReadMS);

						if (m_tileRegionClearer.IsDone())
						{
							m_layer->Pop();

							if (!m_layer->IsEmpty())
							{
								m_tileRegionClearer.Begin("Clearing TileLayer", m_layer->Back(), m_delayMS);
							}
							else
							{
								m_isDone = true;
								break;
							}
						}

						// for debug purposes only, we can add a delay here to simulate longer processing time
						{
							timer::StopWatch delay;
							delay.Start();
							while (delay.Peek<timer::milliseconds>() < m_delayMS) {}
							delay.Stop();
						}

						if (sw.Peek<timer::milliseconds>() >= maxTimeToReadMS)
						{
							break;
						}
					}
				}

				size_t GetCurrent() const override
				{
					return m_layer ? m_total - m_layer->GetElementCount() : 0;
				}
				
				size_t GetTotal() const override
				{
					return m_total;
				}

				std::string GetLabel() const override
				{
					return m_label;
				}

				double GetProgress() const override
				{
					return m_total > 0 ? static_cast<double>(GetCurrent()) / m_total : 0.0;
				}

				bool IsDone() const override
				{
					return m_isDone;
				}



			};
		
			template<typename T, typename U>
			class AsyncCSVMapToTileRegionLoader : public engine::loader::IAsyncLoader
			{
			private:
				// loaders
				io::AsyncFileReader m_fileReader;
				engine::loader::tile::AsyncTileRegionLoader<T, U> m_tileRegionLoader;
				engine::loader::container::AsyncContainerClearer<engine::component::tile::Tile<T>> m_tileRegionClearer;
				engine::loader::container::AsyncContainerClearer<std::string> m_tableClearer;

				// current loader pointer
				engine::loader::IAsyncLoader* m_currentLoader;

				// parser 
				engine::utilities::parser::CSVParser m_csvParser;

				// data holder
				engine::container::Table<std::string> m_table;

				// references
				std::function<component::tile::Tile<T>(const U&)> m_tileLoader;

				// target region
				component::tile::TileRegion<T>* m_region;

				// states
				bool m_isFinished;

				// when tile region is already loaded with data
				bool m_regionIsLoaded;

				// holds current progress. total is hard coded to 100 
				size_t m_current;

				

			public:
				AsyncCSVMapToTileRegionLoader(size_t maxBytesPerRead = 0x1) :
					m_isFinished(false),
					m_fileReader(maxBytesPerRead),
					m_currentLoader(nullptr),
					m_region(nullptr),
					m_regionIsLoaded(false),
					m_current(0)
				{
					// chain our events where CSV parser listens to file reader when it extract chunk of data from file
					m_fileReader.ProcessChunkEvent += event::Handler(&m_csvParser, &engine::utilities::parser::CSVParser::ParseChunk);
					m_fileReader.EndOfFileFoundEvent += event::Handler(&m_csvParser, &engine::utilities::parser::CSVParser::ParseRemaining);

					// chain CSV table to CSV parser to acquire row of data from CSV Parser when it parse chunk of data and extracts rows of CSV data
					m_csvParser.ParseRowEvent += event::Handler(&m_table, &engine::container::Table<std::string>::AddRow);
					m_csvParser.ParseRemainingEvent += event::Handler(&m_table, &engine::container::Table<std::string>::AddRange);
				}

				~AsyncCSVMapToTileRegionLoader()
				{
					// unchain events
					m_fileReader.ProcessChunkEvent -= event::Handler(&m_csvParser, &engine::utilities::parser::CSVParser::ParseChunk);
					m_fileReader.EndOfFileFoundEvent -= event::Handler(&m_csvParser, &engine::utilities::parser::CSVParser::ParseRemaining);
					m_csvParser.ParseRowEvent -= event::Handler(&m_table, &engine::container::Table<std::string>::AddRow);
					m_csvParser.ParseRemainingEvent -= event::Handler(&m_table, &engine::container::Table<std::string>::AddRange);
				}

				bool Open(
					const std::string& filename,
					std::function<component::tile::Tile<T>(const U&)> tileLoader,
					component::tile::TileRegion<T>& region
				)
				{
					// make sure we can open the file first. if not, don't bother
					bool rslt = m_fileReader.Open(filename);

					if (!rslt) return false;

					m_tileLoader = tileLoader;
					m_region = &region;
					m_isFinished = false;
					m_regionIsLoaded = false;
					m_current = 0;

					// error if region object is invalid
					if (!m_region)
					{
						throw std::exception("AsyncCSVMapToTileRegionLoader::Open - component::tile::TileRegion<T> is null");
					}

					// let's start with clearing table first just in case
					m_currentLoader = &m_tableClearer;

					// begin clearing table
					m_tableClearer.Begin("Clearing Table", m_table, 0.01);

					return rslt;
				}

				void Update(double delta)
				{
					// do nothing if no current loader
					if (!m_currentLoader)
					{
						return;
					}

					// if we have a loader, update it
					m_currentLoader->Update(delta);

					// if current loader is not done yet, return. let it finish first
					if (!m_currentLoader->IsDone())
					{
						return;
					}

					// if we reach this point, current loader is done. setup next loader

					// if we just cleared the table and we haven't loaded data to tile region yet, setup region clearer
					if (m_currentLoader == &m_tableClearer && !m_regionIsLoaded)
					{
						m_currentLoader = &m_tileRegionClearer;
						m_tileRegionClearer.Begin("Clearing TileRegion", *m_region, 0.01);

						m_current += 10;
					}
					// if we just cleared region, read file
					else if (m_currentLoader == &m_tileRegionClearer)
					{
						m_currentLoader = &m_fileReader;

						m_current += 10;
					}
					// if we just finished reading the file, setup tileregion loader
					else if (m_currentLoader == &m_fileReader)
					{
						m_current += 35;

						// all clearing done, setup tileregion loader
						m_tileRegionLoader.Begin(
							"Loading TileRegion",
							*m_region,
							m_table,
							m_tileLoader
						);
						// switch to tileregion loader
						m_currentLoader = &m_tileRegionLoader;
					}
					// we just finished loading data into tileregion, clean up the table
					else if (m_currentLoader == &m_tileRegionLoader)
					{
						m_current += 35;

						m_regionIsLoaded = true;
						m_currentLoader = &m_tableClearer;
						m_tableClearer.Begin("Clearing Table", m_table, 0.01);
					}
					// we are done
					else
					{
						m_current += 10;
						m_currentLoader = nullptr;
						m_isFinished = true;
					}
				}

				virtual size_t GetCurrent() const
				{
					return m_current; // m_currentLoader ? m_currentLoader->GetCurrent() : 0;
				}

				virtual size_t GetTotal() const
				{
					return 100; // m_currentLoader ? m_currentLoader->GetTotal() : 0;
				}

				virtual std::string GetLabel() const
				{
					return IsDone()? "Loading Complete!" : m_currentLoader ? m_currentLoader->GetLabel() : "";
				}

				virtual double GetProgress() const
				{
					return IsDone() ? 100.0 : static_cast<double>(m_current) / 100; //m_currentLoader ? m_currentLoader->GetProgress() : 0.0;
				}

				virtual bool IsDone() const
				{
					return m_isFinished;
				}
			
				bool LoadImmediate(
					const std::string& filename,
					std::function<component::tile::Tile<T>(const U&)> tileLoader,
					component::tile::TileRegion<T>& region
				)
				{
					if (!Open(filename, tileLoader, region))
					{
						return false;
					}

					while (!IsDone())
					{
						Update(1000.0);
					}

					return true;
				}
			};
		}
		
		namespace container
		{
			// ------------------------------------------------------------------------------------------------------------------
			// Design considerations
			// - Purpose:
			//     Utility class that incrementally or synchronously clears an IContainer<T> by repeatedly invoking Pop().
			//     Provides a time-sliced Update() method for gradual clearing, or can be driven until completion for blocking use.
			//
			// - Features:
			//     - Non-owning reference: m_container is borrowed; caller manages its lifetime.
			//     - Supports both incremental clearing (Update with time budget) and synchronous clearing (loop until IsDone()).
			//     - Tracks total number of elements scheduled for clearing, current progress, and completion state.
			//     - Provides progress reporting via GetCurrent(), GetTotal(), and GetProgress().
			//     - Labeling support: caller can tag each clearing operation with a descriptive string.
			//     - Optional delay parameter (m_delayMS) to simulate longer processing for debugging.
			//
			// - Highlights:
			//     - Async clearing: Update() pops elements until the time budget expires, allowing cooperative multitasking.
			//     - Sync clearing: caller can drive Update() in a loop until IsDone() is true.
			//     - Safe null handling: if Begin is never called or container is null, loader is immediately marked done.
			//     - Lightweight: does not own or allocate; simply drives container’s Pop() method.
			//     - Encapsulated state: m_isDone flag ensures predictable completion semantics.
			//     - Deferred Pop(): container elements are only removed during Update(), avoiding dangling references.
			//
			// - Limitations:
			//     - Assumes container implements Pop(), GetElementCount(), IsEmpty(), etc. via IContainer<T>.
			//     - No parallelism: Update() runs on caller thread; external scheduling required.
			//     - No built-in events; client must poll IsDone() or GetProgress().
			//     - Does not reset m_total automatically; caller must call Begin() again to reuse for a new container.
			//     - Debug delay uses busy-wait; replace with sleep for production use if needed.
			//
			// - Usage:
			//     // Blocking clear
			//     AsyncContainerClearer<MyType> clearer;
			//     clearer.Begin("Clear Layer", myContainer);
			//     while (!clearer.IsDone()) {
			//         clearer.Update(1.0); // process within 1ms budget
			//     }
			//
			//     // Async clear (time-sliced)
			//     AsyncContainerClearer<MyType> clearer;
			//     clearer.Begin("Clear Table", myContainer, 0.5); // optional delay for debug
			//     clearer.Update(0.5); // clear incrementally within 0.5ms
			// ------------------------------------------------------------------------------------------------------------------
			template<typename T>
			class AsyncContainerClearer : public engine::loader::IAsyncLoader
			{
			private:
				engine::container::IContainer<T>* m_container;
				size_t m_total;
				std::string m_label;
				bool m_isDone;

				// for debug purposes only, we can add a delay to simulate longer processing time
				double m_delayMS;

			public:
				AsyncContainerClearer() :
					m_container(nullptr),
					m_total(0),
					m_isDone(false),
					m_label("")
				{
				}

				virtual ~AsyncContainerClearer() = default;

				void Begin(
					const std::string& label,
					engine::container::IContainer<T>& container,
					double delayMS = 0.0
				)
				{
					m_container = &container;
					m_label = label;
					m_isDone = false;
					m_delayMS = delayMS;

					// remember the actual number of cells 
					m_total = m_container ? m_container->GetElementCount() : 0;
				}

				void Update(double maxTimeToReadMS) override
				{
					// if no container, we are done
					if (!m_container)
					{
						m_isDone = true;
						return;
					}

					timer::StopWatch sw;
					sw.Start();
					while (!m_isDone)
					{
						if (m_container->IsEmpty())
						{
							m_isDone = true;
							break;
						}

						// pop one element to reduce the container size
						m_container->Pop();

						// for debug purposes only, we can add a delay here to simulate longer processing time
						{
							timer::StopWatch delay;
							delay.Start();
							while (delay.Peek<timer::milliseconds>() < m_delayMS) {}
							delay.Stop();
						}

						if (sw.Peek<timer::milliseconds>() >= maxTimeToReadMS)
						{
							break;
						}
					}
				}

				size_t GetCurrent() const override
				{
					return m_container ? m_total - m_container->GetElementCount() : 0;
				}

				size_t GetTotal() const override
				{
					return m_total;
				}

				std::string GetLabel() const override
				{
					return m_label;
				}

				double GetProgress() const override
				{
					return m_total > 0 ? static_cast<double>(GetCurrent()) / m_total : 0.0;
				}

				bool IsDone() const override
				{
					return m_isDone;
				}
			};
		}
	}	

}
