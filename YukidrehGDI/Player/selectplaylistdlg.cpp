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
// File: SelectPlaylistDlg.cpp
//
// Desc: The implementation for the Playlist Selection dialog is contained in
//       this file.
//
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>

#include "SelectPlaylistDlg.h"

#include "aygshell_helper.h"

extern bool g_bSmallScreen;

BOOL CSelectPlaylistDialog::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    BOOL fHandled = FALSE;
    int iCommand = HIWORD(wParam);
    int iControl = LOWORD(wParam);
    int cxDlg, cyDlg, cxParent, cyParent, xPos, yPos;
    RECT rcDlg;
    RECT rcParent;

    switch (msg)
    {
        case WM_INITDIALOG:
            // center on the parent window
            GetWindowRect(m_hwndDlg, &rcDlg);
            GetClientRect(GetParent(m_hwndDlg), &rcParent);

            cxDlg = rcDlg.right - rcDlg.left;
            cyDlg = rcDlg.bottom - rcDlg.top;
            cxParent = rcParent.right - rcParent.left;
            cyParent = rcParent.bottom - rcParent.top;

            xPos = max(0, (cxParent - cxDlg) / 2);
            yPos = max(0, (cyParent - cyDlg) / 2);

            SetWindowPos(m_hwndDlg, NULL, xPos, yPos, cxDlg, cyDlg, SWP_NOZORDER);

            // pre-select All My Music
            SendMessage(GetDlgItem(m_hwndDlg,IDC_SELECT_ALL_MUSIC), BM_SETCHECK,
                        BST_CHECKED, 0);
            fHandled = TRUE;

            if( g_bSmallScreen && g_AygshellHelper.Loaded() )
            {
                SHINITDLGINFO shidi;
                shidi.dwMask = SHIDIM_FLAGS;
                shidi.dwFlags = SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_SIPDOWN;
                shidi.hDlg = m_hwndDlg;
                g_AygshellHelper.SHInitDialog( &shidi );
            }
        break;

        case WM_COMMAND:
            if (IDOK == iControl)
            {
                // See if All My Music is selected
                LRESULT res = SendMessage(GetDlgItem(m_hwndDlg, IDC_SELECT_ALL_MUSIC),
                                          BM_GETCHECK, 0, 0);

                if (BST_CHECKED == res)
                {
                   iControl = ID_ALL_MUSIC;
                }

                // Kill ourselves
                EndDialog(m_hwndDlg, iControl);
                fHandled = TRUE;
            }
        break;
    }

    return fHandled;
}
