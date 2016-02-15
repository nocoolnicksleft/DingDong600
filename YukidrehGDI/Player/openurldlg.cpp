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
// File: OpenURLDlg.cpp
//
// Desc: This file implements the DialogProc for the OpenURL Dialog.
//
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <tchar.h>
#define _COMCTL32_
#include <commctrl.h>
#undef _COMCTL32_
#include <commdlg.h>

#include "OpenURLDlg.h"
#include "PlayerWindow.h"

#include "resource.h"

#include "aygshell_helper.h"

extern bool g_bSmallScreen;

COpenURLDlg::COpenURLDlg() : m_hWnd(NULL)
{
}

COpenURLDlg::~COpenURLDlg()
{
   (void)Fini();
}

///////////////////////////////////////////////////////////////////////////////
// Name: COpenURLDlg::Init()
// Desc: This function initializes the OpenURLDlg class.  The parameter hWnd
//       is a handle to the OpenURL dialog box.
///////////////////////////////////////////////////////////////////////////////
bool COpenURLDlg::Init(HWND hWnd)
{
   m_hWnd       = hWnd;

   return true;
}

///////////////////////////////////////////////////////////////////////////////
// Name: COpenURLDlg::Fini()
// Desc: This function takes care of all the clean-up necessary for the
//       OpenURLDlg class, including destroying the window.
///////////////////////////////////////////////////////////////////////////////
bool COpenURLDlg::Fini()
{
   bool bResult = false;

   if (NULL != m_hWnd)
   {
      //DestroyWindow(m_hWnd);
      bResult = true;
   }

   return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: OpenURLDialogProc
// Desc: This function handles the dialog messages for the OpenURL dialog box.
///////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK OpenURLDialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // This is a "persitant" variable used to get the string a user types in
    // back to the function which creates an OpenURL Dialog with the
    // DialogBoxParam() function
    static TCHAR **szURL      = NULL;
    static TCHAR  *szFilename = NULL;
    args_t *args = reinterpret_cast<args_t*>(lParam);
    int    i;
    HWND hWndControl = NULL;
    OPENFILENAME ofn;
    
    static COpenURLDlg *pOpenDlg = NULL;
    
    if( g_AygshellHelper.Loaded() )
        g_AygshellHelper.SHInputDialog( hWndDlg, uMsg, wParam );

    switch (uMsg)
    {
        case WM_INITDIALOG:
            pOpenDlg = new COpenURLDlg;
            if( pOpenDlg == NULL || pOpenDlg->Init( hWndDlg ) == FALSE )
            {
                DestroyWindow( hWndDlg );
            }
            else
            {
                // The lParam value contains the address of a TCHAR* back in the function
                // which created this dialog.  When someone presses the Okay button, a
                // new string will be allocated with this pointer
                szURL = args->szURL;
                szFilename = args->szFilename;
            
                if (NULL != szFilename)
                {
                    SetDlgItemText(hWndDlg, IDC_URL_HISTORY, szFilename);
                }
            
                for( i = 0; i < MAX_FILEOPEN_HISTORY; i++ )
                {
                    if( args->szFilenameHistory[i] && args->szFilenameHistory[i][0] )
                    {
                        SendMessage( GetDlgItem( hWndDlg, IDC_URL_HISTORY ), CB_ADDSTRING, 0, (LPARAM)args->szFilenameHistory[i] );
                    }
                }

                if( g_bSmallScreen && g_AygshellHelper.Loaded() )
                {
                    SHINITDLGINFO shidi;
                    shidi.dwMask = SHIDIM_FLAGS;
                    shidi.dwFlags = SHIDIF_SIZEDLG; // No DONE/OK button in top-right corner for this dialog...
                    shidi.hDlg = hWndDlg;
                    g_AygshellHelper.SHInitDialog( &shidi );
                }
            }        
        return TRUE;
        
        case SW_SHOW:
            // This allows the text field to be selected by default, as opposed to
            // the Okay button.
            SetFocus(GetDlgItem(hWndDlg, IDC_URL_HISTORY));
        return TRUE;
        
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_URL_HISTORY:
                    switch( HIWORD( wParam ) )
                    {
                        case CBN_SELENDOK:
                            SendMessage( hWndDlg, WM_COMMAND, ID_DLG_OKAY, 0 );
                        return TRUE;
                    }
                break;
                case IDOK:
                    // Error check
                    if (NULL == szURL)
                    {
                        EndDialog(hWndDlg, 0);
                    }
                    else
                    {               
                        // Allocate a new string.
                        // NOTE: The parent window is responsible for freeing this memory
                        *szURL = new TCHAR[MAX_URL_LENGTH];

                        if (NULL != *szURL)
                        {
                            // Get (MAX_URL_LENTH - 1) characters from the OpenURL text box, and
                            // append a nul character.
                            GetDlgItemText(hWndDlg, IDC_URL_HISTORY, *szURL, (MAX_URL_LENGTH-1));
                            (*szURL)[MAX_URL_LENGTH-1] = TEXT('\0');
                        }
                
                        // Destroy the dialog, returning ID_DLG_OKAY to indicate success.
                        EndDialog(hWndDlg, ID_DLG_OKAY);
                    }
                return TRUE;
                
                case IDCANCEL:
                    // If the "Cancel" button was pressed destroy the dialog and return
                    // a ID_DLG_CANCEL (failure).
                    EndDialog(hWndDlg, ID_DLG_CANCEL);
                return TRUE;
                
                case ID_DLG_BROWSE:
                    // Initialize the OPENFILENAME struct
                    memset(&ofn, 0, sizeof (ofn));
                    ofn.lStructSize     = sizeof (ofn);
                    ofn.hwndOwner       = hWndDlg;
                    ofn.hInstance       = g_hInst;
                    ofn.lpstrFilter     = g_pPlayerWindow->m_szFilter;
                    ofn.nFilterIndex    = g_pPlayerWindow->m_nFilterIndex;
                    ofn.lpstrFile       = new TCHAR[_MAX_PATH];
                    ofn.nMaxFile        = _MAX_PATH;
                    ofn.lpstrInitialDir = g_pPlayerWindow->m_szPath;
                    ofn.Flags           = OFN_FILEMUSTEXIST;
                
                    if (NULL != ofn.lpstrFile)
                    {
                        ofn.lpstrFile[0] = TEXT('\0');
                    }

                    if (TRUE == ::GetOpenFileName(&ofn))
                    {
                        // Remember the path for next time...
                        g_pPlayerWindow->m_szPath = new TCHAR[ofn.nFileOffset + 1];
                        if(g_pPlayerWindow->m_szPath && ofn.lpstrFile)
                        {
                            StringCchCopy(g_pPlayerWindow->m_szPath, ofn.nFileOffset + 1, ofn.lpstrFile);
                            g_pPlayerWindow->m_szPath[ofn.nFileOffset] = TEXT('\0');
                        }
                    }

                    g_pPlayerWindow->m_nFilterIndex = ofn.nFilterIndex;

                    SetDlgItemText( hWndDlg, IDC_URL_HISTORY, ofn.lpstrFile );
                
                    delete [] ofn.lpstrFile;
                return TRUE;
            }
        break;
        
        case WM_CLOSE:
            // If the "X" button on the command bar was pressed destroy the dialog
            // and returning ID_DLG_CANCEL  (failure).
            EndDialog(hWndDlg, ID_DLG_CANCEL);
        return TRUE;

        case WM_DESTROY:
            delete pOpenDlg;
            pOpenDlg = NULL;
        return TRUE;
   }
   
   return FALSE;
}
