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
			using AnimationController = engine::graphics::animation::AnimationController<Sprite, Animated>;
			using AnimationSet = engine::graphics::animation::AnimationSet<Sprite>;

		private:
			AnimationController m_animationController;

		public:
			Animated(AnimationSet& set, const std::string& name) :
				m_animationController(set, nullptr)
			{
				// safer to set owner here only so we know Animated is now fully constructed
				m_animationController.SetOwner(this);
				Play(name);
			}

			virtual ~Animated() = default;

			Sprite GetSprite() const noexcept override final
			{
				return m_animationController.GetCurrent();
			}

			bool Play(const std::string& name) override final
			{
				return m_animationController.Play(name);
			}

			void Update(double time) override final
			{
				m_animationController.Update(time);
			}
		};
	}
}