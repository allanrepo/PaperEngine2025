#include <Components/Tile.h>
#include <Containers/Dictionary.h>

namespace engine
{
	namespace manager
	{
		template<typename T>
		class TileSetManager
		{
		private:
			engine::container::Dictionary<std::string, std::unique_ptr<engine::component::tile::Tileset<T>>> m_tilesets;

		public:
			bool Create(const std::string& name)
			{
				if (m_tilesets.Has(name))
				{
					return false;
				}
				m_tilesets[name] = std::make_unique<engine::component::tile::Tileset<T>>();

				return true;
			}

			bool Register(const std::string& name, int id, std::unique_ptr<T> tile)
			{
				if (!m_tilesets.Has(name))
				{
					throw std::runtime_error("Tileset not found");
				}
				return m_tilesets[name]->Register(id, std::move(tile));
			}

			// creates a tile instance for the given id. returns invalid tile if id not found
			engine::component::tile::Tile<T> MakeTile(const std::string& name, int id) const
			{
				if (!m_tilesets.Has(name))
				{
					throw std::runtime_error("Tileset not found");
				}
				return m_tilesets[name]->MakeTile(id);
			}
		};
	}
}