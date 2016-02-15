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
// File: BaseDialog.cpp
//
// Desc: This file contains the implementation for the CBaseDialog class,
//       which is the base class for all of the playlist dialogs.
//
///////////////////////////////////////////////////////////////////////////////

#include    "BaseDialog.h"

HWND g_hwndActiveDlg = NULL;

//////////////////////////////////////////////////////////////////////////////
CBaseDialog::CBaseDialog(HWND hwndParent, int nDialogResID) :
    m_hwndDlg       ( NULL ),
    m_hwndParent    ( hwndParent ),
    m_nDialogResID  ( nDialogResID )
{
}

    
//////////////////////////////////////////////////////////////////////////////
int CBaseDialog::DoModal()
{
    HWND    hwndPrevDlg = g_hwndActiveDlg;

    int res = DialogBoxParam(g_hInst,
                             MAKEINTRESOURCE(m_nDialogResID),
                             m_hwndParent,
                             DlgProc,
                             (LPARAM) this);

    g_hwndActiveDlg = hwndPrevDlg;

    return( res );
}

HWND CBaseDialog::DoModeless( HWND hwndParent )
{
    m_hwndParent = hwndParent;
    HWND hwnd = CreateDialogParam( g_hInst, MAKEINTRESOURCE(m_nDialogResID), m_hwndParent, DlgProc, (LPARAM)this);

    return( hwnd );
}



//////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK CBaseDialog::DlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch( msg )
    {
     case WM_DESTROY:
        break;

     case WM_COMMAND:
        break;

     case WM_INITDIALOG:
        //
        // Remember this topmost dialog's handle
        //
        g_hwndActiveDlg = hwndDlg;

        //
        // Preserve the 'this' pointer in the window's user data
        //
        CBaseDialog *pBaseDlg   = (CBaseDialog *) lParam;

        pBaseDlg->SetHwnd(hwndDlg);

        SetLastError(0);

        LONG    lResult = SetWindowLong(hwndDlg, GWL_USERDATA, (LONG) pBaseDlg);

        if (0 == lResult)
        {
            DWORD   dwError = GetLastError();
            if (0 != dwError)
            {
                EndDialog(hwndDlg, -1);
                return  TRUE;
            }
        }
        break;
    }

    //
    // Get the 'this' pointer from the window's user data
    //
    CBaseDialog *pBaseDlg   = (CBaseDialog *) GetWindowLong(hwndDlg, GWL_USERDATA);
    if (NULL == pBaseDlg)
    {
        return  FALSE;
    }
    else
    {
        return  pBaseDlg->DialogProc(msg, wParam, lParam);
    }
}
