//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
///////////////////////////////////////////////////////////////////////////////
// File: CEPlayerUtil.cpp
//
// Desc: This file contains the implementation for various utility functions.
//
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>

#include "CEPlayerUtil.h"
#include "decibels.h"

extern TCHAR s_szBasePath[];

int DrawEllipsisText(HDC hDC, LPCTSTR pszText, int cchText, LPRECT prcText, UINT uFormat)
{
    //
    // Calculate number of characters that will fit in given rectangle
    //
    int cchFit = 0;
    int cxText = prcText->right - prcText->left;
    RECT rcText = *prcText;
    SIZE szText;

    GetTextExtentExPoint(hDC, pszText, cchText, cxText, &cchFit, NULL, &szText);

    //
    // If ellipsis is necessary (and this is left-justified text), draw it
    //
    if( ( 0 == ( uFormat & ( DT_CENTER | DT_RIGHT ) ) ) && ( cchFit < cchText ) )
    {
        //
        // Get width of ellipsis
        //
        SIZE szEllipsis;

        GetTextExtentExPoint( hDC, TEXT("..."), 3, 0, NULL, NULL, &szEllipsis );

        //
        // Get number of characters (and their width) that will fit along with
        // the ellipsis
        //
        cxText -= szEllipsis.cx;

        GetTextExtentExPoint( hDC, pszText, cchText, cxText, &cchFit, NULL, &szText);

        //
        // Draw the ellipsis to the right of the characters that fit
        //
        rcText.left += szText.cx;

        DrawText( hDC, TEXT("..."), 3, &rcText, uFormat );

        //
        // Adjust the text rectangle to exclude the ellipsis
        //
        rcText.right = rcText.left;
        rcText.left = prcText->left;
    }

    //
    // Draw as many characters of given text will fit in remaining space
    //
    int iRes = DrawText( hDC, pszText, cchFit, &rcText, uFormat );

    return( iRes );
}

bool IsBadFilenameChar(TCHAR c)
{
    if (c < 32 || NULL != _tcschr(TEXT("\"/\\[]:;|=,?*<>"), c))
    {
        return true;
    }

    return false;
}

bool IsLocalPlaylist(LPCTSTR pszPath)
{
    LPCTSTR pszRealPath = pszPath;
    DWORD   dwAttrib;
    int     iLength;
    bool    bResult     = false;

    // remove a "file://"

    while (isspace(*pszRealPath))
    {
        pszRealPath++;
    }

    if (TEXT('F') == toupper(pszRealPath[0])
        && TEXT('I') == toupper(pszRealPath[1])
        && TEXT('L') == toupper(pszRealPath[2])
        && TEXT('E') == toupper(pszRealPath[3])
        && TEXT(':') == pszRealPath[4]
        && TEXT('/') == pszRealPath[5]
        && TEXT('/') == pszRealPath[6])
    {
        pszRealPath += 6;
    }

    dwAttrib = GetFileAttributes(pszRealPath);

    if (0xffffffff != dwAttrib)
    {
        bResult = true;
    }

    //
    // A network file "\\blah\blah\blah.asx" isn't local
    //

    if (TEXT('\\') == pszRealPath[0]
        && TEXT('\\') == pszRealPath[1])
    {
        bResult = false;
    }

    if (bResult)
    {
        bResult = false;
        iLength = _tcslen(pszRealPath);

        if (iLength > 4
            && TEXT('X') == toupper(pszRealPath[iLength - 1])
            && TEXT('S') == toupper(pszRealPath[iLength - 2])
            && TEXT('A') == toupper(pszRealPath[iLength - 3])
            && TEXT('.') == pszRealPath[iLength - 4])
        {
            // Only consider a playlist to be local if it is in the base path
            // directory
            if (iLength > (int)_tcslen(s_szBasePath)
                && 0 == _tcsnicmp(s_szBasePath, pszRealPath, _tcslen(s_szBasePath)))
            {
                bResult = true;
            }
        }
    }

    return bResult;
}

HFONT GetBoldFont()
{
    LOGFONT logfont;
    HDC     hdc;

    memset(&logfont, 0, sizeof (logfont));

    hdc = GetDC(NULL);
    logfont.lfHeight = -(8 * GetDeviceCaps(hdc, LOGPIXELSY) / 72);
    ReleaseDC(NULL, hdc);

    logfont.lfWeight         = FW_BOLD;
    logfont.lfCharSet        = DEFAULT_CHARSET;
    logfont.lfOutPrecision   = OUT_DEFAULT_PRECIS;
    logfont.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
    logfont.lfQuality        = DEFAULT_QUALITY;
    logfont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

    StringCchCopy(logfont.lfFaceName, LF_FACESIZE, TEXT("MS Sans Serif"));

    return CreateFontIndirect(&logfont);
}

HFONT GetNormalFont()
{
    LOGFONT logfont;
    HDC     hdc;

    memset(&logfont, 0, sizeof (logfont));

    hdc = GetDC(NULL);
    logfont.lfHeight = -(8 * GetDeviceCaps(hdc, LOGPIXELSY) / 72);
    ReleaseDC(NULL, hdc);

    logfont.lfWeight         = FW_NORMAL;
    logfont.lfCharSet        = DEFAULT_CHARSET;
    logfont.lfOutPrecision   = OUT_DEFAULT_PRECIS;
    logfont.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
    logfont.lfQuality        = DEFAULT_QUALITY;
    logfont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

    StringCchCopy(logfont.lfFaceName, LF_FACESIZE, TEXT("MS Sans Serif"));

    return CreateFontIndirect(&logfont);
}

LONG VolumeLinToLog(short nLinKnobValue)
{
    LONG lLinMin = DBToAmpFactor(AX_MIN_VOLUME);
    LONG lLinMax = DBToAmpFactor(AX_MAX_VOLUME);

    LONG lLinTemp = (LONG)(nLinKnobValue - MIN_VOLUME_RANGE) * (lLinMax - lLinMin)
        / (MAX_VOLUME_RANGE - MIN_VOLUME_RANGE) + lLinMin;

    LONG lLogValue = AmpFactorToDB(lLinTemp);

    return lLogValue;
}

short VolumeLogToLin(LONG lLogValue)
{
    LONG lLinMin = DBToAmpFactor(AX_MIN_VOLUME);
    LONG lLinMax = DBToAmpFactor(AX_MAX_VOLUME);

    short nLinKnobValue = (short)( ((LONG) DBToAmpFactor(lLogValue) - lLinMin) *
        (MAX_VOLUME_RANGE - MIN_VOLUME_RANGE) / (lLinMax - lLinMin) + MIN_VOLUME_RANGE);

    return nLinKnobValue;
}
