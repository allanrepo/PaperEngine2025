#pragma once
#include <Core/Event.h>

namespace engine::timer
{
	class FrameRateController
	{
	private:
		double m_targetInterval;
		double m_elapsedTimeAccumulator;

		// TODO: remove this once IsReady() is removed
		double m_lastElapsedTime;

		engine::event::Event<double> FrameCompletedEvent;

	public:
		explicit FrameRateController(double targetFPS = 60.0f) :
			m_targetInterval(1.0f / targetFPS),
			m_elapsedTimeAccumulator(0.0f),
			m_lastElapsedTime(0.0f)
		{
		}

		void Update(double elapsedTime)
		{
			m_elapsedTimeAccumulator += elapsedTime;

			if (m_elapsedTimeAccumulator >= m_targetInterval)
			{
				FrameCompletedEvent(m_elapsedTimeAccumulator);

				m_elapsedTimeAccumulator = 0.0f;
			}
		}

		// subscribe as free function
		void operator+=(engine::event::Handler<void, void, double> handler)
		{
			FrameCompletedEvent += handler;
		}

		// unsubscribe as free function
		void operator-=(engine::event::Handler<void, void, double> handler)
		{
			FrameCompletedEvent -= handler;
		}

		// subscribe as class method
		template <typename C>
		void operator+=(engine::event::Handler<void, C, double> handler)
		{
			FrameCompletedEvent += handler;
		}

		// unsubscribe as class method
		template <typename C>
		void operator-=(engine::event::Handler<void, C, double> handler)
		{
			FrameCompletedEvent -= handler;
		}

		// subscribe as class lambda or std::function. TODO: test if this works
		void operator+=(engine::event::Handler<void, std::function<void(double)>, double> handler)
		{
			FrameCompletedEvent += handler;
		}

		// unsubscribe as class lambda or std::function. TODO: test if this works
		void operator-=(engine::event::Handler<void, std::function<void(double)>, double> handler)
		{
			FrameCompletedEvent -= handler;
		}


		// TODO:
		// this method is a way to use this as polling, where listener polls if ready to render
		// but i added a "subscription" method to align other timer class designs. 
		// i should be removing this now as it is redundant, but let us keep for now...	
		bool IsReady(double delta)
		{
			m_elapsedTimeAccumulator += delta;

			if (m_elapsedTimeAccumulator >= m_targetInterval)
			{
				m_lastElapsedTime = m_elapsedTimeAccumulator;

				m_elapsedTimeAccumulator = 0.0f; // reset fully
				return true;
			}
			return false;
		}

		// TODO:
		// remove this to if removing IsReady. this only exist for IsReady
		double GetLastElapsedTime() const
		{
			return m_lastElapsedTime;
		}
	};

}