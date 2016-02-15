
#ifndef _ALPHABET_H_
#define _ALPHABET_H_

#include "stdafx.h"

#include "stdio.h"

#include <windows.h>


#define ALPHABET_TEXTLEN 255
#define MAX_LETTERS 30
#define MAX_IMAGES 10

class letterImageStruct {

 protected:

	int _type;
	TCHAR _title[ALPHABET_TEXTLEN];
	TCHAR _filename[MAX_PATH];

	HBITMAP _bitmap;

 public:

	 letterImageStruct() 
	{
		_bitmap = 0;
	}

	letterImageStruct(int filetype,TCHAR * filename,TCHAR * title)
	{
		_bitmap = 0;

		_type = filetype;				

		wcsncpy_s(_filename,
			ALPHABET_TEXTLEN,L"\\Storage Card\\alphabet\\",_TRUNCATE);		

		wcscat_s(_filename,
			ALPHABET_TEXTLEN,filename);

		wcsncpy_s(_title,
			ALPHABET_TEXTLEN,title,_TRUNCATE);
	}
				
	TCHAR * GetTitle()
	{
		return _title;
	}

	HBITMAP GetBitmapHandle()
	{
		
		if (_bitmap == 0) {

			_bitmap = SHLoadDIBitmap(_filename);

			RETAILMSG ((!_bitmap),(TEXT("Error loading image %s\r\n"),_filename) ) ;

		}

		return _bitmap;

	}

};

class letterInfoStruct {

 protected:

	int _imagecount;
	letterImageStruct * _images[MAX_IMAGES];
	TCHAR _letter;

 public:

	letterInfoStruct() 
	{
		_letter = 0;
		_imagecount = 0;
	}

	letterInfoStruct(TCHAR letter) 
	{
		_letter = letter;
		_imagecount = 0;
	}

	TCHAR GetLetter()
	{
		return _letter;
	}

	int GetImageCount()
	{
		return _imagecount;
	}

	letterImageStruct * GetImage(int imageid) 
	{
		if (imageid < _imagecount) {

			return _images[imageid];

		}

		return NULL;

	}

	void AddImage(letterImageStruct * imageinfo) 
	{
		if (imageinfo) 
		{

			if (_imagecount < MAX_IMAGES) 
			{
		
				_images[_imagecount] = imageinfo;
			
				_imagecount++;

			}

		}

	}

};





class Alphabet {

protected:

	letterInfoStruct * _newletters[MAX_LETTERS];

	int _letterCount;

	TCHAR _filename[MAX_PATH];
	TCHAR _filepath[MAX_PATH];

public:

	Alphabet() 
	{
	}
		
	void Load(TCHAR * workingDirectory, TCHAR * alphabetFile) 
	{
		letterImageStruct * imageinfo;
		letterInfoStruct * letterinfo;

		TCHAR lastLetter = 0;
		TCHAR letter = 0;
		TCHAR title[ALPHABET_TEXTLEN];
		TCHAR filename[ALPHABET_TEXTLEN];
		int filetype;


		wcsncpy_s(_filepath, MAX_PATH, workingDirectory, _TRUNCATE);		

		wcsncpy_s(_filename, MAX_PATH, _filepath, _TRUNCATE);		
		wcscat_s (_filename, MAX_PATH, L"\\");
		wcscat_s (_filename, MAX_PATH, alphabetFile);



		_letterCount = 0;
		int _lastletterid = 0;

		FILE * alphafile = 0;

		_wfopen_s (&alphafile, _filename, L"r");

		RETAILMSG ((!alphafile),(TEXT("Error opening alphabet file\r\n")) ) ;

		if (alphafile != NULL) 
		{

			while (fwscanf_s (alphafile, L"%c,%d,%[^,],%[^\n]\r\n",&letter,1,&filetype,&title,ALPHABET_TEXTLEN,&filename,ALPHABET_TEXTLEN) != EOF) 
			{

				// RETAILMSG ((alphafile),(TEXT("Found %c - %s\r\n"),letter,title) ) ;

				if (letter != lastLetter) {

					_lastletterid = _letterCount;

					letterinfo = new letterInfoStruct(letter);

					_newletters[_lastletterid] = letterinfo;

					_letterCount++;

					lastLetter = letter;
	
				}

				imageinfo = new letterImageStruct(filetype,filename,title);

				letterinfo->AddImage(imageinfo);

			}

			fclose(alphafile);

		}

	}

	int LetterExists(TCHAR letter)
	{
		
	}


	letterInfoStruct * GetLetter(TCHAR letter) 
	{
		int i;

		for (i=0; i < _letterCount; i++) {
			if (_newletters[i]->GetLetter() == letter) {
				return _newletters[i];
			}
		}

		return NULL;
	}

	~Alphabet()
	{

	}


};



#endif


