#pragma once
#include <Spatial/Position.h>
#include <Spatial/Size.h>

namespace spatial
{
	template<typename T>
	class Transform
	{
	private:
		spatial::Position<T> m_position;
		spatial::Size<T> m_scale;
		float m_rotation = 0; // in radians
		math::VecF m_translate;

	public:
		const spatial::Position<T> GetPosition() const
		{
			return m_position;
		}

		void SetPosition(const spatial::Position<T> position)
		{
			m_position = position;
		}
	};

	using TransformF = Transform<float>;
}
