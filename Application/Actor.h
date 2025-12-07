#pragma once
#include <string>
#include <Spatial/Transform.h>
#include <Spatial/Motion.h>
#include <Graphics/Animation/Animation.h>
#include <Graphics/Renderable/ISprite.h>
#include <Cache/Dictionary.h>
#include <State/StateMachine.h>

namespace component
{
	class Actor
	{
	private:
		std::string m_name;
		spatial::TransformF m_transform;
		spatial::MotionF m_motion;
		graphics::animation::Animator<graphics::renderable::ISprite> m_animator;
		cache::Dictionary <std::string, std::unique_ptr<graphics::animation::Animation<graphics::renderable::ISprite>>> m_animations;
		state::StateMachine<component::Actor> m_stateMachine;

		// TODO: can change this as enum instead of vector. unless i want to use its granularity e.g. specific angle orientation
		math::VecF m_faceDirection;

		bool m_targetable;

	public:
		Actor(const std::string& name = "default") :
			m_stateMachine(this),
			m_name(name),
			m_targetable(true)
		{

		}
		virtual ~Actor() = default;


		virtual void SetTargetable(bool enable)
		{
			// do nothing if state does not change
			if (m_targetable == enable)
			{
				return;
			}
			m_targetable = enable;
			OnTargetableChanged(this, m_targetable);
		}


		virtual const math::geometry::RectF GetCollisionBox(bool absolute = true) const
		{
			return m_animator.GetCurrentFrame()? m_animator.GetCurrentFrame()->region.contact.Translate(m_transform.GetPosition()) : math::geometry::RectF{};
		}

		bool Contains(spatial::PosF position)
		{
			return GetCollisionBox().Contains(position);
		}

		void Update(float delta)
		{
			m_animator.Update(delta);
			m_stateMachine.Update(delta);
		}

		void SetState(std::unique_ptr<state::State<component::IActor>> state)
		{
			m_stateMachine.Set(std::move(state));
		}

		void QueueState(std::unique_ptr<state::State<component::IActor>> state)
		{
			m_stateMachine.Queue(std::move(state));
		}

		void AddAnimation(const std::string& name, std::unique_ptr<graphics::animation::Animation<graphics::resource::Sprite>> animation)
		{
			m_animations.Register(name, std::move(animation));
		}

		void SetAnimation(const std::string& key)
		{
			m_animator.Play(m_animations.Has(key) ? m_animations.Get(key).get() : nullptr);
		}

		graphics::animation::Animator<graphics::resource::Sprite>& GetAnimator()
		{
			return m_animator;
		}

		virtual const graphics::animation::Animator<graphics::resource::Sprite>& GetAnimator() const
		{
			return m_animator;
		}

		virtual state::StateMachine<component::IActor>& GetStateMachine()
		{
			return m_stateMachine;
		}

		virtual spatial::Transform& GetTransform() 
		{
			return m_transform;
		}

		virtual const spatial::Transform& GetTransform() const
		{
			return m_transform;
		}

		spatial::Motion& GetMotion()
		{
			return m_motion;
		}

		virtual math::VecF GetFaceDirection() const
		{
			return m_faceDirection;
		}

		virtual void SetFaceTo(spatial::PosF pos)
		{
			m_faceDirection = (pos - m_transform.GetPosition()).Normalize();
		}

		// IRenderable methods implementation
		virtual math::geometry::RectF GetUVRect() const override final
		{
			if (m_animator.GetCurrentFrame())
			{
				return m_animator.GetCurrentFrame()->element.GetUVRect();
			}
			return math::geometry::RectF{ 0,0,0,0 };
		}

		virtual void Bind() const override final
		{
			if (m_animator.GetCurrentFrame())
			{
				m_animator.GetCurrentFrame()->element.Bind();
			}
		}

		virtual bool CanBind() const override final
		{
			if (m_animator.GetCurrentFrame())
			{
				return m_animator.GetCurrentFrame()->element.CanBind();
			}
			return false;
		}

		virtual const float GetWidth() const override final
		{
			if (m_animator.GetCurrentFrame())
			{
				return m_animator.GetCurrentFrame()->element.GetWidth();
			}
			return 0.0f;
		}

		virtual const float GetHeight() const override final
		{
			if (m_animator.GetCurrentFrame())
			{
				return m_animator.GetCurrentFrame()->element.GetHeight();
			}
			return 0.0f;
		}
	
		virtual const spatial::SizeF GetSize() const override final
		{
			if (m_animator.GetCurrentFrame())
			{
				return m_animator.GetCurrentFrame()->element.GetSize();
			}
			return spatial::SizeF{ 0.0f, 0.0f };
		}

	};

}

