#pragma once

namespace engine::math
{
	template<typename T>
	struct Size
	{
		T width, height;

		Size<T> operator+(const Size<T>& other) const
		{
			return { width + other.width, height + other.height };
		}

		Size<T> operator-(const Size<T>& other) const
		{
			return { width - other.width, height - other.height };
		}

		Size<T> operator*(T scalar) const
		{
			return { width * scalar, height * scalar };
		}

		Size<T> operator/(T scalar) const
		{
			return { width / scalar, height / scalar };
		}

		// equality 
		bool operator==(const Size<T>& other) const 
		{ 
			return width == other.width && height == other.height; 
		} 
		
		// inequality
		bool operator!=(const Size<T>& other) const 
		{ 
			return !(*this == other); 
		}

		Size():
			width(0),
			height(0)
		{

		}

		Size(const T& w, const T& h)
		{
			width = w;
			height = h;
		}

		template<typename U>
		Size<U> As() const
		{
			return {
				static_cast<U>(width),
				static_cast<U>(height)
			};
		}

		Size<T>& operator += (const Size<T>& rhs)
		{
			width += rhs.width;
			height += rhs.height;
			return *this;
		}

		Size<T>& operator -= (const Size<T>& rhs)
		{
			width -= rhs.width;
			height -= rhs.height;
			return *this;
		}

		Size<T>& operator *= (T scalar)
		{
			width *= scalar;
			height *= scalar;
			return *this;
		}

		Size<T>& operator /= (T scalar)
		{
			width /= scalar;
			height /= scalar;
			return *this;
		}
	};

	template<typename T>
	Size<T> operator*(T scalar, const Size<T>& size)
	{
		return { size.width * scalar, size.height * scalar };
	}

	using SizeF = Size<float>;
	using SizeI = Size<int>;
}
