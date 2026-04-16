#pragma once
#include <Containers/Dictionary.h>
#include <Components/Tile.h>
#include <Engine/Loader/AsyncLoader.h>

namespace engine
{
	namespace manager
	{
		template<typename T>
		class TileMapManager
		{
		public:
			enum class MapState
			{
				NotLoaded,
				Loading,
				Loaded
			};

			using TileLoader = std::function<engine::component::tile1::Tile<T>(const int&)>;

		private:
			engine::container::Dictionary<std::string, std::unique_ptr<engine::component::tile1::TileRegion<T>>> m_regions;
			engine::container::Dictionary<std::string, engine::loader::tile::AsyncCSVMapToTileRegionLoader<T, int>> m_loaders;
			engine::container::Dictionary<std::string, std::unique_ptr<engine::component::tile1::Tileset<T>>> m_tilesets;

			TileLoader m_defaultTileLoader;

			std::string m_activeRegion;

		public:
			TileMapManager(TileLoader tileLoader = nullptr):
				m_defaultTileLoader(tileLoader)
			{
			}

			bool Load(
				const std::string& mapName,
				const std::string& filename,
				engine::job::JobQueue& jobQueue, 
				std::function<engine::component::tile1::Tile<T>(const int&)> tileLoader = nullptr
				)
			{
				// if no valid tile loader, throw
				if (!tileLoader && !m_defaultTileLoader)
				{
					throw std::runtime_error("No valid tile loader.");
				}

				// if loading, we ignore this call. if not loaded, or already loaded, we proceed
				if (GetState(mapName) == MapState::Loading)
				{
					return false;
				}

				// if we currently have this region, we need to unload it gracefully. but wait, we do that in our loader alrady...
				if (!m_regions.Has(mapName))
				{
					// create our tileregion as unique pointer
					std::unique_ptr<engine::component::tile1::TileRegion<T>> region = std::make_unique<engine::component::tile1::TileRegion<T>>();

					// move it to our map
					m_regions[mapName] = std::move(region);
				}

				// Each map gets its own loader instance return 
				if (!m_loaders[mapName].Open(filename, tileLoader, *m_regions[mapName]))
				{
					return false;
				}

				// create job 
				std::unique_ptr<engine::job::Job> job = std::make_unique<engine::job::Job>(
					nullptr,
					[this, mapName]() { m_loaders[mapName].Update(0.001); },
					true,
					[this, mapName]() { return m_loaders[mapName].IsDone(); },
					nullptr
				);

				jobQueue.Submit(std::move(job));

				return true;
			}

			bool LoadImmediate(
				const std::string& mapName,
				const std::string& filename,
				std::function<engine::component::tile1::Tile<T>(const int&)> tileLoader = nullptr
			)
			{
				// if no valid tile loader, throw
				if (!tileLoader && !m_defaultTileLoader)
				{
					throw std::runtime_error("No valid tile loader.");
				}

				// if loading, we ignore this call. if not loaded, or already loaded, we proceed
				if (GetState(mapName) == MapState::Loading)
				{
					return false;
				}

				// if we currently have this region, we need to unload it gracefully. but wait, we do that in our loader alrady...
				if (!m_regions.Has(mapName))
				{
					// create our tileregion as unique pointer
					std::unique_ptr<engine::component::tile1::TileRegion<T>> region = std::make_unique<engine::component::tile1::TileRegion<T>>();

					// move it to our map
					m_regions[mapName] = std::move(region);
				}

				// Each map gets its own loader instance return 
				if (!m_loaders[mapName].LoadImmediate(filename, tileLoader? tileLoader : m_defaultTileLoader, *m_regions[mapName]))
				{
					return false;
				}

				return true;
			}

			MapState GetState(const std::string& mapName) const
			{
				if (!m_regions.Has(mapName) || !m_loaders.Has(mapName))
				{
					return MapState::NotLoaded;
				}
				return m_loaders[mapName].IsDone() ? MapState::Loaded : MapState::Loading;
			}

			double GetProgress(const std::string& mapName) const
			{
				if (!m_regions.Has(mapName) || !m_loaders.Has(mapName))
				{
					return 0.0;
				}
				return m_loaders[mapName].GetProgress();
			}

			bool Unload(
				const std::string& mapName
			)
			{
				// TODO: use clearer to unload map asynchronously. can implement later...
				return true;
			}

			engine::component::tile1::TileMap<T> GetTileMap(const std::string& mapName)
			{
				return m_regions[mapName]->MakeTileMap();
			}
		};
	}
}