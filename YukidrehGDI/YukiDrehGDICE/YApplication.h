


#ifndef _YAPPLICATION_H_
#define _YAPPLICATION_H_


class YApplication 
{

protected:

	HWND hWnd;
	HINSTANCE hInstance;
	RECT rect;

public:

	// Constructor/Destructor
	YApplication(HWND phWnd, HINSTANCE phInstance);
	YApplication(HWND phWnd, HINSTANCE phInstance, WORD left, WORD top, WORD right, WORD bottom);
	YApplication(HWND phWnd, HINSTANCE phInstance, LPRECT inRect);

	virtual ~YApplication();

	// YApplication methods
	virtual bool Init();
	virtual bool Fini();

	virtual bool OnPaint(HDC dc);

	virtual bool OnKeyUp(int virtualKeyCode);
	virtual bool OnKeyDown(int virtualKeyCode);
	virtual bool OnChar(int characterCode);

	virtual bool OnLButtonDown(int x, int y);
	virtual bool OnLButtonUp(int x, int y);

	virtual bool OnCommand(int identifier);
	virtual bool OnTimer(int id);

	virtual bool OnFocus(bool haveFocus);

	virtual bool IsHit(int x, int y);

	virtual bool Invalidate(bool erase);

};


#endif

