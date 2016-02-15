
#ifndef _YAPPVIDEO_H_
#define _YAPPVIDEO_H_

#include "stdafx.h"

#include "YApplication.h"

#define MAX_VIDEOS 9
#define VIDEOS_PER_PAGE 9

#define VBUTTON_WIDTH 80
#define VBUTTON_HEIGHT 80

#define THUMBNAIL_WIDTH 72
#define THUMBNAIL_HEIGHT 72

#define OFFSET_TOP 4
#define OFFSET_LEFT 4


class video 
{

public:

	HBITMAP _bitmap;

	TCHAR _videoPath[MAX_PATH];
	TCHAR _thumbailPath[MAX_PATH];

	RECT _buttonRect;

	int _showFrame;

public:

	video(TCHAR * videoPath, int position) 
	{
		_showFrame = 0;
		_bitmap = 0;

		wcsncpy_s(_videoPath, MAX_PATH, videoPath, _TRUNCATE);		

		wcsncpy_s(_thumbailPath, MAX_PATH, _videoPath, wcsrchr(_videoPath,'.') - _videoPath);
		wcscat_s (_thumbailPath, MAX_PATH, L".bmp");

		_buttonRect.top = (position / 3) * VBUTTON_HEIGHT + OFFSET_TOP;
		_buttonRect.left = (position % 3) * VBUTTON_WIDTH + OFFSET_LEFT;
		_buttonRect.bottom = _buttonRect.top + VBUTTON_HEIGHT + OFFSET_TOP;
		_buttonRect.right = _buttonRect.left + VBUTTON_WIDTH + OFFSET_LEFT;
		
	}

	HBITMAP GetBitmapHandle()
	{
		
		if (_bitmap == 0) {

			_bitmap = SHLoadDIBitmap(_thumbailPath);

			RETAILMSG ((!_bitmap),(TEXT("Error loading thumbnail %s\r\n"),_thumbailPath) ) ;

		}

		return _bitmap;

	}

	bool LaunchPlayer()
	{

		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		// TCHAR command[MAX_PATH];

		// wcsncpy_s(command, MAX_PATH, L"\"\\Program Files\\YukiDrehMediaPlayer\\YukiDrehMediaPlayer.exe\"", _TRUNCATE);
		// wcscat_s (command, MAX_PATH, _videoPath);


		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );

		// Start the child process. 
		if( !CreateProcess( L"\\Program Files\\Player\\Player.exe",   // No module name (use command line)
			_videoPath,        // Command line
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			FALSE,          // Set handle inheritance to FALSE
			0,              // No creation flags
			NULL,           // Use parent's environment block
			NULL,           // Use parent's starting directory 
			//&si,            // Pointer to STARTUPINFO structure
			NULL,
			&pi )           // Pointer to PROCESS_INFORMATION structure
		) 
		{
			printf( "CreateProcess failed (%d).\n", GetLastError() );
			return false;
		}

		// Wait until child process exits.
		WaitForSingleObject( pi.hProcess, INFINITE );

		// Close process and thread handles. 
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );


		return true;
	}

	bool IsHit(int x, int y)
	{
		return ((x >= _buttonRect.left) && (x < _buttonRect.right) && (y >= _buttonRect.top) && (y < _buttonRect.bottom));
	}


};


class YAppVideo : public YApplication 
{

protected:

	TCHAR applicationPath[MAX_PATH];
	TCHAR applicationDirectory[MAX_PATH];
	WIN32_FIND_DATA *fd;
    int _videocount ;
	TCHAR _video[MAX_VIDEOS][MAX_PATH];
	video * _videos[MAX_VIDEOS];

public:

	YAppVideo(HWND hWnd, HINSTANCE hInstance);
	virtual ~YAppVideo();

	// YAppAlphabet methods
	virtual bool Init();
	virtual bool Fini();

	virtual bool OnPaint(HDC dc);
	virtual bool OnKeyUp(int virtualKeyCode);

	virtual bool OnLButtonDown(int x, int y);
//	virtual bool OnCommand(int identifier);
//	virtual bool OnChar(int characterCode);
//	virtual bool OnTimer(int id);

	int ScanDir(const TCHAR *_path);
	int FixPath(const TCHAR *inpath, TCHAR *outpath);


};

#endif