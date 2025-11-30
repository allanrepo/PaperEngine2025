#pragma once

#include <Win32/WindowBase.h>
#include <Core/Event.h>
#include <unordered_map>
#
namespace Win32
{
	class Window : public WindowBase
	{
	private:
		// static helper class to manage window class register. 
		class WindowClassManager
		{
		private:
			static std::unordered_map<std::wstring, int> s_mapRefCount;

		public:
			static bool Register(HINSTANCE hInstance, const std::wstring& wszClassName, WNDCLASSEXW& wcex);
			static void Unregister(HINSTANCE hInstance, const std::wstring& wszClassName);
		};

		static int s_nRefCount;

	private:
		// NOTE: not really needed to be exported because it's private anyway. it will never be called by client so no need for implementation
		virtual LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override final;

		virtual bool Register(WNDCLASSEXW& wcex) override final;
		virtual void Unregister() override final;

	public:
		Window(const std::wstring& wszClassName = L"WindowClass");

		event::Event<UINT, WPARAM, LPARAM> OnWindowMessage;
		event::Event<size_t, size_t> OnSize;
		event::Event<> OnClose;
		event::Event<> OnDestroy;
		event::Event<void*> OnCreate;
		event::Event<int> OnKeyDown;
		event::Event<int> OnChar;
		event::Event<int, int> OnMouseMove;
		event::Event<> OnLeftClick;
		event::Event<> OnRightClick;
		event::Event<int, int> OnLeftMouseDown;
		event::Event<int, int> OnLeftMouseUp;
		event::Event<int, int> OnRightMouseDown;
		event::Event<int, int> OnRightMouseUp;
		event::Event<> OnShow;

		static void Run();
		static event::Event<> OnInitialize;
		static event::Event<> OnIdle;
		static event::Event<> OnExit;
	};
};
