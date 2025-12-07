#pragma once

namespace graphics
{
	template<typename T>
	struct Color
	{
		T red;
		T green;
		T blue;
		T alpha;
	};

	using ColorF = Color<float>;
}

