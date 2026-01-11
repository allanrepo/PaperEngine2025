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
	//  -   similarly on OverflowReachedEvent event...
	//      -   having the max interval per update value passed to event handlers
    //  -   on OverflowReachEvent arguments
    //      -   we passing the event caller e.g. Pulse so that listeners have option
    //          to query Pulse why overflow happened e.g. do we reset the pulse 
    //          to flush remaining accumulated elapsed time to prevent runaway 
    //          overflow that can cause nasty bug?
    //      -   as above, pass the value of accumulated elapsed time in case listener
    //          want to do something with it as well as number of interval updates
    //          which may not be a useful information, but passing it nonetheless
    //  -   on adding resetOnOverflow option
    //      -   originally, we let listeners do this themselves by listening to 
    //          OverflowReachedEvent. i even pass the Pulse caller reference so they
    //          can conveniently reset themselves.
    //      -   but it becomes very inconvenient to have to subscribe to the event
    //          just to reset. so this option is provided. if listeners decided to 
    //          not use this and instead do it via event, it can still do so.
    //------------------------------------------------------------------------------
    class Pulse
    {
    public:
        enum class Mode
        {
            Persistent, // Continues firing OnAlarm event every interval.
            OneShot     // Fires OnTimeOut event once then stops until Reset() is called.
        };

        event::Event<float> TimeOutEvent;
        event::Event<float> IntervalEvent;
        event::Event<Pulse&, float, size_t> OverflowReachedEvent;

    private:
        float m_interval;      
        Pulse::Mode m_mode;
        float m_elapsedTimeAccumulator;
        bool m_running;
        size_t m_maxTriggerPerUpdate;
        bool m_resetOnOverflow;

    public:
        Pulse(float interval, Mode mode = Mode::Persistent, bool resetOnOverflow = false, size_t maxTriggerPerUpdate = 5);

        ~Pulse() = default;

        void Reset();

        float GetInterval() const
        {
            return m_interval;
		}

        size_t GetMaxTriggerPerUpdate() const
        {
            return m_maxTriggerPerUpdate;
        }

        bool IsResetOnOverflow() const
        {
            return m_resetOnOverflow;
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

