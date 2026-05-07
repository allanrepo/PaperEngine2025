#pragma once
#include <Spatial/Size.h>
#include <Spatial/Position.h>

namespace engine
{
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

				const bool Contains(const engine::spatial::Position<T> pos) const
				{
					if (pos.x > right) return false;
					if (pos.x < left) return false;
					if (pos.y > bottom) return false;
					if (pos.y < top) return false;
					return true;
				}

				const Rect<T> Translate(const engine::spatial::Position<T> pos) const
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

				const engine::spatial::Size<T> GetSize() const
				{
					return spatial::Size<T>{ GetWidth(), GetHeight() };
				}

				const engine::spatial::Position<T> GetCenter() const
				{
					return engine::spatial::Position<T>{ (left + right) / 2, (top + bottom) / 2 };
				}

				const engine::spatial::Position<T> GetTopLeft() const
				{
					return engine::spatial::Position<T>{ left, top };
				}

				const engine::spatial::Position<T> GetBottomRight() const
				{
					return engine::spatial::Position<T>{ right, bottom };
				}

				const engine::spatial::Position<T> GetTopRight() const
				{
					return engine::spatial::Position<T>{ right, top };
				}

				const engine::spatial::Position<T> GetBottomLeft() const
				{
					return engine::spatial::Position<T>{ left, bottom };
				}

				void Inflate(const T dx, const T dy)
				{
					left -= dx;
					top -= dy;
					right += dx;
					bottom += dy;
				}

				bool Intersects(const Rect<T>& other) const
				{
					return !(right <= other.left ||
						left >= other.right ||
						bottom <= other.top ||
						top >= other.bottom);
				}

				Rect<T> GetOverlap(const Rect<T>& other) const
				{
					Rect<T> r{};

					r.left = std::max<T>(left, other.left);
					r.top = std::max<T>(top, other.top);
					r.right = std::min<T>(right, other.right);
					r.bottom = std::min<T>(bottom, other.bottom);

					return r;
				}

				bool HasArea() const
				{
					return GetWidth() > 0 && GetHeight() > 0;
				}

			};

			using RectF = engine::math::geometry::Rect<float>;
		}
	}
}

namespace std
{
	// STL container classes that uses key for mapping e.g. std::unordered_map requires has function to map a key to a bucker quickly
	// if Rect<T> is used as key, the STL container will certainly look for a has function for it, and this is why we need to define one here
	template<typename T>
	struct hash<engine::math::geometry::Rect<T>>
	{
		size_t operator()(const engine::math::geometry::Rect<T>& r) const noexcept
		{
			size_t h1 = std::hash<T>{}(r.left);
			size_t h2 = std::hash<T>{}(r.top);
			size_t h3 = std::hash<T>{}(r.right);
			size_t h4 = std::hash<T>{}(r.bottom);

			return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
		}
	};
}
