#pragma once

#include <stdexcept>

namespace core
{
	// A lightweight, non-owning read-only view into an object of type T.
	// 
	// A View<T> behaves like a safe pointer that provides controlled access
	// to an existing resource without owning it. It is conceptually similar
	// to how a sprite is a view into a sprite atlas, or how std::string_view
	// provides a window into a string.
	//
	// Key characteristics:
	//   • Non-owning — it does not manage or extend the lifetime of the resource.
	//   • Read-only  — exposes const T* to prevent accidental modification.
	//   • Safe access — operator->() and operator*() validate the view.
	//   • Lightweight — only stores a pointer with no additional overhead.
	//   • Intentional — constructor is protected, so only trusted systems
	//                    can create valid views.
	//
	// A View is typically returned by engine systems (e.g., tilemaps, 
	// sprite atlases, scene graphs) to expose internal elements without
	// giving ownership or allowing mutation.
	template<typename T>
	class View
	{
	protected:
		const T* m_data;

		View(const T* data) :
			m_data(data)
		{
		}

	public:
		// check if view is valid
		inline bool isValid() const
		{
			return m_data != nullptr;
		}

		// pointer-like access
		inline const T* operator->() const
		{
			if (!isValid())
			{
				throw std::runtime_error("View<T>::operator-> - invalid view access");
			}
			return m_data;
		}

		// dereference access
		inline const T& operator*() const
		{
			if (!isValid())
			{
				throw std::runtime_error("View<T>::operator* - invalid view access");
			}
			return *m_data;
		}
	};
}

