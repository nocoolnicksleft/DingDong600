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
// Desc: This file contains the implementation for the Organize Playlists
//       dialog.
//
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <commctrl.h>

#include "resource.h"

#include "AddDeleteDlg.h"
#include "playlistmgr.h"
#include "CEPlayerUtil.h"
#include "PlayerWindow.h"

#include "aygshell_helper.h"

#define MAX_PLAYLIST_NAME 60

extern bool g_bSmallScreen;

CAddDeleteDialog::CAddDeleteDialog(HWND hwndParent, HWND hwndMain) :
    CBaseDialog(hwndParent, IDD_PLAYLIST_ADD_DELETE),
    m_hwndAdd(NULL),
    m_hwndDelete(NULL),
    m_hwndRename(NULL),
    m_hwndList(NULL),
    m_hwndMain(NULL),
    m_hwndStatic(NULL),
    m_iCurrentPlaylist(-1),
    m_fSipOn(FALSE)
{
	memset(&m_shai, 0, sizeof(SHACTIVATEINFO));
	m_shai.cbSize = sizeof(SHACTIVATEINFO);
}

HRESULT CAddDeleteDialog::AddPlaylist(LPTSTR szPlaylistName, int iPos)
{
	// validate arguments
	if (NULL == szPlaylistName)
	{
		ASSERT(FALSE);
		return E_INVALIDARG;
	}

	LVITEM item;
	memset(&item, 0, sizeof (item));

	item.mask    = LVIF_TEXT | LVIF_PARAM;
	item.iItem   = ListView_GetItemCount(m_hwndList);
	item.pszText = szPlaylistName;

	if (iPos >= 0)
		item.iItem = iPos;

	int nIndex = ListView_InsertItem(m_hwndList, &item);

    return (nIndex >= 0) ? S_OK : E_FAIL;
}

HRESULT CAddDeleteDialog::DeletePlaylist(int iPlaylist)
{
    HRESULT hr;

    do
    {
        LVITEM item;

        memset(&item, 0, sizeof (item));

        item.mask = LVIF_PARAM;
        item.iItem = iPlaylist;

        if (!ListView_GetItem(m_hwndList, &item))
        {
            hr = E_FAIL;
            break;
        }

        delete [] (TCHAR*)item.pszText;

        if (!ListView_DeleteItem(m_hwndList, iPlaylist))
        {
            hr = E_FAIL;
            break;
        }

        hr = S_OK;
    } while (FALSE);

    return hr;
}

void CAddDeleteDialog::DisplayContextMenu(POINT ptAction)
{
    HMENU hmenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDM_PLAYLISTS_CONTEXT));

    if (NULL == hmenu)
    {
        return;
    }

    HMENU hmenuDD = GetSubMenu(hmenu, 0);

    if (NULL != hmenuDD)
    {
        int cSelected = ListView_GetSelectedCount(m_hwndList);

        // optionally disable various menu items, based on count of selected items
        if (1 != cSelected)
        {
           EnableMenuItem(hmenuDD, ID_CREATE_COPY, MF_BYCOMMAND | MF_GRAYED);
           EnableMenuItem(hmenuDD, IDC_RENAME_PLAYLIST, MF_BYCOMMAND | MF_GRAYED);
        }

        if (cSelected <= 0)
        {
           EnableMenuItem(hmenuDD, IDC_DELETE_PLAYLIST, MF_BYCOMMAND | MF_GRAYED);
        }

        TrackPopupMenuEx(hmenuDD, TPM_LEFTALIGN | TPM_TOPALIGN, ptAction.x,
                         ptAction.y, m_hwndDlg, NULL);
    }

    DestroyMenu(hmenu);
}

HRESULT CAddDeleteDialog::LoadAllPlaylists()
{
	CPlaylistMgr * pManager = CPlaylistMgr::GetInstance();
	int iFileCount;

	if (NULL != pManager)
		iFileCount = pManager->GetPlaylistCount();
	else
		iFileCount = 0;

	ListView_DeleteAllItems(m_hwndList);
	ListView_SetItemCount(m_hwndList, iFileCount);

	if (!pManager) return S_OK;

	for (int i = 0; i < iFileCount; i++)
	{
		LPTSTR szName = pManager->GetDisplayName(i);
		AddPlaylist(szName, i);
		if (szName) delete [] szName;
	}

    return S_OK;
}

BOOL CAddDeleteDialog::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    BOOL fHandled = FALSE;
    int iCommand = HIWORD(wParam);
    int iControl = LOWORD(wParam);

    switch (msg)
    {
        case WM_INITDIALOG:
            OnInitDialog();
            fHandled = TRUE;
            if( g_bSmallScreen && g_AygshellHelper.Loaded() )
            {
                SHINITDLGINFO shidi;
                shidi.dwMask = SHIDIM_FLAGS;
                shidi.dwFlags = SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_SIPDOWN | SHIDIF_DONEBUTTON;
                shidi.hDlg = m_hwndDlg;
                g_AygshellHelper.SHInitDialog( &shidi );
            }
        break;

        case WM_COMMAND:
            if (0 == iCommand || 0x1000 == iCommand)
            {
                fHandled = OnCommand(iControl);
            }
        break;

        case WM_NOTIFY:
            fHandled = OnNotify((NMHDR*)lParam);
        break;

		case WM_SETTINGCHANGE:
			if (g_bSmallScreen && g_AygshellHelper.Loaded())
			{
				BOOL fRet, fSipOn;

				// fHandled = 0 if WM_SETTINGSCHANGE handled
				fRet = g_AygshellHelper.SHHandleWMSettingChange(
								m_hwndDlg, wParam, lParam, &m_shai);
				ASSERT(fRet);

				SIPINFO si;
				memset(&si, 0, sizeof(SIPINFO));
				si.cbSize = sizeof(SIPINFO);
				fRet = g_AygshellHelper.SHSipInfo(
						SPI_GETSIPINFO, (UINT)lParam, &si, 0);
				ASSERT(fRet);
				fSipOn = (si.fdwFlags & SIPF_ON);

				// if SIP status changed, snap listview ctl to dlg
				if (fSipOn != m_fSipOn)
				{
					// resize listview control inside dialog
					RECT rcDlg, rcLst;
					int i = -1;

					fRet = GetWindowRect(m_hwndDlg, &rcDlg); ASSERT(fRet);
					fRet = GetWindowRect(m_hwndList, &rcLst); ASSERT(fRet);
					fRet = SetWindowPos(m_hwndList, NULL, 0, 0,
								rcLst.right - rcLst.left,
								rcDlg.bottom - 10 - rcLst.top,
								SWP_NOMOVE | SWP_NOREPOSITION |
								SWP_NOZORDER); ASSERT(fRet);

					i = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
					if (i >= 0) ListView_EnsureVisible(m_hwndList, i, FALSE);
				}

				m_fSipOn = fSipOn;
				fHandled = FALSE;	// fHandled = 0 if WM_SETTINGSCHANGE handled
			}
			else fHandled = TRUE;	// return non-zero if WM_SETTINGSCHANGE not handled
		break;
    }

    return fHandled;
}

BOOL CAddDeleteDialog::OnCommand(int iControl)
{
    int iPlaylist = -1;
    HRESULT hr = S_OK;
    HWND hwndEdit = NULL;
    BOOL fHandled = FALSE;
    LPCTSTR szFullName;
    TCHAR szNewName[MAX_PATH];
    TCHAR szCaption[MAX_PATH];
    TCHAR szText[MAX_PATH];
    int iItem;
    int iItemBefore;
    int cItem;
    int cmd;
    CPlaylist * pNewPlaylist = NULL;
    CPlaylist * pPlaylist    = NULL;
    CPlaylistMgr * pManager  = CPlaylistMgr::GetInstance();

    switch (iControl)
    {
        case ID_OPEN_PLAYLIST:
            if (NULL == pManager)
            {
                break;
            }

            if (1 != ListView_GetSelectedCount(m_hwndList))
            {
                break;
            }

            iPlaylist = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
            if (iPlaylist < 0)
            {
                break;
            }

            hr = S_OK;

            pManager->SetCurrentPlaylist(iPlaylist);
        break;

        case ID_CREATE_COPY:
            if (NULL == pManager)
            {
                break;
            }

            if (1 != ListView_GetSelectedCount(m_hwndList))
            {
                break;
            }

            iPlaylist = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
            if (iPlaylist < 0)
            {
                break;
            }

            pPlaylist = pManager->GetPlaylist(iPlaylist);

            if (pPlaylist)
            {
                szFullName = pPlaylist->GetPath();
            }
            else
            {
                break;
            }

            if (!pManager->CreateUniqueName(szNewName, MAX_PATH))
            {
                break;
            }

            if (!CopyFile(szFullName, szNewName, FALSE))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                break;
            }

            {
                int iList = pManager->AddPlaylist(szNewName, iPlaylist+1);
                if (-1 == iList)
                {
                    hr = E_FAIL;
                    break;
                }

                // szNewName will be unique within it's folder, but not
                // necessarily unique across the file system.  So
                // differentiate the display name if necessary by adding
                // the path (szPath is NULL if playlist is unique)
                LPTSTR szName = pManager->GetDisplayName(iList);
                hr = AddPlaylist(szName, iPlaylist+1);
                if (szName) delete [] szName;
                if (FAILED(hr)) break;
            }

            if (iPlaylist < m_iCurrentPlaylist)
            {
                m_iCurrentPlaylist++;
            }

            LoadAllPlaylists();

            SetFocus(m_hwndList);
            hwndEdit = ListView_EditLabel(m_hwndList, iPlaylist+1);
            SendMessage(hwndEdit, EM_SETLIMITTEXT, MAX_PLAYLIST_NAME, 0);
        break;

        case IDC_NEW_PLAYLIST:
            pNewPlaylist = NULL;

            if (NULL == pManager)
            {
                break;
            }

            do
            {
                if (!CPlaylistMgr::CreateUniqueName(szNewName, MAX_PATH))
                {
                    break;
                }

                int iIndex = pManager->AddPlaylist(szNewName);
                if (iIndex < 0)
                {
                    hr = E_FAIL;
                    break;
                }

                pNewPlaylist = pManager->GetPlaylist(iIndex);
                if (NULL == pNewPlaylist)
                {
                    hr = E_FAIL;
                    break;
                }

                pNewPlaylist->IsCEPlaylist(true);
                if (!pNewPlaylist->Save())
                {
                    pNewPlaylist = NULL;
                    break;
                }

                LoadAllPlaylists();

                ListView_EnsureVisible(m_hwndList, iIndex, FALSE);

                SetFocus(m_hwndList);
                hwndEdit = ListView_EditLabel(m_hwndList, iIndex);
                SendMessage(hwndEdit, EM_SETLIMITTEXT, MAX_PLAYLIST_NAME, 0);
            } while (FALSE);

            pNewPlaylist = NULL;

            fHandled = TRUE;
            break;
        break;

        case IDC_DELETE_PLAYLIST:
            if (NULL == pManager)
            {
                break;
            }

            LoadString(g_hInst, IDS_APP_TITLE, szCaption, MAX_PATH);
            LoadString(g_hInst, IDS_DELETE_CONFIRM, szText, MAX_PATH);

            cmd = MessageBox(m_hwndDlg, szText, szCaption, MB_YESNO | MB_ICONEXCLAMATION);
            if (IDYES != cmd)
            {
                break;
            }

            iItem       = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
            iItemBefore = iItem - 1;

            while (iItem >= 0)
            {
                // Let the player know to stop playing if the current playlist
                // is being deleted
                g_pPlayerWindow->PlaylistDelete(pManager->GetPlaylist(iItem));

                if (false == pManager->DeletePlaylist(iItem))
                {
                    ListView_SetItemState(m_hwndList, iItem, 0, LVIS_SELECTED);
                    break;
                }

                if (-1 == pManager->GetCurrentPlaylistID())
                {
                    m_iCurrentPlaylist = -1;
                }

                SendMessage(m_hwndParent, WM_COMMAND, ID_DELETED_PLAYLIST, (LPARAM)NULL);

                if (iItem < m_iCurrentPlaylist)
                {
                    m_iCurrentPlaylist--;
                }

                iItem = ListView_GetNextItem(m_hwndList, iItem, LVNI_SELECTED);
            }

            SetFocus(m_hwndList);

            LoadAllPlaylists();

            cItem = ListView_GetItemCount(m_hwndList);

            if (cItem > 0)
            {
                int iNewItem = min(iItemBefore+1, cItem-1);

                if (0 == iItemBefore)
                {
                    iNewItem = 0;
                }

                ListView_SetItemState(m_hwndList, iNewItem, LVIS_SELECTED, LVIS_SELECTED);
                ListView_EnsureVisible(m_hwndList, iNewItem, FALSE);
            }

            fHandled = TRUE;
        break;

        case IDC_RENAME_PLAYLIST:
            if (1 != ListView_GetSelectedCount(m_hwndList))
            {
                break;
            }

            iPlaylist = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);

            if (iPlaylist < 0)
            {
                break;
            }

            SetFocus(m_hwndList);
            hwndEdit = ListView_EditLabel(m_hwndList, iPlaylist);
            SendMessage(hwndEdit, EM_SETLIMITTEXT, MAX_PLAYLIST_NAME, 0);

            fHandled = TRUE;
        break;

        case IDOK:
        case IDCANCEL:
            // Treat the OK button as a request to open the currently selected item
            SendMessage(m_hwndDlg, WM_COMMAND, ID_OPEN_PLAYLIST, 0);

            EndDialog(m_hwndDlg, iControl);

            fHandled = TRUE;
        break;
    }

    return fHandled;
}

void CAddDeleteDialog::OnInitDialog()
{
    RECT rc;

    // save our child window handles for later use
    m_hwndStatic = GetDlgItem(m_hwndDlg, IDC_ADD_DELETE_TITLE);
    m_hwndAdd    = GetDlgItem(m_hwndDlg, IDC_NEW_PLAYLIST);
    m_hwndRename = GetDlgItem(m_hwndDlg, IDC_RENAME_PLAYLIST);
    m_hwndDelete = GetDlgItem(m_hwndDlg, IDC_DELETE_PLAYLIST);
    m_hwndList   = GetDlgItem(m_hwndDlg, IDC_PLAYLISTS);

    //
    // Check to see if the screen is small
    //
    if (g_bSmallScreen)
    {
        RECT rcWorkArea;
        RECT rcWnd, rcList;

        GetWindowRect(m_hwndDlg,  &rcWnd);
        GetWindowRect(m_hwndList, &rcList);

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

        MoveWindow(m_hwndList,
                   rcWorkArea.left,
                   rcWorkArea.top,
                   rcWorkArea.right,
                   rcWorkArea.bottom,
                   TRUE);
    }

    ListView_SetExtendedListViewStyle(m_hwndList, LVS_EX_FULLROWSELECT);

    GetClientRect(m_hwndList, &rc);

    // add a column to the list of playlists
    LVCOLUMN lvc;
    memset(&lvc, 0, sizeof (lvc));
    lvc.mask = LVCF_FMT | LVCF_WIDTH;
    lvc.fmt  = LVCFMT_LEFT;
    lvc.cx   = (rc.right - rc.left);

    ListView_InsertColumn(m_hwndList, 0, &lvc);

    // Load a snapshot of all the playlist files on the device
    LoadAllPlaylists();

    SetFocus(m_hwndList);

    // Scroll so that the current playlist (the selected item) is visible
    if (1 == ListView_GetSelectedCount(m_hwndList))
    {
        int iPlaylist = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
        if (iPlaylist >= 0)
        {
            ListView_EnsureVisible(m_hwndList, iPlaylist, FALSE);
        }
    }
}

BOOL CAddDeleteDialog::OnNotify(NMHDR * pNotify)
{
    NMLISTVIEW *pnlv = (NMLISTVIEW *)pNotify;
    BOOL        fHandled = FALSE;
    TCHAR       szCaption[MAX_PATH];
    TCHAR       szText[MAX_PATH];

    if (LVN_ITEMCHANGED == pNotify->code)
    {
        int cSelectedFiles = ListView_GetSelectedCount(m_hwndList);

        EnableWindow(m_hwndRename, 1 == cSelectedFiles);
        EnableWindow(m_hwndDelete, cSelectedFiles >= 1);

        SetWindowLong(m_hwndDlg, DWL_MSGRESULT, 0);
        fHandled = TRUE;
    }
    else if (NM_CUSTOMDRAW == pNotify->code
             && m_hwndList == pNotify->hwndFrom)
    {
        NMLVCUSTOMDRAW *pcd = (NMLVCUSTOMDRAW*)pNotify;

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
    else if (LVN_BEGINLABELEDIT == pNotify->code)
    {
    	// enable the dialog buttons
        EnableWindow(m_hwndAdd, FALSE);
        EnableWindow(m_hwndDelete, FALSE);
        EnableWindow(m_hwndRename, FALSE);

		// get this playlist's name
		LPTSTR szName = NULL;
		NMLVDISPINFO *pInfo = (NMLVDISPINFO*)pNotify;

		do
		{
	        CPlaylistMgr * pManager = CPlaylistMgr::GetInstance();
			CPlaylist* pList = pManager->GetPlaylist(pInfo->item.iItem);
			ASSERT(pList);
			if (!pList)
			{
				fHandled = TRUE;
				break;
			}

			int iLen = _tcslen(pList->GetName()) + 1;
			szName = new TCHAR[iLen];
			ASSERT(szName);
			if (!szName)
			{
				fHandled = TRUE;
				break;
			}

			szName[iLen - 1] = 0;
			StringCchCopy(szName, iLen, pList->GetName());
			ASSERT(0 == szName[iLen - 1]);
			szName[iLen - 1] = 0;

			// grab the edit box and replace the "name (path)"
			// form with just "name"
			HWND hEdit = ListView_GetEditControl(m_hwndList);
			ASSERT(hEdit);
			if (!hEdit)
			{
				fHandled = TRUE;
				break;
			}

			SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)szName);
		} while (FALSE);

		if (szName)
			delete [] szName;
    }
    else if (LVN_ENDLABELEDIT == pNotify->code)
    {
        NMLVDISPINFO *pInfo = (NMLVDISPINFO*)pNotify;

        BOOL fAcceptUserName = FALSE;
        int cSelectedFiles   = ListView_GetSelectedCount(m_hwndList);
        bool bEmpty          = true;
        bool bBad            = false;

        do
        {
            TCHAR szNewName[MAX_PATH];
            LPCTSTR pszSlash;
            TCHAR * pszExt;

            if (NULL == pInfo->item.pszText)
            {
               // The user cancelled the edit
               break;
            }

            // check to see if it's a bad name
            int  cchUserText = _tcslen(pInfo->item.pszText);

            for (int i = 0; i < cchUserText; i++)
            {
                if (!_istspace(pInfo->item.pszText[i]))
                {
                    bEmpty = false;
                }

                if (IsBadFilenameChar(pInfo->item.pszText[i]))
                {
                    bBad = true;
                    break;
                }
            }

            if (bEmpty || bBad)
            {
                LoadString( g_hInst, IDS_APP_TITLE, szCaption, MAX_PATH);
                LoadString( g_hInst, IDS_PLAYLIST_NAME_INVALID, szText, MAX_PATH);
                MessageBox( m_hwndDlg, szText, szCaption, MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL );
                break;
            }

            // strip off ".asx" if they typed it
            pszExt   = _tcsrchr(pInfo->item.pszText, TEXT('.'));
            pszSlash = _tcsrchr(pInfo->item.pszText, TEXT('\\'));

            if (NULL != pszExt && pszSlash < pszExt)
            {
                *pszExt = TEXT('\0');
                StringCchCopy(szNewName, MAX_PATH, pInfo->item.pszText);
                szNewName[ MAX_PATH - 1 ] = L'\0';
                *pszExt = TEXT('.');
            }
            else
            {
                StringCchCopy(szNewName, MAX_PATH, pInfo->item.pszText);
                szNewName[ MAX_PATH - 1 ] = L'\0';
            }

            // let the playlist manager change the name
            CPlaylistMgr * pManager = CPlaylistMgr::GetInstance();

            if (NULL != pManager)
            {
                if( !pManager->RenamePlaylist(pInfo->item.iItem, szNewName) )
                {
                    LoadString( g_hInst, IDS_APP_TITLE, szCaption, MAX_PATH);
                    LoadString( g_hInst, IDS_RENAME_FAIL, szText, MAX_PATH);
                    MessageBox( m_hwndDlg, szText, szCaption, MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL );
                }
            }

            // update the list view
            LoadAllPlaylists();
            

        } while (FALSE);

        SetWindowLong(m_hwndDlg, DWL_MSGRESULT, fAcceptUserName);

        ListView_SetItemState(m_hwndList, pInfo->item.iItem, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);

        EnableWindow(m_hwndAdd,    TRUE);
        EnableWindow(m_hwndRename, TRUE);
        EnableWindow(m_hwndDelete, TRUE);

        fHandled = TRUE;
    }
    else if (LVN_DELETEALLITEMS == pNotify->code)
    {
        SetWindowLong(m_hwndDlg, DWL_MSGRESULT, TRUE);
        fHandled = TRUE;
    }
    else if (LVN_ITEMACTIVATE == pNotify->code)
    {
    	SendMessage(m_hwndDlg, WM_COMMAND, IDOK, 0);
        fHandled = TRUE;
    }
    else if (LVN_BEGINDRAG == pNotify->code)
    {
#ifdef UNDER_CE
        SetWindowLong(m_hwndDlg, DWL_MSGRESULT, LVBD_DRAGSELECT);
#endif /* UNDER_CE */

        fHandled = TRUE;
    }
    else if (NM_RCLICK == pNotify->code)
    {
    	ClientToScreen(m_hwndList, &pnlv->ptAction);
        DisplayContextMenu(pnlv->ptAction);
        fHandled = TRUE;
    }
    else if (GN_CONTEXTMENU == pNotify->code)
	{
		if (g_AygshellHelper.Loaded())
		{
			POINT pt;
			BOOL fRet = GetCursorPos(&pt);
			ASSERT(fRet);
			DisplayContextMenu(pt);
			fHandled = TRUE;
		}
    }

    return fHandled;
}

void CAddDeleteDialog::OnSize(int cx, int cy)
{
}
