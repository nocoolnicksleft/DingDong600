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
// File: CEPlayerUtil.h
//
// Desc: These utility functions are by various components of the player.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _CEPLAYERUTIL_H_
#define _CEPLAYERUTIL_H_

int DrawEllipsisText(HDC hDC, LPCTSTR szText, int cchText, LPRECT prcText, UINT uFormat);

// This function returns true if the given character is not allowed in a filename
bool IsBadFilenameChar(TCHAR c);

// This function returns true if the given playlist is on the local device.
bool IsLocalPlaylist(LPCTSTR pszPath);

// These functions are used to change fonts.
HFONT GetBoldFont();
HFONT GetNormalFont();

// These functions are used to convert the volume to or from linear values
LONG  VolumeLinToLog(short nLinKnobValue);
short VolumeLogToLin(LONG lLogValue);

#endif /* _CEPLAYERUTIL_H_ */
