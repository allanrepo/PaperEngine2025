#pragma once
#include <Core/Event.h>
#include <chrono>
#include <thread>
#include <Windows.h>
#include <ratio>

#pragma comment(lib, "winmm.lib")

namespace timer
{
    using milliseconds = std::milli;
    using microseconds = std::micro;
    using seconds = std::ratio<1>;

    //------------------------------------------------------------------------------
    // Measures *active* time (excluding pauses) across multiple pause/resume cycles.
    // Also supports lap functionality for measuring intervals between checkpoints.
    //------------------------------------------------------------------------------
    class StopWatch
    {
    private:
        // snapshot of time when last lap occur. everytime pause happens, this is set as the paused time and time occurred before the pause and last lap time is accumulated
        std::chrono::steady_clock::time_point m_lastLapTime;

        std::chrono::steady_clock::time_point m_startTime;

        // snapshot of time when last pause occur
        std::chrono::steady_clock::time_point m_pausedStartTime;

        // sum of all non-paused time between last and current lap
        std::chrono::duration<float> m_lastLapDurationAccumulator = std::chrono::duration<float>::zero();

        // sum of all paused durations between start and stop
        std::chrono::duration<float> m_PausedDurationAccumulator = std::chrono::duration<float>::zero();

        bool m_running = false;
        bool m_paused = false;

    public:
        // event that fires up when StopWatch starts 
        event::Event<>OnStart;

        // event that fires up when StopWatch stops
        event::Event<float> OnStop;

        // event that fires up when StopWatch measure lap
        event::Event<float> OnLap;

        // event that fires up when StopWatch peeks time since it started
        event::Event<float> OnPeek;

        // event that fires up when StopWatch is paused
        event::Event<> OnPause;

        // event that fires up when StopWatch is resumed
        event::Event<> OnResume;

    public:
        StopWatch() noexcept;
        ~StopWatch() = default;

        const bool IsRunning() const;
        const bool IsPaused() const;

        // Starts or restarts the timer from zero active time.
        void Start() noexcept;

        // Pauses the timer, adding active time since the last lap/resume
        // into the accumulator. Does nothing if already paused.
        void Pause() noexcept;

        // Resumes the timer from a paused state.
        // Does nothing if already running.
        void Resume() noexcept;

        // Ends the current lap and returns the total *active* time since the
        // previous lap. Resets the accumulator for the next lap.
        template<typename T = timer::seconds>
        float Lap() noexcept
        {
            if (!m_running)
            {
                return 0;
            }

            // this function is meant to return the duration between now and last call to Lap()
            // but since pause/resume can happen in between now and last call to Lap(), the paused duration is not included
            // things to consider:
            // 1. timer can be paused anytime between now and last lap
            // 2. between now and last lap, there may have been 1 or more occurrence of pause/resume
            // 3. last lap may have started when timer is paused but is now resumed
            // 4. last lap may have started when timer is paused and is still paused now

            // measure current time (now or last paused time if paused)
            std::chrono::steady_clock::time_point now = m_paused ? m_pausedStartTime : std::chrono::steady_clock::now();

            // calculate difference between start and previous lap 
            std::chrono::duration<float> elapsedTime = now - m_lastLapTime;

            // subtract accumulated pause duration from start to now
            elapsedTime += m_lastLapDurationAccumulator;

            // reset total lap time since we are flushing it now
            m_lastLapDurationAccumulator = std::chrono::steady_clock::duration::zero();

            // start next lap now
            m_lastLapTime = now;

            float elapsed = std::chrono::duration_cast<std::chrono::duration<float, T>>(elapsedTime).count();

            OnLap(elapsed);

            // return it
            return elapsed;
        }

        // Returns the total *active* time since the last lap without resetting.
        template<typename T = timer::seconds>
        float Peek() noexcept
        {
            if (!m_running)
            {
                return 0;
            }

            // this function is meant to return the duration between now and start. 
            // but since pause/resume can happen in between, the paused duration is not included.
            // things to consider:
            // 1. timer can be paused anytime between start and stop
            // 2. between now and start, there may have been 1 or more occurrence of pause/resume

            // let's get the current time now. if timer is paused, we get the snapshot of when paused occurred instead
            // this is to disregard the duration between now and when paused started
            std::chrono::steady_clock::time_point now = m_paused ? m_pausedStartTime : std::chrono::steady_clock::now();

            // calculate difference between start and now
            std::chrono::duration<float> elapsedTime = now - m_startTime;

            // we subtract any pause duration we accumulated
            elapsedTime -= m_PausedDurationAccumulator;

            float elapsed = std::chrono::duration_cast<std::chrono::duration<float, T>>(elapsedTime).count();

            OnPeek(elapsed);

            // return it
            return elapsed;
        }

        // Ends the run and returns total active time since start.
        template<typename T = timer::seconds>
        float Stop() noexcept
        {
            if (!m_running)
            {
                return 0;
            }

            // this function is meant to return the duration between now and start. 
            // but since pause/resume can happen in between, the paused duration is not included.
            // things to consider:
            // 1. timer can be paused anytime between start and stop
            // 2. between now and start, there may have been 1 or more occurrence of pause/resume

            // let's get the current time now. if timer is paused, we get the snapshot of when paused occurred instead
            // this is to disregard the duration between now and when paused started
            std::chrono::steady_clock::time_point now = m_paused ? m_pausedStartTime : std::chrono::steady_clock::now();

            // calculate difference between start and now
            std::chrono::duration<float> elapsedTime = now - m_startTime;

            // we subtract any pause duration we accumulated
            elapsedTime -= m_PausedDurationAccumulator;

            // disable running since we stopped
            m_running = false;

            m_paused = false;

            float elapsed = std::chrono::duration_cast<std::chrono::duration<float, T>>(elapsedTime).count();

            OnStop(elapsed);

            // return it
            return elapsed;
        }
    };


}