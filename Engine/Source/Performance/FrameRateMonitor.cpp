#include <Performance/FrameRateMonitor.h>

engine::performance::FrameRateMonitor::FrameRateMonitor(double measureRange) :
	m_measureRange(measureRange),
	m_elapsedTimeAccumulator(0.0f),
	m_elapsedTimes()
{
}

void engine::performance::FrameRateMonitor::SetMeasureRange(double range)
{
	m_measureRange = range;
}

double engine::performance::FrameRateMonitor::GetAverageFrameRate() const
{
	if (m_elapsedTimeAccumulator > 0.0f && !m_elapsedTimes.empty())
	{
		return static_cast<double>(m_elapsedTimes.size()) / m_elapsedTimeAccumulator;
	}
	return 0;
}

double engine::performance::FrameRateMonitor::GetLastFrameRate() const
{
	if (m_elapsedTimes.size() > 0)
	{
		double lastElapsed = m_elapsedTimes.back();
		if (lastElapsed > 0.0f)
		{
			return 1.0f / lastElapsed;
		}
	}
	return 0;
}

// using "On" prefix to follow event handler naming convention since this method is used as event handler - it is called when a frame is completed
void engine::performance::FrameRateMonitor::OnFrameCompleted(double elapsedTime)
{
	m_elapsedTimes.push_back(elapsedTime);
	m_elapsedTimeAccumulator += elapsedTime;

	while (!m_elapsedTimes.empty() && m_elapsedTimeAccumulator > m_measureRange)
	{
		double front = m_elapsedTimes.front();
		m_elapsedTimes.pop_front();
		m_elapsedTimeAccumulator -= front;

		// guard against negative accumulator due to floating point precision issues. make sure it stays zero or positive
		if (m_elapsedTimeAccumulator < 0.0f) m_elapsedTimeAccumulator = 0.0f;
	}
}