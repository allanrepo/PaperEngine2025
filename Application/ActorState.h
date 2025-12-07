#pragma once
#include "State.h"
#include "IActor.h"
#include <functional>
#include "Math.h"

namespace state
{

	class ActorAttackState : public state::State<component::IActor>
	{
	private:
		std::string m_name = "idle";
		math::VecF lastFaceDirection;
		bool first = true;
		std::string currAnim;

	public:
		ActorAttackState() = default;
		virtual ~ActorAttackState() = default;

		virtual void Enter(component::IActor& owner) override final
		{
			currAnim = owner.GetFaceDirection().x > 0 ? "attack_right" : "attack_left";
			owner.SetAnimation(currAnim);

			owner.GetAnimator().GetCurrentAnimation()->loop = false;
		}

		virtual void Exit(component::IActor& owner) override final
		{
		}

		virtual void Update(component::IActor& owner, float delta) override final
		{
		}

		virtual bool IsFinished(component::IActor& owner) override final
		{
			return owner.GetAnimator().IsFinished();
		}
	};

	class ActorWalkToState : public state::State<component::IActor>
	{
	private:
		std::string m_name = "walk";
		std::string m_nextState;
		float m_speed = 0.0f;
		spatial::PosF m_target;
		bool m_isFinished = false;

	public:
		ActorWalkToState(const spatial::PosF target, const float speed) :
			m_target(target),
			m_speed(speed)
		{
		}

		virtual ~ActorWalkToState() = default;

		virtual void Enter(component::IActor& owner) override final
		{
			// calculate and set velocity
			math::VecF direction = m_target - owner.GetTransform().GetPosition();
			direction = direction.Normalize();
			owner.GetMotion().SetVelocity(direction * m_speed);

			// update owner to set its face direction
			owner.SetFaceTo(m_target);

			// check direction and decide what animation we'll use
			owner.SetAnimation(owner.GetFaceDirection().x > 0 ? "walk_right" : "walk_left");

			m_isFinished = false;
		}

		virtual void Exit(component::IActor& owner) override final
		{
			owner.GetMotion().Stop();
		}

		virtual void Update(component::IActor& owner, float delta) override final
		{
			owner.GetMotion().Update(owner.GetTransform(), delta);

			math::VecF direction = m_target - owner.GetTransform().GetPosition();
			direction = direction.Normalize();

			float dot = direction.Dot(owner.GetMotion().GetVelocity());
			if (dot < 0)
			{
				owner.GetMotion().Stop();
				m_isFinished = true;
			}
		}

		virtual bool IsFinished(component::IActor& owner) override final
		{
			return m_isFinished;
		}
	};

	// it can loop

	class ActorIdleState : public state::State<component::IActor>
	{
	private:
		std::string m_name = "idle";
		math::VecF lastFaceDirection;
		bool first = true;
		std::string currAnim;

	public:
		ActorIdleState() = default;
		virtual ~ActorIdleState() = default;

		virtual void Enter(component::IActor& owner) override final
		{
			currAnim = owner.GetFaceDirection().x > 0 ? "idle_right" : "idle_left";
			owner.SetAnimation(currAnim);
		}

		virtual void Exit(component::IActor& owner) override final
		{
		}

		virtual void Update(component::IActor& owner, float delta) override final
		{
			std::string newAnim = owner.GetFaceDirection().x > 0 ? "idle_right" : "idle_left";
			if (currAnim != newAnim)
			{
				currAnim = newAnim;
				owner.SetAnimation(currAnim);
			}
		}

		virtual bool IsFinished(component::IActor& owner) override final
		{
			return false;
		}
	};

	class ActorPatrolIdleState : public state::State<component::IActor>
	{
	private:
		std::string m_name = "patrol_idle";
		float m_duration = 0.0f;
		spatial::PosF m_position;
		bool m_isFinished = false;

		std::string currAnim;

	public:
		ActorPatrolIdleState(spatial::PosF position) :
			m_position(position),
			m_isFinished(false)
		{

		}

		virtual ~ActorPatrolIdleState() = default;

		virtual void Enter(component::IActor& owner) override final
		{
			// set how long it takes for this state to run
			m_duration = math::Random(2000.0f, 3000.0f);

			// set how long it takes to change face direction. 


			currAnim = owner.GetFaceDirection().x > 0 ? "idle_right" : "idle_left";
			owner.SetAnimation(currAnim);
		}

		virtual void Exit(component::IActor& owner) override final
		{
		}

		virtual void Update(component::IActor& owner, float delta) override final
		{
			// if state is already finished, and update is still called, let's bail out.
			if (m_isFinished)
			{
				return;
			}

			// count down idle state
			m_duration -= delta;

			// if we still have time to be idle
			if (m_duration > 0)
			{

			}
			// if no more time to be idle
			else
			{
				// generate random position to target walking. must be within the radius
				math::VecF direction{ math::Random(-1.0f, 1.0f), math::Random(-1.0f, 1.0f) };
				direction = direction.Normalize(); // normalize
				direction *= math::Random(100.0f, 150.0f); // radius
				spatial::PosF target = m_position + direction;

				// queue walkto state
				owner.QueueState(std::make_unique<state::ActorWalkToState>(std::move(target), 0.2f));

				// queue patrolidle state and pass original position 
				owner.QueueState(std::make_unique<state::ActorPatrolIdleState>(std::move(m_position)));

				// set to finish
				m_isFinished = true;
			}
		}

		virtual bool IsFinished(component::IActor& owner) override final
		{
			return m_isFinished;
		}
	};

	class ActorPursueTargetState : public state::State<component::IActor>
	{
	private:
		component::IActor* m_target;
		bool m_isFinished;
		bool m_targetIsTargetable;
		float m_duration;
		float m_elapsed;
		float m_speed;
		bool m_first;
		std::function<bool(component::IActor&, component::IActor&)> m_endCondition;

	public:
		ActorPursueTargetState(
			component::IActor* target,	// object to pursue
			float duration,				// amount of time to pursue before it refresh the target's position again
			const float speed,			// how fast the actor owner's is in pursuing
			std::function<bool(component::IActor&, component::IActor&)> endCondition = nullptr
		) :
			m_target(target),
			m_isFinished(false),
			m_targetIsTargetable(true),
			m_duration(duration),
			m_speed(speed),
			m_elapsed(0.0f),
			m_first(true),
			m_endCondition(endCondition)
		{
			// set default end condition handler if nothing is specified

		}

		void HandleTargetableChange(component::IActor* sender, bool enable)
		{
			if (!enable)
			{
				m_targetIsTargetable = false;
			}
		}

		virtual void Enter(component::IActor& owner) override final
		{
			// listen to target's targetable state event so we know if we can't target it anymore
			m_target->OnTargetableChanged += event::Handler(this, &ActorPursueTargetState::HandleTargetableChange);

			// set animation
			std::string currAnim = owner.GetFaceDirection().x > 0 ? "walk_right" : "walk_left";
			owner.SetAnimation(currAnim);
			owner.GetAnimator().GetCurrentAnimation()->loop = true;
		}

		virtual void Exit(component::IActor& owner) override final
		{
			// stop listening to targetable state change event
			m_target->OnTargetableChanged -= event::Handler(this, &ActorPursueTargetState::HandleTargetableChange);

			// stop moving
			owner.GetMotion().Stop();
		}

		virtual void Update(component::IActor& owner, float delta) override final
		{
			// if state is already finished, and update is still called, let's bail out.
			if (m_isFinished)
			{
				return;
			}

			// if target cannot be target anymore, this state is finished. let's bail out
			if (!m_targetIsTargetable)
			{
				// this state is finished now
				m_isFinished = true;

				// bail out now
				return;
			}

			// accumulate elapsed time
			m_elapsed += delta;

			// if accumulated elapsed time, let's refresh target to walk to 
			if (m_elapsed > m_duration || m_first)
			{
				// no need to keep left over. 
				m_elapsed = 0;

				// get position of target
				spatial::PosF target = m_target->GetTransform().GetPosition();

				// calculate and set velocity
				math::VecF direction = target - owner.GetTransform().GetPosition();
				direction = direction.Normalize();
				owner.GetMotion().SetVelocity(direction * m_speed);

				// if new direction changes compared to current direction, update face to
				{
					if (
						(owner.GetFaceDirection().x > 0 && direction.x <= 0) ||
						(owner.GetFaceDirection().x < 0 && direction.x >= 0) ||
						m_first
						)
					{
						// update owner to set its face direction
						owner.SetFaceTo(target);

						// check direction and decide what animation we'll use
						owner.SetAnimation(owner.GetFaceDirection().x > 0 ? "walk_right" : "walk_left");
						owner.GetAnimator().GetCurrentAnimation()->loop = true;
					}
				}

				m_first = false;
			}

			// update our position
			owner.GetMotion().Update(owner.GetTransform(), delta);

			// check if target is reached
			{
				if (m_endCondition)
				{
					if (m_endCondition(owner, *m_target))
					{
						owner.GetMotion().Stop();
						m_isFinished = true;
					}
				}
				// use default end condition handler: collision overlap
				else
				{
					if (owner.GetCollisionBox().Overlaps(m_target->GetCollisionBox()))
					{
						owner.GetMotion().Stop();
						m_isFinished = true;
					}
				}
			}
		}

		virtual bool IsFinished(component::IActor& owner) override final
		{
			return m_isFinished;
		}
	};

	// attackable actor to target
	// state needs an active attackable actor to target
	// state needs to subscribe to this actor's attackable state change event
	// state needs to determine the best position to be to attack the target. there are 2 positions - left and right
	// state needs to know the interval at which it will check if the target is still in the same position. if not, it needs to recalculate the position it needs to go before attacking
	class ActorAttackTargetState : public state::State<component::IActor>
	{
	private:
		std::string m_name = "idle";
		math::VecF lastFaceDirection;
		bool first = true;
		std::string currAnim;

		component::IActor* m_target;
		bool m_isFinished = false;
		bool m_targetIsTargetable = false;



	public:
		ActorAttackTargetState(component::IActor* target) :
			m_target(target),
			m_isFinished(false),
			m_targetIsTargetable(true)
		{
		}

		virtual ~ActorAttackTargetState() = default;


		void TargetableChange(component::IActor* sender, bool enable)
		{
			if (!enable)
			{
				m_targetIsTargetable = false;
			}
		}

		virtual void Enter(component::IActor& owner) override final
		{
			// listen to target's targetable state event so we know if we can't target it anymore
			m_target->OnTargetableChanged += event::Handler(this, &ActorAttackTargetState::TargetableChange);

			// set animation
			currAnim = owner.GetFaceDirection().x > 0 ? "attack_right" : "attack_left";
			owner.SetAnimation(currAnim);
			owner.GetAnimator().GetCurrentAnimation()->loop = false;
		}

		virtual void Exit(component::IActor& owner) override final
		{
			// stop listening to targetable state change event
			m_target->OnTargetableChanged -= event::Handler(this, &ActorAttackTargetState::TargetableChange);
		}

		virtual void Update(component::IActor& owner, float delta) override final
		{
			// if state is already finished, and update is still called, let's bail out.
			if (m_isFinished)
			{
				return;
			}

			if (!m_targetIsTargetable)
			{
				// this state is finished now
				m_isFinished = true;

				// queue idle state 
				owner.QueueState(std::make_unique<state::ActorIdleState>());

				// bail out now
				return;
			}
		}

		virtual bool IsFinished(component::IActor& owner) override final
		{
			return m_isFinished;
		}
	};

}
