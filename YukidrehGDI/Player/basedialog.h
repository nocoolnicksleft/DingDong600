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
//////////////////////////////////////////////////////////////////////////////
// File: BaseDialog.h
//
// Desc: This class is used as a base class for all of the playlist related
//       dialogs.  It allows a dialog to be created as modal or modeless even
//       though all playlist dialogs are modal.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BASEDIALOG_H
#define BASEDIALOG_H

#include <windows.h>

extern HINSTANCE g_hInst;

class CBaseDialog
{
public:
    CBaseDialog( HWND hwndParent, int nDialogResID );

    //////////////////////////////////////////////////////////////////////////
    //
    // Display the derived dialog class
    //
    int DoModal();
    HWND DoModeless( HWND hwndParent );
    operator HWND() const { return(m_hwndDlg); }

protected:
    //////////////////////////////////////////////////////////////////////////
    //
    // Derived classes override to provide useful functionality
    //
    virtual BOOL DialogProc( UINT msg, WPARAM wParam, LPARAM lParam )
    {
        return( FALSE );
    }

    static BOOL CALLBACK DlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam );

    void SetHwnd( HWND hwndDlg ) { m_hwndDlg = hwndDlg; }

    //////////////////////////////////////////////////////////////////////////
protected:
    HWND    m_hwndDlg;
    HWND    m_hwndParent;
    int     m_nDialogResID;
};

#endif  // BASEDIALOG_H
