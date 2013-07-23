// simpleWindow.cpp
// Authored by Alex Shows
// Released under the MIT License (http://opensource.org/licenses/mit-license.php) 
//
// Implementation of the CSimpleWindow class 
// An abstract class which encapsulates Win32 window creation

#include "simpleWindow.h"

CSimpleWindow::CSimpleWindow()
{
	appName = g_AppName;
	hWnd = NULL;
	width = SIMWIN_DEFAULT_WIDTH;
	height = SIMWIN_DEFAULT_HEIGHT;
}

CSimpleWindow::CSimpleWindow(const int w, const int h)
{
	appName = g_AppName;
	hWnd = NULL;
	width = w;
	height = h;
}

CSimpleWindow::~CSimpleWindow(void)
{
	SendMessage(hWnd, WM_CLOSE, NULL, NULL);
}

void CSimpleWindow::createWindow()
{
	WNDCLASSEX wcex;

	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= &staticWindowProc; // The static function calls the child's window proc
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= GetModuleHandle(NULL);
	wcex.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= appName.c_str();
	wcex.hIconSm		= LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&wcex);

	hWnd = CreateWindow(appName.c_str(), appName.c_str(), 
						WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, width, height, 
						NULL, NULL, GetModuleHandle(NULL), (LPVOID)this);

	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
}

void CSimpleWindow::messageLoop()
{
	MSG msg = {0};

	while(GetMessage(&msg, NULL, 0, 0))
	{
		if(msg.message == WM_QUIT)
		{
			break;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

// The static version of the window proc function which is provided to window class registration
// during window creation
// 
// The function must be static for window class registration, but that prevents access to other 
// functions of this class from within this function
// 
// So, we use a trick with the Set/GetWindowLongPtr
// When this static proc is called as part of window creation (WM_CREATE), we pass a pointer
// to this class in as the GWL_USERDATA for the window (this isn't secure)
//
// Then, subsequent calls into this static window proc will get the pointer back into the class
// and use that pointer to call the window proc of the child class
// 
// NOTE: This is not secure as other applications can fetch this pointer, and there is a bit 
//		of performance overhead associated with constantly getting this pointer over and over
//		every time a message is dispatched
//
// But...it works
LRESULT CALLBACK CSimpleWindow::staticWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CSimpleWindow *thisSimWin;
	if(uMsg == WM_CREATE)
	{
		thisSimWin = (CSimpleWindow*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)thisSimWin);
	}
	else
		thisSimWin = (CSimpleWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if(thisSimWin == NULL)
		return DefWindowProc(hWnd, uMsg, wParam, lParam);

	return thisSimWin->windowProc(hWnd, uMsg, wParam, lParam);
}

// This is an implementation of the virtual window proc function
// It won't ever be called as this is an abstract class
// Instead, the child class' implementation will be called
LRESULT CALLBACK CSimpleWindow::windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}