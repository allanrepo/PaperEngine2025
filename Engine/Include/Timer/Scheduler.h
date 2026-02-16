#pragma once
#include <vector>
#include <Timer/Pulse.h>

// -----------------------------------------------------------------------------------------------------------
// design consideration:
// 	-	originally designed as frame rate controller but end up being more general purpose scheduler
// 	-	on function signature of schedule
// 		-	an owner or caller is not passed as argument in schedules
// 		-	because this is likely needed only when listener is free function
// 		-	there is also a work around to feed free function with owner if necessary
// 		-	that is why argument is only interval
// 	-	on adding maxTriggerPerUpdate on Schedule
// 	-	on adding reset on overflow option on schedule
// 		-	we don't really want elapsed time accumulator of Pulse to run away.
// 		-	that can creep into extremely large value and although maxTriggerUpdate caps it,
// 			it will still cost unnecessary overhead
// 		-	Pulse has option to reset on overflow. so if a schedule wants to clear accumulated
// 			elapsed time on overflow, it can enable it
// 
// -----------------------------------------------------------------------------------------------------------


namespace engine::timer
{
	template <typename C>
	class Schedule
	{
	private:
		void (C::* m_pFunc)(double);
		C* m_pInst;
		double m_interval;
		size_t m_maxTriggerPerUpdate;
		bool m_resetOnOverflow;

		friend class Scheduler;

	public:
		Schedule(double interval, C* inst, void (C::* func)(double), bool resetOnOverflow = false, size_t maxTriggerPerUpdate = 5):
			m_interval(interval), 
			m_pInst(inst), 
			m_pFunc(func),
			m_resetOnOverflow(resetOnOverflow),
			m_maxTriggerPerUpdate(maxTriggerPerUpdate)
		{
		}
	};

	// Specialization for free functions
	template <>
	class Schedule<void> 
	{
	private:
		void (*m_pFunc)(double);
		double m_interval;
		size_t m_maxTriggerPerUpdate;
		bool m_resetOnOverflow;

		friend class Scheduler;

	public:
		Schedule(double interval, void (*func)(double), bool resetOnOverflow = false, size_t maxTriggerPerUpdate = 5):
			m_interval(interval), 
			m_pFunc(func),
			m_resetOnOverflow(resetOnOverflow),
			m_maxTriggerPerUpdate(maxTriggerPerUpdate)
		{
		}
	};

	// Specialization for lambdas/std::function
	template <>
	class Schedule<std::function<void(double)>> 
	{
	private:
		std::function<void(double)> m_func;
		double m_interval;
		size_t m_maxTriggerPerUpdate;
		bool m_resetOnOverflow;

		friend class Scheduler;

	public:
		Schedule(double interval, std::function<void(double)> func, bool resetOnOverflow = false, size_t maxTriggerPerUpdate = 5):
			m_interval(interval), 
			m_func(std::move(func)),
			m_resetOnOverflow(resetOnOverflow),
			m_maxTriggerPerUpdate(maxTriggerPerUpdate)
		{
		}
	};

	// deduction guide for free function
	Schedule(double, void(*)(double))->Schedule<void>;

	// deduction guide for member function
	template <typename C>
	Schedule(double, C*, void(C::*)(double)) -> Schedule<C>;

	// deduction guide for Lambda/std::function
	Schedule(double, std::function<void(double)>)->Schedule<std::function<void(double)>>;

	class Scheduler
	{
	private:
		std::vector<std::unique_ptr<engine::timer::Pulse>> m_pulses;

		engine::timer::Pulse& GetOrCreatePulse(double interval, bool resetOnOverflow, size_t maxTriggerPerUpdate)
		{
			for (auto& pulse : m_pulses)
			{
				if (pulse->GetInterval() == interval && pulse->GetMaxTriggerPerUpdate() == maxTriggerPerUpdate)
				{
					return *pulse;
				}
			}
			// if not found, create a new one
			m_pulses.push_back(std::make_unique<engine::timer::Pulse>(interval, engine::timer::Pulse::Mode::Persistent, resetOnOverflow, maxTriggerPerUpdate));
			return *m_pulses.back();
		}

	public:
		Scheduler() = default;
		virtual ~Scheduler() = default;

		void Update(double time)
		{
			for (auto& pulse : m_pulses)
			{
				pulse->Update(time);
			}
		}		

		void operator += (const Schedule<void>& sched) 
		{
			Pulse& pulse = GetOrCreatePulse(sched.m_interval, sched.m_resetOnOverflow, sched.m_maxTriggerPerUpdate);
			pulse.IntervalEvent += event::Handler(sched.m_pFunc);		
		}

		template <typename C>
		void operator += (const Schedule<C>& sched) 
		{
			Pulse& pulse = GetOrCreatePulse(sched.m_interval, sched.m_resetOnOverflow, sched.m_maxTriggerPerUpdate);
			pulse.IntervalEvent += event::Handler(sched.m_pInst, sched.m_pFunc);
		}

		void operator += (const Schedule<std::function<void(double)>>& sched) 
		{
			Pulse& pulse = GetOrCreatePulse(sched.m_interval, sched.m_resetOnOverflow, sched.m_maxTriggerPerUpdate);
			pulse.IntervalEvent += event::Handler(sched.m_func);
		}

		void operator -= (const Schedule<std::function<void(double)>>& sched)
		{
			Pulse& pulse = GetOrCreatePulse(sched.m_interval, sched.m_resetOnOverflow, sched.m_maxTriggerPerUpdate);
			pulse.IntervalEvent -= event::Handler(sched.m_func);
		}

		// symmetric -= for unsubscription
		void operator -= (const Schedule<void>& sched) 
		{
			Pulse& pulse = GetOrCreatePulse(sched.m_interval, sched.m_resetOnOverflow, sched.m_maxTriggerPerUpdate);
			pulse.IntervalEvent -= event::Handler(sched.m_pFunc);
		}

		template <typename C>
		void operator -= (const Schedule<C>& sched) 
		{
			Pulse& pulse = GetOrCreatePulse(sched.m_interval, sched.m_resetOnOverflow, sched.m_maxTriggerPerUpdate);
			pulse.IntervalEvent -= event::Handler(sched.m_pInst, sched.m_pFunc);
		}
	};

}