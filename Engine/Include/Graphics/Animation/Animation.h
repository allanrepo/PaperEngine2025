#pragma once
#include <Core/Event.h>
#include <Utilities/Logger.h>
#include <Timer/StopWatch.h>
#include <vector>
#include <memory>
#include <cassert>

namespace graphics::animation
{
	// represents a single frame in an animation sequence.
	// holds the payload element T and the duration to display this frame.
	template<typename T>
	struct Frame
	{
		T element;
		float duration;
	};

	// represents an animation sequence composed of multiple frames.
	template<typename T>
	struct Animation
	{
		std::string name;
		std::vector<Frame<T>> frames;
		bool loop = false;
	};

	// drives an Animation<T> forward over time.
	// handles frame progression, looping, and event notifications.
	// param T The type of the payload stored in each frame.  
	template<typename T>
	class Animator
	{
	private:
		bool m_running = false;					// whether the animation is currently playing.
		float m_elapsedTimeAccumulator = 0.0;	// accumulated time since last frame change.
		Animation<T>* m_animation;				// pointer to the current animation being played.
		int m_currFrame = -1;					// index of the current frame in the animation.

	public:
		// event fired when the frame changes. provides the new frame index and frame data.
		event::Event<int, graphics::animation::Frame<T>> OnFrame;

		// event fired when the animation ends.
		event::Event<> OnEnd;

		// event fired when a new animation starts playing. provides pointer to the animation.
		event::Event<graphics::animation::Animation<T>*> OnPlay;

		// constructs a new Animator instance with no animation playing.
		Animator():
			m_running(false),
			m_elapsedTimeAccumulator(0.0),
			m_currFrame(-1),
			m_animation(nullptr)
		{
		}

		// starts playing the specified animation from the beginning.
		void Play(graphics::animation::Animation<T>* animation) noexcept
		{
			// guard against invalid animations
			if (!animation || animation->frames.empty())
			{
				return;
			}
				
			// validate animation
			m_animation = animation;

			// if animation is valid and we can store it
			m_running = true;

			// reset to first frame
			m_currFrame = 0;

			OnPlay(m_animation);

			// trigger first frame event
			OnFrame(m_currFrame, m_animation->frames[m_currFrame]);
		}
		 
		// updates the animator by the specified delta time.
		void Update(float delta) noexcept
		{
			// do nothing if not running or no animation assigned
			if (!m_running || !m_animation)
			{
				return;
			}

			// accumulate elapsed time
			m_elapsedTimeAccumulator += delta;

			// handle first frame event
			assert(m_currFrame < m_animation->frames.size());
			float currFrameDuration = m_animation->frames[m_currFrame].duration;

			while (m_elapsedTimeAccumulator >= currFrameDuration)
			{
				m_elapsedTimeAccumulator -= currFrameDuration;
				m_currFrame++;

				if (m_currFrame >= m_animation->frames.size())
				{
					if (m_animation->loop)
					{
						// loop back to start frame
						m_currFrame = 0;
					}
					else
					{
						m_running = false;
						// clamp to max valid range so if it gets queried, it will return last valid frame
						m_currFrame = (int)m_animation->frames.size() - 1;

						// we're likely to fire this more than once if m_elapsedTimeAccumulator is large so we break after
						OnEnd();
						break;
					}
				}

				OnFrame(m_currFrame, m_animation->frames[m_currFrame]);
				currFrameDuration = m_animation->frames[m_currFrame].duration;
			}
		}

		graphics::animation::Frame<T>* GetCurrentFrame() const noexcept
		{
			return m_animation? &m_animation->frames[m_currFrame] : nullptr;
		}

		const int GetCurrentFrameIndex() const noexcept
		{
			return m_currFrame;
		}

		bool IsFinished()
		{
			return (m_currFrame == m_animation->frames.size() - 1) && !m_animation->loop;
		}

		Animation<T>* GetCurrentAnimation()
		{
			return m_animation;
		}

	};


	namespace Test
	{
		class TestClass
		{
		private:
			timer::StopWatch stopwatch;
			animation::Animator<int> animator;
			animation::Animation<int> animation;

			// listener to game loop's interval (triggered by stopwatch' Lap)
			void OnLoop(float delta)
			{
				animator.Update(delta);
			}

			void OnFrame(int index, graphics::animation::Frame<int> frame)
			{
				//LOG("[" << std::to_string(stopwatch.Peek<timer::milliseconds>()) << "] <" << std::to_string(index) << 
				//	">  index: " << std::to_string(frame.index) << " duration: " << std::to_string(frame.duration));
			}
			void OnEnd()
			{
				LOG("Last frame happened");
			}

		public:
			// on constructor, build the two timers - one is persistent, another is one-shot
			TestClass() 
			{
				animation =
				{
					"test",
					{
						{1, 1000},
						{4, 1500},
						{3, 2000},
						{6, 1000},
						{2, 2500},
						{1, 1000},
						{0, 1500},
					},
					true // loop
				};
				animator.Play(&animation);

				animator.OnFrame += event::Handler(this, &TestClass::OnFrame);

				// listen to stopwatch' Lap(). timers will be updated by this event listener
				stopwatch.OnLap += event::Handler(this, &TestClass::OnLoop);

				// start the stopwatch
				stopwatch.Start();

				// simulate game loop
				while (true)
				{
					// measure elapsed time per loop. this will fire up OnLap event
					stopwatch.Lap<timer::milliseconds>();
				}
			}

		};

		static void Test()
		{
			TestClass test;
		}
	}
}
