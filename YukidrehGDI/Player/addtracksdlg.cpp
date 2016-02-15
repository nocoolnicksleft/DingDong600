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
// File: AddTracksDlg.cpp
//
// Desc: This file contains the implementation for the Add Tracks dialog.
//
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "AddTracksDlg.h"

#include "PlaylistMgr.h"

#define MENU_ICON_WIDTH 16
#define MINSCREEN_WIDTH 640
#define FILENAME_WIDTH  240
#define PATHNAME_WIDTH  240

extern bool g_bSmallScreen;

CAddTracksDialog::CAddTracksDialog(HWND hwndParent, CPlaylist * pPlaylist) :
    CBaseDialog(hwndParent, IDD_ADD_TRACKS),
    m_pPlaylist(NULL),
    m_pModPlaylist(pPlaylist),
    m_himgFolders(NULL),
    m_himgLocationList(NULL),
    m_hwndMB(NULL),
    m_hwndTitle(NULL),
    m_hwndTrackList(NULL),
    m_bSelectedAll(false)
{
    m_himgFolders      = ImageList_LoadBitmap(g_hInst,
                                              MAKEINTRESOURCE(IDB_FOLDERS),
                                              MENU_ICON_WIDTH - 1,
                                              2,
                                              CLR_DEFAULT);

    m_himgLocationList = ImageList_LoadBitmap(g_hInst,
                                              MAKEINTRESOURCE(IDB_LOCATIONLIST),
                                              MENU_ICON_WIDTH - 1,
                                              4,
                                              CLR_DEFAULT);
}

CAddTracksDialog::~CAddTracksDialog()
{
    if (NULL != m_himgFolders)
    {
        ImageList_Destroy(m_himgFolders);
    }

    if (NULL != m_himgLocationList)
    {
        ImageList_Destroy(m_himgLocationList);
    }

    if (NULL != m_pModPlaylist)
    {
        m_pModPlaylist->Save();
    }
}

HRESULT CAddTracksDialog::CreateBars()
{
    HRESULT hr = S_OK;

    return hr;
}

HRESULT CAddTracksDialog::UpdateInfo()
{
    LONG cx     = 0,
         cxItem = 0;
    int  i      = 0;

    CPlaylistMgr * pManager = CPlaylistMgr::GetInstance();

    if (NULL == pManager)
    {
        return E_FAIL;
    }

    // loop through the current playlist and print out all media clips
    ListView_DeleteAllItems(m_hwndTrackList);

    pManager->UpdateLocalContent();
    m_pPlaylist = pManager->LocalContent();
    if (NULL == m_pPlaylist)
    {
        return S_OK;
    }

    CPlaylist::playlist_t * pListItem = m_pPlaylist->GetFirst();

    while (pListItem)
    {
        cxItem = UpdateItem(i, pListItem->pClip);

        if (cxItem > cx)
        {
            cx = cxItem;
        }

        pListItem = pListItem->pNext;
        i++;
    }

    //
    // Resize the list view if needed
    //
    RECT rc;

    GetWindowRect(m_hwndTrackList, &rc);

    if (cx < rc.right - rc.left)
    {
        cx = rc.right - rc.left;
    }

    if (ListView_GetCountPerPage(m_hwndTrackList) < ListView_GetItemCount(m_hwndTrackList))
    { 
        cx -= GetSystemMetrics(SM_CXVSCROLL);
    }

    ListView_SetColumnWidth(m_hwndTrackList, 0, cx);

    SendMessage(m_hwndTrackList, WM_SETREDRAW, TRUE, 0);

    InvalidateRect(m_hwndTrackList, NULL, TRUE);

    return S_OK;
}

LONG CAddTracksDialog::UpdateItem(int i, CMediaClip * pClip)
{
    LVITEM item;
    LPCTSTR pszPath;
    EFileLocation eLocation;

    pszPath   = pClip->GetPath();
    eLocation = pClip->GetLocation();

    item.mask     = LVIF_TEXT | LVIF_DI_SETITEM | LVIF_IMAGE;
    item.iItem    = i;
    item.iSubItem = 0;
    item.pszText  = (LPTSTR)pszPath;
    item.iImage   = (int)eLocation;

    ListView_InsertItem(m_hwndTrackList, &item);

    //
    // Get the text size
    //
    HDC  hdc;
    LONG cx = 0;
    SIZE size;

    hdc = ::GetDC(m_hwndDlg);

    if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size))
    {
        cx = size.cx;
    }

    ::ReleaseDC(m_hwndDlg, hdc);

    return cx;
}

BOOL CAddTracksDialog::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    BOOL fHandled = FALSE;

    switch (msg)
    {
        case WM_ACTIVATE:
        break;

        case WM_INITDIALOG:
            OnInitDialog();

            fHandled = TRUE;
        break;

        case WM_NOTIFY:
            fHandled = OnNotify((NMHDR*)lParam);
        break;

        case WM_COMMAND:
            int iCommand = HIWORD(wParam);
            int iControl = LOWORD(wParam);

            if (0 == iCommand || 0x1000 == iCommand)
            {
                fHandled = OnCommand(iControl);
            }
        break;
    }

    return fHandled;
}

BOOL CAddTracksDialog::OnCommand(int iControl)
{
    int  cTracks     = ListView_GetItemCount(m_hwndTrackList);
    int  iLastTrack  = m_pModPlaylist->GetCount();
    int  iTrack;
    BOOL fHandled    = TRUE;

    switch (iControl)
    {
        case ID_SELECT_ALL:
           for (iTrack = 0; iTrack <= cTracks; iTrack++)
           {
               ListView_SetItemState(m_hwndTrackList, iTrack, INDEXTOSTATEIMAGEMASK(m_bSelectedAll?1:2), LVIS_STATEIMAGEMASK);
           }

           m_bSelectedAll = !m_bSelectedAll;

           if (m_bSelectedAll)
           {
               SetDlgItemText(m_hwndDlg, ID_SELECT_ALL, L"Unselect All");
           }
           else
           {
               SetDlgItemText(m_hwndDlg, ID_SELECT_ALL, L"Select All");
           }
        break;

        case IDCANCEL:
            EndDialog(m_hwndDlg, iControl);
        break;

        case IDOK:
            for (iTrack = 0; iTrack < cTracks; iTrack++)
            {
                BOOL fChecked = ListView_GetCheckState(m_hwndTrackList, iTrack);

                if (NULL != m_pPlaylist && fChecked)
                {
                    CMediaClip * pClip = m_pPlaylist->GetTrack(iTrack);

                    if (NULL != pClip)
                    {
                        m_pModPlaylist->InsertTrack(++iLastTrack, pClip->GetPath());
                        m_pModPlaylist->IsCEPlaylist(true);
                    }
                }
            }

            EndDialog(m_hwndDlg, iControl);
        break;
    }

    return fHandled;
}

void CAddTracksDialog::OnInitDialog()
{
    if (NULL == m_pModPlaylist)
    {
        PostMessage(m_hwndDlg, WM_COMMAND, IDCANCEL, 0);

        return;
    }

    m_hwndTitle     = GetDlgItem(m_hwndDlg, IDC_ADD_TRACKS);
    m_hwndTrackList = GetDlgItem(m_hwndDlg, IDC_TRACK_LIST);

    //
    // Check to see if the screen is small
    //
    if (g_bSmallScreen)
    {
        RECT rcWorkArea;
        RECT rcWnd, rcList;

        GetWindowRect(m_hwndDlg,       &rcWnd);
        GetWindowRect(m_hwndTrackList, &rcList);

        if (!SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, 0))
        {
            HDC hdc = ::GetDC(NULL);

            rcWorkArea.left = 0;
            rcWorkArea.top  = 0;

            rcWorkArea.right  = GetDeviceCaps(hdc, HORZRES);
            rcWorkArea.bottom = GetDeviceCaps(hdc, VERTRES) - GetSystemMetrics(SM_CYMENU);

            ::ReleaseDC(NULL, hdc);
        }

        MoveWindow(m_hwndDlg,
                   rcWorkArea.left,
                   rcWorkArea.top,
                   rcWorkArea.right,
                   rcWorkArea.bottom,
                   TRUE);

        rcWorkArea.left   += rcList.left - rcWnd.left;
        rcWorkArea.right  -= rcList.left - rcWnd.left;
        rcWorkArea.right  += rcList.right - rcWnd.right - 2*GetSystemMetrics(SM_CXDLGFRAME);

        rcWorkArea.top    += rcList.top  - rcWnd.top - GetSystemMetrics(SM_CYCAPTION);
        rcWorkArea.bottom -= rcList.top  - rcWnd.top;
        rcWorkArea.bottom += rcList.bottom - rcWnd.bottom;

        MoveWindow(m_hwndTrackList,
                   rcWorkArea.left,
                   rcWorkArea.top,
                   rcWorkArea.right,
                   rcWorkArea.bottom,
                   TRUE);
    }

    ListView_SetExtendedListViewStyle(m_hwndTrackList, LVS_EX_CHECKBOXES);

    HRESULT hr;

    hr = CreateBars();

    if (FAILED(hr))
    {
        return;
    }

    if (m_himgLocationList)
    {
        ListView_SetImageList(m_hwndTrackList, m_himgLocationList, LVSIL_SMALL);
    }

    TCHAR szTitle[MAX_PATH];

    ULONG cchTitleLength = SendMessage(m_hwndTitle, WM_GETTEXT, MAX_PATH, (LPARAM)szTitle);

    LPCTSTR pszShortName = NULL;

    if (NULL != m_pModPlaylist)
    {
        pszShortName = m_pModPlaylist->GetName();
    }

    if (SUCCEEDED(hr) && NULL != pszShortName)
    {
        // cchTitleLength is the length of the string retrieved by WM_GETTEXT
        // to avoid overrunning szTitle, we limit the copy to the remaining room in the array 
        StringCchCopy(&szTitle[cchTitleLength], MAX_PATH - cchTitleLength, pszShortName);
        szTitle[MAX_PATH - 1] = TEXT('\0');
    }

    SendMessage(m_hwndTitle, WM_SETTEXT, 0, (LPARAM)szTitle);

    LVCOLUMN lvc;

    memset(&lvc, 0, sizeof (lvc));

    lvc.mask = LVCF_FMT;
    lvc.fmt  = LVCFMT_LEFT;

    ListView_InsertColumn(m_hwndTrackList, 0, &lvc);

    lvc.mask     = LVCF_WIDTH | LVCF_FMT | LVCF_SUBITEM;
    lvc.fmt      = LVCFMT_LEFT;
    lvc.iSubItem = 1;
    lvc.cx       = FILENAME_WIDTH;
    ListView_SetColumn(m_hwndTrackList, 0, &lvc);

    UpdateInfo();

    SetFocus(m_hwndTrackList);
}

BOOL CAddTracksDialog::OnNotify(NMHDR * pNotify)
{
    NMLISTVIEW * pnmlv = (NMLISTVIEW *)pNotify;
    BOOL fHandled = FALSE;

    if (NM_CUSTOMDRAW == pNotify->code
        && m_hwndTrackList == pNotify->hwndFrom)
    {
        NMLVCUSTOMDRAW * pcd = (NMLVCUSTOMDRAW *)pNotify;

        if (CDDS_PREPAINT == pcd->nmcd.dwDrawStage)
        {
            SetWindowLong(m_hwndDlg, DWL_MSGRESULT, CDRF_NOTIFYITEMDRAW);

            fHandled = TRUE;
        }
        else if (CDDS_ITEMPREPAINT == pcd->nmcd.dwDrawStage)
        {
            SetBkMode(pcd->nmcd.hdc, OPAQUE);
            pcd->clrTextBk = RGB(255,255,255);

            SetWindowLong(m_hwndDlg, DWL_MSGRESULT, CDRF_NEWFONT);

            fHandled = TRUE;
        }
    }
    else if (LVN_DELETEALLITEMS == pNotify->code)
    {
        SetWindowLong(m_hwndDlg, DWL_MSGRESULT, TRUE);

        fHandled = TRUE;
    }

    return fHandled;
}

void CAddTracksDialog::OnSize(int cx, int cy)
{
}
