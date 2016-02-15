#include "stdafx.h"

#include "stdio.h"

#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

#include <string>

#define ALPHABET_TEXTLEN 255

class LetterImage {

public:
	TCHAR _filename[ALPHABET_TEXTLEN];
	TCHAR _title[ALPHABET_TEXTLEN];
	HBITMAP _bitmap;
	int _type;

public:
	LetterImage(TCHAR * filename, TCHAR * title, int type) 
	{		
		wcsncpy_s(_filename,ALPHABET_TEXTLEN,L"bilderout\\",_TRUNCATE);		
		wcscat_s(_filename,ALPHABET_TEXTLEN,filename);
		// wcsncpy_s(_filename,ALPHABET_TEXTLEN,filename,_TRUNCATE);

		wcsncpy_s(_title,ALPHABET_TEXTLEN,title,_TRUNCATE);
		_type = type;
		_bitmap = 0;
	}

	HBITMAP GetBitmapHandle()
	{
		
		if (_bitmap == 0) {

			_bitmap = (HBITMAP)LoadImageW(0,_filename,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);

		}

		return _bitmap;

	}

    ~LetterImage()
	{

	}

};



class LetterInfo {
    
	TCHAR _letter;
	int _imagecount;
	map<int, LetterImage *> _images;

public:
	LetterInfo(TCHAR letter) 
	{
		_letter = letter;
		_imagecount = 0;
	}

	~LetterInfo()
	{

	}

	void AddImage(TCHAR * filename, TCHAR * title, int type)
	{
		LetterImage * letterimage;
		letterimage = new LetterImage(filename, title, type);
		_images.insert(pair<int, LetterImage *>(_imagecount,letterimage));
		_imagecount++;

	}

	int GetImageCount()
	{
		return _imagecount;
	}

	LetterImage * GetImage(int i) 
	{
		if (_images.count(i))
			return _images[i];
		else
			return NULL;
	}




};

class Alphabet {

	map <TCHAR, LetterInfo *> _alphabet;

public:
	Alphabet() 
	{
		LetterInfo * letterinfo;

		FILE * alphafile;

		_wfopen_s (&alphafile,L"alphabet.csv",L"r");

		if (alphafile != NULL) 
		{

			TCHAR lastLetter = 0;
			TCHAR letter = 0;
			TCHAR title[ALPHABET_TEXTLEN];
			TCHAR filename[ALPHABET_TEXTLEN];
			int filetype;

			while (fwscanf_s (alphafile, L"%c,%d,%[^,],%[^\n]\r\n",&letter,1,&filetype,&title,ALPHABET_TEXTLEN,&filename,ALPHABET_TEXTLEN) != EOF) 
			{
				if (letter != lastLetter) {
					if (lastLetter) {
						_alphabet.insert(pair<TCHAR, LetterInfo *>(lastLetter,letterinfo));
					}
					letterinfo = new LetterInfo(letter);
					lastLetter = letter;
				}

				letterinfo->AddImage(filename,title,filetype);
			}

			fclose(alphafile);

		}

	}

	int LetterExists(TCHAR letter)
	{
		
	}

	LetterInfo * GetLetter(TCHAR letter) 
	{
		if (_alphabet.count(letter))
			return _alphabet[letter];
		else
			return NULL;
	}

	~Alphabet()
	{

	}


};



