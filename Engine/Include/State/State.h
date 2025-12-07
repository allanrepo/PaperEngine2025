#pragma once

namespace state
{
	template<typename T>
	class State
	{
	public:
		virtual ~State() = default;
		virtual void Enter(T& owner) = 0;
		virtual void Exit(T& owner) = 0;
		virtual void Update(T& owner, float delta) = 0;
		virtual bool IsFinished(T& owner) = 0;
	};
}

