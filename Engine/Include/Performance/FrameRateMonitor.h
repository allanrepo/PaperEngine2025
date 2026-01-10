#pragma once
#include <deque>

namespace performance
{
	// design consideration
	//	- this will be used exclusively by the engine to monitor frame rate. 
	//	- engine will provide facade methods to query frame rate info and will be responsible to feed elapsed time and define unit resolution
	// 	- the unit resolution is arbitrary since the engine is the one providing elapsed time. it is up to the engine to interpret the unit correctly
	//	- this is why the query methods are named GetFrameRate instead of GetFPS
	// how it works
	//	- engine will provide elapsed time every frame via OnFrameCompleted method. so this method must be called every frame in application loop
	//	- the class will accumulate elapsed times until the total elapsed time exceeds the measure range
	//	- the measure range is a unit of time that defines how long the average frame rate is calculated. it can be set via SetMeasureRange method
	//	- once the accumulated elapsed time exceeds the measure range, the oldest elapsed times are removed until the total elapsed time is within the measure range
	//	- to achieve this, a deque is used to store elapsed times for easy removal from the front (oldest)
	//  - this enables efficient frame rate monitoring and management. no iteration over all frames is needed to calculate average frame rate
	// 	- the class provides methods to get average frame rate over the measure range and last frame rate
	// limitations
	// 	- if measure range is smaller than the elapsed time of a single frame, average frame rate will be zero until enough frames accumulate
	//	  to exceed the measure range. so if frame rate is fixed low (e.g. 1 FPS) and measure range is lower than that (e.g. 0.5 second), average frame rate will be zero
	class FrameRateMonitor
	{
	private:
		float m_measureRange;
		std::deque<float> m_elapsedTimes;
		float m_elapsedTimeAccumulator;

	public:
		FrameRateMonitor(float measureRange = 1.0f);

		void SetMeasureRange(float range);
		float GetAverageFrameRate() const;
		float GetLastFrameRate() const;

		// using "On" prefix to follow event handler naming convention since this method is used as event handler - it is called when a frame is completed
		void OnFrameCompleted(float elapsedTime);
	};
}