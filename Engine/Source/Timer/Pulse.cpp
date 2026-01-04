#include <Timer/Pulse.h>

timer::Pulse::Pulse(
    float interval,
    Mode mode,
    size_t maxAlarmPerUpdate
) :
    m_interval(interval),
    m_mode(mode),
    m_elapsedTimeAccumulator(0.0),
    m_running(true),
    m_maxAlarmPerUpdate(maxAlarmPerUpdate)
{
}

void timer::Pulse::Reset()
{
    m_elapsedTimeAccumulator = 0.0;
    m_running = true;
}

// Advances the timer by delta.
// delta - this value is relative. the provider for this value defines its unit.
// Accumulates time and fires:
// - OnAlarm every interval  
// - OnInterval once for OneShot  
// - OnMaxIntervalPerUpdateReached if triggers exceed cap 
void timer::Pulse::Update(float delta)
{
    if (!m_running)
    {
        return;
    }

    m_elapsedTimeAccumulator += delta;

    size_t numUpdate = 0;
    while (m_elapsedTimeAccumulator >= m_interval && numUpdate < m_maxAlarmPerUpdate)
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
        if (numUpdate == m_maxAlarmPerUpdate)
        {
            OnMaxIntervalPerUpdateReached();
        }
    }
}
