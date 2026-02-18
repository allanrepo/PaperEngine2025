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

		public:

			Actor(const std::string& name = "default") :
				m_stateMachine(this),
				m_name(name),
				m_direction(Direction::Right)
			{
				m_stateMachine.Set<ActorIdleState>();
			}

			virtual ~Actor() = default;

			// update
			void Update(double delta)
			{
				m_stateMachine.Update(delta);
				m_animator.Update(delta);

				m_motion.Update(m_transform, delta);
			}

			bool PlayAnimation(const std::string& name)
			{
				if (!m_animator.Has(name))
				{
					if (!Registry<Animation<Sprite>>::Instance().Has(name))
					{
						return false;
					}
					m_animator.Add(name, Registry<Animation<Sprite>>::Instance().Get(name));
				}
				return m_animator.Play(name);
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
				return m_direction;
				//if (m_motion.GetVelocity().x > 0) return Direction::Right;
				//else if (m_motion.GetVelocity().x < 0) return Direction::Left;
				//else return Direction::None;
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

			void Idle()
			{
				m_stateMachine.Set<ActorIdleState>();
			}

			void WalkTo(const PositionF& target, const float speed)
			{
				m_stateMachine.Set<ActorWalkToState>(target, speed);
				m_stateMachine.Queue<ActorIdleState>();
			}
		};
	}

	namespace state
	{
		class ActorIdleState : public engine::state::State<engine::component::Actor>
		{
		private:
			std::string m_name = "idle";

		public:
			ActorIdleState()
			{
			};
			virtual ~ActorIdleState() = default;

			virtual void Enter(Actor& owner) override final
			{
				switch (owner.GetDirection())
				{
				case Actor::Direction::Left:
					owner.PlayAnimation("hero idle left");
					break;
				default:
					owner.PlayAnimation("hero idle right");
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

		public:
			ActorWalkToState(const PositionF& target, const float speed) :
				m_target(target),
				m_speed(speed),
				m_isFinished(false)
			{
			}

			virtual ~ActorWalkToState() = default;

			virtual void Enter(Actor& owner) override final
			{
				// calculate and set velocity
				math::VecF direction = m_target - owner.GetPosition();
				direction = direction.Normalize();
				owner.SetVelocity(direction * m_speed);

				// update direction
				owner.SetDirectionBasedOnVelocity();

				// set animation
				switch (owner.GetDirection())
				{
				case Actor::Direction::Left:
					owner.PlayAnimation("hero walk left");
					break;
				case Actor::Direction::Right:
					owner.PlayAnimation("hero walk right");
					break;
				default:
					owner.PlayAnimation("hero idle right");
					break;
				}

				m_isFinished = false;
			}

			virtual void Exit(Actor& owner) override final
			{
				owner.StopMoving();
				//owner.SetPosition(m_target);
			}

			virtual void Update(Actor& owner, double delta) override final
			{
				math::VecF direction = m_target - owner.GetPosition();
				direction = direction.Normalize();

				float dot = direction.Dot(owner.GetVelocity());
				if (dot < 0)
				{
					m_isFinished = true;
				}
			}

			virtual bool IsFinished(Actor& owner) override final
			{
				return m_isFinished;
			}
		};

	}
}




