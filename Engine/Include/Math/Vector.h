#pragma once
#include <cmath>
#include <cassert>

namespace math
{
	template <typename T>
	struct Vector
	{
	//public:
		static constexpr float Epsilon = 1e-6f;

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

		Vector operator-() const { return Vector(-x, -y); }

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
	};


	template<typename T>
	using Vec = Vector<T>;

	using VecF = Vec<float>;
}
