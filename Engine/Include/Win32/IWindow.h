#pragma once

#include <iostream>

namespace engine::win32
{
	class IWindow
	{
	public:
		IWindow() {};
		virtual ~IWindow() {};
		virtual bool Create(const std::wstring& wszWindowTitle, const int width, const int height) = 0;
		virtual void Run() = 0;

		virtual void Quit() = 0;
		virtual void Close() = 0;

		virtual void SetClientSize(int width, int height) = 0;
	};
}



