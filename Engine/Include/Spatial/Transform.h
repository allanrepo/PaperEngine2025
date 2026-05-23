#pragma once
#include <Spatial/Position.h>
#include <Math/Size.h>

namespace engine::spatial
{
	template<typename T>
	class Transform
	{
	private:
		engine::spatial::Position<T> m_position;
		math::Size<T> m_scale;
		float m_rotation = 0; // in radians
		math::VecF m_translate;

	public:
		const engine::spatial::Position<T> GetPosition() const
		{
			return m_position;
		}

		void SetPosition(const engine::spatial::Position<T> position)
		{
			m_position = position;
		}
	};

	using TransformF = Transform<float>;
}
