#pragma once

#include <Engine/Manager/TileSetManager.h>
#include <Graphics/Animation/Animation.h>

namespace engine
{
	namespace component
	{
		namespace tile
		{
			// this is a tile definition class, not exactly tile class. tile class is Tile<T> and this is what is assigned to T
			class AnimatedTile
			{
			private:
				graphics::animation::Animator<graphics::renderable::Sprite> m_animator;
				bool m_walkable;

			public:
				AnimatedTile(bool walkable, const std::string& name, const graphics::animation::Animation<graphics::renderable::Sprite>& anim) :
					m_walkable(walkable)
				{
					m_animator.Add(name, anim);
					m_animator.Play(name);
				}

				bool IsRunning() const
				{
					return m_animator.IsRunning();
				}

				const graphics::renderable::Sprite& GetSprite() const
				{
					return m_animator.GetCurrent();
				}

				void Update(double delta)
				{
					m_animator.Update(delta);
				}

				void Add(const std::string& name, const graphics::animation::Animation<graphics::renderable::Sprite>& anim, bool play = false)
				{
					m_animator.Add(name, anim);
					if (play) m_animator.Play(name);
				}

				void Play(const std::string& name)
				{
					m_animator.Play(name);
				}
			};

			class AnimatedTileset : public engine::component::tile::Tileset<AnimatedTile>
			{
			public:
				void Update(double delta)
				{
					for (auto& [id, tileDef] : m_registry)
					{
						tileDef->Update(delta);
					}
				}
			};
		}
	}
	namespace manager
	{
		using AnimatedTile = engine::component::tile::AnimatedTile;

		class AnimatedTileSetManager : public engine::manager::TileSetManager<AnimatedTile>
		{
		public:
			// Update all animators in all tilesets
			void Update(double delta)
			{
				for (std::pair<const std::string, std::unique_ptr<engine::component::tile::Tileset<AnimatedTile>>>& tileset : m_tilesets)
				{
					engine::component::tile::Tileset<AnimatedTile>& ts = *tileset.second;

					for (std::pair<const int, std::unique_ptr<AnimatedTile>>& tile : ts)
					{
						tile.second->Update(delta);
					}
				}
			}
		};
	}
}