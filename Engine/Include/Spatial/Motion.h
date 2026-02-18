#pragma once
#include <Spatial/Transform.h>

namespace engine::spatial
{
	template<typename T>
	class Motion
	{
	private:
		math::Vec<T> m_velocity;
		float m_speed;

	public:
		void Update(spatial::Transform<T>& transform, double delta) const
		{
			transform.SetPosition(transform.GetPosition() + (m_velocity * (float)delta));
		}

		void MoveTo(const spatial::Transform<T>& transform, const engine::spatial::Position<T> target, float speed)
		{
			math::VecF direction = target - transform.GetPosition();
			direction = direction.Normalize();
			m_velocity = direction * speed;
		}

		const math::Vec<T> GetVelocity() const
		{
			return m_velocity;
		}

		void SetVelocity(const math::Vec<T> velocity)
		{
			m_velocity = velocity;
		}

		void Stop()
		{
			m_velocity = { 0, 0 };
		}
	};

	using MotionF = Motion<float>;
}
