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
				engine::graphics::animation::Animator<engine::graphics::renderable::Sprite> m_animator;
				std::unordered_map<std::string, engine::graphics::animation::Animation<engine::graphics::renderable::Sprite>> m_animations;
				bool m_walkable;

			public:
				AnimatedTile(bool walkable, const std::string& name, const engine::graphics::animation::Animation<engine::graphics::renderable::Sprite>& anim) :
					m_walkable(walkable)
				{
					// copy the animation into our container
					m_animations[name] = anim;

					// assign the animation from our container into animator (don't assign the passed animation. that is reference to animation outside which is not safe
					m_animator.Play(m_animations[name]);
				}

				bool IsRunning() const
				{
					return m_animator.IsRunning();
				}

				const engine::graphics::renderable::Sprite& GetSprite() const
				{
					return m_animator.GetCurrent();
				}

				void Update(double delta)
				{
					m_animator.Update(delta);
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