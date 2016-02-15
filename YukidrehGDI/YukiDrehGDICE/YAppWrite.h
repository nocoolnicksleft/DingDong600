
#ifndef _YAPPWRITE_H_
#define _YAPPWRITE_H_

#include "stdafx.h"

#include "YApplication.h"

class YAppWrite : public YApplication 
{
protected:

	HWND hEditWnd;

public:

	YAppWrite(HWND hWnd, HINSTANCE hInstance);
	virtual ~YAppWrite();

	// YAppAlphabet methods
	virtual bool Init();
	virtual bool Fini();

	virtual bool OnPaint(HDC dc);

	virtual bool OnFocus(bool haveFocus);

//	virtual bool OnKeyUp(int virtualKeyCode);
//	virtual bool OnCommand(int identifier);
//	virtual bool OnChar(int characterCode);
//	virtual bool OnTimer(int id);


};

#endif