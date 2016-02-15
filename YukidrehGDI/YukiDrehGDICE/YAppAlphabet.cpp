
#include "stdafx.h"

#include "YAppAlphabet.h"

#define IMAGE_Y 120


YAppAlphabet::~YAppAlphabet()
{

}


YAppAlphabet::YAppAlphabet (HWND phWnd, HINSTANCE phInstance) : YApplication(phWnd, phInstance)
{

	wcsncpy_s(szAlphabetStringUpper,ALPHABET_TEXTLEN,L"",_TRUNCATE);
	wcsncpy_s(szAlphabetStringLower,ALPHABET_TEXTLEN,L"",_TRUNCATE);

	wcsncpy_s(szImageTitle,ALPHABET_TEXTLEN,L"",_TRUNCATE);

	currentLetter = 0;
	
	letterinfo = 0;
	newletterinfo = 0;
	letterimage = 0;
	
	currentLetterId = 0;

	hBackgroundImage = 0;

	hcurrentImage = 0;
	currentImageId = 0;

}

bool YAppAlphabet::Init()
{
	GetModuleFileName(NULL,applicationPath,MAX_PATH);

	wcsncpy_s(applicationDirectory,MAX_PATH,applicationPath,wcsrchr(applicationPath,'\\') - applicationPath);

	// alphabet.Load(applicationDirectory, L"alphabet.csv");
	alphabet.Load(L"\\Storage Card\\alphabet", L"alphabet.csv");
		
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
	wcscpy_s(lf.lfFaceName, LF_FACESIZE, L"Tahoma");

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
	SetRect(&rectTitle,        5, IMAGE_Y+100+4, IMAGE_Y+100+4+12, 255);
	SetRect(&rectImage,       26, IMAGE_Y-4, 237, IMAGE_Y+100+4);


	hpYellow = CreatePen(PS_SOLID,1,RGB(255,216,0)); 
	hpBrown = CreatePen(PS_SOLID,3,RGB(201,105,0)); 

	SetCharacter(L'A');

	return true;
}

bool YAppAlphabet::Fini()
{
	DeleteObject(fontBig);
	DeleteObject(fontMedium);
	DeleteObject(fontSmall);
	DeleteObject(fontMini);

	DeleteObject(hpYellow);
	DeleteObject(hpBrown);

	return true;
}

bool YAppAlphabet::OnPaint(HDC hdc)
{
	HDC cdc;
    BITMAP bm;
	int xpos;
	int lasterror = 0;

	SetBkMode(hdc, TRANSPARENT);
		
	if (hcurrentImage != NULL) {
    
		// Create compatible memory dc
		cdc = CreateCompatibleDC(hdc);

		hbmold = (HBITMAP)SelectObject(cdc,hcurrentImage);

		GetObject(hcurrentImage, sizeof(bm), &bm);

		xpos = (240 - bm.bmWidth) / 2;

		BitBlt(hdc, xpos, IMAGE_Y, bm.bmWidth, bm.bmHeight, cdc, 0, 0, SRCCOPY);
		
		SelectObject(hdc,hpYellow);
		SelectObject(hdc,GetStockObject(NULL_BRUSH));
		Rectangle(hdc,xpos-1,IMAGE_Y-1,xpos+bm.bmWidth+1,IMAGE_Y+100+1);
		
		SelectObject(hdc,hpBrown);
		RoundRect(hdc,xpos-3,IMAGE_Y-3,xpos+bm.bmWidth+3,IMAGE_Y+100+3,1,1);

		SelectObject(cdc,hbmold);

		DeleteDC(cdc);

	}
	
	SelectObject(hdc,GetStockObject(BLACK_PEN));

	SelectObject(hdc, fontBig);
	DrawText(hdc, szAlphabetStringUpper, (int)wcslen(szAlphabetStringUpper), &rectLetterLarge, DT_LEFT | DT_BOTTOM | DT_SINGLELINE );

	SelectObject(hdc, fontMedium);
	DrawText(hdc, szAlphabetStringLower, (int)wcslen(szAlphabetStringLower), &rectLetterSmall, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE );
    
	SelectObject(hdc, fontSmall);
	DrawText(hdc, szImageTitle, (int)wcslen(szImageTitle), &rectTitle, DT_CENTER);

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


	return true;
}

bool YAppAlphabet::OnKeyUp(int virtualKeyCode)
{
	return true;
}

bool YAppAlphabet::OnCommand(int identifier)
{
	return true;
}

bool YAppAlphabet::SetCharacter(TCHAR newcharacter)
{

	int lasterror = 0;

	newletterinfo = alphabet.GetLetter(newcharacter);

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

			swprintf_s(szAlphabetStringUpper,MAX_LOADSTRING,L"%c",towupper(newcharacter));
			swprintf_s(szAlphabetStringLower,MAX_LOADSTRING,L"%c",towlower(newcharacter));

			InvalidateRect(hWnd,&rectLetterLarge,true);
			InvalidateRect(hWnd,&rectLetterSmall,true);
			InvalidateRect(hWnd,&rectAlphabet,true);

		}

	}


	letterimage = letterinfo->GetImage(currentImageId);

	if (letterimage) {

		wcsncpy_s(szImageTitle,ALPHABET_TEXTLEN,letterimage->GetTitle(),_TRUNCATE);

		strToUpper(szImageTitle,szImageTitle);

		hcurrentImage = letterimage->GetBitmapHandle();

		if (hcurrentImage == 0) {

			MessageBox(hWnd, L"Could not load Bitmap!", L"Error", MB_OK | MB_ICONEXCLAMATION);

		}
	
		lasterror = GetLastError();

		InvalidateRect(hWnd,&rectTitle,true);
		InvalidateRect(hWnd,&rectImage,true);

	}

	return true;
}


bool YAppAlphabet::OnChar(int characterCode)
{

	TCHAR newLetter;

	newLetter = towupper((TCHAR)characterCode);

	SetCharacter(newLetter);

	return true;
}

bool YAppAlphabet::OnTimer(int id)
{
	return true;
}

void YAppAlphabet::strToUpper(TCHAR * in, TCHAR *out) {

	TCHAR * tin = in;
	TCHAR * tout = out;

	while (*tin) {
		*tout = towupper(*tin);
		tout++;
		tin++;
	}

	*tout = 0;

}

