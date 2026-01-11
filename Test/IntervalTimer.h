#pragma once
#include <stdexcept>
#include <iostream>
#include "Event.h"

namespace timer
{
    //------------------------------------------------------------------------------
    // Fires events every specified time - interval.
    // Resolution is relative. you can set interval in seconds, but must feed
    // update with delta time in seconds too
    // Supports single-fire (OneShot) or repeating (Persistent)
    // modes, and caps triggers per Update call to prevent spiraling.
    //------------------------------------------------------------------------------
    class IntervalTimer
    {
    public:
        enum class Mode
        {
            Persistent, // Continues firing OnAlarm event every interval.
            OneShot     // Fires OnTimeOut event once then stops until Reset() is called.
        };

        event::Event<> OnTimeOut;
        event::Event<> OnInterval;
        event::Event<> OnMaxIntervalPerUpdateReached;

    private:
        float m_interval;      
        IntervalTimer::Mode m_mode;
        float m_elapsedTimeAccumulator;
        bool m_running;
        size_t m_maxTriggerPerUpdate;

    public:
        IntervalTimer(
            float interval,                
            Mode mode = Mode::Persistent,
            size_t maxTriggerPerUpdate = 5
        ) :
            m_interval(interval),
            m_mode(mode),
            m_elapsedTimeAccumulator(0.0),
            m_running(true),
            m_maxTriggerPerUpdate(maxTriggerPerUpdate)
        {
        }

        ~IntervalTimer() = default;

        void Reset()
        {
            m_elapsedTimeAccumulator = 0.0;
            m_running = true;
        }

        // Advances the timer by deltaSeconds.
        // delta - Time since last call, in seconds.
        // Accumulates time and fires:
        // - OnAlarm every interval  
        // - OnInterval once for OneShot  
        // - OnMaxIntervalPerUpdateReached if triggers exceed cap 
        void Update(float delta)
        {
            if (!m_running)
            {
                return;
            }

            m_elapsedTimeAccumulator += delta;

            size_t numUpdate = 0;
            while (m_elapsedTimeAccumulator >= m_interval && numUpdate < m_maxTriggerPerUpdate)
            {
                m_elapsedTimeAccumulator -= m_interval;

                numUpdate++;

                switch (m_mode)
                {
                case Mode::OneShot:
                    // set to false so we don't alarm anymore, until it gets reset
                    m_running = false;
                    OnTimeOut();
                    return;
                case Mode::Persistent:
                    // fire snooze event. we'll keep firing this on interval
                    OnInterval();
                    break;
                default:
                    throw std::runtime_error("invalid alarm clock mode");
                }

                // if we reach max alarm per update, we bail. remaining elapsed time will be processed in next update
                if (numUpdate == m_maxTriggerPerUpdate)
                {
                    OnMaxIntervalPerUpdateReached();
                }
            }
        }
    };
}

