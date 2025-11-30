#include <Win32/WindowBase.h>

Win32::WindowBase::WindowBase(const std::wstring& wszClassName)
{
	m_hWnd = nullptr;
	m_hInstance = GetModuleHandle(NULL);
	m_wszClassName = wszClassName;
}

Win32::WindowBase::~WindowBase()
{
}

bool Win32::WindowBase::Create(const std::wstring& wszWindowTitle, const int width, const int height)
{
	// check if window class is already registered. if not, register it
	WNDCLASSEX wcex = {};
	if (!GetClassInfoEx(m_hInstance, m_wszClassName.c_str(), &wcex))
	{
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)GetStockObject(COLOR_WINDOW + 1); //(HBRUSH)GetStockObject(NULL_BRUSH);
		wcex.hIcon = LoadIcon(0, IDI_APPLICATION);
		wcex.hIconSm = LoadIcon(0, IDI_APPLICATION);
		wcex.lpszClassName = m_wszClassName.c_str();
		wcex.lpszMenuName = nullptr;
		wcex.hInstance = m_hInstance; // GetModuleHandle(NULL); 
		wcex.lpfnWndProc = staticWindowProc;

		// register window class
		if (!Register(wcex))
		{
			return false;
		}
	}

	// create window 
	m_hWnd = CreateWindowW(m_wszClassName.c_str(), wszWindowTitle.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, nullptr, nullptr, m_hInstance, (void*)this);
	if (!m_hWnd)
	{
		MessageBox(nullptr, L"Failed to create window!", nullptr, MB_OK | MB_ICONERROR);
		return false;
	}

	// show window
	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);
	return true;
}

void Win32::WindowBase::Run()
{
	// listen for message events
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			OnIdle();
		}
	}

}

void Win32::WindowBase::OnIdle()
{
}

bool Win32::WindowBase::Register(WNDCLASSEXW& wcex)
{
	if (!RegisterClassExW(&wcex))
	{
		MessageBox(nullptr, L"Failed to register window class!", nullptr, MB_OK | MB_ICONERROR);
		return false;
	}
	return true;
}

void Win32::WindowBase::Unregister()
{
	WNDCLASSEX wcex = {};
	if (GetClassInfoEx(m_hInstance, m_wszClassName.c_str(), &wcex))
	{
		UnregisterClass(m_wszClassName.c_str(), m_hInstance);
	}
}

/// <summary>
/// sends WM_QUIT message to the application that spawns this window
/// typically, the application's message loop handles WM_QUIT message to break off message loop and exit the application
/// </summary>
void Win32::WindowBase::Quit()
{
	PostQuitMessage(0);
}

/// <summary>
/// close this window thereby destroying it
/// </summary>
void Win32::WindowBase::Close()
{
	if (m_hWnd)
	{
		// this sends WM_DESTROY to window of specified handle
		DestroyWindow(m_hWnd);
		m_hWnd = nullptr;
		Unregister();
	}
}

void Win32::WindowBase::SetClientSize(int width, int height)
{
	RECT rc = { 0, 0, width, height };
	unsigned long dwStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	AdjustWindowRectEx(&rc, dwStyle, false, WS_EX_LEFT);
	width = rc.right - rc.left;
	height = rc.bottom - rc.top;
	GetWindowRect(m_hWnd, &rc);
	SetWindowPos(m_hWnd, HWND_NOTOPMOST, rc.left, rc.top, width, height, SWP_SHOWWINDOW);
}

LRESULT CALLBACK Win32::WindowBase::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
	{
		Close();
		break;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		break;
	}
	default: return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	// If something was not done, let it go
	return 0;
}

LRESULT CALLBACK Win32::WindowBase::staticWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WindowBase* pWnd;

	// if the window calling this procedure is about to be created, then set this window's long to pointer to itself so we can route messages to it later
	if (uMsg == WM_NCCREATE)
	{
		LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pWnd = static_cast<Win32::WindowBase*>(pCreateStruct->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
	}

	// Get the pointer to the window that call this procedure
	pWnd = reinterpret_cast<Win32::WindowBase*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	// if the window exists, call this window's specific message handler
	if (pWnd)
	{
		return pWnd->WindowProc(hWnd, uMsg, wParam, lParam);
	}
	// otherwise, default it
	else return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

