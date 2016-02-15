
#include "stdafx.h"

#include "YAppAlphabet.h"


YApplication::~YApplication()
{

}

YApplication::YApplication (HWND phWnd, HINSTANCE phInstance)
{
	hWnd = phWnd;
	hInstance = phInstance;
	rect.left = 0;
	rect.top = 0;
	rect.right = 0;
	rect.bottom = 0;
}

YApplication::YApplication (HWND phWnd, HINSTANCE phInstance, WORD left, WORD top, WORD right, WORD bottom)
{
	hWnd = phWnd;
	hInstance = phInstance;
	rect.left = left;
	rect.top = top;
	rect.right = right;
	rect.bottom = bottom;
}

YApplication::YApplication (HWND phWnd, HINSTANCE phInstance, LPRECT inRect)
{
	hWnd = phWnd;
	hInstance = phInstance;
	rect.left = inRect->left;
	rect.top = inRect->top;
	rect.right = inRect->right;
	rect.bottom = inRect->bottom;
}



bool YApplication::Init()
{
	return true;
}

bool YApplication::Fini()
{
	return true;
}

bool YApplication::OnPaint(HDC hdc)
{
	return true;
}

bool YApplication::OnKeyUp(int virtualKeyCode)
{
	return false;
}

bool YApplication::OnKeyDown(int virtualKeyCode)
{
	return false;
}

bool YApplication::OnCommand(int identifier)
{
	return false;
}

bool YApplication::OnChar(int characterCode)
{
	return false;
}

bool YApplication::OnLButtonDown(int x, int y)
{
	return false;
}

bool YApplication::OnLButtonUp(int x, int y)
{
	return false;
}


bool YApplication::IsHit(int x, int y)
{
	return true;
}

bool YApplication::OnFocus(bool haveFocus) 
{
	return true;
}

bool YApplication::OnTimer(int id)
{
	return false;
}

bool YApplication::Invalidate(bool erase)
{
	InvalidateRect(hWnd,&rect,erase);
	return true;
}