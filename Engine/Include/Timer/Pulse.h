#pragma once
#include <stdexcept>
#include <iostream>
#include <Core/Event.h>

namespace timer
{
    //------------------------------------------------------------------------------
    // Fires events every specified time - interval.
    // Resolution is relative. you can set interval in seconds, but must feed
    // update with delta time in seconds too
    // Supports single-fire (OneShot) or repeating (Persistent)
    // modes, and caps triggers per Update call to prevent spiraling.
    // 
    // design consideration:
    //  -   on events having interval as arguments
    //      -   although pulse will trigger exactly on interval, having the interval 
    //          value passed to event handlers will be useful for subscribers as it 
    //          provides convenience so subscribers don't have to query the pulse 
    //          instance for its interval value
	//  -   similarly on OnMaxIntervalPerUpdateReached event...
	//      -   having the max interval per update value passed to event handlers
    //------------------------------------------------------------------------------
    class Pulse
    {
    public:
        enum class Mode
        {
            Persistent, // Continues firing OnAlarm event every interval.
            OneShot     // Fires OnTimeOut event once then stops until Reset() is called.
        };

        event::Event<float> OnTimeOut;
        event::Event<float> OnInterval;
        event::Event<size_t> OnMaxIntervalPerUpdateReached;

    private:
        float m_interval;      
        Pulse::Mode m_mode;
        float m_elapsedTimeAccumulator;
        bool m_running;
        size_t m_maxAlarmPerUpdate;

    public:
        Pulse(float interval, Mode mode = Mode::Persistent, size_t maxAlarmPerUpdate = 5);

        ~Pulse() = default;

        void Reset();

        float GetInterval() const
        {
            return m_interval;
		}

        // Advances the timer by deltaSeconds.
        // delta - this value is relative. the provider for this value defines its unit.
        // Accumulates time and fires:
        // - OnAlarm every interval  
        // - OnInterval once for OneShot  
        // - OnMaxIntervalPerUpdateReached if triggers exceed cap 
        void Update(float delta);
    };
}

