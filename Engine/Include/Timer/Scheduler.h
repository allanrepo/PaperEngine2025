#pragma once
#include <vector>
#include <Timer/Pulse.h>

// design consideration:
//	- originally designed as frame rate controller but end up being more general purpose scheduler
//	- on function signature of schedule
//		- an owner or caller is not passed as argument in schedules
//		- because this is likely needed only when listener is free function
//		- there is also a work around to feed free function with owner if necessary
//		- that is why argument is only interval

namespace timer
{
	template <typename C>
	class Schedule
	{
	private:
		void (C::* m_pFunc)(float);
		C* m_pInst;
		float m_interval;

		friend class Scheduler;

	public:
		Schedule(float interval, C* inst, void (C::* func)(float)): 
			m_interval(interval), 
			m_pInst(inst), 
			m_pFunc(func)
		{
		}
	};

	// Specialization for free functions
	template <>
	class Schedule<void> 
	{
	private:
		void (*m_pFunc)(float);
		float m_interval;

		friend class Scheduler;

	public:
		Schedule(float interval, void (*func)(float)): 
			m_interval(interval), 
			m_pFunc(func) 
		{
		}
	};

	// Specialization for lambdas/std::function
	template <>
	class Schedule<std::function<void(float)>> 
	{
	private:
		std::function<void(float)> m_func;
		float m_interval;

		friend class Scheduler;

	public:
		Schedule(float interval, std::function<void(float)> func): 
			m_interval(interval), 
			m_func(std::move(func)) 
		{
		}
	};

	// deduction guide for free function
	Schedule(float, void(*)(float))->Schedule<void>;

	// deduction guide for member function
	template <typename C>
	Schedule(float, C*, void(C::*)(float)) -> Schedule<C>;

	// deduction guide for Lambda/std::function
	Schedule(float, std::function<void(float)>)->Schedule<std::function<void(float)>>;


	class Scheduler
	{
	private:
		std::vector<std::unique_ptr<timer::Pulse>> m_pulses;

		timer::Pulse& GetOrCreatePulse(float interval)
		{
			for (auto& pulse : m_pulses)
			{
				if (pulse->GetInterval() == interval)
				{
					return *pulse;
				}
			}
			// if not found, create a new one
			m_pulses.push_back(std::make_unique<timer::Pulse>(interval));
			return *m_pulses.back();
		}

	public:
		void Update(float time)
		{
			for (auto& pulse : m_pulses)
			{
				pulse->Update(time);
			}
		}

		void operator += (const Schedule<void>& sched) 
		{
			Pulse& pulse = GetOrCreatePulse(sched.m_interval);
			pulse.OnInterval += event::Handler(sched.m_pFunc);
		}

		template <typename C>
		void operator += (const Schedule<C>& sched) 
		{
			Pulse& pulse = GetOrCreatePulse(sched.m_interval);
			pulse.OnInterval += event::Handler(sched.m_pInst, sched.m_pFunc);
		}

		void operator += (const Schedule<std::function<void(float)>>& sched) 
		{
			Pulse& pulse = GetOrCreatePulse(sched.m_interval);
			pulse.OnInterval += event::Handler(sched.m_func);
		}

		void operator -= (const Schedule<std::function<void(float)>>& sched)
		{
			Pulse& pulse = GetOrCreatePulse(sched.m_interval);
			pulse.OnInterval -= event::Handler(sched.m_func);
		}

		// symmetric -= for unsubscription
		void operator -= (const Schedule<void>& sched) 
		{
			Pulse& pulse = GetOrCreatePulse(sched.m_interval);
			pulse.OnInterval -= event::Handler(sched.m_pFunc);
		}

		template <typename C>
		void operator -= (const Schedule<C>& sched) 
		{
			Pulse& pulse = GetOrCreatePulse(sched.m_interval);
			pulse.OnInterval -= event::Handler(sched.m_pInst, sched.m_pFunc);
		}
	};

}