#pragma once

namespace engine
{
	namespace command
	{
		enum class Type 
		{ 
			Logic, 
			Render, 
			Audio, 
			Input 
		};

		class ICommand
		{
		public:
			virtual ~ICommand() = default;
			virtual void Execute() = 0;
			virtual Type GetType() const = 0;
		};
	}
}
