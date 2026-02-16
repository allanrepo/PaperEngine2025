#pragma once
#include <Math/Vector.h>

namespace engine
{
	namespace spatial
	{
		template<typename T>
		using Position = engine::math::Vector<T>;
		using PositionF = Position<float>;
	}
}

