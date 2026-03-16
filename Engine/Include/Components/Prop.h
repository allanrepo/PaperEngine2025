#pragma once
#include <Graphics/Core/Sprite.h>
#include <Graphics/Animation/Animation.h>
#include <Algorithm/Pathfinding.h>
#include <string>

namespace engine
{
	namespace component
	{
		namespace graphics
		{
			class IProp;

			using AnimationSet = engine::graphics::animation::AnimationSet<engine::graphics::Sprite>;
			using AnimationController = engine::graphics::animation::AnimationController<engine::graphics::Sprite, IProp>;
			using AnimationFactory = engine::graphics::factory::AnimationFactory;

			// Prop is a visual object, therefore it is tightly coupled with a IRenderable - Sprite
			// Prop being a visual object, can be animated. Update() and Play() methods provide support for animation 
			class IProp
			{
			public:
				virtual void Update(double delta) = 0;
				virtual void Play(const std::string& key) = 0;
				virtual bool IsValid() const = 0;
				virtual const engine::graphics::Sprite GetSprite() const = 0;

				virtual ~IProp() = default;
			};

			class AnimatedProp : public IProp
			{
			private:
				AnimationController m_animationController;

			public:
				AnimatedProp(AnimationSet* set) :
					m_animationController(*set)
				{
				}

				void Update(double delta) override
				{
					m_animationController.Update(delta);
				}

				void Play(const std::string& key) override
				{
					m_animationController.Play(key);
				}

				const engine::graphics::Sprite GetSprite() const override
				{
					return m_animationController.GetCurrent();
				}

				bool IsValid() const override
				{
					return m_animationController.IsValid();
				}
			};

			class SimpleProp : public IProp
			{
			private:
				engine::graphics::Sprite m_sprite;

			public:
				SimpleProp(const engine::graphics::Sprite& sprite) :
					m_sprite(sprite)
				{
				}

				void Update(double delta) override
				{
				}

				void Play(const std::string& key) override
				{
				}

				const engine::graphics::Sprite GetSprite() const override
				{
					return m_sprite;
				}

				bool IsValid() const override
				{
					return m_sprite.IsValid();
				}
			};

			class PropHandle
			{
			private:
				friend class PropSet;
				friend class PropTile;
				core::Handle<IProp> m_handle;

			protected:
				// use this constructor if you have the sprite atlas and the source rect
				PropHandle(IProp* prop) :
					m_handle(prop)
				{
				}

				PropHandle() :
					m_handle(nullptr)
				{
				}

			public:
				~PropHandle() = default;

				inline bool IsValid() const
				{
					return m_handle.IsValid();
				}

				inline const engine::graphics::Sprite GetSprite() const
				{
					return m_handle->GetSprite();
				}

				void Play(const std::string& key)
				{
					m_handle->Play(key);
				}
			};

			class PropSet
			{
			protected:
				container::Dictionary<int, std::unique_ptr<IProp>> m_registry;

			public:
				PropSet() = default;
				~PropSet() = default;

				bool Register(int id, std::unique_ptr<IProp> data)
				{
					return m_registry.Register(id, std::move(data));
				}

				bool IsValid(int id) const
				{
					return m_registry.Has(id);
				}

				// creates a prop instance for the given id. returns invalid prop if id not found
				PropHandle MakePropHandle(int id) const
				{
					return m_registry.Has(id) ? PropHandle(m_registry.Get(id).get()) : PropHandle(nullptr);
				}

				void Update(double delta)
				{
					for (auto& [id, prop] : m_registry)
					{
						prop->Update(delta);
					}
				}

				// define iterator for our container
				using iterator = typename container::Dictionary<int, std::unique_ptr<IProp>>::iterator;
				using const_iterator = typename container::Dictionary<int, std::unique_ptr<IProp>>::const_iterator;

				// iterator access
				iterator begin() { return m_registry.begin(); }
				iterator end() { return m_registry.end(); }
				const_iterator begin() const { return m_registry.begin(); }
				const_iterator end() const { return m_registry.end(); }
				const_iterator cbegin() const { return m_registry.cbegin(); }
				const_iterator cend() const { return m_registry.cend(); }
			};

			// TODO: this should be inner class of PropMap
			class PropTile
			{
			private:
				friend class PropMap;
				engine::container::Dictionary<engine::navigation::tile::TileConstraint, PropHandle> m_props;

			public:
				PropTile()
				{
				}

				void Set(engine::navigation::tile::TileConstraint constraint, const PropHandle& prop)
				{
					if (m_props.Has(constraint))
					{
						m_props.Unregister(constraint);
					}
					m_props.Register(constraint, prop);
				}

				void Remove(engine::navigation::tile::TileConstraint constraint)
				{
					m_props.Unregister(constraint);
				}

				bool Has(engine::navigation::tile::TileConstraint constraint) const
				{
					return m_props.Has(constraint);
				}

				PropHandle Get(engine::navigation::tile::TileConstraint constraint) const
				{
					// unsafe. caller should check Has() before calling this. 
					return m_props[constraint];
				}

				// clears all props from this tile
				void Clear()
				{
					m_props.Clear();
				}
			};

		}
	}
}