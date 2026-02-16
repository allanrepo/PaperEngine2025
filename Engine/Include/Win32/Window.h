#pragma once

#include <Win32/WindowBase.h>
#include <Core/Event.h>
#include <unordered_map>
#
namespace engine::win32
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

		engine::event::Event<UINT, WPARAM, LPARAM> OnWindowMessage;
		engine::event::Event<size_t, size_t> OnSize;
		engine::event::Event<> OnClose;
		engine::event::Event<> OnDestroy;
		engine::event::Event<void*> OnCreate;
		engine::event::Event<int> OnKeyDown;
		engine::event::Event<int> OnChar;
		engine::event::Event<int, int> OnMouseMove;
		engine::event::Event<> OnLeftClick;
		engine::event::Event<> OnRightClick;
		engine::event::Event<int, int> OnLeftMouseDown;
		engine::event::Event<int, int> OnLeftMouseUp;
		engine::event::Event<int, int> OnRightMouseDown;
		engine::event::Event<int, int> OnRightMouseUp;
		engine::event::Event<> OnShow;

		static void Run();
		static engine::event::Event<> OnInitialize;
		static engine::event::Event<> OnIdle;
		static engine::event::Event<> OnExit;
	};
};
