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
// File: AddDeleteDlg.h
//
// Desc: This dialog (Organize Favorites) allows playlists to be created,
//       renamed, copied, or deleted.  When leaving this dialog, which ever
//       entry is selected at the time the OK button is pressed will be used
//       as the "current playlist."  If that playlist is empty, the Add Tracks
//       dialog will be brought up to allow tracks to be added.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _ADDDELETEDLG_H_
#define _ADDDELETEDLG_H_

#include "BaseDialog.h"
#include <shellsdk.h>

class CAddDeleteDialog : public CBaseDialog
{
public:
    CAddDeleteDialog(HWND hwndParent, HWND hwndMain);

    HRESULT AddPlaylist(LPTSTR szPlaylistName, int iPos);
    HRESULT DeletePlaylist(int iPlaylist);

    void    DisplayContextMenu(POINT ptAction);

    HRESULT LoadAllPlaylists();

    BOOL    DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);

    BOOL    OnCommand(int iControl);
    void    OnInitDialog();
    BOOL    OnNotify(NMHDR * pNotify);
    void    OnSize(int cx, int cy);

private:
    HWND m_hwndAdd;
    HWND m_hwndDelete;
    HWND m_hwndRename;
    HWND m_hwndList;
    HWND m_hwndMain;
    HWND m_hwndStatic;

    int  m_iCurrentPlaylist;

    SHACTIVATEINFO m_shai;
    BOOL m_fSipOn;
};

#endif /* _ADDDELETEDLG_H_ */
