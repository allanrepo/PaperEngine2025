#pragma once
#include <Containers/Dictionary.h>
#include <Core/Event.h>

namespace engine
{
	namespace algorithm
	{
		// a generic utility designed to resolve keys into values and trigger an event when a lookup succeeds.
		// it decouples the act of resolving from the action taken afterward, making it highly reusable across different subsystems of your engine(tilemaps, UI grids, dialogue systems, etc.).
		// Generic: Works with any coordinate or parameter type (T), any key type (K), and any value type (V).
		// Event‑driven: Fires a LookupEvent whenever a key is successfully resolved, passing along the coordinate / parameter and the resolved value.
		// Decoupled : Does not know about maps, tiles, or rendering — it only manages key / value pairs and coordinates.
		// 
		// Parameters:
		// T - represents the context parameter you want to pass along when a lookup succeeds.
		// K — The key type used for lookup (e.g., integers, strings, enums).
		// V — The value type associated with each key(e.g., props, buttons, resources).
		// So T is essentially the “where” or “context” of the lookup, while K is the “what” key, and V is the “resolved value.”
		template<typename T, typename K, typename V>
		class LookupResolver
		{
		private:
			engine::container::Dictionary<K, V> m_keyValues;

		public:
			LookupResolver()
			{
			}

			void Register(K key, V value)
			{
				m_keyValues[key] = value;
			}

			void Set(T param, K key)
			{
				if (!m_keyValues.Has(key)) return;

				LookupEvent(param, m_keyValues[key]);
			}

			engine::event::Event<T, V> LookupEvent;
		};
	}
}