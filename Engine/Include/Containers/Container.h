#pragma once
#include <vector>

namespace engine::container
{
	template<typename T>
	class IContainer
	{
	public:

		virtual void Add(const T& data) = 0;

		virtual void Take(T&& data) = 0;

		virtual void AddRange(const std::vector<T>& data) = 0;

		virtual void TakeRange(std::vector<T>&& data) = 0;

		virtual void Pop() = 0;

		virtual const T& Get(size_t index) const = 0;

		virtual T& Get(size_t index) = 0;

		virtual void Reserve(const math::Size<size_t>& size) = 0;

		virtual size_t GetElementCount() const = 0;

		virtual bool IsEmpty() const = 0;

		virtual void Clear() = 0;

		virtual bool IsInBounds(const size_t index) const = 0;

		virtual T& Back() = 0;

		virtual const T& Back() const = 0;
	};
}
