#pragma once
#include <Spatial/Size.h>
#include <Spatial/Position.h>

namespace math
{
	namespace geometry
	{
		template<typename T>
		struct Rect
		{
			T left, top, right, bottom;

			bool Overlaps(const Rect<T>& other) const
			{
				if (other.left > right) return false;
				if (other.right < left) return false;
				if (other.top > bottom) return false;
				if (other.bottom < top) return false;
				return true;
			}

			const bool Contains(const spatial::Position<T> pos) const
			{
				if (pos.x > right) return false;
				if (pos.x < left) return false;
				if (pos.y > bottom) return false;
				if (pos.y < top) return false;
				return true;
			}

			const Rect<T> Translate(const spatial::Position<T> pos) const
			{
				Rect<T> result{};

				result.left = left + pos.x;
				result.top = top + pos.y;
				result.right = right + pos.x;
				result.bottom = bottom + pos.y;
				return result;
			}

			const T GetWidth() const
			{
				return right - left;
			}

			const T GetHeight() const
			{
				return bottom - top;
			}

			const spatial::Size<T> GetSize() const
			{
				return spatial::Size<T>{ GetWidth(), GetHeight() };
			}

			const spatial::Position<T> GetCenter() const
			{
				return spatial::Position<T>{ (left + right) / 2, (top + bottom) / 2 };
			}

			const spatial::Position<T> GetTopLeft() const
			{
				return spatial::Position<T>{ left, top };
			}

			const spatial::Position<T> GetBottomRight() const
			{
				return spatial::Position<T>{ right, bottom };
			}

			const spatial::Position<T> GetTopRight() const
			{
				return spatial::Position<T>{ right, top };
			}

			const spatial::Position<T> GetBottomLeft() const
			{
				return spatial::Position<T>{ left, bottom };
			}

			void Inflate(const T dx, const T dy)
			{
				left -= dx;
				top -= dy;
				right += dx;
				bottom += dy;
			}

		};

		using RectF = math::geometry::Rect<float>;
	}
}
