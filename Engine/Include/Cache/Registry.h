#pragma once
#include <Core/Singleton.h>
#include <Cache/Dictionary.h>

#include <unordered_map>
#include <memory>
#include <string>

namespace cache
{
	// TODO: add reload support
	// TODO:: add load if not found support, or fallback support

	// Description:
	// Repository<T> is a global resource registry keyed by string.
	// It owns objects of type T via std::unique_ptr and provides centralized
	// registration, lookup, and lifetime management.
	//
	// Singleton:
	// Implemented as a singleton so there is only one authoritative registry
	// per resource type. This ensures consistency across subsystems: when one
	// part of the engine registers or retrieves a resource, every other part
	// sees the same instance. You don’t need to pass references around —
	// Repository<T>::Instance() is always the single source of truth.
	//
	// Lifetime:
	// Because resources are stored as std::unique_ptr<T>, the repository
	// guarantees ownership and automatic cleanup. When you unregister or clear,
	// the objects are destroyed safely. This prevents dangling pointers and
	// ensures that resource lifetimes are tied directly to their presence in
	// the repository. In other words, the singleton repository is not just a
	// global catalog — it is also the lifetime manager for the resources it holds.
	template<typename T>
	class Registry : public core::Singleton<Registry<T>>
	{
		friend class core::Singleton<Registry<T>>;

	private:
		cache::Dictionary<std::string, std::unique_ptr<T>> registry;

	public:
		Registry() = default;

		bool Register(const std::string& key, std::unique_ptr<T> value)
		{
			return registry.Register(key, std::move(value));
		}

		T& Get(const std::string& key) 
		{
			return *registry.Get(key).get();
		}

		const T& Get(const std::string& key) const
		{
			return *registry.Get(key).get();
		}

		bool Has(const std::string& key) const
		{
			return registry.Has(key);
		}

		bool Unregister(const std::string& key)
		{
			return registry.Unregister(key);
		}

		void Clear()
		{
			registry.Clear();
		}

		// replace an existing resource with a new one
		bool Reload(const std::string& key, std::unique_ptr<T> newValue)
		{
			if (Has(key))
			{
				registry.Unregister(key); // destroy old resource
				return registry.Register(key, std::move(newValue));
			}
			return false; // nothing to reload
		}
	};
}
