#pragma once
#include <Command/ICommand.h>
#include <unordered_map>

namespace engine
{
	namespace command
	{
		class CommandQueue
		{
		private:
			std::unordered_map<engine::command::Type, std::vector<std::unique_ptr<ICommand>>> queues;

		public:
			virtual ~CommandQueue() = default;

			void Enqueue(std::unique_ptr<ICommand> command)
			{
				queues[command->GetType()].emplace_back(std::move(command));
			}

			void Dispatch(engine::command::Type type, bool clear = true)
			{
				auto& queue = queues[type];
				for (auto& cmd : queue)
				{
					cmd->Execute();
				}

				if (clear)
				{
					queue.clear();
				}
			}

			void Clear(engine::command::Type type)
			{
				auto& queue = queues[type];
				queue.clear();
			}

			virtual bool IsEmpty() const
			{
				for (const auto& pair : queues)
				{
					if (!pair.second.empty())
					{
						return false;
					}
				}
				return true;
			}

			void Clear()
			{
				for (auto& [type, q] : queues)
				{
					q.clear();
				}
			}
		};
	}
}
