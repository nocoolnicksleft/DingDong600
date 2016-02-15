#include "stdafx.h"

#include "YAppVideo.h"

#include "YukiDrehGDICE.h"


YAppVideo::~YAppVideo()
{

}


YAppVideo::YAppVideo (HWND phWnd, HINSTANCE phInstance) : YApplication(phWnd, phInstance)
{
	
}

bool YAppVideo::Init() 
{

	GetModuleFileName(NULL,applicationPath,MAX_PATH);

	wcsncpy_s(applicationDirectory,MAX_PATH,applicationPath,wcsrchr(applicationPath,'\\') - applicationPath);

	ScanDir(L"\\Storage Card\\videos");

	return true;

}

bool YAppVideo::Fini() 
{

	return true;

}

bool YAppVideo::OnPaint(HDC hdc)
{

	for (int i = 0; i < VIDEOS_PER_PAGE; i++)
	{

		HBITMAP hcurrentImage = _videos[i]->GetBitmapHandle();
		HBITMAP hbmold;

		HDC cdc;
		BITMAP bm;

		int xpos;
		int ypos;

		if (hcurrentImage != 0) 
		{

			// Create compatible memory dc
			cdc = CreateCompatibleDC(hdc);

			hbmold = (HBITMAP)SelectObject(cdc,hcurrentImage);

			GetObject(hcurrentImage, sizeof(bm), &bm);

			xpos = _videos[i]->_buttonRect.left;
			ypos = _videos[i]->_buttonRect.top;

			BitBlt(hdc, xpos, ypos, bm.bmWidth, bm.bmHeight, cdc, 0, 0, SRCCOPY);
			/*
			SelectObject(hdc,hpYellow);
			SelectObject(hdc,GetStockObject(NULL_BRUSH));
			Rectangle(hdc,xpos-1,IMAGE_Y-1,xpos+bm.bmWidth+1,IMAGE_Y+100+1);
			
			SelectObject(hdc,hpBrown);
			RoundRect(hdc,xpos-3,IMAGE_Y-3,xpos+bm.bmWidth+3,IMAGE_Y+100+3,1,1);
			*/

			SelectObject(cdc,hbmold);

			DeleteDC(cdc);

		}
	}


	return true;

}

bool YAppVideo::OnKeyUp(int virtualKeyCode)
{

	return true;

}

bool YAppVideo::OnLButtonDown(int x, int y)
{
	for (int i=0; i < VIDEOS_PER_PAGE; i++)
	{
		if (_videos[i]->IsHit(x,y)) {
			_videos[i]->LaunchPlayer();
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------------------------------------------
// ScanDir()
//------------------------------------------------------------------------------------------------------------------
int YAppVideo::ScanDir(const TCHAR *_path)
{
  HANDLE fh;

  int    filecnt=0;
  TCHAR	 path[MAX_PATH];
  TCHAR	 parmpath[MAX_PATH];

  _videocount = 0;

  fd = (WIN32_FIND_DATA *)malloc(sizeof(WIN32_FIND_DATA));

  FixPath(_path,parmpath);

  FixPath(_path,path);
  // strcat(path,"*.wmv");
  wcscat_s (path, MAX_PATH, L"*.wmv");

  // printf("\nScanning: %s\n",path);

  fh = FindFirstFile( path,fd);

  if(fh != INVALID_HANDLE_VALUE)
  {
    do
    {
      
      if(fd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {

	  }
      else
      {
	    wcsncpy_s(_video[_videocount], MAX_PATH, parmpath, _TRUNCATE);	
		wcscat_s (_video[_videocount], MAX_PATH, fd->cFileName);
	    //wcsncpy_s(_video[_videocount], MAX_PATH, fd->cFileName, _TRUNCATE);		

		_videos[_videocount] = new video(_video[_videocount],_videocount);
	    _videocount++;

        //printf("%-35.35s "  ,fd->cFileName);
		
      }

      // printf(" Size : %16lu bytes\n",((fd->nFileSizeHigh * MAXDWORD)+fd->nFileSizeLow));

    }
    while(FindNextFile(fh,fd));
  }

  FindClose(fh);

  return 1;
}

//------------------------------------------------------------------------------------------------------------------
// fixpath() - Adds \ to the end of a path if not present.
//------------------------------------------------------------------------------------------------------------------
int YAppVideo::FixPath(const TCHAR *inpath, TCHAR *outpath)
{
  int   n=0;

  //strcpy(outpath,inpath);
  wcsncpy_s(outpath, MAX_PATH, inpath, _TRUNCATE);		

  while(inpath[n]) n++;

  if(inpath[n-1] != '\\')
  {
    //strcat(outpath,"\\");
	wcscat_s (outpath, MAX_PATH, L"\\");
    return 1;
  }

  return 0;
}



