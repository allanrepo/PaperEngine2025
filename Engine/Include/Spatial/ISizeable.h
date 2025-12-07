#pragma once
#include <Spatial/Size.h>

namespace spatial
{
	template<typename T>
	class ISizeable
	{
	public:
		virtual const T GetWidth() const = 0;
		virtual const T GetHeight() const = 0;
		virtual const spatial::Size<T> GetSize() const = 0;
	};
}
