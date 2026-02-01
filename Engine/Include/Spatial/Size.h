#pragma once

namespace spatial
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

	};

	using SizeF = Size<float>;
	using SizeI = Size<int>;
}
