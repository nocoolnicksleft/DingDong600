#include "stdafx.h"

#include "YAppWrite.h"

#include "YukiDrehGDICE.h"


YAppWrite::~YAppWrite()
{

}

YAppWrite::YAppWrite (HWND phWnd, HINSTANCE phInstance) : YApplication(phWnd, phInstance)
{

	hEditWnd = NULL;

}

bool YAppWrite::Init() 
{

	#define EDITID 1

	// Specify the window style for the edit control.
	DWORD dwStyle = WS_CHILD | WS_VISIBLE |  
					WS_BORDER | ES_LEFT | ES_MULTILINE | ES_NOHIDESEL | 
					ES_UPPERCASE ;  

	// Create the window for the edit control window.
	hEditWnd = CreateWindow (
					TEXT("edit"),   // Class name
					NULL,           // Window text
					dwStyle,        // Window style
					10,             // x-coordinate of the upper-left corner
					10,             // y-coordinate of the upper-left corner
					220,            // Width of the window for the edit control
					250,            // Height of the window for the edit control
					hWnd,           // Window handle to the parent window
					(HMENU) EDITID, // Control identifier
					hInstance,      // Instance handle
					NULL);          // Specify NULL for this parameter when 
									// you create a control

	if (hEditWnd) {

		EnableWindow(hEditWnd, FALSE);
		ShowWindow(hEditWnd, SW_HIDE);

	}
		
	return true;

}

bool YAppWrite::Fini() 
{

	if (hEditWnd) {
		DestroyWindow(hEditWnd);
	}

	return true;

}

bool YAppWrite::OnPaint(HDC dc)
{

	return true;

}

bool YAppWrite::OnFocus(bool haveFocus) 
{
	if (haveFocus) 
	{

		if (hEditWnd) {
			EnableWindow(hEditWnd, TRUE);
			ShowWindow(hEditWnd, SW_SHOW);
			SetFocus(hEditWnd);

		}

	} else {

		if (hEditWnd) {
			EnableWindow(hEditWnd, FALSE);
			ShowWindow(hEditWnd, SW_HIDE);
			SetFocus(hWnd);
		}

	}
	return true;
}


