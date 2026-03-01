#pragma once
#include <Spatial/ISizeable.h>
#include <Containers/Container.h>
#include <Spatial/Coord.h>

namespace engine::container
{
	template<typename T>
	class IGrid : public IContainer<T>, public spatial::ISizeable<size_t>
	{
		virtual T& Get(int row, int col) = 0;

		virtual const T& Get(int row, int col) const = 0;

		virtual void Set(int row, int col, const T& data) = 0;

		virtual void Set(const T& data) = 0;

		virtual bool IsInBounds(int row, int col) const = 0;

		virtual bool IsInBounds(const engine::spatial::Coord& coord) const = 0;

		virtual T& Get(const engine::spatial::Coord& coord) = 0;

		virtual const T& Get(const engine::spatial::Coord& coord) const = 0;

		virtual void Set(const engine::spatial::Coord& coord, const T& data) = 0;
	};
}
