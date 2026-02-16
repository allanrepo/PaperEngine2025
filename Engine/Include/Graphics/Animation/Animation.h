#pragma once
#include <Core/Event.h>
#include <Core/View.h>
#include <Utilities/Logger.h>
#include <Timer/StopWatch.h>
#include <vector>
#include <memory>
#include <cassert>

namespace engine::graphics::animation
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
		double m_elapsedTimeAccumulator = 0.0;	// accumulated time since last frame change.
		Animation<T>* m_animation;				// pointer to the current animation being played.
		int m_currFrame = -1;					// index of the current frame in the animation.
		std::unordered_map<std::string, engine::graphics::animation::Animation<T>> m_animations;

		// starts playing the specified animation from the beginning.
		void Play(engine::graphics::animation::Animation<T>* animation) noexcept
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

	public:
		// event fired when the frame changes. provides the new frame index and frame data.
		engine::event::Event<int, engine::graphics::animation::Frame<T>> OnFrame;

		// event fired when the animation ends.
		engine::event::Event<> OnEnd;

		// event fired when a new animation starts playing. provides pointer to the animation.
		engine::event::Event<engine::graphics::animation::Animation<T>*> OnPlay;

		// constructs a new Animator instance with no animation playing.
		Animator():
			m_running(false),
			m_elapsedTimeAccumulator(0.0),
			m_currFrame(-1),
			m_animation(nullptr)
		{
		}

		bool IsRunning() const
		{
			return m_animation && !m_animation->frames.empty() && m_running;
		}

		virtual ~Animator() = default;
				 
		// updates the animator by the specified delta time.
		void Update(double delta) noexcept
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

		const engine::graphics::animation::Frame<T>& GetCurrentFrame() const
		{			
			if (!IsRunning())
			{
				throw std::runtime_error("No current frame available");
			}
			//return m_animation? &m_animation->frames[m_currFrame] : nullptr;
			return m_animation->frames[m_currFrame];
		}

		const T& GetCurrent() const
		{
			if (!IsRunning())
			{
				throw std::runtime_error("No current frame available");
			}
			return m_animation->frames[m_currFrame].element;
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

		// Add a new animation by name 
		void Add(const std::string& name, const Animation<T>& anim) 
		{ 
			m_animations[name] = anim; 
		} 
		
		// Remove an animation by name 
		void Remove(const std::string& name) 
		{ 
			auto it = m_animations.find(name); 
			if (it != m_animations.end()) 
			{ 
				// If currently playing this animation, stop it 
				if (m_animation == &it->second) 
				{ 
					m_running = false; 
					m_animation = nullptr; 
					m_currFrame = -1; 
				} 
				m_animations.erase(it); 
			} 
		}

		// Set and play an animation by name 
		bool Play(const std::string& name) 
		{ 
			auto it = m_animations.find(name); 
			if (it == m_animations.end()) 
			{ 
				return false;
			} 

			Play(&it->second);
			return true;			
		}

		bool Has(const std::string& name) const
		{
			auto it = m_animations.find(name);
			return it != m_animations.end();
		}


		void Clear() noexcept 
		{
			m_running = false; 
			m_elapsedTimeAccumulator = 0.0; 
			m_animation = nullptr; 
			m_currFrame = -1; 
	
			// releases all stored animations 
			m_animations.clear();			
		}
	};


	namespace Test
	{
		class TestClass
		{
		private:
			engine::timer::StopWatch stopwatch;
			animation::Animator<int> animator;
			animation::Animation<int> animation;

			// listener to game loop's interval (triggered by stopwatch' Lap)
			void OnLoop(double delta)
			{
				animator.Update(delta);
			}

			void OnFrame(int index, engine::graphics::animation::Frame<int> frame)
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
				animator.Add("default", animation);
				animator.Play("default");

				animator.OnFrame += engine::event::Handler(this, &TestClass::OnFrame);

				// listen to stopwatch' Lap(). timers will be updated by this event listener
				stopwatch.OnLap += engine::event::Handler(this, &TestClass::OnLoop);

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
