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
// File: AddTracksDlg.h
//
// Desc: This dialog allows media to be added to a playlist.  When the OK
//       button is pressed, each selected track is added to the playlist.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _ADDTRACKSDLG_H_
#define _ADDTRACKSDLG_H_

#include <windows.h>
#include <commctrl.h>

#include "BaseDialog.h"
#include "Plist.h"

class CAddTracksDialog : public CBaseDialog
{
public:
    CAddTracksDialog(HWND hwndParent, CPlaylist * pPlaylist);
    ~CAddTracksDialog();

    HRESULT CreateBars();
    HRESULT UpdateInfo();
    LONG    UpdateItem(int i, CMediaClip * pClip);

    virtual BOOL DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);

    BOOL OnCommand(int iControl);
    void OnInitDialog();
    BOOL OnNotify(NMHDR *pNotify);
    void OnSize(int cx, int cy);

private:
    CPlaylist * m_pPlaylist;
    CPlaylist * m_pModPlaylist;

    HIMAGELIST m_himgFolders;
    HIMAGELIST m_himgLocationList;

    HWND m_hwndMB;
    HWND m_hwndTitle;
    HWND m_hwndTrackList;

    bool m_bSelectedAll;
};

#endif /* _ADDTRACKSDLG_H_ */
