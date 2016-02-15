
#ifndef _YAPPHOME_H_
#define _YAPPHOME_H_

#include "stdafx.h"

#include "YApplication.h"

class YAppHome : public YApplication 
{

public:

	YAppHome(HWND hWnd, HINSTANCE hInstance);
	virtual ~YAppHome();

	// YAppAlphabet methods
	virtual bool Init();
	virtual bool Fini();

	virtual bool OnPaint(HDC dc);
//	virtual bool OnKeyUp(int virtualKeyCode);
//	virtual bool OnCommand(int identifier);
//	virtual bool OnChar(int characterCode);
//	virtual bool OnTimer(int id);


};

#endif