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
// File: OptionsDlg.cpp
//
// Desc: This file implements the DialogProc for the Options Dialog
//
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <windev.h>
#include <commctrl.h>
#include "pkfuncs.h"
#include "OptionsDlg.h"
#include "PlayerWindow.h"
#include "resource.h"
#include "PlaylistMgr.h"

#include "aygshell_helper.h"

extern bool g_bSmallScreen;

static DWORD GetBufferingTime( void )
{
    DWORD result = 2;
    DWORD dwType;
    DWORD dwValue;
    DWORD dwCount = sizeof (DWORD);
    HKEY  hkResult = NULL;

    if( RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Netshow\\Player\\General", 0, 0, &hkResult) == ERROR_SUCCESS )
    {
        if( ( RegQueryValueEx( hkResult, L"Buffering Time", 0, &dwType, (LPBYTE)&dwValue, &dwCount ) == ERROR_SUCCESS ) &&
            ( dwType == REG_DWORD ) )
        {
            result = ( dwValue / 1000 ) - 1;
        }
        RegCloseKey( hkResult );
    }

    return result;
}

static void SetBufferingTime( LPDWORD dwAmount )
{
    HKEY  hkResult = NULL;
    DWORD dwDisp;

    *dwAmount *= 1000;
    if( RegCreateKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Netshow\\Player\\General", 0, NULL, 0, 0, NULL, &hkResult, &dwDisp ) == ERROR_SUCCESS )
    {
        RegSetValueEx( hkResult, L"Buffering Time", 0, REG_DWORD, (LPBYTE)dwAmount, sizeof( DWORD ) );
        RegCloseKey( hkResult );
    }
}

static DWORD GetMaximumBufferingTime( void )
{
    DWORD result = 4;
    DWORD dwType;
    DWORD dwValue;
    DWORD dwCount = sizeof (DWORD);
    HKEY  hkResult = NULL;

    if( RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Netshow\\Player\\General", 0, 0, &hkResult) == ERROR_SUCCESS )
    {
        if( ( RegQueryValueEx( hkResult, L"Maximum Buffering Time", 0, &dwType, (LPBYTE)&dwValue, &dwCount ) == ERROR_SUCCESS ) &&
            ( dwType == REG_DWORD ) )
        {
            result = ( dwValue / 1000 ) - 1;
        }
        RegCloseKey( hkResult );
    }

    return result;
}

static void SetBufferingTextValue( HWND hWndDlg, DWORD result )
{
    WCHAR szBuffering[ 12 ];
    if( result )
        StringCchPrintf(szBuffering, 12, L"%d seconds", result + 1 );      
    else
        StringCchCopy(szBuffering, 12, L"1 second");
    SetDlgItemText( hWndDlg, IDC_WMBUFFERINGVALUE, szBuffering );
}

///////////////////////////////////////////////////////////////////////////////
// Name: OptionsDialogProc
// Desc: This function handles the dialog messages for the Options dialog box.
///////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK OptionsDialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HWND   hWndControl = NULL;
    WPARAM wp;
    LPARAM lp;
    LONG   result;
    TCHAR  szShare[MAX_PATH];

    switch (uMsg)
    {
    case WM_INITDIALOG:
        // Initalize the SipPref control if necessary
        if( g_bSmallScreen && g_AygshellHelper.Loaded() )
        {
            WNDCLASS wc;
            if( GetClassInfo( g_hInst, WC_SIPPREF, &wc ) )
            {
                CreateWindow( WC_SIPPREF, NULL, WS_CHILD, -10, -10,
                            5, 5, hWndDlg, NULL, g_hInst, NULL );
            }
        }

        if( g_pPlayerWindow->m_bPlayForever )
        {
            CheckRadioButton( hWndDlg, IDC_PLAY_ONCE, IDC_PLAY_FOREVER, IDC_PLAY_FOREVER );
        }
        else
        {
            CheckRadioButton( hWndDlg, IDC_PLAY_ONCE, IDC_PLAY_FOREVER, IDC_PLAY_ONCE );
        }

        SendDlgItemMessage( hWndDlg, IDC_ZOOM, CB_ADDSTRING, (WPARAM)0, (LPARAM)L"50%" );
        SendDlgItemMessage( hWndDlg, IDC_ZOOM, CB_ADDSTRING, (WPARAM)0, (LPARAM)L"100%" );
        SendDlgItemMessage( hWndDlg, IDC_ZOOM, CB_ADDSTRING, (WPARAM)0, (LPARAM)L"200%" );
        SendDlgItemMessage( hWndDlg, IDC_ZOOM, CB_SETCURSEL, (WPARAM)g_pPlayerWindow->m_dwZoomLevel, (LPARAM)0 );

        wp = (WPARAM)TRUE;
        lp = (LPARAM)MAKELONG( 0, GetMaximumBufferingTime() );
        SendDlgItemMessage( hWndDlg, IDC_WMBUFFERINGSLIDER, TBM_SETRANGE, wp, lp );

        wp = (WPARAM)TRUE;
        lp = (LPARAM)GetBufferingTime();
        SendDlgItemMessage( hWndDlg, IDC_WMBUFFERINGSLIDER, TBM_SETPOS, wp, lp );
        SetBufferingTextValue( hWndDlg, (DWORD)lp );

        if (WAIT_OBJECT_0 == WaitForAPIReady(SH_WNET, 0))
        {
            CPlaylistMgr::GetInstance()->GetMediaShare(szShare, MAX_PATH);
            SetDlgItemText(hWndDlg, IDC_MEDIASHARE, szShare);
        }
        else
        {
            HWND  hWndShare = GetDlgItem(hWndDlg, IDC_MEDIASHARE);

            SendMessage(hWndShare, EM_SETREADONLY, (WPARAM)TRUE, NULL);
        }

        if( g_bSmallScreen && g_AygshellHelper.Loaded() )
        {
            SHINITDLGINFO shidi;
            shidi.dwMask = SHIDIM_FLAGS;
            shidi.dwFlags = SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_SIPDOWN | SHIDIF_DONEBUTTON;
            shidi.hDlg = hWndDlg;
            g_AygshellHelper.SHInitDialog( &shidi );
        }

        return TRUE;
        
    case SW_SHOW:
        
        return TRUE;
    case WM_HSCROLL:
        result = SendDlgItemMessage( hWndDlg, IDC_WMBUFFERINGSLIDER, TBM_GETPOS, (WPARAM)0, (LPARAM)0 );
        SetBufferingTextValue( hWndDlg, result );
        return TRUE;

    case WM_KEYDOWN:
        return FALSE;

    case WM_KEYUP:
        if (VK_ESCAPE == (int) wParam)
        {
            EndDialog(hWndDlg, ID_DLG_CANCEL);

            return TRUE;
        }
        else
        {
            return FALSE;
        }

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        case ID_DLG_OKAY:
            {
                result = SendDlgItemMessage( hWndDlg, IDC_PLAY_FOREVER, BM_GETCHECK, (WPARAM)0, (LPARAM)0 );

                g_pPlayerWindow->m_bPlayForever = ( result == BST_CHECKED );
                g_pPlayerWindow->ApplyPlayCount();

                g_pPlayerWindow->m_dwZoomLevel = SendDlgItemMessage( hWndDlg, IDC_ZOOM, CB_GETCURSEL, (WPARAM)0, (LPARAM)0 );
                g_pPlayerWindow->ApplyZoomLevel();

                result = SendDlgItemMessage( hWndDlg, IDC_WMBUFFERINGSLIDER, TBM_GETPOS, (WPARAM)0, (LPARAM)0 ) + 1;
                SetBufferingTime( (LPDWORD)&result );

                g_pPlayerWindow->SaveRegState();

                GetDlgItemText(hWndDlg, IDC_MEDIASHARE, szShare, MAX_PATH);
                CPlaylistMgr * pManager = CPlaylistMgr::GetInstance();

                if (NULL != pManager)
                {
                    pManager->SetMediaShare(szShare);
                }

                EndDialog(hWndDlg, 0);
            }
            
            // Destroy the dialog, returning ID_DLG_OKAY to indicate success.
            EndDialog(hWndDlg, ID_DLG_OKAY);
            
            return TRUE;
            
        case IDCANCEL:
        case ID_DLG_CANCEL:
            // If the "Cancel" button was pressed destroy the dialog and return
            // a ID_DLG_CANCEL (failure).
            EndDialog(hWndDlg, ID_DLG_CANCEL);
            
            return TRUE;
        }
       
        break;
        
        case WM_CLOSE:
        case WM_DESTROY:
            // If the "X" button on the command bar was pressed destroy the dialog
            // and returning ID_DLG_CANCEL  (failure).
            EndDialog(hWndDlg, ID_DLG_CANCEL);
            
            return TRUE;
    }
    
    return FALSE;
}
