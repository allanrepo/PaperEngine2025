#pragma once
#include <Math/Vector.h>

namespace spatial
{
	template<typename T>
	using Position = math::Vector<T>;
	using PositionF = Position<float>;
}
