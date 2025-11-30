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
    //------------------------------------------------------------------------------
    class Pulse
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
        Pulse::Mode m_mode;
        float m_elapsedTimeAccumulator;
        bool m_running;
        size_t m_maxAlarmPerUpdate;

    public:
        Pulse(float interval, Mode mode = Mode::Persistent, size_t maxAlarmPerUpdate = 5);

        ~Pulse() = default;

        void Reset();

        // Advances the timer by deltaSeconds.
        // delta - Time since last call, in seconds.
        // Accumulates time and fires:
        // - OnAlarm every interval  
        // - OnInterval once for OneShot  
        // - OnMaxIntervalPerUpdateReached if triggers exceed cap 
        void Update(float delta);
    };
}

