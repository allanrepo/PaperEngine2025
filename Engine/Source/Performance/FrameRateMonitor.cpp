#include <Performance/FrameRateMonitor.h>

performance::FrameRateMonitor::FrameRateMonitor(float measureRange) :
	m_measureRange(measureRange),
	m_elapsedTimeAccumulator(0.0f),
	m_elapsedTimes()
{
}

void performance::FrameRateMonitor::SetMeasureRange(float range)
{
	m_measureRange = range;
}

float performance::FrameRateMonitor::GetAverageFrameRate() const
{
	if (m_elapsedTimeAccumulator > 0.0f && !m_elapsedTimes.empty())
	{
		return static_cast<float>(m_elapsedTimes.size()) / m_elapsedTimeAccumulator;
	}
	return 0;
}

float performance::FrameRateMonitor::GetLastFrameRate() const
{
	if (m_elapsedTimes.size() > 0)
	{
		float lastElapsed = m_elapsedTimes.back();
		if (lastElapsed > 0.0f)
		{
			return 1.0f / lastElapsed;
		}
	}
	return 0;
}

// using "On" prefix to follow event handler naming convention since this method is used as event handler - it is called when a frame is completed
void performance::FrameRateMonitor::OnFrameCompleted(float elapsedTime)
{
	m_elapsedTimes.push_back(elapsedTime);
	m_elapsedTimeAccumulator += elapsedTime;

	while (!m_elapsedTimes.empty() && m_elapsedTimeAccumulator > m_measureRange)
	{
		float front = m_elapsedTimes.front();
		m_elapsedTimes.pop_front();
		m_elapsedTimeAccumulator -= front;

		// guard against negative accumulator due to floating point precision issues. make sure it stays zero or positive
		if (m_elapsedTimeAccumulator < 0.0f) m_elapsedTimeAccumulator = 0.0f;
	}
}