#pragma once
#include <Core/Event.h>
#include <Core/View.h>
#include <Utilities/Logger.h>
#include <Timer/StopWatch.h>
#include <Containers/Dictionary.h>
#include <vector>
#include <memory>
#include <cassert>
#include <unordered_set>
#include <Core/Singleton.h>

namespace engine::graphics::animation
{
	template<typename T>
	class AnimationSystem;

	//template<typename T, typename Owner>
	//class AnimationController;

	template<typename T>
	class AnimationSystemCache;

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

	enum class PlaybackPolicy 
	{
		Loop,
		FiniteLoop,
		Default
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
		PlaybackPolicy m_policy = PlaybackPolicy::Loop; // playback policy
		int m_remainingLoops = -1;  // in case policy is finite loop
		bool m_isFinished = false; // if animation finishes, this is true

	public:
		// Events
		engine::event::Event<Animator&> EndEvent;

		// Non-copyable, non-movable
		Animator(const Animator&) = delete;
		Animator& operator=(const Animator&) = delete;
		Animator(Animator&&) = delete;
		Animator& operator=(Animator&&) = delete;

		Animator() :
			m_animation(nullptr),
			m_running(false),
			m_elapsedTimeAccumulator(0.0),
			m_currFrame(-1),
			m_policy(PlaybackPolicy::Default),
			m_remainingLoops(-1)
		{
		}

		~Animator() = default;

		void SetPolicy(PlaybackPolicy policy = PlaybackPolicy::Default, int loopCount = -1) 
		{
			m_policy = policy;
			m_remainingLoops = loopCount;
		}

		bool IsRunning() const 
		{
			return m_animation && !m_animation->frames.empty() && m_running;
		}

		void Update(double delta) noexcept 
		{
			if (!m_running || !m_animation) return;

			m_elapsedTimeAccumulator += delta;
			double currFrameDuration = m_animation->frames[m_currFrame].duration;

			while (m_elapsedTimeAccumulator >= currFrameDuration) 
			{
				m_elapsedTimeAccumulator -= currFrameDuration;
				m_currFrame++;

				if (m_currFrame >= m_animation->frames.size()) 
				{
					switch (m_policy)
					{
						// in default policy, we use the animation's condition
					case PlaybackPolicy::Default:
						if (m_animation->loop)
						{
							m_currFrame = 0;
						}
						else
						{
							m_running = false;
							// clamp to max valid range so if it gets queried, it will return last valid frame
							m_currFrame = (int)m_animation->frames.size() - 1;

							m_isFinished = true;

							// we're likely to fire this more than once if m_elapsedTimeAccumulator is large so we return after
							EndEvent(*this);

							return;
						}
						break;

					case PlaybackPolicy::Loop:
						m_currFrame = 0;
						break;

					case PlaybackPolicy::FiniteLoop:
						if (--m_remainingLoops > 0)
						{
							m_currFrame = 0;
						}
						else
						{
							m_running = false;
							m_currFrame = (int)m_animation->frames.size() - 1;
							m_isFinished = true;
							EndEvent(*this);
							return;
						}
						break;
					}
				}
				currFrameDuration = m_animation->frames[m_currFrame].duration;
			}
		}

		void Play(const Animation<T>& animation) noexcept 
		{
			// assign animation
			m_animation = &animation;

			// if animation is valid and we can store it
			m_running = true;

			// reset to first frame
			m_currFrame = 0;

			// reset finish flag
			m_isFinished = false;
		}

		const T& GetCurrent() const 
		{
			if(!m_animation || m_currFrame < 0 || static_cast<size_t>(m_currFrame) >= m_animation->frames.size())
			{
				throw std::runtime_error("No current frame available");
			}
			return m_animation->frames[m_currFrame].element;
		}

		int GetCurrentFrameIndex() const noexcept 
		{
			return m_currFrame; 
		}

		bool IsFinished() const
		{ 
			return m_isFinished; 
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
			m_isFinished = false;
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

		// none-copyable, non-movable. 
		// this is because AnimationController holds view of AnimationSet. moving AnimationSet will make its View stale.
		AnimationSet(const AnimationSet&) = delete;
		AnimationSet& operator=(const AnimationSet&) = delete;
		AnimationSet(AnimationSet&&) = delete;
		AnimationSet& operator=(AnimationSet&&) = delete;

		bool Register(const std::string& name, std::unique_ptr<Animation<T>> data)
		{
			return m_registry.Register(name, std::move(data));
		}

		bool Register(const std::string& name, const Animation<T>& data)
		{
			return m_registry.Register(name, std::make_unique<Animation<T>>(data));
		}
		bool Has(const std::string& name) const
		{
			return m_registry.Has(name);
		}

		Animation<T>& Get(const std::string& name) const
		{
			return *m_registry.Get(name).get();
		}

		// define iterator for our container
		using iterator = typename engine::container::Dictionary<std::string, std::unique_ptr<Animation<T>>>::iterator;
		using const_iterator = typename engine::container::Dictionary<std::string, std::unique_ptr<Animation<T>>>::const_iterator;

		// iterator access
		iterator begin() { return m_registry.begin(); }
		iterator end() { return m_registry.end(); }
		const_iterator begin() const { return m_registry.begin(); }
		const_iterator end() const { return m_registry.end(); }
		const_iterator cbegin() const { return m_registry.cbegin(); }
		const_iterator cend() const { return m_registry.cend(); }
	};

	template<typename T>
	class AnimationSystem
	{
	private:
		engine::container::Dictionary<std::string, std::unique_ptr<Animator<T>>> m_registry;

	public:
		AnimationSystem() 
		{
		}

		virtual ~AnimationSystem() = default;

		// Non-copyable, but movable
		AnimationSystem(const AnimationSystem&) = delete;
		AnimationSystem& operator=(const AnimationSystem&) = delete;
		AnimationSystem(AnimationSystem&&) noexcept = delete;
		AnimationSystem& operator=(AnimationSystem&&) noexcept = delete;

		void Update(double delta)
		{
			for (auto& [key, animator] : m_registry)
			{
				animator->Update(delta); // or whatever method Animator exposes
			}
		}

		size_t Size() const
		{
			return m_registry.Size();
		}

		bool Create(const std::string& name)
		{
			// if already exists, return false
			return m_registry.Register(name, std::make_unique<Animator<T>>());

		}

		bool Destroy(const std::string& name)
		{
			// if does not exist, return false
			return m_registry.Unregister(name);
		}

		AnimationController<T> MakeAnimationController(const std::string& name, const AnimationSet<T>& set)
		{			
			// do we have an animator with the given key? if not, let us make one.
			if (!m_registry.Has(name))
			{
				Create(name);
			}

			// get reference to the animator
			Animator<T>* animator = m_registry.Get(name).get();
						
			// create animator handle and return
			AnimationController<T> animCtrl(&set, animator);
			return animCtrl;
		}
	};

	template<typename T>
	class AnimationSystemCache : public AnimationSystem<T>,  public engine::core::Singleton<AnimationSystemCache<T>>
	{
		// Allow Singleton to construct the global instance
		friend class engine::core::Singleton<AnimationSystemCache<T>>;

	private:
		// Private ctor for singleton
		AnimationSystemCache() = default;

	public:
		// Non-copyable, non-movable
		AnimationSystemCache(const AnimationSystemCache&) = delete;
		AnimationSystemCache& operator=(const AnimationSystemCache&) = delete;
		AnimationSystemCache(AnimationSystemCache&&) noexcept = delete;
		AnimationSystemCache& operator=(AnimationSystemCache&&) noexcept = delete;
	};

	template<typename T>
	class AnimationController
	{
	private:
		core::Handle<Animator<T>> m_animator;
		core::View<AnimationSet<T>> m_set;

		friend class AnimationSystem<T>;

		AnimationController(const AnimationSet<T>* set, Animator<T>* animator) 
			: m_set(set)
			, m_animator(animator)
		{
		}

	public:
		~AnimationController()
		{
		}

		bool Play(const std::string& key)
		{
			if (!m_set.IsValid()) return false;

			if (m_set->Has(key))
			{
				m_animator->Play(m_set->Get(key));
				return true;
			}

			return false;
		}

		void Play(const std::string& name, int loopCount)
		{
			Play(name);
			SetPolicy(PlaybackPolicy::FiniteLoop, loopCount);
		}

		const T& GetCurrent() const
		{
			return m_animator->GetCurrent();
		}

		bool IsValid() const
		{
			return (m_set.IsValid() && m_animator.IsValid());
		}

		void SetPolicy(PlaybackPolicy policy = PlaybackPolicy::Default, int loopCount = -1)
		{
			m_animator->SetPolicy(policy, loopCount);
		}

		bool IsRunning() const
		{
			return m_animator->IsRunning();
		}

		static AnimationController MakeInvalid()
		{
			return AnimationController(nullptr, nullptr);
		}
	};

}
