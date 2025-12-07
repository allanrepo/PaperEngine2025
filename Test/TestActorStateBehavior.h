#pragma once
#include "AnimationFactory.h"
#include "Engine.h"
#include "Sprite.h"
#include "SpriteFactory.h"
#include "IDrawableSurface.h"
#include <typeindex>
#include "Animation.h"
#include "Input.h"
#include <queue>
#include "Factory.h"
//#include "Actor.h"
#include "ActorFactory.h"
#include "Logger.h"
#include "IFontAtlas.h"
#include "TextureFactory.h"
#include "Math.h"

namespace debug
{
	template<typename T, typename... Args>
	class StateFactory
	{
	public:
		static std::unique_ptr<T> Create(Args&&... args)
		{
			return std::make_unique<T>(std::forward<Args>(args)...);
		}
	};


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
				owner.QueueState(debug::StateFactory<debug::ActorWalkToState, spatial::PosF, float>::Create(std::move(target), 0.2f));

				// queue patrolidle state and pass original position 
				owner.QueueState(debug::StateFactory<debug::ActorPatrolIdleState, spatial::PosF>::Create(std::move(m_position)));

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
				owner.QueueState(debug::StateFactory<debug::ActorIdleState>::Create());

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

void CalcUV(int row, int col, int fileWidth, int fileHeight)
{
	float width = static_cast<float>(fileWidth / col);
	float height = static_cast<float>(fileHeight / row);
	float left = 0;
	float top = 0;
	float right = left + width;
	float bottom = top + height;

	for (int r = 0; r < row; r++)
	{
		for (int c = 0; c < col; c++)
		{
			left = width * c;
			top = height * r;
			right = left + width;
			bottom = top + height;

			left /= fileWidth;
			top /= fileHeight;
			right /= fileWidth;
			bottom /= fileHeight;

			LOG(std::to_string(left) << ", " << std::to_string(top) << ", " << std::to_string(right) << ", " << std::to_string(bottom));
		}
	}
}

class TestActorStateBehavior
{
private:
	engine::Engine m_engine;
	std::unordered_map<std::string, std::unique_ptr<component::IActor>> m_actors;
	std::unordered_map<std::string, spatial::PosF> m_positions;
	spatial::PosF m_mousePosition;

public:
	TestActorStateBehavior() :
		m_engine("DirectX11", "Batch")
	{
		m_engine.OnStart += event::Handler(this, &TestActorStateBehavior::OnStart);
		m_engine.OnUpdate += event::Handler(this, &TestActorStateBehavior::OnUpdate);
		m_engine.OnRender += event::Handler(this, &TestActorStateBehavior::OnRender);
		m_engine.OnResize += event::Handler(this, &TestActorStateBehavior::OnResize);

		input::Input::Instance().OnKeyDown += event::Handler(this, &TestActorStateBehavior::OnKeyDown);
		input::Input::Instance().OnMouseDown += event::Handler(this, &TestActorStateBehavior::OnMouseDown);
		input::Input::Instance().OnMouseMove += event::Handler(this, &TestActorStateBehavior::OnMouseMove);

		m_engine.Run();
	}

	void OnResize(size_t width, size_t height)
	{
	}

	void OnUpdate(float delta)
	{
		// update all actors
		for (const auto& kv : m_actors)
		{
			kv.second->Update(delta);
		}
	}

	void OnMouseMove(int x, int y)
	{
		m_mousePosition = spatial::PosF{ static_cast<float>(x), static_cast<float>(y) };

		// set hero character's face direction based on where mouse position is
		m_actors["hero"]->SetFaceTo(m_mousePosition);

	}

	void OnMouseDown(int btn, int x, int y)
	{
		if (btn == 1)
		{
			bool isContact = false;//  m_actors["roaming enemy"]->Contains(spatial::PosF{ static_cast<float>(x), static_cast<float>(y) });
			if (isContact)
			{
				component::IActor* enemy = m_actors["roaming enemy"].get();
				std::unique_ptr<state::State<component::IActor>> state = std::make_unique<debug::ActorAttackTargetState>(enemy);
				m_actors["hero"]->SetState(std::move(state));
				m_actors["hero"]->SetTargetable(true);
			}
			else
			{
				m_actors["hero"]->SetState(debug::StateFactory<debug::ActorWalkToState, spatial::PosF, float>::Create(spatial::PosF{ static_cast<float>(x), static_cast<float>(y) }, 0.2f));
				m_actors["hero"]->QueueState(debug::StateFactory<debug::ActorIdleState>::Create());
			}
		}
		else if (btn == 2)
		{
			// are we targeting an enemy?
			if (m_actors["roaming enemy"]->Contains(spatial::PosF{ static_cast<float>(x), static_cast<float>(y) }))
			{
				// get distance of hero against target enemy
				float distance = (m_actors["roaming enemy"]->GetTransform().GetPosition() - m_actors["hero"]->GetTransform().GetPosition()).Magnitude();

				// get distance between hero and target enemy as if they are overlapping within half their radius (in this case since they are rectangles, their width)
				float contactDistance = ((m_actors["roaming enemy"]->GetCollisionBox().GetWidth() + m_actors["hero"]->GetCollisionBox().GetWidth()) / 2.0f) / 2.0f;

				// are we close enough to attack already?
				if (distance < contactDistance)
				{
					m_actors["roaming enemy"]->SetTargetable(true);
					m_actors["hero"]->GetStateMachine().Flush();
					std::unique_ptr<state::State<component::IActor>> state = std::make_unique<debug::ActorAttackTargetState>(m_actors["roaming enemy"].get());
					m_actors["hero"]->GetStateMachine().Set(std::move(state));
				}
				// if we are too far to attack
				else
				{
					m_actors["hero"]->GetStateMachine().Flush();

					// set state to pursue target
					std::unique_ptr<state::State<component::IActor>> state = std::make_unique<debug::ActorPursueTargetState>(
						m_actors["roaming enemy"].get(),
						1000.0f,
						0.2f,
						[](component::IActor& self, component::IActor& target)
						{
							math::geometry::RectF selfCB = self.GetCollisionBox();
							math::geometry::RectF targetCB = target.GetCollisionBox();
							float radius = (selfCB.GetWidth() + targetCB.GetWidth()) / 4.0f;

							float distance = (self.GetTransform().GetPosition() - target.GetTransform().GetPosition()).Magnitude();

							return distance < radius;
						}
					);
					m_actors["hero"]->GetStateMachine().Set(std::move(state));

					// then queue attack
					m_actors["hero"]->QueueState(debug::StateFactory<debug::ActorAttackState>::Create());
					m_actors["hero"]->QueueState(debug::StateFactory<debug::ActorIdleState>::Create());
				}
			}
		}
	}

	void OnKeyDown(int key)
	{
		LOG("OnKeyDown Key: " << key);


		// get distance of enemy against hero
		float distance = (m_actors["roaming enemy"]->GetTransform().GetPosition() - m_actors["hero"]->GetTransform().GetPosition()).Magnitude();

		// get distance between enemy and hero as if they are overlapping within half their radius (in this case since they are rectangles, their width)
		float contactDistance = ((m_actors["roaming enemy"]->GetCollisionBox().GetWidth() + m_actors["hero"]->GetCollisionBox().GetWidth()) / 2.0f) / 2.0f;

		// are we close enough to attack already?
		if (distance < contactDistance)
		{
			m_actors["hero"]->SetTargetable(true);
			m_actors["roaming enemy"]->GetStateMachine().Flush();
			std::unique_ptr<state::State<component::IActor>> state = std::make_unique<debug::ActorAttackTargetState>(m_actors["hero"].get());
			m_actors["roaming enemy"]->GetStateMachine().Set(std::move(state));
		}
		// if we are too far to attack
		else
		{
			m_actors["roaming enemy"]->GetStateMachine().Flush();

			// set state to pursue target
			std::unique_ptr<state::State<component::IActor>> state = std::make_unique<debug::ActorPursueTargetState>(
				m_actors["hero"].get(),
				1000.0f,
				0.1f,
				[](component::IActor& self, component::IActor& target)
				{
					math::geometry::RectF selfCB = self.GetCollisionBox();
					math::geometry::RectF targetCB = target.GetCollisionBox();
					float radius = (selfCB.GetWidth() + targetCB.GetWidth()) / 4.0f;

					float distance = (self.GetTransform().GetPosition() - target.GetTransform().GetPosition()).Magnitude();

					return distance < radius;
				}
			);
			m_actors["roaming enemy"]->GetStateMachine().Set(std::move(state));

			// then queue attack
			m_actors["roaming enemy"]->QueueState(debug::StateFactory<debug::ActorAttackState>::Create());
			m_actors["roaming enemy"]->QueueState(std::make_unique<debug::ActorPatrolIdleState>(m_actors["roaming enemy"]->GetTransform().GetPosition()));
		}

	}


	void OnStart()
	{
		//CalcUV(8, 12, 2304, 1536);

		// create actor for hero character. set default state and position
		{
			m_actors["hero"] = component::factory::ActorFactory::Create("CharacterTestStates.csv");
			m_actors["hero"]->SetState(debug::StateFactory<debug::ActorIdleState>::Create());
			m_actors["hero"]->GetTransform().SetPosition(spatial::PosF{ 150, 150 });
		}

		// create actor for roaming enemy
		{
			m_actors["roaming enemy"] = component::factory::ActorFactory::Create("CharacterTestStates.csv");

			// set position
			m_actors["roaming enemy"]->GetTransform().SetPosition(spatial::PosF{ 600, 300 });

			// set state where it randomly roams at a given position
			m_actors["roaming enemy"]->SetState(std::make_unique<debug::ActorPatrolIdleState>(m_actors["roaming enemy"]->GetTransform().GetPosition()));
			//m_actors["roaming enemy"]->SetState(std::make_unique<debug::ActorIdleState>());
		}
	}

	void OnRender()
	{
		// for debugging purposes, we draw the bounding regions of an enemy actor
		{
			// draw the rectangle area of the sprite used for this actor. this is to see the footprint of the sprite. uses sprite's render offset to draw aligned 
			math::VecF renderOffset = m_actors["roaming enemy"]->GetAnimator().GetCurrentFrame()->element.GetRenderOffset();
			m_engine.GetRenderer().Draw(
				m_actors["roaming enemy"]->GetTransform().GetPosition() + renderOffset,
				spatial::SizeF{ m_actors["roaming enemy"]->GetWidth(), m_actors["roaming enemy"]->GetHeight() },
				graphics::ColorF{ 0, 1, 1, 0.5f },
				0);

			// draw a rectangle to fill the actor's contact/collision region if mouse cursor is inside it
			math::geometry::RectF collisionBox = m_actors["roaming enemy"]->GetCollisionBox();
			bool isContact = m_actors["roaming enemy"]->Contains(m_mousePosition);

			m_engine.GetRenderer().Draw(
				spatial::PosF{ collisionBox.left, collisionBox.top },
				collisionBox.GetSize(),
				isContact ? graphics::ColorF{ 1, 0, 0, 0.9f } : graphics::ColorF{ 0, 1, 1, 0.2f },
				0
			);
		}

		// for debugging purposes, draw the contact region of the hero actor
		{

			// draw a rectangle to fill the actor's contact/collision region if mouse cursor is inside it
			math::geometry::RectF collisionBox = m_actors["hero"]->GetCollisionBox();

			m_engine.GetRenderer().Draw(
				spatial::PosF{ collisionBox.left, collisionBox.top },
				collisionBox.GetSize(),
				graphics::ColorF{ 0, 1, 1, 0.2f },
				0
			);
		}

		// draw the actors
		for (const auto& kv : m_actors)
		{
			m_engine.GetRenderer().Draw(*kv.second);
		}

		// print mouse position
		std::stringstream ss;
		ss << "X: " << std::to_string(m_mousePosition.x) << " Y: " << std::to_string(m_mousePosition.y);
		m_engine.PrintText(ss.str().c_str(), spatial::PosF{ 0, 0 });

		// draw a dot showing position of enemy actor
		//m_engine.GetRenderer().Draw(m_positions["enemy2"], spatial::SizeF{ 10, 10 }, graphics::ColorF{ 1, 1, 1, 0.2f }, 0);
	}
};

