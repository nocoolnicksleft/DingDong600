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
// File: PlaylistDlg.h
//
// Desc: This dialog shows a view of the current playlist and allows another
//       playlist to be selected from a dropdown menu.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _PLAYLISTDLG_H_
#define _PLAYLISTDLG_H_

#include <windows.h>

#include "BaseDialog.h"
#include "Plist.h"

enum
{
    I_ALL_MY_MUSIC = 0,
    I_PLAYLIST,
    I_ALL_PLAYLISTS
};

typedef struct
{
    int    iImage;
    TCHAR  szText[50];
} DrawMenuStruct;

class CPlaylistDialog : public CBaseDialog
{
public:
    CPlaylistDialog(HWND hwndParent, bool bOrganizeFavorites = false);
    ~CPlaylistDialog();

    virtual BOOL DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);

protected:
    void    OnInitDialog();
    BOOL    OnNotify(NMHDR *);

    void    OnDropdownMenu();
    void    OnMeasureItem(MEASUREITEMSTRUCT *);
    void    OnDrawItem(DRAWITEMSTRUCT *);
    void    OnContextMenu(POINT);

    HRESULT CreateBars();
    void    EnableMenubarIcons();
    void    UpdateInfo();
    void    SetDropdownText(LPTSTR, int);

private:
    HIMAGELIST  m_himgPlaylist;
    HIMAGELIST  m_himgLocationList;
    HWND        m_hwndDropdown;
    HWND        m_hwndMB;
    HWND        m_hwndTrackList;
    HFONT       m_hFont;

    CPlaylist * m_pPlaylist;

    bool        m_bOrganizeFavorites;
};

#endif /* _PLAYLISTDLG_H_ */
