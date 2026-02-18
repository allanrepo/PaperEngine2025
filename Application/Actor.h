#pragma once

#include <State/StateMachine.h>
#include <Spatial/Motion.h>
#include <Spatial/Transform.h>
#include <Graphics/Renderable/Sprite.h>
#include <Graphics/Animation/Animation.h>
#include <Containers/Dictionary.h>
#include <Math/Vector.h>
#include <Cache/Registry.h>
#include <Spatial/Position.h>
#include <string>

using namespace std;
using namespace engine::spatial;
using namespace engine::graphics::animation;
using namespace engine::graphics::renderable;
using namespace engine::container;
using namespace engine::state;
using namespace engine::math;
using namespace engine::component;
using namespace engine::cache;

namespace engine
{
	namespace component
	{
		class Actor;
	}

	namespace state
	{
		class ActorIdleState;
		class ActorWalkToState;
	}
}

namespace engine
{
	namespace component
	{
		class Actor
		{
		public:
			enum class Direction
			{
				None,
				Up,
				Down,
				Left,
				Right
			};

		private:
			string m_name;
			TransformF m_transform;
			MotionF m_motion;
			Animator<Sprite> m_animator;
			StateMachine<Actor> m_stateMachine;
			Direction m_direction;
			AnimationManager<Sprite> m_animManager;

		public:

			Actor(const AnimationManager<Sprite>& animManager, const std::string& name = "default") :
				m_stateMachine(this),
				m_name(name),
				m_direction(Direction::Right),
				m_animManager(animManager)
			{
				m_animManager.Set(m_animator);
				m_stateMachine.Set<ActorIdleState>();
			}

			virtual ~Actor() = default;

			void Update(double delta)
			{
				// update state and animation manager, position
				m_stateMachine.Update(delta);
				m_animManager.Update(delta);
				m_motion.Update(m_transform, delta);
			}

			bool PlayAnimation(const std::string& name)
			{
				return m_animManager.Play(name);
			}

			bool IsDrawable() const
			{
				return m_animator.IsRunning();
			}

			Sprite GetSprite() const
			{
				return m_animator.GetCurrent();
			}

			void SetDirectionBasedOnVelocity() 
			{ 
				if (m_motion.GetVelocity().x > 0) m_direction = Direction::Right;
				else if (m_motion.GetVelocity().x < 0) m_direction = Direction::Left;
				else m_direction = Direction::None;
			} 
			
			Direction GetDirection() const 
			{ 
				if (m_motion.GetVelocity().x > 0) return Direction::Right;
				else if (m_motion.GetVelocity().x < 0) return Direction::Left;
				else return Direction::None;
			}

			PositionF GetPosition() const
			{
				return m_transform.GetPosition();
			}

			void SetVelocity(const VecF& velocity)
			{
				m_motion.SetVelocity(velocity);
			}

			VecF GetVelocity() const
			{
				return m_motion.GetVelocity();
			}

			void SetPosition(const PositionF& pos)
			{
				m_transform.SetPosition(pos);
			}

			void StopMoving()
			{
				m_motion.Stop();
			}

			bool IsMovingTowards(const PositionF& target)
			{
				math::VecF direction = (target - m_transform.GetPosition()).Normalize();

				return direction.Dot(m_motion.GetVelocity()) > 0;
			}


			template<typename State, typename... Args>
			void SetState(Args&&... args) 
			{
				m_stateMachine.Set<State>(std::forward<Args>(args)...);
			}

			template<typename State, typename... Args>
			void QueueState(Args&&... args) 
			{
				m_stateMachine.Queue<State>(std::forward<Args>(args)...);
			}
		};
	}

	namespace state
	{
		class ActorIdleState : public engine::state::State<engine::component::Actor>
		{
		private:
			std::string m_name = "idle";
			Actor::Direction m_dir;

		public:
			ActorIdleState(Actor::Direction dir = Actor::Direction::Right):
				m_dir(dir)
			{

			}

			virtual ~ActorIdleState() = default;

			virtual void Enter(Actor& owner) override final
			{
				switch (m_dir)
				{
				case Actor::Direction::Left:
					owner.PlayAnimation("idle left");
					break;
				default:
					owner.PlayAnimation("idle right");
					break;
				}
			}

			virtual void Exit(Actor& owner) override final
			{
			}

			virtual void Update(Actor& owner, double delta) override final
			{
			}

			virtual bool IsFinished(Actor& owner) override final
			{
				return false;
			}
		};

		class ActorWalkToState : public engine::state::State<engine::component::Actor>
		{
		private:
			std::string m_name = "walk";
			float m_speed = 0.0f;
			PositionF m_target;
			bool m_isFinished;
			Actor::Direction m_dir;

		public:
			ActorWalkToState(const PositionF& target, const float speed) :
				m_target(target),
				m_speed(speed),
				m_isFinished(false),
				m_dir(Actor::Direction::None)
			{
			}

			virtual ~ActorWalkToState() = default;

			virtual void Enter(Actor& owner) override final
			{
				// calculate and set velocity
				math::VecF direction = (m_target - owner.GetPosition()).Normalize();
				owner.SetVelocity(direction * m_speed);

				// save direction
				m_dir = owner.GetDirection();

				// set animation
				switch (m_dir)
				{
				case Actor::Direction::Left:
					owner.PlayAnimation("walk left");
					break;
				case Actor::Direction::Right:
					owner.PlayAnimation("walk right");
					break;
				default:
					owner.PlayAnimation("idle right");
					break;
				}

				m_isFinished = false;
			}

			virtual void Exit(Actor& owner) override final
			{
				owner.StopMoving();
			}

			virtual void Update(Actor& owner, double delta) override final
			{
				if (!owner.IsMovingTowards(m_target))
				{
					owner.SetPosition(m_target);
					m_isFinished = true;
					owner.QueueState<ActorIdleState>(m_dir);
				}
			}

			virtual bool IsFinished(Actor& owner) override final
			{
				return m_isFinished;
			}
		};

	}
}




