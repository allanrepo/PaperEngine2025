#pragma once
#include <Math/Size.h>

namespace engine::spatial
{
	template<typename T>
	class ISizeable
	{
	public:
		virtual T GetWidth() const = 0;
		virtual T GetHeight() const = 0;
		virtual math::Size<T> GetSize() const = 0;
	};
}
