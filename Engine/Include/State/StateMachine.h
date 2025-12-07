#pragma once
#include <State/State.h>	
#include <memory>
#include <queue>

namespace state
{
	template<typename T>
	class StateMachine
	{
	private:
		std::unique_ptr<State<T>> m_current;
		std::queue<std::unique_ptr<State<T>>> m_queue;
		T* m_owner;

	public:
		StateMachine(T* owner):
			m_owner(owner)
		{
		}

		// set state immediately, exiting current state if any. does not flush queued states.
		void Set(std::unique_ptr<State<T>> state)
		{
			if (m_current)
			{
				m_current->Exit(*m_owner);
			}
			m_current = std::move(state);
			if (m_current)
			{
				m_current->Enter(*m_owner);
			}
		}

		void Queue(std::unique_ptr<State<T>> state)
		{
			m_queue.push(std::move(state));
		}

		void Flush()
		{
			while (!m_queue.empty())
			{
				m_queue.pop();
			}
		}

		void Update(float dt)
		{
			if (m_current)
			{
				m_current->Update(*m_owner, dt);

				if (m_current->IsFinished(*m_owner) && !m_queue.empty())
				{
					Set(std::move(m_queue.front()));
					m_queue.pop();
				}
			}
		}

		State<T>* Get() const
		{
			return m_current.get();
		}
	};
}
