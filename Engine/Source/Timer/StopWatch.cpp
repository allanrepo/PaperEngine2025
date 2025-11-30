#include <Timer/StopWatch.h>

timer::StopWatch::StopWatch() noexcept :
    m_paused(false),
    m_lastLapDurationAccumulator(0.0),
    m_lastLapTime(std::chrono::steady_clock::now()),
    m_PausedDurationAccumulator(0.0),
    m_startTime(std::chrono::steady_clock::now()),
    m_running(false)
{
}

const bool timer::StopWatch::IsRunning() const
{
    return m_running;
}

const bool timer::StopWatch::IsPaused() const
{
    return m_paused;
}

// starts the StopWatch
void timer::StopWatch::Start() noexcept
{
    // measure current time (start)
    m_startTime = std::chrono::steady_clock::now();

    // set last lap to now. so lap calls will return duration between the lap call and this start
    m_lastLapTime = m_startTime;

    // this is our total pause duration accumulator. set it to 0 since we are beginning
    m_PausedDurationAccumulator = std::chrono::steady_clock::duration::zero();

    // this is our last lap duration accumulator. set it to 0 since we are beginning
    m_lastLapDurationAccumulator = std::chrono::steady_clock::duration::zero();
    
    m_running = true;

    m_paused = false;

    OnStart();
}

void timer::StopWatch::Pause() noexcept
{
    if (!m_running || m_paused)
    {
        return;
    }

    // now is the pause time
    m_pausedStartTime = std::chrono::steady_clock::now();

    // since we paused, we are storing elapsed time since lap time and now
    m_lastLapDurationAccumulator += (m_pausedStartTime - m_lastLapTime);

    // this is now the last lap time
    m_lastLapTime = m_pausedStartTime;

    m_paused = true;

    OnPause();
}

void timer::StopWatch::Resume() noexcept
{
    if (!m_running || !m_paused)
    {
        return;
    }

    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

    // add this pause duration to our total pause duration
    m_PausedDurationAccumulator += (now - m_pausedStartTime);

    // this is now the last lap time
    m_lastLapTime = now;

    m_paused = false;

    OnResume();
}