#pragma once
#include <string>

namespace engine
{
	namespace graphics
	{
		// IAnimated is a small, generic interface that standardizes animation control for renderable objects. 
		// It defines the minimal contract required to start an animation by name and to advance animation state over time.
		template<typename T = std::string>
		class IAnimated
		{
		public:
			virtual ~IAnimated() = default;
			virtual bool Play(const T& name) = 0;
			virtual void Update(double time) = 0;
		};
	}
}