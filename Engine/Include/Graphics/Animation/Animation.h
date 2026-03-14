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

namespace engine::graphics::animation
{
	template<typename T, typename Owner>
	class AnimationSystem;

	template<typename T, typename Owner>
	class AnimationController;

	template<typename T, typename Owner>
	class Animated;

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

		// Non-copyable, but movable
		Animator(const Animator&) = delete;
		Animator& operator=(const Animator&) = delete;
		Animator(Animator&&) = default;
		Animator& operator=(Animator&&) = default;

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
			float currFrameDuration = m_animation->frames[m_currFrame].duration;

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
			if (!IsRunning())
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
		bool Has(const std::string& name) const
		{
			return m_registry.Has(name);
		}

		Animation<T>& Get(const std::string& name) const
		{
			return *m_registry.Get(name).get();
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
	template<typename T, typename Owner>
	class AnimationController
	{
	private:
		using AnimSet = AnimationSet<T>;

		friend class AnimationSystem<T, Owner>;

		Animator<T> m_animator;
		core::View<AnimSet> m_handle;
		Owner* m_owner;

		// event handler for our Animator's EndEvent so we can emit it as well
		void OnAnimatorEnd(Animator<T>& animator)
		{
			EndEvent(*m_owner);
		}
		
		// event fired up when this is destroyed. this is exclusive to AnimationSystem so AnimationSystem can perform cleanup
		// the object itself is passed so whoever handles this event can queue it for deletion
		engine::event::Event<AnimationController&> DestroyEvent;

	public: 
		AnimationController(const AnimSet* set, Owner* owner = nullptr) :
			m_handle(set),
			m_owner(owner)
		{
			m_animator.EndEvent += engine::event::Handler(this, &AnimationController::OnAnimatorEnd);
		}

		AnimationController(const engine::core::View<AnimSet>& set, Owner* owner = nullptr) :
			m_handle(set),
			m_owner(owner)
		{
			m_animator.EndEvent += engine::event::Handler(this, &AnimationController::OnAnimatorEnd);
		}

		~AnimationController()
		{
			m_animator.EndEvent -= engine::event::Handler(this, &AnimationController::OnAnimatorEnd);
		}

		// Non-copyable, but movable
		AnimationController(const AnimationController&) = delete;
		AnimationController& operator=(const AnimationController&) = delete;
		AnimationController(AnimationController&&) noexcept = default;
		AnimationController& operator=(AnimationController&&) noexcept = default;

		// chains end animation sequence event from Animator
		engine::event::Event<Owner&> EndEvent;

		bool Play(const std::string& key)
		{
			if (!m_handle.IsValid()) return false;

			if (m_handle->Has(key))
			{
				m_animator.Play(m_handle->Get(key));
				return true;
			}

			return false;
		}

		void Play(const std::string& name, int loopCount)
		{
			Play(name);
			SetPolicy(PlaybackPolicy::FiniteLoop, loopCount);
		}

		void SetOwner(Owner* owner)
		{
			m_owner = owner;
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

		// decommisions the object. it will sever its connection with animation set and stop the animator. it will also call its destroy event
		void Destroy()
		{
			// stop the animator
			m_animator.Clear();

			// sever from animation set
			m_handle.Invalidate();

			// invoke destroy event
			DestroyEvent(*this);
		}

		void SetPolicy(PlaybackPolicy policy = PlaybackPolicy::Default, int loopCount = -1)
		{
			m_animator.SetPolicy(policy, loopCount);
		}

		bool IsRunning() const
		{
			return m_animator.IsRunning();
		}

		Animated<T, Owner> MakeAnimated()
		{
			return Animated(this);
		}
	};

	template<typename T, typename Owner>
	class Animated
	{
		using AnimationController = engine::graphics::animation::AnimationController<T, Owner>;

	private:
		engine::core::Handle<AnimationController> m_handle;
	public:
		Animated(AnimationController* data = nullptr) :
			m_handle(data)
		{
		}

		// do we have valid animation controller? is the animation controller running?
		bool IsValid() const
		{
			return m_handle.IsValid() && m_handle->IsValid();
		}

		void Update(double delta)
		{
			m_handle->Update(delta);
		}

		void Play(const std::string& name)
		{
			m_handle->Play(name);
		}

		void Play(const std::string& name, int loopCount)
		{
			m_handle->Play(name);
			m_handle->SetPolicy(PlaybackPolicy::FiniteLoop, loopCount);
		}

		const T& GetCurrent() const
		{
			return m_handle->GetCurrent();
		}

		void SetPolicy(PlaybackPolicy policy = PlaybackPolicy::Default, int loopCount = -1)
		{
			m_handle->SetPolicy(policy, loopCount);
		}

		void Destroy()
		{
			m_handle->Destroy();
			m_handle.Invalidate();
		}

		// equality operators. compare object's handle
		bool operator==(const Animated& rhs) const 
		{
			return m_handle == rhs.m_handle;
		}

		bool operator!=(const Animated& rhs) const
		{
			return !(*this == rhs);
		}

		engine::event::Event<Owner&>& GetEndEvent() 
		{
			return m_handle->EndEvent;
		}

		void SetOwner(Owner* owner)
		{
			m_handle->SetOwner(owner);
		}
	};

	template<typename T, typename Owner>
	class AnimationSystem
	{
	private:
		using Animator = engine::graphics::animation::Animator<T>;
		using AnimationController = engine::graphics::animation::AnimationController<T, Owner>;
		using AnimationSet = engine::graphics::animation::AnimationSet<T>;
		using Dictionary = engine::container::Dictionary <std::string, std::unique_ptr<AnimationController>>;

	protected:
		std::vector<std::unique_ptr<AnimationController>> m_animationControllers;
		engine::core::View<AnimationSet> m_animationSetHandle;
		std::unordered_set<AnimationController*> m_pendingDestroy;

	public:
		AnimationSystem(AnimationSet* set) :
			m_animationSetHandle(set)
		{
		}

		~AnimationSystem() = default;

		// Non-copyable, but movable
		AnimationSystem(const AnimationSystem&) = delete;
		AnimationSystem& operator=(const AnimationSystem&) = delete;
		AnimationSystem(AnimationSystem&&) noexcept = default;
		AnimationSystem& operator=(AnimationSystem&&) noexcept = default;

		void OnDestroy(AnimationController& ac)
		{
			m_pendingDestroy.emplace(&ac);
		}

		void Update(double delta)
		{
			for (auto& controller : m_animationControllers)
			{
				controller->Update(delta);
			}
		}

		void Flush()
		{
			// use explicit iterator type, not auto
			typename std::vector<std::unique_ptr<AnimationController>>::iterator it = m_animationControllers.begin();
			while (it != m_animationControllers.end())
			{
				// check if this controller is in the pending destroy set
				if (m_pendingDestroy.count(it->get()) > 0)
				{
					// erase returns the next valid iterator
					it = m_animationControllers.erase(it);
				}
				else
				{
					++it; // advance normally
				}
			}

			m_pendingDestroy.clear();
		}

		Animated<T, Owner> MakeAnimated(Owner* owner = nullptr)
		{
			// create animation controller
			m_animationControllers.push_back(std::make_unique<AnimationController>(m_animationSetHandle, owner));

			// subscribe to this animation controller's destroy event so we will actually delete this object
			m_animationControllers.back()->DestroyEvent += engine::event::Handler(this, &AnimationSystem::OnDestroy);

			// finally create Animated - handle to animation controller
			return m_animationControllers.back()->MakeAnimated();
		}

		// mainly for debug purpose. it creates a handle to controller that already exists. but it does not check if index is valid so beware
		Animated<T, Owner> GetAnimated(int i)
		{
			return m_animationControllers[i]->MakeAnimated();
		}

		size_t GetAnimationControllerCount() const
		{
			return m_animationControllers.size();
		}
	};

	// TODO: deprecated. AnimationController should be used now
	//template<typename T>
	//class AnimationManager
	//{
	//private:
	//	Animator<T>* m_animator; // playback engine (not owned)
	//	std::unordered_map<std::string, Animation<T>*> m_animations; // available animations

	//public:
	//	AnimationManager(Animator<T>* animator = nullptr) :
	//		m_animator(animator)
	//	{
	//	}

	//	void Set(Animator<T>& animator)
	//	{
	//		m_animator = &animator;
	//	}

	//	void Add(const std::string& key, Animation<T>& anim)
	//	{
	//		m_animations[key] = &anim;
	//	}

	//	bool Remove(const std::string& key)
	//	{
	//		auto it = m_animations.find(key);
	//		if (it != m_animations.end())
	//		{
	//			m_animations.erase(it);
	//			return true;
	//		}
	//		return false;
	//	}

	//	bool Play(const std::string& key) const
	//	{
	//		auto it = m_animations.find(key);
	//		if (it != m_animations.end())
	//		{
	//			m_animator->Play(*it->second);
	//			return true;
	//		}
	//		return false;
	//	}

	//	void Update(double delta)
	//	{
	//		m_animator->Update(delta);
	//	}
	//};
}
