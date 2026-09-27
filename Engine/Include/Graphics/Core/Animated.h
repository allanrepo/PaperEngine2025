#pragma once
#include <string>
#include <Graphics/Core/IRenderable.h>
#include <Graphics/Core/IAnimated.h>
#include <Graphics/Animation/Animation.h>

namespace engine
{
	namespace graphics
	{
		class Animated : public IRenderable, public IAnimated<std::string>
		{
			using AnimationSet = engine::graphics::animation::AnimationSet<Sprite>;
			using AnimationController = engine::graphics::animation::AnimationController<Sprite>;
			using AnimationSystem = engine::graphics::animation::AnimationSystem<Sprite>;
			using AnimationSystemCache = engine::graphics::animation::AnimationSystemCache<Sprite>;

		private:
			AnimationController m_AnimationController;

		public:
			Animated(const AnimationSet& set, const std::string& name, AnimationSystem* system = nullptr)
				: m_AnimationController(AnimationController::MakeInvalid())
			{
				// if no specific animation system is passed, use the cached one.
				m_AnimationController = system ? system->MakeAnimationController(name, set) : AnimationSystemCache::Instance().MakeAnimationController(name, set);

				Play(name);
			}

			virtual ~Animated() = default;

			Sprite GetSprite() const noexcept override final
			{
				return m_AnimationController.GetCurrent();
			}

			bool Play(const std::string& name) override final
			{
				return m_AnimationController.Play(name);
			}

			// we have to implement to satisfy IAnimated interface but animation is updated by animation system, so we do nothing here.
			void Update(double time) override final
			{
				// do nothing
			}
		};
	}
}