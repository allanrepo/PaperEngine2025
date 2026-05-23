#pragma once
#include <Spatial/ISizeable.h>

namespace engine::spatial
{
	template<typename T>
	class IResizeable : public ISizeable<T>
	{
	public:
		virtual void SetSize(const math::Size<int>& size) = 0;
		virtual void SetHeight(const int height) = 0;
		virtual void SetWidth(const int width) = 0;
	};
}
