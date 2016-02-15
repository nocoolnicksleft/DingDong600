


#ifndef _YAPPALPHABET_H_
#define _YAPPALPHABET_H_

#include "stdafx.h"


#include "YApplication.h"
#include "alphabet.h"

#define NUMBER_OF_LETTERS 26
#define MAX_LOADSTRING 100

class YAppAlphabet : public YApplication 
{

public:

	TCHAR szAlphabetStringUpper[MAX_LOADSTRING];
	TCHAR szAlphabetStringLower[MAX_LOADSTRING];

	TCHAR szImageTitle[MAX_LOADSTRING];
	TCHAR szImageFilename[MAX_LOADSTRING];

	TCHAR currentLetter;
	TCHAR letterText[2][NUMBER_OF_LETTERS];

	RECT rectLetterLarge;
	RECT rectLetterSmall;
	RECT rectImage;
	RECT rectTitle;
	RECT rectAlphabet;

	Alphabet alphabet; 

	letterInfoStruct * letterinfo;
	letterInfoStruct * newletterinfo;
	letterImageStruct * letterimage;

	HFONT fontBig;
	HFONT fontMedium;
	HFONT fontSmall;
	HFONT fontMini;

	HPEN hpYellow;
	HPEN hpBrown;

	int currentLetterId;

	HBITMAP hBackgroundImage;
	HBITMAP hcurrentImage;
	int currentImageId;


	TCHAR applicationPath[MAX_PATH];
	TCHAR applicationDirectory[MAX_PATH];

	HBITMAP hbmold; // n/static??


	YAppAlphabet(HWND hWnd, HINSTANCE hInstance);
	virtual ~YAppAlphabet();

	// YAppAlphabet methods
	virtual bool Init();
	virtual bool Fini();

	virtual bool OnPaint(HDC dc);
	virtual bool OnKeyUp(int virtualKeyCode);
	virtual bool OnCommand(int identifier);
	virtual bool OnChar(int characterCode);
	virtual bool OnTimer(int id);

	void strToUpper(TCHAR * in, TCHAR *out);

	bool SetCharacter(TCHAR character);

};

#endif