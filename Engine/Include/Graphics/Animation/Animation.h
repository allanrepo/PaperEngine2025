#pragma once
#include <Core/Event.h>
#include <Core/View.h>
#include <Utilities/Logger.h>
#include <Timer/StopWatch.h>
#include <Containers/Dictionary.h>
#include <vector>
#include <memory>
#include <cassert>

namespace engine::graphics::animation
{
	template<typename T>
	class AnimationController;

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
		const Animation<T>* m_animation;				// pointer to the current animation being played.
		int m_currFrame = -1;					// index of the current frame in the animation.
		//std::unordered_map<std::string, engine::graphics::animation::Animation<T>> m_animations;

	public:
		// event fired when the frame changes. provides the new frame index and frame data.
		engine::event::Event<int, engine::graphics::animation::Frame<T>> OnFrame;

		// event fired when the animation ends.
		engine::event::Event<> OnEnd;

		// event fired when a new animation starts playing. provides pointer to the animation.
		engine::event::Event<const engine::graphics::animation::Animation<T>&> OnPlay;

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

		const Animation<T>* GetCurrentAnimation() const
		{
			return m_animation;
		}

		void Clear() noexcept 
		{
			m_running = false; 
			m_elapsedTimeAccumulator = 0.0; 
			m_animation = nullptr; 
			m_currFrame = -1; 
		}

		// starts playing the specified animation from the beginning.
		void Play(const engine::graphics::animation::Animation<T>& animation) noexcept
		{
			// validate animation
			m_animation = &animation;

			// if animation is valid and we can store it
			m_running = true;

			// reset to first frame
			m_currFrame = 0;

			OnPlay(*m_animation);

			// trigger first frame event
			OnFrame(m_currFrame, m_animation->frames[m_currFrame]);
		}
	};

	// TODO: deprecated. AnimationController should be used now
	template<typename T>
	class AnimationManager 
	{
	private:
		Animator<T>* m_animator; // playback engine (not owned)
		std::unordered_map<std::string, Animation<T>*> m_animations; // available animations

	public:
		AnimationManager(Animator<T>* animator = nullptr) :
			m_animator(animator) 
		{
		}

		void Set(Animator<T>& animator)
		{
			m_animator = &animator;
		}

		void Add(const std::string& key, Animation<T>& anim) 
		{
			m_animations[key] = &anim;
		}

		bool Remove(const std::string& key) 
		{ 
			auto it = m_animations.find(key);
			if (it != m_animations.end()) 
			{ 
				m_animations.erase(it);
				return true;
			} 
			return false;
		}

		bool Play(const std::string& key) const
		{
			auto it = m_animations.find(key);
			if (it != m_animations.end()) 
			{
				m_animator->Play(*it->second);
				return true;
			}
			return false;
		}

		void Update(double delta) 
		{
			m_animator->Update(delta);
		}
	};

	// container or cache for animation
	// frame definition is templated for flexibility
	// provides a factory for AnimationController, which is a handle for AnimationSet's animation collection
	template<typename T>
	class AnimationSet
	{
	protected:
		engine::container::Dictionary<std::string, std::unique_ptr<Animation<T>>> m_registry;

	public:
		AnimationSet() = default;
		~AnimationSet() = default;

		AnimationSet(const AnimationSet&) = default;
		AnimationSet& operator=(const AnimationSet&) = default;
		AnimationSet(AnimationSet&&) = default;
		AnimationSet& operator=(AnimationSet&&) = default;

		bool Register(const std::string& name, std::unique_ptr<Animation<T>> data)
		{
			return m_registry.Register(name, std::move(data));
		}

		bool Register(const std::string& name, const Animation<T>& data)
		{
			return m_registry.Register(name, std::make_unique<Animation<T>>(data));
		}
		bool IsValid(const std::string& name) const
		{
			return m_registry.Has(name);
		}

		// creates a prop instance for the given id. returns invalid prop if id not found
		AnimationController<T> MakeAnimationController()
		{
			return AnimationController<T>(&m_registry);
		}

		// define iterator for our container
		using iterator = typename engine::container::Dictionary<int, std::unique_ptr<Animation<T>>>::iterator;
		using const_iterator = typename engine::container::Dictionary<int, std::unique_ptr<Animation<T>>>::const_iterator;

		// iterator access
		iterator begin() { return m_registry.begin(); }
		iterator end() { return m_registry.end(); }
		const_iterator begin() const { return m_registry.begin(); }
		const_iterator end() const { return m_registry.end(); }
		const_iterator cbegin() const { return m_registry.cbegin(); }
		const_iterator cend() const { return m_registry.cend(); }
	};

	// lightweight class that provides handle to AnimationSet's animation collection
	// exclusively created by AnimationSet class as it is a handle to AnimationSet's internal animation collection
	// it is lightweight so can be copied or pass by value. 
	// works as animator component of an object. 
	// as animator component, it conveniently contains or have access to animation collection of the AnimationSet it is associated with
	template<typename T>
	class AnimationController
	{
	private:
		friend class AnimationSet<T>;
		using AnimSet = engine::container::Dictionary <std::string, std::unique_ptr <Animation<T>>>;

		Animator<T> m_animator;
		core::Handle<AnimSet> m_handle;

	protected:
		AnimationController(AnimSet* set = nullptr) :
			m_handle(set)
		{
		}

	public:
		bool Play(const std::string& key)
		{
			if (m_handle->Has(key))
			{
				m_animator.Play(*m_handle->Get(key).get());
				return true;
			}

			return false;
		}

		void Update(double delta)
		{
			m_animator.Update(delta);
		}

		const T& GetCurrent() const
		{
			return m_animator.GetCurrent();
		}

		bool IsValid() const
		{
			return m_animator.IsRunning();
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
				//animator.Add("default", animation);
				animator.Play(animation);

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
