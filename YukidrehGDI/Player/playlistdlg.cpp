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
// File: PlaylistDlg.cpp
//
// Desc: This file implements the CPlaylistDialog object, which contains the
//       playlist view dialog.
//
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <commctrl.h>

#include "resource.h"
#include "PlaylistDlg.h"
#include "CEPlayerUtil.h"
#include "PlaylistMgr.h"
#include "AddTracksDlg.h"
#include "AddDeleteDlg.h"
#include "PlayerWindow.h"
#include "SelectPlaylistDlg.h"

#include "aygshell_helper.h"

#define MENU_ICON_WIDTH          16
#define NUM_TRACKS_MENU_ICONS    7
#define I_IMAGENONE              (-2)
#define APPBAR_HEIGHT            22
#define MENU_ITEM_HEIGHT         20
#define MENU_ITEM_HORZ_MARGIN    2
#define MAX_MENU_ITEM_WIDTH      200

extern bool g_bSmallScreen;

CPlaylistDialog::CPlaylistDialog(HWND hwndParent, bool bOrganizeFavorites) :
    CBaseDialog(hwndParent, IDD_PLAYLIST_TRACKS),
    m_hwndDropdown(NULL),
    m_hwndMB(NULL),
    m_hwndTrackList(NULL),
    m_hFont(NULL),
    m_pPlaylist(NULL),
    m_bOrganizeFavorites(bOrganizeFavorites)
{
    CPlaylistMgr * pManager = CPlaylistMgr::GetInstance();

    m_hFont = GetBoldFont();

    m_himgPlaylist  = ImageList_LoadBitmap(g_hInst,
                                           MAKEINTRESOURCE(IDB_PLAYLIST),
                                           MENU_ICON_WIDTH,
                                           1,
                                           CLR_DEFAULT);

    m_himgLocationList = ImageList_LoadBitmap(g_hInst,
                                              MAKEINTRESOURCE(IDB_LOCATIONLIST),
                                              MENU_ICON_WIDTH,
                                              1,
                                              CLR_DEFAULT);

    if (!m_bOrganizeFavorites && pManager)
    {
        m_pPlaylist = pManager->CurrentPlaylist();
    }

    if (NULL != m_pPlaylist)
    {
        m_pPlaylist->Load();

        //
        // Check to see if this playlist is empty
        //
        CMediaClip * pClip;
        int          iAvailCount = 0;
        int          iCount;

        iCount = (int)m_pPlaylist->GetCount();

        for (int i = 0; i < iCount; i++)
        {
            pClip = m_pPlaylist->GetTrack(i);

            if (pClip && pClip->IsAvailable())
            {
                iAvailCount++;
            }
        }

        if (0 == iAvailCount && iCount > 0)
        {
            CSelectPlaylistDialog selPlaylist(hwndParent);

            if (ID_ALL_MUSIC != selPlaylist.DoModal())
            {
                CAddDeleteDialog addDelete(hwndParent, hwndParent);

                addDelete.DoModal();

                m_pPlaylist = pManager->CurrentPlaylist();

                if (m_pPlaylist) m_pPlaylist->Load();
            }
            else
            {
                m_pPlaylist = NULL;
            }
        }
    }
}

CPlaylistDialog::~CPlaylistDialog()
{
    if (NULL != m_hFont)
    {
        DeleteObject(m_hFont);
    }

    if (NULL != m_himgPlaylist)
    {
        ImageList_Destroy(m_himgPlaylist);
    }

    if (NULL != m_himgLocationList)
    {
        ImageList_Destroy(m_himgLocationList);
    }

#ifdef UNDER_CE
    if (NULL != m_hwndDropdown)
    {
        CommandBar_Destroy(m_hwndDropdown);
    }

    if (NULL != m_hwndMB)
    {
        CommandBar_Destroy(m_hwndMB);
    }
#endif /* UNDER_CE */
}

HRESULT CPlaylistDialog::CreateBars()
{
    HRESULT hr = S_OK;
    DWORD   dwStyle;
    int     iImg = 0;

    do
    {
        //
        // Setup the "Menu Bar" (this appears at the bottom of the window)
        //

#ifdef UNDER_CE
        m_hwndMB = CommandBar_Create(g_hInst, m_hwndDlg, 1234);
#endif /* UNDER_CE */

        dwStyle = SendMessage(m_hwndMB, TB_GETSTYLE, 0, 0);
        dwStyle &= ~CCS_TOP;
        dwStyle |= CCS_BOTTOM | TBSTYLE_FLAT;

        SendMessage(m_hwndMB, TB_SETSTYLE, 0, dwStyle);

#ifdef UNDER_CE
        iImg  = CommandBar_AddBitmap(m_hwndMB,
                                     g_hInst,
                                     IDB_PLAYLIST,
                                     2,
                                     16,
                                     16);
#endif /* UNDER_CE */

        if (-1 == iImg)
        {
            hr = E_FAIL;
            break;
        }

#ifdef UNDER_CE
        iImg = CommandBar_AddBitmap(m_hwndMB,
                                    g_hInst,
                                    IDB_PLAYLISTMB,
                                    6,
                                    16,
                                    16);
#endif /* UNDER_CE */

        if (-1 == iImg)
        {
            hr = E_FAIL;
            break;
        }

        TBBUTTON buttons[NUM_TRACKS_MENU_ICONS];
        int iButton;

        static int commands[NUM_TRACKS_MENU_ICONS-1] =
        {
            ID_ADD_TRACKS,
            ID_DELETE_TRACKS,
            ID_REORDER_UP,
            ID_REORDER_DOWN,
            ID_PLAY_SONG,
            ID_TRACK_INFO
        };

        for (iButton = 0; iButton < NUM_TRACKS_MENU_ICONS-3; iButton++)
        {
            buttons[iButton].iBitmap   = iButton + 2;
            buttons[iButton].idCommand = commands[iButton];
            buttons[iButton].fsState   = TBSTATE_ENABLED;
            buttons[iButton].fsStyle   = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;
            buttons[iButton].dwData    = 0;
            buttons[iButton].iString   = 0;
        }

        buttons[iButton].iBitmap   = I_IMAGENONE;
        buttons[iButton].idCommand = ID_DEAD_SPACE;
        buttons[iButton].fsState   = 0;
        buttons[iButton].fsStyle   = TBSTYLE_BUTTON;
        buttons[iButton].dwData    = 0;
        buttons[iButton].iString   = 0;

        iButton++;

        for (; iButton < NUM_TRACKS_MENU_ICONS; iButton++)
        {
            buttons[iButton].iBitmap   = iButton + 1;
            buttons[iButton].idCommand = commands[iButton-1];
            buttons[iButton].fsState   = TBSTATE_ENABLED;
            buttons[iButton].fsStyle   = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;
            buttons[iButton].dwData    = 0;
            buttons[iButton].iString   = 0;
        }

#ifdef UNDER_CE
        if (!CommandBar_AddButtons(m_hwndMB, NUM_TRACKS_MENU_ICONS, buttons))
        {
            hr = E_FAIL;
            break;
        }
#endif /* UNDER_CE */

        //
        // This sets up the "Drop Down" command bar...
        //

#ifdef UNDER_CE
        m_hwndDropdown = CommandBar_Create(g_hInst, m_hwndDlg, 2134);
#endif /* UNDER_CE */

        dwStyle = SendMessage(m_hwndDropdown, TB_GETSTYLE, 0, 0);
        dwStyle &= ~CCS_BOTTOM;
        dwStyle |= CCS_TOP | TBSTYLE_FLAT;

        SendMessage(m_hwndDropdown, TB_SETSTYLE, 0, dwStyle);

#ifdef UNDER_CE
        iImg  = CommandBar_AddBitmap(m_hwndDropdown,
                                     g_hInst,
                                     IDB_PLAYLIST,
                                     2,
                                     16,
                                     16);
#endif /* UNDER_CE */

        if (-1 == iImg)
        {
            hr = E_FAIL;
            break;
        }

        // Add our dropdown button to the command bar
        TBBUTTON button;
        button.idCommand = IDC_PLAYLIST_DROPDOWN;
        button.fsState   = TBSTATE_ENABLED;
        button.fsStyle   = TBSTYLE_BUTTON | TBSTYLE_DROPDOWN | TBSTYLE_AUTOSIZE;
        button.dwData    = 0;

        if (m_pPlaylist)
        {
            button.iBitmap   = I_PLAYLIST;
            button.iString   = (int)m_pPlaylist->GetName();
        }
        else if (m_bOrganizeFavorites)
        {
            button.iBitmap   = -1;
            button.iString   = (int)TEXT("Organize Playlists");
        }
        else
        {
            button.iBitmap   = -1;
            button.iString   = (int)TEXT("Local Content");
        }

#ifdef UNDER_CE
        if (!CommandBar_AddButtons(m_hwndDropdown, 1, &button))
        {
            hr = E_FAIL;
            break;
        }
#endif /* UNDER_CE */

    } while (0);

    return hr;
}

void CPlaylistDialog::EnableMenubarIcons()
{
    int iCount = (int)ListView_GetSelectedCount(m_hwndTrackList);
    int iIndex = (int)ListView_GetNextItem(m_hwndTrackList, -1, LVNI_SELECTED);

    CPlaylistMgr * pManager = CPlaylistMgr::GetInstance();

    // always enable the "All My Music" and "All Playlists" buttons
    TBBUTTONINFO tbbi;

    tbbi.cbSize  = sizeof (tbbi);
    tbbi.dwMask  = TBIF_STATE;

    if (NULL != m_pPlaylist || m_bOrganizeFavorites)
    {
        tbbi.fsState = TBSTATE_ENABLED;
    }
    else
    {
        tbbi.fsState = 0;
    }

    SendMessage(m_hwndMB, TB_SETBUTTONINFO, ID_ADD_TRACKS,    (LPARAM)&tbbi);

    if (iCount == 1)
    {
        tbbi.fsState = TBSTATE_ENABLED;
    }
    else
    {
        tbbi.fsState = 0;
    }

    SendMessage(m_hwndMB, TB_SETBUTTONINFO, ID_DELETE_TRACKS, (LPARAM)&tbbi);

    if ((NULL != m_pPlaylist || m_bOrganizeFavorites)
        && iIndex > 0 && 1 == iCount)
    {
        tbbi.fsState = TBSTATE_ENABLED;
    }
    else
    {
        tbbi.fsState = 0;
    }

    SendMessage(m_hwndMB, TB_SETBUTTONINFO, ID_REORDER_UP,    (LPARAM)&tbbi);

    if (NULL != m_pPlaylist
        && -1 != iIndex && 1 == iCount
        && m_pPlaylist->GetCount() > (UINT)(iIndex + 1))
    {
        tbbi.fsState = TBSTATE_ENABLED;
    }
    else if (m_bOrganizeFavorites
             && -1 != iIndex
             && pManager
             && pManager->GetFavorites()
             && pManager->GetFavorites()->GetCount() > (UINT)(iIndex + 1))
    {
        tbbi.fsState = TBSTATE_ENABLED;
    }
    else
    {
        tbbi.fsState = 0;
    }

    SendMessage(m_hwndMB, TB_SETBUTTONINFO, ID_REORDER_DOWN,  (LPARAM)&tbbi);

    CPlaylist * pPlaylist = NULL;

    if (NULL != m_pPlaylist)
    {
        pPlaylist = m_pPlaylist;
    }
    else if (m_bOrganizeFavorites && pManager)
    {
        pPlaylist = pManager->GetFavorites();
    }
    else if (pManager)
    {
        pPlaylist = pManager->LocalContent();
    }

    CMediaClip * pClip = NULL;

    if (pPlaylist) pClip = pPlaylist->GetTrack(iIndex);

    if ((1 == iCount
         && NULL != pClip
         && pClip->IsAvailable())
        || 0 == iCount)
    {
        tbbi.fsState = TBSTATE_ENABLED;
    }
    else
    {
        tbbi.fsState = 0;
    }

    SendMessage(m_hwndMB, TB_SETBUTTONINFO, ID_PLAY_SONG,     (LPARAM)&tbbi);


    if (1 == iCount
        && NULL != pClip
        && pClip->IsLocal())
    {
        tbbi.fsState = TBSTATE_ENABLED;
    }
    else
    {
        tbbi.fsState = 0;
    }

    SendMessage(m_hwndMB, TB_SETBUTTONINFO, ID_TRACK_INFO,    (LPARAM)&tbbi);
}

BOOL CPlaylistDialog::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    BOOL fHandled = FALSE;

    CPlaylistMgr * pManager = CPlaylistMgr::GetInstance();

    switch (msg)
    {
        case WM_INITDIALOG:
            OnInitDialog();

            g_pPlayerWindow->m_hWndPlay = m_hwndDlg;

            fHandled = FALSE;  // we set focus, so don't change it

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
            switch (LOWORD(wParam))
            {
                case IDOK:
                case IDCANCEL:
                {
                    CPlaylist * pPlaylist = NULL;

                    if (m_pPlaylist)
                    {
                        pPlaylist = m_pPlaylist;
                    }
                    else if (m_bOrganizeFavorites && pManager)
                    {
                        pPlaylist = pManager->GetFavorites();
                    }

                    if (pPlaylist) pPlaylist->Save();

                    EndDialog(m_hwndDlg, 0);
                }
                return TRUE;

                case ID_ADD_TRACKS:
                {
                    CPlaylist * pPlaylist = NULL;

                    if (NULL != m_pPlaylist)
                    {
                        pPlaylist = m_pPlaylist;
                    }
                    else if (m_bOrganizeFavorites && pManager)
                    {
                        pPlaylist = pManager->GetFavorites();
                    }

                    // Don't handle this message if the playlist is invalid.
                    if (NULL != pPlaylist)
                    {
                        CAddTracksDialog addTracks(m_hwndDlg, pPlaylist);
                        addTracks.DoModal();

                        fHandled = TRUE;
                    }

                    UpdateInfo();
                }
                break;

                case ID_DELETE_TRACKS:
                {
                    CPlaylist * pPlaylist = NULL;

                    int iCount = (int)ListView_GetSelectedCount(m_hwndTrackList);
                    int iIndex = (int)ListView_GetNextItem(m_hwndTrackList, -1,
                                                           LVNI_SELECTED);

                    if (1 == iCount && iIndex >= 0)
                    {
                        if (NULL != m_pPlaylist)
                        {
                            pPlaylist = m_pPlaylist;
                        }
                        else if (m_bOrganizeFavorites && pManager)
                        {
                            pPlaylist = pManager->GetFavorites();
                        }
                        else if (pManager)
                        {
                            int   cmd;
                            TCHAR szCaption[MAX_PATH];
                            TCHAR szTemp[MAX_PATH];

                            pPlaylist = pManager->LocalContent();

                            LoadString(g_hInst, IDS_APP_TITLE,    szCaption, MAX_PATH);
                            LoadString(g_hInst, IDS_DELETE_TRACK, szTemp,    MAX_PATH);

                            cmd = MessageBox(m_hwndDlg, szTemp, szCaption, MB_YESNO | MB_ICONEXCLAMATION);

                            if (IDYES == cmd)
                            {
                                CMediaClip * pClip = pPlaylist->GetTrack(iIndex);

                                if (NULL != pClip)
                                {
                                    StringCchCopy(szTemp, MAX_PATH, pClip->GetPath());
                                    if (DeleteFile(szTemp))
                                    {
                                        pPlaylist->DeleteTrack(iIndex);

                                        UpdateInfo();
                                    }
                                    else
                                    {
                                        DWORD dwError = GetLastError();

                                        LoadString(g_hInst, IDS_APP_TITLE,   szCaption, MAX_PATH);
                                        LoadString(g_hInst, IDS_DELETE_FAIL, szTemp,    MAX_PATH);

                                        MessageBox(m_hwndDlg, szTemp, szCaption, MB_OK | MB_ICONWARNING);
                                    }

                                    if (iIndex >= (int)pPlaylist->GetCount())
                                    {
                                        iIndex = (int)pPlaylist->GetCount() - 1;
                                    }

                                    if (iIndex >= 0)
                                    {
                                        ListView_SetItemState(m_hwndTrackList, iIndex, LVIS_SELECTED, LVIS_SELECTED);
                                        ListView_EnsureVisible(m_hwndTrackList, iIndex, FALSE);
                                    }
                                }
                            }

                            pPlaylist = NULL;
                        }

                        if (NULL != pPlaylist) 
                        {
                            // we need to stop this track if it is
                            // currently playing
                            if (pPlaylist->IsPlaying(iIndex))
                            {
                                SendMessage(m_hwndParent, WM_COMMAND,
                                    ID_PLAYLIST_NEXT, (LPARAM)pPlaylist);
                            }

                            pPlaylist->DeleteTrack(iIndex);

                            UpdateInfo();

                            if (iIndex >= (int)pPlaylist->GetCount())
                            {
                                iIndex = (int)pPlaylist->GetCount() - 1;
                            }

                            if (iIndex >= 0)
                            {
                                ListView_SetItemState(m_hwndTrackList, iIndex, LVIS_SELECTED, LVIS_SELECTED);
                                ListView_EnsureVisible(m_hwndTrackList, iIndex, FALSE);
                            }
                        }
                    }

                    fHandled = TRUE;
                }
                break;

                case ID_REORDER_UP:
                {
                    CPlaylist * pPlaylist = NULL;
                    int iCount = (int)ListView_GetSelectedCount(m_hwndTrackList);
                    int iIndex = ListView_GetNextItem(m_hwndTrackList, -1, LVNI_SELECTED);

                    if (NULL != m_pPlaylist)
                    {
                        pPlaylist = m_pPlaylist;
                    }
                    else if (m_bOrganizeFavorites && pManager)
                    {
                        pPlaylist = pManager->GetFavorites();
                    }

                    if (NULL != pPlaylist
                        && iIndex > 0
                        && pPlaylist->ShiftTrackUp(iIndex))
                    {
                        iIndex--;

                        UpdateInfo();

                        ListView_SetItemState(m_hwndTrackList, iIndex, LVIS_SELECTED, LVIS_SELECTED);
                        ListView_EnsureVisible(m_hwndTrackList, iIndex, FALSE);
                    }

                    fHandled = TRUE;
                }
                break;

                case ID_REORDER_DOWN:
                {
                    CPlaylist * pPlaylist = NULL;
                    int iCount = (int)ListView_GetSelectedCount(m_hwndTrackList);
                    int iIndex = ListView_GetNextItem(m_hwndTrackList, -1, LVNI_SELECTED);

                    if (NULL != m_pPlaylist)
                    {
                        pPlaylist = m_pPlaylist;
                    }
                    else if (m_bOrganizeFavorites && pManager)
                    {
                        pPlaylist = pManager->GetFavorites();
                    }

                    if (NULL != pPlaylist
                        && iIndex >= 0
                        && iIndex < (int)pPlaylist->GetCount() - 1
                        && pPlaylist->ShiftTrackDown(iIndex))
                    {
                        iIndex++;

                        UpdateInfo();

                        ListView_SetItemState(m_hwndTrackList, iIndex, LVIS_SELECTED, LVIS_SELECTED);
                        ListView_EnsureVisible(m_hwndTrackList, iIndex, FALSE);
                    }

                    fHandled = TRUE;
                }
                break;

                case ID_PLAY_SONG:
                    // play the currently selected track
                    if (1 == ListView_GetSelectedCount(m_hwndTrackList))
                    {
                        CPlaylist * pPlaylist = NULL;

                        int iIndex = ListView_GetNextItem(m_hwndTrackList, -1, LVNI_SELECTED);

                        if (NULL != m_pPlaylist)
                        {
                            pPlaylist = m_pPlaylist;
                        }
                        else if (m_bOrganizeFavorites && pManager)
                        {
                            pPlaylist = pManager->GetFavorites();
                        }
                        else if (pManager)
                        {
                            pPlaylist = pManager->LocalContent();
                        }

                        if (pPlaylist)
                        {
                            pPlaylist->CurrentTrackDonePlaying();

                            if (-1 != iIndex)
                            {
                                pPlaylist->SetCurrent(iIndex);
                            }

                            SendMessage(m_hwndParent, WM_COMMAND, ID_PLAY_SONG, (LPARAM)pPlaylist);
                        }

                        EndDialog(m_hwndDlg, 0);
                    }
                    else if (0 == ListView_GetSelectedCount(m_hwndTrackList))
                    {
                        CPlaylist * pPlaylist = NULL;

                        if (NULL != m_pPlaylist)
                        {
                            pPlaylist = m_pPlaylist;
                        }
                        else if (m_bOrganizeFavorites && pManager)
                        {
                            pPlaylist = pManager->GetFavorites();
                        }
                        else if (pManager)
                        {
                            pPlaylist = pManager->LocalContent();
                        }

                        if (pPlaylist)
                        {
                            if (0 == pPlaylist->GetCount())
                            {
                                TCHAR szViewContent[MAX_PATH];
                                TCHAR szEmpty[MAX_PATH];
                                int iRet = LoadString(g_hInst,
                                            IDS_VIEW_LOCAL_CONTENT,
                                            szViewContent, MAX_PATH);
                                ASSERT(iRet);

                                iRet = LoadString(g_hInst, 
                                            IDS_PLAYLIST_EMPTY,
                                            szEmpty, MAX_PATH);
                                ASSERT(iRet);

                                iRet = MessageBox(m_hwndDlg, szViewContent, 
                                        szEmpty, MB_YESNO | MB_ICONQUESTION);
                                ASSERT(iRet);
                                if (IDYES == iRet)
                                {
                                    CAddTracksDialog addTracks(m_hwndDlg, pPlaylist);
                                    addTracks.DoModal();
                                    UpdateInfo();
                                }

                                break;
                            }

                            if (-1 == pPlaylist->GetCurrent())
                            {
                                pPlaylist->SetCurrent(0);
                            }

                            SendMessage(m_hwndParent, WM_COMMAND, ID_PLAY_SONG, (LPARAM)pPlaylist);
                        }

                        EndDialog(m_hwndDlg, 0);
                    }

                    fHandled = TRUE;
                break;

                case ID_TRACK_INFO:
                    if (1 == ListView_GetSelectedCount(m_hwndTrackList))
                    {
                        CPlaylist * pPlaylist;

                        int iIndex = ListView_GetNextItem(m_hwndTrackList, -1, LVNI_SELECTED);

                        if (NULL != m_pPlaylist)
                        {
                            pPlaylist = m_pPlaylist;
                        }
                        else if (m_bOrganizeFavorites && pManager)
                        {
                            pPlaylist = pManager->GetFavorites();
                        }
                        else if (pManager)
                        {
                            pPlaylist = pManager->LocalContent();
                        }

                        if (-1 != iIndex && NULL != pPlaylist)
                        {
                            CMediaClip * pClip = pPlaylist->GetTrack(iIndex);

                            if (NULL != pClip)
                            {
                                g_pPlayerWindow->UpdatePropertyDlg(pClip->GetPath());
                            }
                        }

                    }

                    fHandled = TRUE;
                break;

                case IDC_PLAYLIST_DROPDOWN:
                    fHandled = TRUE;
                break;

                case ID_ALL_MUSIC:
                    m_bOrganizeFavorites = false;

                    if (m_pPlaylist)
                    {
                        m_pPlaylist->Save();
                        m_pPlaylist = NULL;
                    }

                    SetDropdownText((TCHAR *)TEXT("Local Content"), I_ALL_MY_MUSIC);

                    UpdateInfo();

                    fHandled = TRUE;
                break;

                case ID_FAVORITES:
                    m_bOrganizeFavorites = true;

                    if (m_pPlaylist)
                    {
                        m_pPlaylist->Save();
                        m_pPlaylist = NULL;
                    }

                    SetDropdownText((TCHAR *)TEXT("Favorites"), I_ALL_MY_MUSIC);

                    UpdateInfo();

                    fHandled = TRUE;
                break;

                case ID_ALL_PLAYLISTS:
                    m_bOrganizeFavorites = false;

                    if (m_pPlaylist) m_pPlaylist->Save();

                    if (TRUE)
                    {
                        CAddDeleteDialog addDeleteDlg(m_hwndDlg, m_hwndParent);
                        addDeleteDlg.DoModal();
                    }

                    if (pManager)
                    {
                        m_pPlaylist = pManager->CurrentPlaylist();
                    }
                    else
                    {
                        m_pPlaylist = NULL;
                    }

                    if (NULL != m_pPlaylist)
                    {
                        if (false == m_pPlaylist->Load())
                        {
                            if (true == m_pPlaylist->IsCEPlaylist()
                                && -1 == m_pPlaylist->GetCurrent())
                            {
                                // this playlist is empty (add tracks!)
                                SendMessage(m_hwndDlg, WM_COMMAND, ID_ADD_TRACKS, 0);

                            }
                            else
                            {
                                m_pPlaylist = NULL;
                            }
                        }

                        if (NULL != m_pPlaylist
                            && false == m_pPlaylist->IsCEPlaylist())
                        {
                            // this isn't a CEPlayer generated playlist
                            OutputDebugString(TEXT("ERROR: this isn't a CEPlaylist"));
                        }

                        //
                        // If we've played this playlist before, resume playing
                        //
                        if (-1 != m_pPlaylist->GetCurrent())
                        {
                            SendMessage(m_hwndParent, WM_COMMAND, ID_PLAY_SONG, (LPARAM)m_pPlaylist);
                        }
                    }

                    UpdateInfo();

                    fHandled = TRUE;
                break;

                case ID_DELETED_PLAYLIST:
                    if (NULL != m_pPlaylist
                        && NULL != pManager
                        && !pManager->IsValid(m_pPlaylist))
                    {
                        m_pPlaylist = NULL;
                    }

                    fHandled = TRUE;
                break;

                case ID_UPDATE_INFO:
                    UpdateInfo();

                    fHandled = TRUE;
                break;
 
                default:
                    if (ID_PLAYLIST1_HISTORY <= LOWORD(wParam)
                        && LOWORD(wParam) <= ID_PLAYLIST7_HISTORY)
                    {
                        m_bOrganizeFavorites = false;

                        if (m_pPlaylist)
                        {
                            m_pPlaylist->Save();
                        }

                        if (pManager)
                        {
                            m_pPlaylist = pManager->MRUPlaylist(LOWORD(wParam) - ID_PLAYLIST1_HISTORY);
                        }
                        else
                        {
                            m_pPlaylist = NULL;
                        }


                        if (m_pPlaylist)
                        {
                            m_pPlaylist->Load();

                            pManager->SetCurrentPlaylist(m_pPlaylist->GetPath());

                            //
                            // If we've played this playlist before, resume playing
                            //
                            if (-1 != m_pPlaylist->GetCurrent())
                            {
                                SendMessage(m_hwndParent, WM_COMMAND, ID_PLAY_SONG, (LPARAM)m_pPlaylist);
                            }
                        }

                        UpdateInfo();
                    }

                break;
            }
        break;

        case WM_NOTIFY:
            fHandled = OnNotify((NMHDR*)lParam);
        break;

        case WM_MEASUREITEM:
            if (0 == wParam)
            {
                OnMeasureItem((MEASUREITEMSTRUCT *)lParam);

                fHandled = TRUE;
            }
        break;

        case WM_DRAWITEM:
            if (0 == wParam)
            {
                OnDrawItem((DRAWITEMSTRUCT*)lParam);

                fHandled = TRUE;
            }
        break;
    }

    return fHandled;
}

void CPlaylistDialog::OnInitDialog()
{
    // Grab the list view's HWND for later use
    m_hwndTrackList = GetDlgItem(m_hwndDlg, IDC_TRACK_LIST);

    //
    // Check to see if the screen is small
    //
    if (g_bSmallScreen)
    {
        RECT rcWorkArea;
        RECT rcWnd, rcList;

        GetWindowRect(m_hwndDlg,    &rcWnd);
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

    // Create the menu bar & command bar
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

    ListView_SetExtendedListViewStyle(m_hwndTrackList, LVS_EX_FULLROWSELECT);

    // add the track list's single column
    LVCOLUMN lvc;
    RECT     rc;

    GetWindowRect(m_hwndTrackList, &rc);

    memset(&lvc, 0, sizeof (lvc));

    lvc.mask     = LVCF_WIDTH | LVCF_FMT;
    lvc.fmt      = LVCFMT_LEFT;
    lvc.cx       = rc.right - rc.left;

    ListView_InsertColumn(m_hwndTrackList, 0, &lvc);

    // update our track list contents according to the current playlist
    UpdateInfo();

    // We do this after we've done the UpdateInfo, so that we know if there are tracks to show icons/buttons for
    EnableMenubarIcons();

    // set focus on the list of tracks
    SetFocus(m_hwndTrackList);
}

BOOL CPlaylistDialog::OnNotify(NMHDR * pNotifyHeader)
{
    NMLISTVIEW * pNotifyListView = (NMLISTVIEW *)pNotifyHeader;
    BOOL         fHandled        = FALSE;

    CPlaylistMgr * pManager = CPlaylistMgr::GetInstance();

    if (TBN_DROPDOWN == pNotifyHeader->code)
    {
        OnDropdownMenu();

        fHandled = TRUE;
    }
    else if (NM_CUSTOMDRAW == pNotifyHeader->code
             && m_hwndTrackList == pNotifyHeader->hwndFrom)
    {
        NMLVCUSTOMDRAW *pcd = (NMLVCUSTOMDRAW*)pNotifyHeader;

        if (CDDS_PREPAINT == pcd->nmcd.dwDrawStage)
        {
            SetWindowLong(m_hwndDlg, DWL_MSGRESULT, CDRF_NOTIFYITEMDRAW);
            fHandled = TRUE;
        }
        else if (CDDS_ITEMPREPAINT == pcd->nmcd.dwDrawStage && pManager)
        {
            CPlaylist * pPlaylist = NULL;

            if (NULL != m_pPlaylist)
            {
                pPlaylist = m_pPlaylist;
            }
            else if (m_bOrganizeFavorites && pManager)
            {
                pPlaylist = pManager->GetFavorites();
            }
            else if (pManager)
            {
                pPlaylist = pManager->LocalContent();
            }

            if (NULL != pPlaylist)
            {
                if ((int)pcd->nmcd.dwItemSpec == pPlaylist->GetCurrent())
                {
                    // Display the current track in bold
                    SelectObject(pcd->nmcd.hdc, m_hFont);
                    SetWindowLong(m_hwndDlg, DWL_MSGRESULT, CDRF_NEWFONT);
                }

                // Gray out tracks whose files don't currently exist
                CMediaClip * pClip = pPlaylist->GetTrack((int)pcd->nmcd.dwItemSpec);
                if (NULL != pClip
                    && pClip->IsAvailable())
                {
                    pcd->clrText = GetSysColor(COLOR_WINDOWTEXT);
                }
                else
                {
                    pcd->clrText = GetSysColor(COLOR_3DSHADOW);
                }

                SetBkMode(pcd->nmcd.hdc, OPAQUE);
                pcd->clrTextBk = RGB(255, 255, 255);
                SetWindowLong(m_hwndDlg, DWL_MSGRESULT, CDRF_NEWFONT);

                fHandled = TRUE;
            }
        }
    }
    else if (LVN_ITEMCHANGED == pNotifyHeader->code)
    {
        SetWindowLong(m_hwndDlg, DWL_MSGRESULT, 0);

        fHandled = TRUE;
    }
    else if (LVN_DELETEALLITEMS == pNotifyHeader->code)
    {
        // Suppress LVN_DELETEITEM notifications
        SetWindowLong(m_hwndDlg, DWL_MSGRESULT, TRUE);
        fHandled = TRUE;
    }
    else if (LVN_ITEMACTIVATE == pNotifyHeader->code)
    {
        SendMessage(m_hwndDlg, WM_COMMAND, ID_PLAY_SONG, 0);
        fHandled = TRUE;
    }
    else if (NM_RCLICK == pNotifyHeader->code)
    {
        ClientToScreen(m_hwndTrackList, &pNotifyListView->ptAction);
        OnContextMenu(pNotifyListView->ptAction);

        fHandled = TRUE;
    }
    else if (GN_CONTEXTMENU == pNotifyHeader->code)
    {
        if (g_AygshellHelper.Loaded())
        {
            POINT pt;
            BOOL fRet = GetCursorPos(&pt);
            ASSERT(fRet);
            OnContextMenu(pt);
            fHandled = TRUE;
        }
    }
    else if (LVN_BEGINDRAG == pNotifyHeader->code)
    {
#ifdef UNDER_CE
        SetWindowLong(m_hwndDlg, DWL_MSGRESULT, LVBD_DRAGSELECT);
#endif /* _UNDER_CE */

        fHandled = TRUE;
    }

    EnableMenubarIcons();

    return fHandled;
}

void CPlaylistDialog::OnDropdownMenu()
{
    HRESULT hr = S_OK;
    HMENU   hmenu      = NULL;
    HMENU   hmenuPopup = NULL;
    MENUITEMINFO mii;
    DrawMenuStruct * pMenuStruct = NULL;
    BOOL fRestoreText = FALSE;
    int cmd = 0;
    int i;

    CPlaylistMgr * pManager = CPlaylistMgr::GetInstance();

    DWORD rgIDs[] =
    {
        ID_ALL_MUSIC,
        ID_FAVORITES,
        ID_ALL_PLAYLISTS
    };

    do
    {
        //
        // set the name temporarily
        //
        if (m_pPlaylist)
        {
            SetDropdownText((LPTSTR)m_pPlaylist->GetName(), I_PLAYLIST);
        }
        else
        {
            SetDropdownText((TCHAR *)TEXT("Local Content"), I_ALL_MY_MUSIC);
        }

        //
        // create an empty popup menu
        //
        hmenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDM_TRACKS_CONTEXT));

        if (NULL != hmenu)
        {
            hmenuPopup = GetSubMenu(hmenu, 1);
        }

        if (NULL == hmenuPopup)
        {
            break;
        }

        //
        // add the "All My Music" owner-drawn item
        //
        mii.cbSize = sizeof (mii);

        for (i = 0; i < sizeof (rgIDs)/sizeof (rgIDs[0]); i++)
        {
            pMenuStruct = new DrawMenuStruct;

            if (NULL == pMenuStruct)
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            pMenuStruct->iImage = I_ALL_MY_MUSIC;

            mii.fMask = MIIM_TYPE;
            mii.dwTypeData = pMenuStruct->szText;
            mii.cch        = sizeof (pMenuStruct->szText)/sizeof (pMenuStruct->szText[0]);

            if (GetMenuItemInfo(hmenuPopup, rgIDs[i], FALSE, &mii))
            {
                mii.fMask = MIIM_TYPE | MIIM_DATA;
                mii.fType = MFT_OWNERDRAW;
                mii.dwTypeData = (LPTSTR)pMenuStruct;
                mii.dwItemData = (DWORD)pMenuStruct;

                SetMenuItemInfo(hmenuPopup, rgIDs[i], FALSE, &mii);
            }
        }

        //
        // Get the menu count and find ID_ALL_PLAYLISTS
        //

        mii.fMask = MIIM_ID;
        int cItems = 0;

        while (GetMenuItemInfo(hmenuPopup, cItems, TRUE, &mii))
        {
            if (ID_ALL_PLAYLISTS == mii.wID)
            {
                cItems--;
                break;
            }

            cItems++;
        }

        if (pManager)
        {
            for (int iMRU = pManager->MRUPlaylistCount() - 1; iMRU >= 0; iMRU--)
            {
                CPlaylist * pPlaylist = pManager->MRUPlaylist(iMRU);

                while (NULL == pPlaylist
                       && iMRU < pManager->MRUPlaylistCount())
                {
                    pManager->MRURemove(iMRU);

                    if (iMRU > 0) iMRU--;

                    pPlaylist = pManager->MRUPlaylist(iMRU);
                }

                if (NULL == pPlaylist)
                {
                    break;
                }

                LPTSTR szName = pManager->GetDisplayName(pPlaylist);
                if (!szName)
                {
                    hr = E_OUTOFMEMORY;
                    break;
                }

                pMenuStruct = new DrawMenuStruct;

                if (NULL == pMenuStruct)
                {
                    delete [] szName;
                    hr = E_OUTOFMEMORY;
                    break;
                }

                pMenuStruct->iImage = I_PLAYLIST;
                StringCchCopy(pMenuStruct->szText, 50, szName);
                pMenuStruct->szText[49] = 0;
                delete [] szName;
                szName = NULL;

                mii.fMask      = MIIM_TYPE | MIIM_DATA;
                mii.fType      = MFT_OWNERDRAW;
                mii.dwTypeData = (LPTSTR)pMenuStruct;
                mii.dwItemData = (DWORD)pMenuStruct;

                if (!InsertMenu(hmenuPopup, cItems, 
                        MF_BYPOSITION | MF_OWNERDRAW, 
                        ID_PLAYLIST1_HISTORY + iMRU, NULL))
                {
                    ASSERT(FALSE);
                    break;
                }

                SetMenuItemInfo(hmenuPopup, cItems, TRUE, &mii);
            }

            //
            // Remove the separator if there are no MRU playlists
            //

            if (0 == pManager->MRUPlaylistCount())
            {
                RemoveMenu(hmenuPopup, cItems, MF_BYPOSITION);
            }
        }

        //
        // Display the menu
        //

        POINT ptPopup = { 0, APPBAR_HEIGHT };

        ClientToScreen(m_hwndDlg, &ptPopup);

        cmd = TrackPopupMenuEx(hmenuPopup,
                               TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD,
                               ptPopup.x, ptPopup.y,
                               m_hwndDlg,
                               NULL);

        if (0 != cmd && ID_FAVORITES != cmd)
        {
            fRestoreText = FALSE;
        }

        //
        // Now re-post the selected command, if any
        //
        if (0 != cmd)
        {
            PostMessage(m_hwndDlg, WM_COMMAND, cmd, 0);
        }
    } while (FALSE);

    if (fRestoreText)
    {
        UpdateInfo();
    }

    if (NULL != hmenuPopup)
    {
        for (i = 0; i < sizeof (rgIDs) / sizeof (rgIDs[0]); i++)
        {
            mii.fMask = MIIM_DATA;
            mii.dwItemData = NULL;

            GetMenuItemInfo(hmenuPopup, rgIDs[i], FALSE, &mii);

            if (NULL != mii.dwItemData)
            {
                delete (DrawMenuStruct*)mii.dwItemData;
            }
        }
    }

    if (NULL != hmenu)
    {
        DestroyMenu(hmenu);
    }
}

void CPlaylistDialog::SetDropdownText(LPTSTR szText, int iImage)
{
    // first make sure the text fits in the control
    SIZE sizText, sizEllipsis;
    TBBUTTONINFO tbbi;
    BOOL fRet, fTrunc = FALSE;
    LPTSTR szDDText;
    INT nMax, nFit, nLen = _tcslen(szText);
    LPINT rgExtent = new INT[nLen];

    if (rgExtent)
        memset(rgExtent, 0, sizeof(rgExtent));

    HDC hdc = ::GetDC(m_hwndDropdown);
    ASSERT(hdc);

    RECT rcDlg;
    fRet = GetWindowRect(m_hwndDlg, &rcDlg);
    ASSERT(fRet);

    nMax = rcDlg.right - rcDlg.left - 60;

    fRet = GetTextExtentExPoint(hdc, szText, nLen,
                nMax, &nFit, rgExtent, &sizText);
    ASSERT(fRet);

    if (nFit && rgExtent && nFit < nLen)
    {
        fRet = GetTextExtentExPoint(hdc, TEXT("\x2026"),
                    1, 0, NULL, NULL, &sizEllipsis);
        ASSERT(fRet);

        nMax -= sizEllipsis.cx;

        nFit++;
        while (rgExtent[--nFit - 1] > nMax);

        szDDText = new TCHAR[nFit + 1];
        if (szDDText)
        {
            StringCchCopyN(szDDText, nFit + 1, szText, nFit - 1);
            szDDText[nFit - 1] = TEXT('\x2026');
            szDDText[nFit] = 0;
            fTrunc = TRUE;
        }
        else ASSERT(FALSE);
    }
    else
    {
        szDDText = szText;
    }

    ReleaseDC(m_hwndDropdown, hdc);
    if (rgExtent) delete [] rgExtent;

    // now fill the control info
    tbbi.cbSize = sizeof (TBBUTTONINFO);
    tbbi.dwMask = TBIF_TEXT | TBIF_IMAGE;
    tbbi.pszText = szDDText;
    tbbi.iImage  = iImage;

    SendMessage(m_hwndDropdown, TB_SETBUTTONINFO,
        IDC_PLAYLIST_DROPDOWN, (LPARAM)&tbbi);

    if (fTrunc) delete [] szDDText;
}

void CPlaylistDialog::UpdateInfo()
{
   LVITEM item;
   SIZE   size;
   HDC    hdc;
   LONG   cx = 0;
   CPlaylist             * pPlaylist = NULL;
   CPlaylist::playlist_t * pList     = NULL;
   LPCTSTR                 pszName   = NULL;
   int iImage;
   int i      = 0;
   int cxText = 0;

   CPlaylistMgr * pManager = CPlaylistMgr::GetInstance();

   hdc = ::GetDC(m_hwndDlg);

   pPlaylist = m_pPlaylist;

   if (NULL != pPlaylist)
   {
       iImage  = I_PLAYLIST;
       pszName = pPlaylist->GetName();
       pList   = pPlaylist->GetFirst();
   }
   else if (m_bOrganizeFavorites && NULL != pManager)
   {
       iImage    = I_ALL_MY_MUSIC;
       pszName   = TEXT("Favorites");
       pPlaylist = pManager->GetFavorites();
       if (pPlaylist) pList = pPlaylist->GetFirst();
   }
   else if (NULL != pManager)
   {
       iImage    = I_ALL_MY_MUSIC;
       pszName   = TEXT("Local Content");
       pManager->UpdateLocalContent();
       pPlaylist = pManager->LocalContent();
       if (pPlaylist) pList = pPlaylist->GetFirst();
   }
   else
   {
       iImage    = I_ALL_MY_MUSIC;
       pszName   = TEXT("Local Content");
       pList     = NULL;
   }

   // This causes a WM_NOTIFY, for which we call OnNotify.  OnNotify calls EnableMenuBarIcons, which can scan for content
   ListView_DeleteAllItems(m_hwndTrackList);

   while (pList)
   {
       CMediaClip * pClip = pList->pClip;

       memset(&item, 0, sizeof (item));

       item.mask    = LVIF_TEXT | LVIF_DI_SETITEM | LVIF_IMAGE;
       item.iItem   = i;
       item.iImage  = (int)pClip->GetLocation();
       item.pszText = (LPTSTR)pClip->GetTitle();
       ListView_InsertItem(m_hwndTrackList, &item);

       if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size))
       {
           if (size.cx > cx)
           {
               cx = size.cx;
           }
       }

       pList = pList->pNext;
       i++;
   }

   if (hdc)
   {
       ::ReleaseDC(m_hwndTrackList, hdc);
   }

   //
   // Clean up the list view
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

   SetDropdownText((LPTSTR)pszName, iImage);

   SendMessage(m_hwndTrackList, WM_SETREDRAW, TRUE, 0);

   InvalidateRect(m_hwndTrackList, NULL, TRUE);
}

void CPlaylistDialog::OnMeasureItem(MEASUREITEMSTRUCT * pmis)
{
    DrawMenuStruct *pMenuStruct = NULL;

    pMenuStruct = (DrawMenuStruct *)pmis->itemData;

    // we have fixed height menu items
    pmis->itemHeight = MENU_ITEM_HEIGHT;

    // calculate the required width for this item's text
    HDC hdc = GetDC(m_hwndDropdown);

    LPCTSTR szMenuItem = NULL;

    if (NULL != pMenuStruct)
    {
        szMenuItem = pMenuStruct->szText;
    }
    else
    {
        szMenuItem = (LPTSTR)pmis->itemData;
    }

    SIZE size;

    if (szMenuItem)
    {
        GetTextExtentPoint32(hdc, szMenuItem, _tcslen(szMenuItem), &size);
    }
    else
    {
        size.cx = 0;
    }

    ReleaseDC(m_hwndDropdown, hdc);

    // the item width includes text, and icon, and spacing
    pmis->itemWidth = size.cx + MENU_ICON_WIDTH + 3*MENU_ITEM_HORZ_MARGIN;
    pmis->itemWidth = min(pmis->itemWidth, MAX_MENU_ITEM_WIDTH);
}

void CPlaylistDialog::OnDrawItem(DRAWITEMSTRUCT *pdis)
{
    HDC hdc = pdis->hDC;
    RECT rcItem = pdis->rcItem;
    DrawMenuStruct *pMenuStruct = NULL;

    pMenuStruct = (DrawMenuStruct *)pdis->itemData;

    // Draw menu item background
    if ((ODA_SELECT == pdis->itemAction) || (ODA_DRAWENTIRE == pdis->itemAction))
    {
        if (0 != (pdis->itemState & ODS_SELECTED))
        {
            SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));

            FillRect(hdc, &rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
        }
        else
        {
            SetTextColor(hdc, GetSysColor(COLOR_MENUTEXT));

            FillRect(hdc, &rcItem, GetSysColorBrush(COLOR_MENU));
        }
    }

    // Draw the appropriate image
    int iImage = I_PLAYLIST;

    if (pMenuStruct)
    {
        iImage = pMenuStruct->iImage;
    }

    ImageList_DrawEx(
        m_himgPlaylist,
        iImage,
        hdc,
        rcItem.left + MENU_ITEM_HORZ_MARGIN,
        rcItem.top +1,
        0,
        0,
        CLR_NONE,
        CLR_NONE,
        ILD_TRANSPARENT);

    // Draw the menu item text
    RECT rcText = rcItem;

    rcText.left += MENU_ICON_WIDTH + (2 * MENU_ITEM_HORZ_MARGIN);

    LPCTSTR szMenuItem = NULL;
    if (NULL != pMenuStruct)
    {
        szMenuItem = pMenuStruct->szText;
    }
    else
    {
        szMenuItem = (LPTSTR) pdis->itemData;
    }

    int cchMenuItem = 0;

    if (szMenuItem)
    {
        cchMenuItem = _tcslen(szMenuItem);
    }
    else
    {
        szMenuItem = (TCHAR *)TEXT("");
    }

    int iOldMode = SetBkMode(hdc, TRANSPARENT);

    DrawEllipsisText(
        hdc,
        szMenuItem,
        cchMenuItem,
        &rcText,
        DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_NOCLIP);

   // clean up
   if (0 != iOldMode) SetBkMode(hdc, iOldMode);
}

void CPlaylistDialog::OnContextMenu(POINT ptHold)
{
    // The user has pushed and help, so give them a context menu
    HMENU hmenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDM_TRACKS_CONTEXT));

    if (NULL == hmenu)
    {
       return;
    }

    HMENU hmenuPopup = GetSubMenu(hmenu, 0);

    if (NULL != hmenuPopup)
    {
        DWORD dwSelected = ListView_GetSelectedCount(m_hwndTrackList);
        DWORD dwSelMark  = ListView_GetSelectionMark(m_hwndTrackList);
        DWORD dwIndex =    ListView_GetNextItem(m_hwndTrackList, -1, LVNI_SELECTED);
 
        DWORD dwType = 0;

        // only enable the play icon when one item is selected and it exists
        BOOL fEnablePlay = (1 == dwSelected);

        if (fEnablePlay)
        {
            int iTrack = ListView_GetNextItem(m_hwndTrackList, -1, LVNI_SELECTED);
            // check if it exists
        }

        if (!fEnablePlay)
        {
            EnableMenuItem(hmenuPopup, ID_PLAY_SONG, MF_BYCOMMAND | MF_GRAYED);
        }

        if( !m_pPlaylist )
        {
            EnableMenuItem(hmenuPopup, ID_REORDER_UP, MF_BYCOMMAND | MF_GRAYED );
            EnableMenuItem(hmenuPopup, ID_REORDER_DOWN, MF_BYCOMMAND | MF_GRAYED );
        }

        // Let's look at the selection. If more than one item is selection, disable Delete
        if( dwSelected != 1 )
        {
            EnableMenuItem(hmenuPopup, ID_DELETE_TRACKS, MF_BYCOMMAND | MF_GRAYED);
        }

        if(( dwIndex == 0 ) || ( dwSelected != 1 ))
        {
            EnableMenuItem(hmenuPopup, ID_REORDER_UP, MF_BYCOMMAND | MF_GRAYED);
        }

        if(!m_pPlaylist || (-1 == dwIndex ) || ( dwIndex == m_pPlaylist->GetCount() -1 ) || ( dwSelected != 1 ))
        {
            EnableMenuItem(hmenuPopup, ID_REORDER_DOWN, MF_BYCOMMAND | MF_GRAYED);
        }


        // optionally disable various menu items

        TrackPopupMenuEx(hmenuPopup,
                         TPM_LEFTALIGN | TPM_TOPALIGN,
                         ptHold.x, ptHold.y,
                         m_hwndDlg,
                         NULL);
    }

    DestroyMenu(hmenu);
}
