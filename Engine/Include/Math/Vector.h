#pragma once
#include <cmath>
#include <cassert>
#include <functional>

namespace engine
{
	namespace math
	{
		template <typename T>
		struct Vector
		{
			//public:
			static constexpr T Epsilon = []() {
				if constexpr (std::is_same_v<T, double>)      return 1e-5;
				else if constexpr (std::is_same_v<T, float>)  return 1e-4f;
				else                                          return T{ 0 };
				}();

			T x;
			T y;

			// Constructors
			Vector() : x(0), y(0) {}
			Vector(T _x, T _y) : x(_x), y(_y) {}
			explicit Vector(T scalar) : x(scalar), y(scalar) {}
			~Vector() {}

			// Operator overloads
			Vector& operator =  (const Vector& rhs) = default;
			Vector& operator += (const Vector& rhs) { x += rhs.x; y += rhs.y; return *this; }
			Vector& operator -= (const Vector& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
			Vector& operator *= (T scalar) { x *= scalar; y *= scalar; return *this; }
			Vector& operator /= (T scalar) { x /= scalar; y /= scalar; return *this; }

			// Arithmetic operators
			friend Vector operator + (Vector lhs, const Vector& rhs) { return lhs += rhs; }
			friend Vector operator - (Vector lhs, const Vector& rhs) { return lhs -= rhs; }
			friend Vector operator * (Vector lhs, T scalar) { return lhs *= scalar; }
			friend Vector operator * (T scalar, Vector rhs) { return rhs *= scalar; }
			friend Vector operator / (Vector lhs, T scalar) { return lhs /= scalar; }

			// unary minus operator. negates the vector
			Vector operator-() const { return Vector(-x, -y); }

			// comparison operators 
			bool operator == (const Vector& rhs) const 
			{ 
				if constexpr (std::is_floating_point_v<T>)
				{
					return std::abs(x - rhs.x) <= static_cast<T>(Epsilon) &&
						std::abs(y - rhs.y) <= static_cast<T>(Epsilon);
				}
				else
				{
					return x == rhs.x && y == rhs.y;
				}
			}
			bool operator != (const Vector& rhs) const { return !(*this == rhs); }

			// Vector operations
			float Magnitude() const
			{
				return std::sqrt(x * x + y * y);
			}

			Vector Normalize(float tolerance = Epsilon) const
			{
				float mag = Magnitude();
				return (mag >= tolerance) ? (*this / mag) : Vector{ 0, 0 };
			}

			float Dot(const Vector& rhs) const
			{
				return x * rhs.x + y * rhs.y;
			}

			// 2D cross product returns scalar
			float Cross(const Vector& rhs) const
			{
				return x * rhs.y - y * rhs.x;
			}

			// Utility
			bool IsZero(float tolerance = Epsilon) const
			{
				return std::abs(x) < tolerance && std::abs(y) < tolerance;
			}

			constexpr Vector Scale(const Vector& rhs) const noexcept
			{
				return Vector
				{
					x * rhs.x,
					y * rhs.y
				};
			}

			template<typename U>
			Vector<U> As() const
			{
				return {
					static_cast<U>(x),
					static_cast<U>(y)
				};
			}
		};


		template<typename T>
		using Vec = Vector<T>;
		using VecF = Vec<float>;
	}
}

// STL container classes that uses key for mapping e.g. std::unordered_map requires has function to map a key to a bucker quickly
// if Vector<T> is used as key, the STL container will certainly look for a has function for it, and this is why we need to define one here
namespace std
{
	template <typename T>
	struct hash<engine::math::Vector<T>>
	{
		size_t operator()(const engine::math::Vector<T>& v) const noexcept
		{
			size_t h1 = std::hash<T>{}(v.x);
			size_t h2 = std::hash<T>{}(v.y);

			return h1 ^ (h2 << 1); // combine
		}
	};
}

