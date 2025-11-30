#pragma once

#include <Windows.h>
#include "IWindow.h"

namespace Win32
{
	class WindowBase : public IWindow
	{
	private:
		static	LRESULT CALLBACK staticWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	protected:
		HWND m_hWnd;
		HINSTANCE m_hInstance;
		std::wstring m_wszClassName;

	protected:
		virtual bool Register(WNDCLASSEXW& wcex);
		virtual void Unregister();
		virtual LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual void OnIdle();

	public:
		WindowBase(const std::wstring& wszClassName = L"WindowClass");
		virtual ~WindowBase();

		virtual bool Create(const std::wstring& wszWindowTitle = L"Window Title", const int width = 800, const int height = 600) override final;
		virtual void Run() override;

		virtual void Quit() override final;
		virtual void Close() override final;

		virtual void SetClientSize(int width, int height) final;
	};

}



