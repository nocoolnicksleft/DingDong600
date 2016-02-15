
#ifndef _YAPPNAVIGATION_H_
#define _YAPPNAVIGATION_H_

#include "stdafx.h"

#include "YApplication.h"

#define NAVIGATION_TOP (319-64)
#define NAVIGATION_COUNT 5
#define NAVIGATION_COUNT_ACTIVE 4

class NavigationButton 
{
	int Number;
	bool Active;
	HBITMAP hBitmapActive;
	HBITMAP hBitmapInactive;
	HINSTANCE hInst;
	HWND hWnd;
	RECT zoneRect;

public:

	NavigationButton(int pNumber, HINSTANCE phInst, HWND phWnd)
	{
		Active = false;
		hInst = phInst;
		Number = pNumber;
		hBitmapActive = 0;
		hBitmapInactive = 0;
		hWnd = phWnd;

	};

	~NavigationButton()
	{
		DeleteObject(hBitmapActive);	
		DeleteObject(hBitmapInactive);
	}

	void Init(int idBitmapActive, int idBitmapInactive)
	{
		hBitmapActive = LoadBitmap(hInst, MAKEINTRESOURCE(idBitmapActive));
		hBitmapInactive = LoadBitmap(hInst, MAKEINTRESOURCE(idBitmapInactive));
		zoneRect.left = Number*48;
		zoneRect.top = NAVIGATION_TOP;
		zoneRect.right =  (Number+1)*48;
		zoneRect.bottom = 319;

		
	}

	void OnPaint(HDC hdc)
	{
		HDC cdc;

		HBITMAP hbmold;
	
	    BITMAP bm;

		cdc = CreateCompatibleDC(hdc);

		if (Active) {

			hbmold = (HBITMAP)SelectObject(cdc,hBitmapActive);
			GetObject(hBitmapActive, sizeof(bm), &bm);

		} else {

			hbmold = (HBITMAP)SelectObject(cdc,hBitmapInactive);
			GetObject(hBitmapInactive, sizeof(bm), &bm);

		}

		BitBlt(hdc, zoneRect.left, zoneRect.top, bm.bmWidth, bm.bmHeight, cdc, 0, 0, SRCCOPY);

		SelectObject(cdc,hbmold);

		DeleteDC(cdc);

	}

	

	void Activate()
	{
		Active = true;
		InvalidateRect(hWnd,&zoneRect,false);
	}

	void Deactivate()
	{
		Active = false;
		InvalidateRect(hWnd,&zoneRect,false);
	}

	bool IsHit(int x, int y)
	{
		return ((x >= zoneRect.left) && (x < zoneRect.right) && (y >= zoneRect.top) && (y < zoneRect.bottom));
	}


};



class YAppNavigation : public YApplication 
{

protected:
	NavigationButton * navigation[5];
	int currentApp;

public:
	YAppNavigation (HWND phWnd, HINSTANCE phInstance);
	~YAppNavigation();

	virtual bool Init();
	virtual bool OnPaint(HDC dc);
	virtual bool OnKeyDown(int virtualKeyCode);

	virtual bool OnLButtonDown(int x, int y);
	virtual bool OnLButtonUp(int x, int y);

	virtual bool OnCommand(int identifier);

	int getCurrentApp();

	bool SetApplication(int id);

};

#endif

