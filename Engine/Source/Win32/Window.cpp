#include <Win32/Window.h>
#include <windowsx.h>

std::unordered_map<std::wstring, int> engine::win32::Window::WindowClassManager::s_mapRefCount;

engine::event::Event<> engine::win32::Window::OnInitialize;
engine::event::Event<> engine::win32::Window::OnIdle;
engine::event::Event<> engine::win32::Window::OnExit;

void engine::win32::Window::Run()
{
	OnInitialize();

	MSG msg;
	bool keepGoing = true;

	while (keepGoing)
	{
		// 1) Drain every pending message
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				keepGoing = false;
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// 2) Now that all messages are handled, run one frame
		if (keepGoing)
			OnIdle();
	}

	OnExit();
}

bool engine::win32::Window::WindowClassManager::Register(HINSTANCE hInstance, const std::wstring& wszClassName, WNDCLASSEXW& wcex)
{
	if (s_mapRefCount[wszClassName]++ == 0)
	{
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.hInstance = hInstance;
		wcex.lpszClassName = wszClassName.c_str();
		if (!RegisterClassExW(&wcex))
		{
			s_mapRefCount[wszClassName]--;
			return false;
		}
	}
	return true;
}

void engine::win32::Window::WindowClassManager::Unregister(HINSTANCE hInstance, const std::wstring& wszClassName)
{
	auto it = s_mapRefCount.find(wszClassName);
	if (it != s_mapRefCount.end() && --it->second <= 0)
	{
		UnregisterClassW(wszClassName.c_str(), hInstance);
		s_mapRefCount.erase(it);
	}
}

LRESULT engine::win32::Window::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	OnWindowMessage(uMsg, wParam, lParam);

	switch (uMsg)
	{
	case WM_DESTROY:
	{
		OnDestroy();

		// if there are no more windows running, call this...
		if (s_nRefCount == 0)
		{
			PostQuitMessage(0);
		}

		break;
	}
	// fires up when window is closed.
	case WM_CLOSE:
	{
		s_nRefCount--;
		OnClose();
		Close();
		break;
	}
	case WM_CREATE:
	{
		OnCreate(hWnd);
		s_nRefCount++;
		break;
	}
	case WM_SIZE:
	{
		OnSize(LOWORD(lParam), HIWORD(lParam)); // width, height
		break;
	}
	case WM_MOUSEMOVE:
	{
		OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); // x, y
		break;
	}
	case WM_XBUTTONDOWN:
	case WM_MBUTTONDOWN:
	{
		break;
	}
	case WM_RBUTTONDOWN:
	{
		OnRightClick();
		OnRightMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); // x, y
		break;
	}
	case WM_LBUTTONDOWN:
	{
		SetCapture(hWnd);
		OnLeftClick();
		OnLeftMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); // x, y
		break;
	}
	case WM_XBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	{
		OnRightMouseUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); // x, y
		break;
	}
	case WM_LBUTTONUP:
	{
		OnLeftMouseUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); // x, y
		ReleaseCapture();
		break;
	}
	case WM_SHOWWINDOW:
	{
		OnShow();
		break;
	}

	case WM_CHAR:
	{
		OnChar(LOWORD(wParam));
		break;
	}
	case WM_KEYDOWN:
	{
		OnKeyDown(LOWORD(wParam));
		break;
	}
	default: return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	// If something was not done, let it go
	return 0;
}

bool engine::win32::Window::Register(WNDCLASSEXW& wcex)
{
	// register window class safely. this helper will register only if there is no window using the same class name yet
	if (!WindowClassManager::Register(m_hInstance, m_wszClassName, wcex))
	{
		MessageBox(nullptr, L"Failed to register window class!", nullptr, MB_OK | MB_ICONERROR);
		return false;
	}
	return true;
}

void engine::win32::Window::Unregister()
{
	WindowClassManager::Unregister(m_hInstance, m_wszClassName);
}

int engine::win32::Window::s_nRefCount = 0;

engine::win32::Window::Window(const std::wstring& wszClassName)
	:WindowBase(wszClassName)
{
}

