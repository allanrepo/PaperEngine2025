#pragma once

#include <string>

namespace engine
{
	namespace loader
	{
		class IAsyncLoader 
		{ 
		public: 
			virtual ~IAsyncLoader() = default; 
			virtual void Update(double delta) = 0; 
			virtual size_t GetCurrent() const = 0; 
			virtual size_t GetTotal() const = 0; 
			virtual std::string GetLabel() const = 0; 
			virtual double GetProgress() const = 0;
			virtual bool IsDone() const = 0; 
		};
	}
}