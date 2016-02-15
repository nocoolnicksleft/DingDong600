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
// File: SelectPlaylistDlg.h
//
// Desc: This dialog is used when the a playlist that contains no available
//       tracks is selected at start-up.  It allows the user to choose another
//       playlist.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _SELECTPLAYLISTDLG_H_
#define _SELECTPLAYLISTDLG_H_

#include "BaseDialog.h"
#include "resource.h"

class CSelectPlaylistDialog : public CBaseDialog
{
public:
    CSelectPlaylistDialog(HWND hwndParent) :
        CBaseDialog(hwndParent, IDD_SELECT_PLAYLIST)
    {
    }

    virtual BOOL DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif /* _SELECTPLAYLISTDLG_H_ */
