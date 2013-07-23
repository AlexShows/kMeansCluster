// simpleWindow.h
// Authored by Alex Shows
// Released under the MIT License (http://opensource.org/licenses/mit-license.php) 
// 
// Header declaring the CSimpleWindow class
// 
// An abstract class which encapsulates Win32 window creation
// 
// To use, create a child class which implements the window procedure: 
//		LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//
// The other basic functions for window creation are taken care of, and
// the child class' windowProc will be called by a static function in the
// parent class that is used when registering the class
//
// Also, declare and implement create_window() and message_looop() with 
// whatever permissions you need, though you can simply call the parent 
// class functions which will handle the implementation

#pragma once

#include <Windows.h>
#include <process.h>
#include <string>
#include <iostream>
using namespace std;

#define SIMWIN_DEFAULT_WIDTH 800
#define SIMWIN_DEFAULT_HEIGHT 600

static const char *g_AppName = "Default Window Name";

class CSimpleWindow
{
public:
	HWND hWnd;
	int width;
	int height;
	string appName;
	CSimpleWindow();
	CSimpleWindow(const int width, const int height);
	~CSimpleWindow(void);
	void createWindow();
	void messageLoop();
protected:
	// This is the static version of the windowProc that is used for class registration
	static LRESULT CALLBACK staticWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	// Children should create their own windowProc with this prototype
	virtual LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};