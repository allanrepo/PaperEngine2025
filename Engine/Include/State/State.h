#pragma once
/*
design notes:
-	states are behaviors, not data holders hence it does not have reference to owner
-	ideally, states should be stateless. however, in practical scenario, states may need to hold some temporary data. an example
	is a "loading" state that may need to hold loading progress percentage. hence, states are allowed to hold data members.	
-	state machine is a component of the owner that manages the states. hence, states have reference to owner when Enter, Exit, Update, IsFinished are called.
*/

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

