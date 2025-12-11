#pragma once
#include <Spatial/Size.h>

namespace spatial
{
	template<typename T>
	class ISizeable
	{
	public:
		virtual T GetWidth() const = 0;
		virtual T GetHeight() const = 0;
		virtual spatial::Size<T> GetSize() const = 0;
	};
}
