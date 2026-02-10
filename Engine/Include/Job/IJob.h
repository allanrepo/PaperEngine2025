#pragma once
#include <Timer/StopWatch.h>
#include <algorithm>
#include <functional>
#include <deque>

namespace engine
{
	namespace job
	{
		class IJob
		{
		public:
			virtual ~IJob() = default;
			virtual void Execute() = 0;
			virtual bool IsDone() const = 0;
			virtual bool IsPersistent() const = 0;
			virtual void Done() = 0;
			virtual void Start() = 0;
			virtual bool IsStarted() const = 0;
		};
	}
}