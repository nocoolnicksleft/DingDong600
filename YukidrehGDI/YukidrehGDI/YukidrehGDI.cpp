// YukidrehGDI.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "YukidrehGDI.h"
#include <stdio.h> 
#include <vector>
#include <map>
#include "winimage.h"
#include "alphabet.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

TCHAR szAlphabetStringUpper[MAX_LOADSTRING] = L"";
TCHAR szAlphabetStringLower[MAX_LOADSTRING] = L"";

// Forward declarations of functions included in this code module:
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

#define TIMER_40MSEC 40

UINT_PTR Timer40msec = 0;

#define MAX_ANI_WIDTH 100
#define MIN_ANI_WIDTH 1

#define MAX_ANI_HEIGHT 100
#define MIN_ANI_HEIGHT 1

#define NUMBER_OF_LETTERS 26


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_YUKIDREHGDI, szWindowClass, MAX_LOADSTRING);

	WNDCLASS wcex;

	wcex.style			= 0;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_YUKIDREHGDI));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;

	RegisterClass(&wcex);

/*
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= 0;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_YUKIDREHGDI));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	// wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.hbrBackground	= NULL;
	// wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_YUKIDREHGDI);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassEx(&wcex);
*/

	HWND hWnd;

    hInst = hInstance; // Store instance handle in our global variable

    hWnd = CreateWindowEx(0,szWindowClass, szTitle, WS_EX_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 240, 320, NULL, NULL, hInstance, NULL);

	//hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
    //  CW_USEDEFAULT, 0, 240, 320, NULL, NULL, hInstance, NULL);

    // Perform application initialization:
    if (!hWnd)
    {

      return FALSE;

    } else {

		Timer40msec = SetTimer ( hWnd, TIMER_40MSEC, 40, NULL );

		ShowWindow(hWnd, nCmdShow);
		UpdateWindow(hWnd);
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_YUKIDREHGDI));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}


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
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
    // RECT invRect;

	static UINT aniHeight = 0;
	static UINT aniHeightDir = 1;
	static UINT aniWidth = 0;
	static UINT aniWidthDir = 1;

	static HFONT fontBig;
	static HFONT fontMedium;
	static HFONT fontSmall;
	static HFONT fontMini;

	static HPEN hpYellow;
	static HPEN hpBrown;
/*
	static C_Image currentImg;
	static C_Image img;
	static C_ImageSet imgset;
	static C_AnimationWindow anim, anim2, anim3;
*/
	static TCHAR szImageTitle[MAX_LOADSTRING] = L" ";
	static TCHAR szImageFilename[MAX_LOADSTRING] = L" ";
	static TCHAR currentLetter = 0;
	static TCHAR letterText[2][NUMBER_OF_LETTERS];

	static RECT rectLetterLarge;
	static RECT rectLetterSmall;
	static RECT rectImage;
	static RECT rectTitle;
	static RECT rectAlphabet;

	static Alphabet alphabet;
	static LetterInfo * letterinfo = 0;
	static LetterInfo * newletterinfo = 0;
	static LetterImage * letterimage = 0;

	static HBITMAP hBackgroundImage = 0;
	static HBITMAP hcurrentImage = 0;
	static int currentImageId = 0;
	HBITMAP hbmold;

	HDC cdc;
    BITMAP bm;

//	int i;
	int xpos;
	int lasterror = 0;


	switch (message)
	{

	 case WM_CREATE:
/*
		fontBig = CreateFont(-100, 0, 0, 0,
					   FW_NORMAL, FALSE, FALSE, FALSE,
					   ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					   CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
					   DEFAULT_PITCH,
			 L"Tahoma");

		fontMedium = CreateFont(-40, 0, 0, 0,
					   FW_NORMAL, FALSE, FALSE, FALSE,
					   ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					   CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
					   DEFAULT_PITCH,
			 L"Tahoma");

		fontSmall = CreateFont(-16, 0, 0, 0,
				   FW_BOLD , FALSE, FALSE, FALSE,
				   ANSI_CHARSET, OUT_DEFAULT_PRECIS,
				   CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
				   DEFAULT_PITCH,
		 L"Tahoma");

		fontMini = CreateFont(-12, 0, 0, 0,
				   FW_BOLD , FALSE, FALSE, FALSE,
				   ANSI_CHARSET, OUT_DEFAULT_PRECIS,
				   CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
				   DEFAULT_PITCH,
		 L"Tahoma");
*/

		LOGFONT lf;
		memset(&lf, 0, sizeof(LOGFONT));       

		lf.lfHeight = -100;
		lf.lfWidth = 0; 
		lf.lfEscapement = 0; 
		lf.lfOrientation = 0; 
		lf.lfWeight= FW_NORMAL; 
		lf.lfItalic = FALSE; 
		lf.lfUnderline = FALSE;
		lf.lfStrikeOut = FALSE;
		lf.lfCharSet = ANSI_CHARSET; 
		lf.lfOutPrecision = OUT_DEFAULT_PRECIS; 
		lf.lfClipPrecision = CLIP_DEFAULT_PRECIS; 
		lf.lfQuality = CLEARTYPE_QUALITY; 
		lf.lfPitchAndFamily = DEFAULT_PITCH; 
		wcscpy(lf.lfFaceName, L"Tahoma");

		lf.lfHeight = -100;
  		fontBig = CreateFontIndirect(&lf);

		lf.lfHeight = -40;
		fontMedium = CreateFontIndirect(&lf);

		lf.lfHeight = -16;
		fontSmall = CreateFontIndirect(&lf);

		lf.lfHeight = -12;
		fontMini = CreateFontIndirect(&lf);


		SetRect(&rectAlphabet,     1,   1, 237, 12);
		SetRect(&rectLetterLarge, 27,  11, 120, 120);
		SetRect(&rectLetterSmall, 27,  70, 206, 108);
		SetRect(&rectTitle,        5, 240, 237, 265);
		SetRect(&rectImage,       26, 130-4, 237, 230+4);

		hBackgroundImage = (HBITMAP)LoadImageW(0,L"bg1.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);

		hpYellow = CreatePen(PS_SOLID,1,RGB(255,216,0)); 
		hpBrown = CreatePen(PS_SOLID,3,RGB(201,105,0)); 
/*
		for (i = 0; i < NUMBER_OF_LETTERS; i++) {
			
			swprintf(letterText[i],2,L"%c",towupper((TCHAR)i));
		}
*/

		break;

	case WM_SIZE:
//			anim.Display (40,140);
		break;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		/*
		// Parse the menu selections:
		switch (wmId)
		{
//		case IDM_EXIT:
//			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		*/

		return DefWindowProc(hWnd, message, wParam, lParam);

		break;

case WM_ERASEBKGND:

	
		hdc = (HDC)wParam;

		cdc = CreateCompatibleDC(hdc);

		hbmold = (HBITMAP)SelectObject(cdc,hBackgroundImage);

		GetObject(hBackgroundImage, sizeof(bm), &bm);

		BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, cdc, 0, 0, SRCCOPY);

		SelectObject(cdc,hbmold);

		DeleteDC(cdc);

		return 1;

	break;

case WM_CHAR:

		currentLetter = towupper((TCHAR)wParam);

		newletterinfo = alphabet.GetLetter(currentLetter);

		if (newletterinfo) {

			if (letterinfo && (newletterinfo == letterinfo)) {

				if ((currentImageId + 1) != letterinfo->GetImageCount()) {

					currentImageId++;

				} else {

					currentImageId = 0;

				}
				

			} else {

				letterinfo = newletterinfo;

				currentImageId = 0;

				swprintf_s(szAlphabetStringUpper,MAX_LOADSTRING,L"%c",towupper((TCHAR)wParam));
				swprintf_s(szAlphabetStringLower,MAX_LOADSTRING,L"%c",towlower((TCHAR)wParam));

				InvalidateRect(hWnd,&rectLetterLarge,true);
				InvalidateRect(hWnd,&rectLetterSmall,true);
				InvalidateRect(hWnd,&rectAlphabet,true);

			}


			letterimage = letterinfo->GetImage(currentImageId);

			if (letterimage) {

				wcsncpy_s(szImageTitle,ALPHABET_TEXTLEN,letterimage->_title,_TRUNCATE);

				strToUpper(szImageTitle,szImageTitle);

				hcurrentImage = letterimage->GetBitmapHandle();

				if (hcurrentImage == 0) {

					MessageBox(hWnd, L"Could not load Bitmap!", L"Error", MB_OK | MB_ICONEXCLAMATION);

				}
			
				lasterror = GetLastError();

				InvalidateRect(hWnd,&rectTitle,true);
				InvalidateRect(hWnd,&rectImage,true);

			}

			

		}

		return 0;

		break;


	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		
        SetBkMode(hdc, TRANSPARENT);
		
		if (hcurrentImage != NULL) {
        
			// Create compatible memory dc
			cdc = CreateCompatibleDC(hdc);

			hbmold = (HBITMAP)SelectObject(cdc,hcurrentImage);

			GetObject(hcurrentImage, sizeof(bm), &bm);

			xpos = (240 - bm.bmWidth) / 2;

			BitBlt(hdc, xpos, 130, bm.bmWidth, bm.bmHeight, cdc, 0, 0, SRCCOPY);
			
			SelectObject(hdc,hpYellow);
			SelectObject(hdc,GetStockObject(NULL_BRUSH));
			Rectangle(hdc,xpos-1,130-1,xpos+bm.bmWidth+1,130+100+1);
			
			SelectObject(hdc,hpBrown);
			RoundRect(hdc,xpos-3,130-3,xpos+bm.bmWidth+3,130+100+3,1,1);

			SelectObject(cdc,hbmold);

			DeleteDC(cdc);

		}

		/*
	    RECT rectCurrentLetter;
	    SelectObject(hdc, fontMini);
		for (TCHAR ch = 0; ch < 26; ch++) {
			SetRect(&rectCurrentLetter,rectAlphabet.left + (ch * 8),rectAlphabet.top,rectAlphabet.right,rectAlphabet.bottom);
				
			if (ch == currentLetter) {
				SelectObject(hdc,GetStockObject(WHITE_PEN));
				DrawText(hdc, letterText[ch], 1, &rectCurrentLetter, DT_LEFT | DT_BOTTOM | DT_SINGLELINE );
				SelectObject(hdc,GetStockObject(BLACK_PEN));
			} else {
				DrawText(hdc, letterText[ch], 1, &rectCurrentLetter, DT_LEFT | DT_BOTTOM | DT_SINGLELINE );
			}
		}
		*/

		SelectObject(hdc,GetStockObject(BLACK_PEN));

		// currentImg.GDIPaint(hdc, 26, 130);

		SelectObject(hdc, fontBig);
		DrawText(hdc, szAlphabetStringUpper, 1, &rectLetterLarge, DT_LEFT | DT_BOTTOM | DT_SINGLELINE );

		SelectObject(hdc, fontMedium);
		DrawText(hdc, szAlphabetStringLower, 1, &rectLetterSmall, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE );
        
		SelectObject(hdc, fontSmall);
		DrawText(hdc, szImageTitle, (int)wcslen(szImageTitle), &rectTitle, DT_CENTER);

		EndPaint(hWnd, &ps);

		break;

	case WM_TIMER:
/*
		if (aniWidthDir) {
			if (aniWidth >= MAX_ANI_WIDTH) aniWidthDir = 0;
			else {
				aniWidth += 2;
				SetRect(&invRect, 200 + aniWidth - 2, 200, 200 + aniWidth, 400);
				InvalidateRect(hWnd,&invRect,true);
			}
		} else {
			if (aniWidth <= MIN_ANI_WIDTH) aniWidthDir = 1;
			else {
				aniWidth -= 2;
				SetRect(&invRect, 200 + aniWidth + 2, 200, 200 + aniWidth, 400);
				InvalidateRect(hWnd,&invRect,true);
			}
		}
		if (aniHeightDir) {
			if (aniHeight >= MAX_ANI_HEIGHT) aniHeightDir = 0;
			else {
				aniHeight += 2;
				SetRect(&invRect, 200, 200 + aniHeight - 2, 400, 200 + aniHeight);
				InvalidateRect(hWnd,&invRect,true);
			}
		} else {
			if (aniHeight <= MIN_ANI_HEIGHT) aniHeightDir = 1;
			else {
				aniHeight -= 2;
				SetRect(&invRect, 200, 200 + aniHeight + 2, 400, 200 + aniHeight);
				InvalidateRect(hWnd,&invRect,true);
			}
		}
		
*/		
		
//		InvalidateRect(hWnd,NULL,TRUE);
		break;

	case WM_DESTROY:

		DeleteObject(fontBig);
		DeleteObject(fontMedium);
		DeleteObject(fontSmall);
		DeleteObject(fontMini);

		DeleteObject(hpYellow);
		DeleteObject(hpBrown);

		/*
		DeleteObject(hbRed);
		DeleteObject(hbGreen);
		DeleteObject(hbBlue);
*/

		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

