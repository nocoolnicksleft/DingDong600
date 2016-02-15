// YukiDrehGDICE.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "YukiDrehGDICE.h"
#include <stdio.h> 

#include "winimage.h"
#include "alphabet.h"
#include <shellsdk.h>
#include "Commctrl.h."

#include "YApplication.h"
#include "YAppAlphabet.h"
#include "YAppNavigation.h"
#include "YAppHome.h"
#include "YAppVideo.h"
#include "YAppWrite.h"

#define MAX_LOADSTRING 100

// Global Variables:

HINSTANCE hInst;								// current instance

TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

#define TIMER_40MSEC 1
#define TIMER_40MSEC_COUNT 40

#define TIMER_400MSEC 2
#define TIMER_400MSEC_COUNT 400

UINT_PTR timer40msec = 0;
UINT_PTR timer400msec = 0;

#define MAX_ANI_WIDTH 100
#define MIN_ANI_WIDTH 1

#define MAX_ANI_HEIGHT 100
#define MIN_ANI_HEIGHT 1

int currentApplicationId = -1;	

YApplication * currentApplication = 0;
YAppHome * homeApplication = 0;
YAppAlphabet * alphabetApplication = 0;
YAppNavigation * navigationApplication = 0;
YAppWrite * writeApplication = 0;
YAppVideo * videoApplication = 0;



int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_YUKIDREHGDICE, szWindowClass, MAX_LOADSTRING);


	WNDCLASS wcex;

	// WNDCLASSEX wcex;
	// wcex.cbSize = sizeof(WNDCLASS);
    // wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_YUKIDREHGDI));
	wcex.hCursor		= 0;
	wcex.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;

	RegisterClass(&wcex);
	// RegisterClassEx(&wcex);

    HWND hWnd;

    hInst = hInstance; // Store instance handle in our global variable

    //hWnd = CreateWindowEx(0,szWindowClass, szTitle, WS_EX_OVERLAPPEDWINDOW,
      //CW_USEDEFAULT, 0, 240, 320, NULL, NULL, hInstance, NULL);

    hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE ,
      // CW_USEDEFAULT, 0, 240, 320, NULL, NULL, hInstance, NULL);
	  CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	// Perform application initialization:
    if (!hWnd)
    {

		return FALSE;

    } else {

		timer40msec = SetTimer ( hWnd, TIMER_40MSEC, TIMER_40MSEC_COUNT, NULL );

		timer400msec = SetTimer ( hWnd, TIMER_400MSEC, TIMER_400MSEC_COUNT, NULL );

		ShowWindow(hWnd, nCmdShow);

		UpdateWindow(hWnd);
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_YUKIDREHGDICE));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{

		if (msg.message == WM_CHAR) {

			if (towupper((TCHAR)msg.wParam) == 'Y') {

				msg.wParam = (WPARAM)'Z';

			} else if (towupper((TCHAR)msg.wParam) == 'Z') {

				msg.wParam = (WPARAM)'Y';

			}

		}

		//if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		if (!TranslateAccelerator(hWnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

/*
void strToUpper(TCHAR * in, TCHAR *out) {

	TCHAR * tin = in;
	TCHAR * tout = out;

	while (*tin) {
		*tout = towupper(*tin);
		tout++;
		tin++;
	}

	*tout = 0;

}
*/





RECT rectNavigation;
RECT rectContent;

bool SetApplication(int newApplicationId)
{

	if (newApplicationId != currentApplicationId) 
	{

		if (currentApplication) {

			currentApplication->OnFocus(false);

		}

		currentApplicationId = newApplicationId;
		

		switch (currentApplicationId)
		{

		case 0:
				currentApplication = homeApplication;
			break;

		case 1:
				currentApplication = alphabetApplication;
			break;

		case 2:
				currentApplication = videoApplication;
			break;

		case 3:
				currentApplication = writeApplication;
			break;


		}

		currentApplication->OnFocus(true);

		return true;

	}

	return false;

}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HBITMAP hBackgroundImage = 0;

	// Status Fullscreen
    static int fFullScreen = FALSE;

	// Handle of task bar window
	static HWND hWndTaskBar = 0;

	// Temporary Rect
    RECT rc;



	switch (message)
	{

	 case WM_CREATE:

	    homeApplication = new YAppHome(hWnd,hInst);
		homeApplication->Init();

	    alphabetApplication = new YAppAlphabet(hWnd,hInst);
		alphabetApplication->Init();

	    videoApplication = new YAppVideo(hWnd,hInst);
		videoApplication->Init();

	    writeApplication = new YAppWrite(hWnd,hInst);
		writeApplication->Init();

		navigationApplication = new YAppNavigation(hWnd,hInst);
		navigationApplication->Init();

		SetApplication(0);



		hBackgroundImage = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BM_BACKGROUND));

		RETAILMSG ((!hBackgroundImage),(TEXT("Error loading background image\r\n")) ) ;

		//for (i = 0; i < NUMBER_OF_LETTERS; i++) {	
		//	swprintf_s(letterText[i],2,L"%c",towupper((TCHAR)i));
		//}

		hWndTaskBar = FindWindow(TEXT("HHTaskBar"), TEXT(""));
		ShowWindow(hWndTaskBar, SW_HIDE);
		EnableWindow( hWndTaskBar, FALSE );

        SetRect(&rc, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
        MoveWindow(hWnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, TRUE);


		SetRect(&rectNavigation,0,NAVIGATION_TOP,239,320);
		SetRect(&rectContent,0,0,240,NAVIGATION_TOP);

		while (ShowCursor(false) >= 0);
		SetCursor(NULL);

		break;

	case WM_SIZE:
//			anim.Display (40,140);
		break;

	case WM_LBUTTONDOWN:

			int x;
			int y;

			x = LOWORD(lParam); 
			y = HIWORD(lParam); 

			if (navigationApplication->IsHit(x,y)) {

				if (navigationApplication->OnLButtonDown(x,y))
				{

					if (SetApplication(navigationApplication->getCurrentApp()))
					{

						InvalidateRect(hWnd,&rectContent,true);

					}

				}

			} else if (currentApplication->IsHit(x,y)) {

			}

			currentApplication->OnLButtonDown(x,y);

		break;

	case WM_LBUTTONUP:

			x = LOWORD(lParam); 
			y = HIWORD(lParam); 

			if (navigationApplication->IsHit(x,y)) {

				if (navigationApplication->OnLButtonUp(x,y)) 
				{

					return 0;

				}

			} else if (currentApplication->IsHit(x,y)) {
			
				currentApplication->OnLButtonUp(x,y);

			}

			break;

	case WM_KEYDOWN:
        {
			/*
            // Toggle between full screen and normal mode when the user presses the space bar.
            if (VK_SPACE == wParam)
            {

				if (fFullScreen)
                {
					// Find taskbar window and show
					hWndTaskBar = FindWindow(TEXT("HHTaskBar"), TEXT(""));
					ShowWindow(hWndTaskBar, SW_SHOW);
					EnableWindow( hWndTaskBar, TRUE );

					// Next resize the main window to the size of the work area.
                    SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, FALSE);
                    MoveWindow(hWnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, TRUE);
                    fFullScreen = !fFullScreen;
                }
                else
                {

					// Find taskbar window and hide
					hWndTaskBar = FindWindow(TEXT("HHTaskBar"), TEXT(""));
					ShowWindow(hWndTaskBar, SW_HIDE);
					EnableWindow( hWndTaskBar, FALSE );

					// Next resize the main window to the size of the screen.
                    SetRect(&rc, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
                    MoveWindow(hWnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, TRUE);

					fFullScreen = !fFullScreen;
                }
            }
			else 
			
			*/

/*			
			if (VK_ESCAPE == wParam) {
			
				DestroyWindow(hWnd);

			}
*/			

			if (navigationApplication->OnKeyDown(wParam)) {
				
				if (SetApplication(navigationApplication->getCurrentApp()))
				{

					InvalidateRect(hWnd,&rectContent,true);

				}

				return 0;

			} else {

				if (currentApplication->OnKeyDown(wParam)) {

					return 0;

				}

			}			

        }
        break;

  
	case WM_COMMAND:

		int wmId, wmEvent;

		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_FILE_EXIT:
		case ID_ACC_ESC:
		case ID_ACC_ALTF4:
			DestroyWindow(hWnd);
			break;

		case ID_ACC_F1:
		case ID_ACC_F2:
		case ID_ACC_F3:
		case ID_ACC_F4:
		case ID_ACC_F5:

			if (navigationApplication->OnCommand(wmId)) {
				
				if (SetApplication(navigationApplication->getCurrentApp()))
				{

					InvalidateRect(hWnd,&rectContent,true);

				}

				return 0;

			} else {

				if (currentApplication->OnCommand(wmId)) {

					return 0;

				}

			}			

			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		

		return DefWindowProc(hWnd, message, wParam, lParam);

		break;

	case WM_ERASEBKGND:

		HDC hdc;

		HDC cdc;

		HBITMAP hbmold;
	
	    BITMAP bm;

		hdc = (HDC)wParam;

		cdc = CreateCompatibleDC(hdc);

		hbmold = (HBITMAP)SelectObject(cdc,hBackgroundImage);

		GetObject(hBackgroundImage, sizeof(bm), &bm);

		BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, cdc, 0, 0, SRCCOPY);

		SelectObject(cdc,hbmold);

		DeleteDC(cdc);


		return 1;

	break;

	case WM_KEYUP:
	
		break;

	case WM_CHAR:

		currentApplication->OnChar(wParam);

		return 0;

		break;


	case WM_PAINT:

		PAINTSTRUCT ps;

		hdc = BeginPaint(hWnd, &ps);
		
		currentApplication->OnPaint(hdc);

		navigationApplication->OnPaint(hdc);

		EndPaint(hWnd, &ps);

		break;

	case WM_TIMER:

		currentApplication->OnTimer(0);

		break;

	case WM_DESTROY:

		currentApplication = 0;

		if (alphabetApplication) {
			alphabetApplication->Fini();
			delete alphabetApplication;
		}

		if (homeApplication) {
			homeApplication->Fini();
			delete homeApplication;
		}

		if (videoApplication) {
			videoApplication->Fini();
			delete videoApplication;
		}

		if (writeApplication) {
			writeApplication->Fini();
			delete writeApplication;
		}

		if (navigationApplication) {
			navigationApplication->Fini();
			delete navigationApplication;
		}

		hWndTaskBar = FindWindow(TEXT("HHTaskBar"), TEXT(""));
		ShowWindow(hWndTaskBar, SW_SHOW);
		EnableWindow( hWndTaskBar, TRUE );

		KillTimer(hWnd,timer40msec);
		KillTimer(hWnd,timer400msec);

		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

