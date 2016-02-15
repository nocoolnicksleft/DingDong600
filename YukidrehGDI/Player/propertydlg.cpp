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
// File: PropertyDlg.cpp
//
// Desc: This file defines the member functions of the Property Dialog class
//       as well as the DialogProc for the dialog.
//
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <stdio.h>

#define _COMCTL32_
#include <commctrl.h>
#undef  _COMCTL32_

#include <nsplay.h>

#include "PropertyDlg.h"
#include "resource.h"

#include <uuids.h>

#ifndef UNDER_CE
#include <initguid.h>
#endif // !UNDER_CE
#include "qnetwork.h"

#include "aygshell_helper.h"
#include <strsafe.h>

extern bool g_bSmallScreen;

///////////////////////////////////////////////////////////////////////////////
// Name: PropertyDialogProc()
// Desc: This function handles dialog messages for the Property dialog box.
///////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK PropertyDialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   static CPropertyDlg *pPropertyDlg = NULL;

   switch (uMsg)
   {
    case WM_INITDIALOG:
      // Once this dialog is created, create the associated PropertyDlg class.
      // There can only be one Property dialog open at a time, hence the need
      // for the static variable pPropertyDlg.
      pPropertyDlg = new CPropertyDlg;

      if (NULL == pPropertyDlg || false == pPropertyDlg->Init(hWndDlg))
      {
         DestroyWindow(hWndDlg);
      }
      else if( g_bSmallScreen && g_AygshellHelper.Loaded() )
      {
          SHINITDLGINFO shidi;
          shidi.dwMask = SHIDIM_FLAGS;
          shidi.dwFlags = SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_SIPDOWN | SHIDIF_DONEBUTTON;
          shidi.hDlg = hWndDlg;
          g_AygshellHelper.SHInitDialog( &shidi );
      }

      return TRUE;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
       case IDCANCEL:
       case IDOK:
       case ID_DLG_CLOSE:
         DestroyWindow(hWndDlg);
         return TRUE;
      }

      return FALSE;

    case WM_NOTIFY:
        if(pPropertyDlg)
            return pPropertyDlg->HandleNotifyMsg( hWndDlg, uMsg, wParam, lParam );
        break;

    case WM_CLOSE:
        DestroyWindow(hWndDlg);
        return FALSE;

    case WM_DESTROY:
      if (NULL != pPropertyDlg)
      {
         delete pPropertyDlg;
         pPropertyDlg = NULL;
      }

      return TRUE;

    // This message is sent by the parent window to tell the dialog to show
    // itself.
    case PD_SHOW:
        if(pPropertyDlg)
        {
            pPropertyDlg->Show(static_cast<int>(wParam));
            return TRUE;
        }
        break;

    // This message is sent by the parent window to get the dialog to update
    // its display.
    case PD_UPDATE:
        if(pPropertyDlg)
        {
            if (!lParam)
                pPropertyDlg->Update(reinterpret_cast<TCHAR*>(wParam));
            else
                pPropertyDlg->UpdateAndRender(reinterpret_cast<TCHAR*>(wParam));
            return TRUE;
        }
        break;
   }

   return FALSE;
}

CPropertyDlg::CPropertyDlg() : m_hWnd(NULL),
                               m_hWndParent(NULL),
                               m_hListView(NULL),
                               m_hFont(NULL)
{
    LOGFONT logfont;
    memset( &logfont, 0, sizeof( logfont ) );
    logfont.lfWeight = FW_SEMIBOLD;
    logfont.lfHeight = -11;
    m_hFont = CreateFontIndirect( &logfont );
}

CPropertyDlg::~CPropertyDlg()
{
   (void)Fini();

   if (m_hFont)
       DeleteObject(m_hFont);
}

BOOL CPropertyDlg::HandleNotifyMsg( HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL fHandled = FALSE;
    NMHDR *pnmh = (NMHDR *)lParam;
    NMLISTVIEW *pnmlv = (NMLISTVIEW *)lParam;

    if( ( NM_CUSTOMDRAW == pnmh->code ) &&
        ( m_hListView == pnmh->hwndFrom ) )
    {
        NMLVCUSTOMDRAW *pcd = (NMLVCUSTOMDRAW *)lParam;

        if( CDDS_PREPAINT == pcd->nmcd.dwDrawStage )
        {
            SetWindowLong( hWndDlg, DWL_MSGRESULT, CDRF_NOTIFYITEMDRAW );
            fHandled = TRUE;
        }
        else if( CDDS_ITEMPREPAINT == pcd->nmcd.dwDrawStage )
        {
            if( 0 == ( pcd->nmcd.dwItemSpec % 2 ) && m_hFont)
            {
                SelectObject( pcd->nmcd.hdc, m_hFont);
                SetWindowLong( hWndDlg, DWL_MSGRESULT, CDRF_NEWFONT );
            }
            fHandled = TRUE;
        }
    }
    else if( LVN_DELETEALLITEMS == pnmh->code )
    {
        //
        // Suppress LVN_DELETEITEM notifications
        //
        SetWindowLong( hWndDlg, DWL_MSGRESULT, TRUE );
        fHandled = TRUE;
    }
    return fHandled;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPropertyDlg::Init()
// Desc: This function initializes the PropertyDlg class.  The parameter hWnd
//       is a handle to the Property dialog box.
///////////////////////////////////////////////////////////////////////////////
bool CPropertyDlg::Init(HWND hWnd)
{
    m_hWnd       = hWnd;
    m_hWndParent = ::GetParent(m_hWnd);
    m_hListView  = GetDlgItem( m_hWnd, IDC_PROPERTIES_LIST );

    //
    // Check to see if the screen is small
    //
    if (g_bSmallScreen)
    {
        RECT rcWorkArea;
        RECT rcWnd, rcList;

        GetWindowRect(m_hWnd,      &rcWnd);
        GetWindowRect(m_hListView, &rcList);

        if (!SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, 0))
        {
            HDC hdc = ::GetDC(NULL);

            rcWorkArea.left = 0;
            rcWorkArea.top  = 0;

            rcWorkArea.right  = GetDeviceCaps(hdc, HORZRES);
            rcWorkArea.bottom = GetDeviceCaps(hdc, VERTRES) - GetSystemMetrics(SM_CYMENU);

            ::ReleaseDC(NULL, hdc);
        }

        MoveWindow(m_hWnd,
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

        MoveWindow(m_hListView,
                   rcWorkArea.left,
                   rcWorkArea.top,
                   rcWorkArea.right,
                   rcWorkArea.bottom,
                   TRUE);
    }

    LVCOLUMN column;
    int i = 0;

    RECT rect;
    GetClientRect( m_hListView, &rect );
    column.mask = LVCF_FMT | LVCF_WIDTH;
    column.fmt = LVCFMT_LEFT;
    column.cx = rect.right - rect.left;
    column.iSubItem = 0;
    column.iOrder = 0;
    column.iImage = 0;
    int result = ListView_InsertColumn( m_hListView, 0, &column );

    ListView_SetExtendedListViewStyle( m_hListView, LVS_EX_GRIDLINES );
   return true;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPropertyDlg::Fini()
// Desc: This function takes care of all the clean-up necessary for the
//       PropertyDlg class, including destroying the window and notifying the
//       parent window that the dialog has closed.
///////////////////////////////////////////////////////////////////////////////
bool CPropertyDlg::Fini()
{
   bool bResult = false;

   if (NULL != m_hWndParent)
   {
      PostMessage(m_hWndParent, PD_CLOSED, NULL, NULL);
      m_hWndParent = NULL;
   }

   return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPropertyDlg::Show()
// Desc: This function makes the dialog show/hide itself.
///////////////////////////////////////////////////////////////////////////////
bool CPropertyDlg::Show(int iShowCmd)
{
   bool  bResult = false;

   // Simple error check
   if (NULL == m_hWnd)
   {
      return bResult;
   }

   if (FALSE == ShowWindow(m_hWnd, iShowCmd))
   {
      bResult = false;
   }
   else
   {
      bResult = true;
   }

   return bResult;
}

static void Time2String( double dTime, __out_ecount (dwcchTime) LPTSTR tszTime, DWORD dwcchTime, BOOL fShowMilliSecs, BOOL fShowZeroHour, LPCTSTR tszTimeSep, LPCTSTR tszDecimalSep )
{
    LONG lRemainingSecs;  // remaining secons we have to convert
    
    lRemainingSecs = (LONG) dTime;  // ignore fractions of secs for a sec :-)
    
    LONG lHours = lRemainingSecs / 3600;   // 3600 seconds in the hour
    lRemainingSecs  = lRemainingSecs % 3600;
    
    LONG lMinutes = lRemainingSecs / 60;   // 60 seconds in the minute
    lRemainingSecs    = lRemainingSecs % 60;
    
    LONG lSeconds = lRemainingSecs;
    
    // take remaining fraction and scale to millisecs
    if (fShowMilliSecs) {
        LONG lMilliSecs = (LONG) ((dTime - (LONG) dTime) * 1000.0);
        
        if (lHours == 0 && !fShowZeroHour) {
            StringCchPrintf( tszTime, dwcchTime, TEXT("%02lu%s%02lu%s%03lu"), lMinutes, tszTimeSep, lSeconds, tszDecimalSep, lMilliSecs );
        } else {
            StringCchPrintf( tszTime, dwcchTime, TEXT("%02lu%s%02lu%s%02lu%s%03lu"), lHours, tszTimeSep, lMinutes, tszTimeSep, lSeconds, tszDecimalSep, lMilliSecs );
        }
    } else {
        if (lHours == 0 && !fShowZeroHour) {
            StringCchPrintf( tszTime, dwcchTime, TEXT("%02lu%s%02lu"), lMinutes, tszTimeSep, lSeconds );
        } else {
            StringCchPrintf( tszTime, dwcchTime, TEXT("%02lu%s%02lu%s%02lu"), lHours, tszTimeSep, lMinutes, tszTimeSep, lSeconds );
        }
    }
}

HRESULT CPropertyDlg::VideoInfo( BSTR * videoInfo, IAMNetShowExProps * pnsep )
{
    HRESULT hResult = E_FAIL;
    *videoInfo = (TCHAR *)L"";

    if (NULL != pnsep)
    {
        long codecCount = 0;

        if (SUCCEEDED(pnsep->get_CodecCount(&codecCount)))
        {
            for (int i = codecCount; i > 0; i--)
            {
                BSTR codecDesc;

                if (SUCCEEDED(pnsep->GetCodecDescription(i, &codecDesc))
                    && !wcsstr(codecDesc, L"Audio"))
                {
                    LPWSTR comma;

                    if (comma = wcsrchr(codecDesc, L'('))
                    {
                        *comma = L'\0';
                    }

                    *videoInfo = codecDesc;
                    hResult = S_OK;
                    break;
                }
            }
        }
    }

    return hResult;
}

HRESULT CPropertyDlg::AudioInfo( LPAUDIOINFO           * audioInfo,
                                 IBasicAudio           * piba,
                                 IAMNetShowExProps     * pnsep,
                                 IAMSecureMediaContent * psmc )
{
    HRESULT hResult = E_FAIL;

    if (NULL == audioInfo)
    {
        hResult = E_INVALIDARG;
    }
    else
    {
        audioInfo->wfmtWaveFormat.nSamplesPerSec = 0;
        audioInfo->wfmtWaveFormat.nChannels = 0;
        audioInfo->lpwcCodecBitrate = NULL;
        audioInfo->bstrCodecDescription = L"";
        audioInfo->bSecure = FALSE;

        if (NULL != piba)
        {
            IBaseFilter *pibf = NULL;
            piba->QueryInterface( IID_IBaseFilter, (void **)&pibf );
            if ( pibf != NULL )
            {
                IEnumPins *piep = NULL;
                DWORD lFetched;
            
                IPin *pp;
                pibf->EnumPins( &piep );
                if ( piep != NULL )
                {
                    piep->Reset();
                
                    while ( (NOERROR == piep->Next( 1, &pp, &lFetched ) ) && ( pp != NULL ) )
                    {
                        AM_MEDIA_TYPE mt;
                        if ( SUCCEEDED( pp->ConnectionMediaType( &mt ) ) )
                        {
                            if ( mt.formattype == FORMAT_WaveFormatEx )
                            {
                                WAVEFORMATEX *pwfx = ( WAVEFORMATEX * )mt.pbFormat;
                                if ( pwfx != NULL )
                                {
                                    *(&audioInfo->wfmtWaveFormat) = *pwfx;
                                    hResult = S_OK;

                                    if( mt.cbFormat )
                                        CoTaskMemFree( mt.pbFormat );
                                    pp->Release();
                                    pp = NULL;
                                    break;
                                }
                                else
                                {
                                    hResult = E_POINTER;
                                }
                            }
                            else
                            {
                                hResult = S_FALSE;
                            }
                            // AM_MEDIA_TYPE is a structure on the stack, NOT an object.
                            // When it goes out of scope, it will not clean up its internal members.
                            if( mt.cbFormat )
                                CoTaskMemFree( mt.pbFormat );
                        }

                        pp->Release();
                        pp = NULL;
                    }
                    piep->Release();
                }
                pibf->Release();
            }
        }

        if (NULL != pnsep)
        {
            long codecCount = 0;
            if( SUCCEEDED( g_pPlayerWindow->m_pMP->get_CodecCount( &codecCount ) ) 
                && codecCount )
            {
                int i;
                for( i = 1; i <= codecCount; i++ )
                {
                    BSTR codecDesc;
                    if( SUCCEEDED( g_pPlayerWindow->m_pMP->GetCodecDescription( i, &codecDesc ) )
                        && wcsstr( codecDesc, L"Audio" ) )
                    {
                        audioInfo->bstrCodecDescription = codecDesc;
                        hResult = S_OK;
                        break;
                    }
                } // for each codec, until audio is found
            } // get_CodecCount succeeded and count > 0
        } // g_pPlayerWindow->m_pMP is valid

#ifdef UNDER_CE
        if( NULL != psmc )
        {
            psmc->get_IsSecure( &audioInfo->bSecure );
            hResult = S_OK;
        }
#endif /* UNDER_CE */
    }

    return hResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPropertyDlg::Update()
// Desc: This function is called when the dialog receives a PD_UPDATE message.
//       It then updates the filename and filters in the dialog.
///////////////////////////////////////////////////////////////////////////////
void CPropertyDlg::Update(TCHAR *szFilename)
{
    IBasicAudio           * piba  = NULL;
    IAMNetShowExProps     * pnsep = NULL;
    IAMSecureMediaContent * psmc  = NULL;

    SIZE size;
    LONG cx = 0;;
    HDC  hdc;

    hdc = ::GetDC(m_hListView);

    g_pPlayerWindow->FindInterfaceOnGraph(IID_IBasicAudio,           (void**)&piba);
    g_pPlayerWindow->FindInterfaceOnGraph(IID_IAMNetShowExProps,     (void**)&pnsep);
    g_pPlayerWindow->FindInterfaceOnGraph(IID_IAMSecureMediaContent, (void**)&psmc);

    ListView_DeleteAllItems( m_hListView );

    int itemNumber = 0;
    BSTR property = NULL;
    LVITEM item;
    item.mask = LVIF_TEXT;
    item.iSubItem = 0;
    item.state = 0;
    item.stateMask = 0;

    // Fill in our Show Title
    g_pPlayerWindow->m_pMP->GetMediaInfoString( mpShowTitle, &property );
    if( property )
    {
        if( *property )
        {
            item.iItem = itemNumber;
            item.pszText = L"Show Title:";
            if( ListView_InsertItem( m_hListView, &item ) != -1 )
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;

            item.iItem = itemNumber;
            item.pszText = property;
            if( ListView_InsertItem( m_hListView, &item ) != -1 )
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;
        }
        SysFreeString(property);
        property = NULL;
    }

    // Fill in our Show Author
    g_pPlayerWindow->m_pMP->GetMediaInfoString( mpShowAuthor, &property );
    if( property )
    {
        if( *property )
        {
            item.iItem = itemNumber;
            item.pszText = L"Show Author:";
            if( ListView_InsertItem( m_hListView, &item ) != -1 )
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;

            item.iItem = itemNumber;
            item.pszText = property;
            if( ListView_InsertItem( m_hListView, &item ) != -1 )
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;
        }
        SysFreeString(property);
        property = NULL;
    }

    // Fill in our Show Copyright
    g_pPlayerWindow->m_pMP->GetMediaInfoString( mpShowCopyright, &property );
    if( property )
    {
        if( *property )
        {
            item.iItem = itemNumber;
            item.pszText = L"Show Copyright:";
            if( ListView_InsertItem( m_hListView, &item ) != -1 )
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;

            item.iItem = itemNumber;
            item.pszText = property;
            if( ListView_InsertItem( m_hListView, &item ) != -1 )
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;
        }
        SysFreeString(property);
        property = NULL;
    }

    // Fill in our Clip Title
    g_pPlayerWindow->m_pMP->GetMediaInfoString( mpClipTitle, &property );
    if( property )
    {
        if( *property )
        {
            item.iItem = itemNumber;
            item.pszText = L"Clip Title:";
            if( ListView_InsertItem( m_hListView, &item ) != -1 )
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;

            item.iItem = itemNumber;
            item.pszText = property;
            if( ListView_InsertItem( m_hListView, &item ) != -1 )
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;
        }
        SysFreeString(property);
        property = NULL;
    }

    // Fill in our Clip Artist
    g_pPlayerWindow->m_pMP->GetMediaInfoString( mpClipAuthor, &property );
    if( property )
    {
        if( *property )
        {
            item.iItem = itemNumber;
            item.pszText = L"Clip Artist:";
            if( ListView_InsertItem( m_hListView, &item ) != -1 )
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;

            item.iItem = itemNumber;
            item.pszText = property;
            if( ListView_InsertItem( m_hListView, &item ) != -1 )
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;
        }
        SysFreeString(property);
        property = NULL;
    }

    // Fill in our Clip Description
    g_pPlayerWindow->m_pMP->GetMediaInfoString( mpClipDescription, &property );
    if( property )
    {
        if( *property )
        {
            item.iItem = itemNumber;
            item.pszText = L"Clip Album:"; // The mpClipDescription maps to IAMMediaContent internally.  On CE 4.2, we override
                                        // the "description" field to be Album information (from ID3 or ASF metadata).  So
                                        // let's call this what it usually is - Clip Album, not Clip Description.
            if( ListView_InsertItem( m_hListView, &item ) != -1 )
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;

            item.iItem = itemNumber;
            item.pszText = property;
            if( ListView_InsertItem( m_hListView, &item ) != -1 )
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;
        }
        SysFreeString(property);
        property = NULL;
    }

    // Fill in our Clip Copyright
    g_pPlayerWindow->m_pMP->GetMediaInfoString( mpClipCopyright, &property );
    if( property )
    {
        if( *property )
        {
            item.iItem = itemNumber;
            item.pszText = L"Clip Copyright:";
            if( ListView_InsertItem( m_hListView, &item ) != -1 )
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;

            item.iItem = itemNumber;
            item.pszText = property;
            if( ListView_InsertItem( m_hListView, &item ) != -1 )
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;
        }
        SysFreeString(property);
        property = NULL;
    }

    g_pPlayerWindow->m_pMP->get_FileName( &property );
    if( property )
    {
        if( *property )
        {
            // Fill in our filename
            item.iItem = itemNumber;
            item.pszText = L"Filename:";
            if( ListView_InsertItem( m_hListView, &item ) != -1 )
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;

            TCHAR *slash = wcsrchr( property, L'\\' );
            item.iItem = itemNumber;
            item.pszText = slash ? ++slash : property;
            if( ListView_InsertItem( m_hListView, &item ) != -1 )
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;
        }
    }
    // Fill in our duration
    double duration = 0.0;
    VARIANT_BOOL fDurationValid;

    g_pPlayerWindow->m_pMP->get_Duration( &duration );
   	g_pPlayerWindow->m_pMP->get_IsDurationValid(&fDurationValid);

    if( fDurationValid && duration != 0.0 )
    {
        WCHAR durationText[10] = L"";
        Time2String( duration, durationText, 10, FALSE, FALSE, L":", L"." );

        item.iItem = itemNumber;
        item.pszText = L"Duration:";
        if( ListView_InsertItem( m_hListView, &item ) != -1 )
            itemNumber++;

        if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
            && size.cx > cx)
            cx = size.cx;

        item.iItem = itemNumber;
        item.pszText = durationText;
        if( ListView_InsertItem( m_hListView, &item ) != -1 )
            itemNumber++;

        if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
            && size.cx > cx)
            cx = size.cx;
    }

    // Fill in our audio-specific information
    AUDIOINFO audioInfo;
	memset( &audioInfo, 0, sizeof( AUDIOINFO ) );
    if( SUCCEEDED( AudioInfo( &audioInfo, piba, pnsep, psmc ) ) )
    {
        // Codec description
        if( audioInfo.bstrCodecDescription && *audioInfo.bstrCodecDescription )
        {
            item.iItem = itemNumber;
            item.pszText = L"Audio Codec:";
            if( ListView_InsertItem( m_hListView, &item ) != -1 )
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;

            item.iItem = itemNumber;
            item.pszText = audioInfo.bstrCodecDescription;
            if( ListView_InsertItem( m_hListView, &item ) != -1 )
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;

            SysFreeString(audioInfo.bstrCodecDescription);
        }

        // audio bitrate
        if( audioInfo.lpwcCodecBitrate && *audioInfo.lpwcCodecBitrate )
        {
            item.iItem = itemNumber;
            item.pszText = L"Audio Bitrate:";
            if( ListView_InsertItem( m_hListView, &item ) != -1 )
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;

            item.iItem = itemNumber;
            item.pszText = audioInfo.lpwcCodecBitrate;
            if( ListView_InsertItem( m_hListView, &item ) != -1 )
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;

            delete [] audioInfo.lpwcCodecBitrate;
        }

        // What's the frequency, Kenneth?
        WCHAR frequency[8];
        if( audioInfo.wfmtWaveFormat.nSamplesPerSec )
        {
            StringCchPrintf(frequency, 8, L"%2.1fKHz", (float)(audioInfo.wfmtWaveFormat.nSamplesPerSec) / 1000.0 );

            item.iItem = itemNumber;
            item.pszText = L"Audio Frequency:";
            if( ListView_InsertItem( m_hListView, &item ) != -1 )
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;

            item.iItem = itemNumber;
            item.pszText = frequency;
            if( ListView_InsertItem( m_hListView, &item ) != -1 )
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;
        }

        // stereo or mono sound?
        WCHAR stereo[4];
        if( audioInfo.wfmtWaveFormat.nChannels )
        {
            StringCchCopy(stereo, 4, audioInfo.wfmtWaveFormat.nChannels > 1 ? L"Yes" : L"No" );

            item.iItem = itemNumber;
            item.pszText = L"Stereo:";
            if( ListView_InsertItem( m_hListView, &item ) != -1 )
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;

            item.iItem = itemNumber;
            item.pszText = stereo;
            if( ListView_InsertItem( m_hListView, &item ) != -1 )
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;
        }
    }

    // Fill in our video-codec information
    BSTR videoCodec = NULL;
    if( SUCCEEDED( VideoInfo( &videoCodec, pnsep ) ) )
    {
        item.iItem = itemNumber;
        item.pszText = L"Video Codec:";
        if( ListView_InsertItem( m_hListView, &item ) != -1 )
            itemNumber++;

        if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
            && size.cx > cx)
            cx = size.cx;

        item.iItem = itemNumber;
        item.pszText = videoCodec;
        if( ListView_InsertItem( m_hListView, &item ) != -1 )
            itemNumber++;

        if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
            && size.cx > cx)
            cx = size.cx;

        SysFreeString(videoCodec);
    }

    // Fill in our video resolution
    long lWidth, lHeight;
    WCHAR resolution[10] = L"";
    if( SUCCEEDED( g_pPlayerWindow->m_pMP->get_ImageSourceWidth( &lWidth ) ) &&
        SUCCEEDED( g_pPlayerWindow->m_pMP->get_ImageSourceHeight( &lHeight ) ) &&
        lWidth && lHeight )
    {
        StringCchPrintf(resolution, 10, L"%d x %d", lWidth, lHeight );

        item.iItem = itemNumber;
        item.pszText = L"Video Size:";
        if( ListView_InsertItem( m_hListView, &item ) != -1 )
            itemNumber++;

        if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
            && size.cx > cx)
            cx = size.cx;

        item.iItem = itemNumber;
        item.pszText = resolution;
        if( ListView_InsertItem( m_hListView, &item ) != -1 )
            itemNumber++;

        if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
            && size.cx > cx)
            cx = size.cx;
    }

    // Fill in our "protected" information
    item.iItem = itemNumber;
    item.pszText = L"Protected:";
    if( ListView_InsertItem( m_hListView, &item ) != -1 )
        itemNumber++;

    if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
        && size.cx > cx)
        cx = size.cx;

    item.iItem = itemNumber;
    item.pszText = audioInfo.bSecure ? L"Yes" : L"No";
    if( ListView_InsertItem( m_hListView, &item ) != -1 )
        itemNumber++;

    if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
        && size.cx > cx)
        cx = size.cx;

    // Tidy up the ListView...
    if (hdc)
    {
        ::ReleaseDC(m_hListView, hdc);
    }

    // First, check if our ListView is too tall...
    RECT rc;
    LONG width;

    GetClientRect(m_hListView, &rc);

    width = rc.right - rc.left;

    if (ListView_GetCountPerPage(m_hListView) < ListView_GetItemCount(m_hListView))
    {
        width -= GetSystemMetrics(SM_CXVSCROLL);
    }

    if (cx < width)
    {
        cx = width;
    }

    ListView_SetColumnWidth(m_hListView, 0, cx);

    SendMessage(m_hListView, WM_SETREDRAW, TRUE, 0);

    InvalidateRect(m_hListView, NULL, TRUE);

    if (NULL != piba)
    {
        piba->Release();
        piba = NULL;
    }

    if (NULL != pnsep)
    {
        pnsep->Release();
        pnsep = NULL;
    }

    if (NULL != psmc)
    {
        psmc->Release();
        psmc = NULL;
    }
}

void CPropertyDlg::UpdateAndRender(TCHAR * szFilename)
{
    IGraphBuilder   * pGraphBuilder = NULL;
    IAMMediaContent * pMediaContent = NULL;
    IEnumFilters    * pEnum         = NULL;
    IBaseFilter     * pFilter       = NULL;
    IBasicAudio           * piba    = NULL;
    IBasicVideo           * pibv    = NULL;
    IAMNetShowExProps     * pnsep   = NULL;
    IAMSecureMediaContent * psmc    = NULL;
    IMediaSeeking         * pims    = NULL;
    BSTR              property      = ::SysAllocString(L"");
    int               itemNumber    = 0;
    LVITEM            item;
    HRESULT           hr            = S_OK;

    SIZE size;
    LONG cx = 0;
    HDC  hdc;

    hdc = ::GetDC(m_hListView);

    item.mask      = LVIF_TEXT;
    item.iSubItem  = 0;
    item.state     = 0;
    item.stateMask = 0;

    ListView_DeleteAllItems(m_hListView);

    //
    // Render file
    //

    HCURSOR hPrevCursor;

    // Display an hour glass while rendering the file
    hPrevCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

    hr = CoCreateInstance(CLSID_FilterGraph,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IGraphBuilder,
                          (void**)&pGraphBuilder);

    if (SUCCEEDED(hr))
    {
        hr = pGraphBuilder->RenderFile(szFilename, NULL);
    }

    //
    // Get IAMMediaContent interface
    //

    if (SUCCEEDED(hr))
    {
         hr = pGraphBuilder->EnumFilters(&pEnum);
    }

    if (NULL != pEnum)
    {
        hr = E_NOINTERFACE;

        while ( ( NULL == pMediaContent
                  || NULL == piba
                  || NULL == pnsep
                  || NULL == psmc )
               && S_OK == pEnum->Next(1, &pFilter, NULL))
        {
            if (NULL == pMediaContent)
                pFilter->QueryInterface(IID_IAMMediaContent,       (void**)&pMediaContent);
            if (NULL == piba)
                pFilter->QueryInterface(IID_IBasicAudio,           (void**)&piba);
            if (NULL == pibv)
                pFilter->QueryInterface(IID_IBasicVideo,           (void**)&pibv);
            if (NULL == pnsep)
                pFilter->QueryInterface(IID_IAMNetShowExProps,     (void**)&pnsep);
            if (NULL == psmc)
                pFilter->QueryInterface(IID_IAMSecureMediaContent, (void**)&psmc);
            if (NULL == pims)
                pFilter->QueryInterface(IID_IMediaSeeking,         (void**)&pims);

            pFilter->Release();
            pFilter = NULL;
        }

        pEnum->Release();
        pEnum = NULL;
    }

    // Restore the old cursor
    SetCursor(hPrevCursor);
 
    //
    // Display info
    //

    // Clip Title
    if (pMediaContent)
    {
        pMediaContent->get_Title(&property);
    }

    if (*property)
    {
        item.iItem   = itemNumber;
        item.pszText = L"Clip Title:";
        if (ListView_InsertItem(m_hListView, &item) != -1)
            itemNumber++;

        if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
            && size.cx > cx)
            cx = size.cx;

        item.iItem   = itemNumber;
        item.pszText = property;
        if (ListView_InsertItem(m_hListView, &item) != -1)
            itemNumber++;

        if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
            && size.cx > cx)
            cx = size.cx;

        SysFreeString(property);
    }
 
    // Clip Author
    if (pMediaContent)
    {
        property = (TCHAR *)L"";
        pMediaContent->get_AuthorName(&property);
    }

    if (*property)
    {
        item.iItem   = itemNumber;
        item.pszText = L"Clip Artist:";
        if (ListView_InsertItem(m_hListView, &item) != -1)
            itemNumber++;

        if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
            && size.cx > cx)
            cx = size.cx;

        item.iItem   = itemNumber;
        item.pszText = property;
        if (ListView_InsertItem(m_hListView, &item) != -1)
            itemNumber++;

        if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
            && size.cx > cx)
            cx = size.cx;

        SysFreeString(property);
    }

    // Clip Description
    if (pMediaContent)
    {
        property = (TCHAR *)L"";
        pMediaContent->get_Description(&property);
    }

    if (*property)
    {
        item.iItem   = itemNumber;
        item.pszText = L"Clip Album:";

        if (ListView_InsertItem(m_hListView, &item) != -1)
            itemNumber++;

        if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
            && size.cx > cx)
            cx = size.cx;

        item.iItem   = itemNumber;
        item.pszText = property;
        if (ListView_InsertItem(m_hListView, &item) != -1)
            itemNumber++;

        if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
            && size.cx > cx)
            cx = size.cx;

        SysFreeString(property);
    }

    // Clip Copyright
    if (pMediaContent)
    {
        property = (TCHAR *)L"";
        pMediaContent->get_Copyright(&property);
    }

    if (*property)
    {
        item.iItem   = itemNumber;
        item.pszText = L"Clip Copyright:";
        if (ListView_InsertItem(m_hListView, &item) != -1)
            itemNumber++;

        if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
            && size.cx > cx)
            cx = size.cx;

        item.iItem   = itemNumber;
        item.pszText = property;
        if (ListView_InsertItem(m_hListView, &item) != -1)
            itemNumber++;

        if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
            && size.cx > cx)
            cx = size.cx;

        SysFreeString(property);
    }

    // Filename
    item.iItem   = itemNumber;
    item.pszText = L"Filename:";
    if (ListView_InsertItem(m_hListView, &item) != -1)
        itemNumber++;

    if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
        && size.cx > cx)
        cx = size.cx;

    TCHAR * slash = wcsrchr(szFilename, L'\\');
    item.iItem   = itemNumber;
    item.pszText = slash ? ++slash : szFilename;
    if (ListView_InsertItem(m_hListView, &item) != -1)
        itemNumber++;

    if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
        && size.cx > cx)
        cx = size.cx;

    // duration
    if (NULL != pims)
    {
        double   dDuration  = 0.0;
        LONGLONG llDuration = 0;

        pims->GetDuration(&llDuration);

        if (llDuration)
        {
            WCHAR durationText[10] = L"";

            dDuration = llDuration / 1E7;

            Time2String(dDuration, durationText, 10, FALSE, FALSE, L":", L".");

            item.iItem = itemNumber;
            item.pszText = L"Duration:";
            if (-1 != ListView_InsertItem(m_hListView, &item))
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;

            item.iItem = itemNumber;
            item.pszText = durationText;
            if (-1 != ListView_InsertItem(m_hListView, &item))
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;
        }
    }

    // audio info
    AUDIOINFO audioInfo;
    memset(&audioInfo, 0, sizeof (AUDIOINFO));
    if (SUCCEEDED(AudioInfo(&audioInfo, piba, pnsep, psmc)))
    {
        // codec description
        if (audioInfo.bstrCodecDescription && *audioInfo.bstrCodecDescription)
        {
            item.iItem = itemNumber;
            item.pszText = L"Audio Codec:";
            if (-1 != ListView_InsertItem(m_hListView, &item))
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;

            item.iItem = itemNumber;
            item.pszText = audioInfo.bstrCodecDescription;
            if (-1 != ListView_InsertItem(m_hListView, &item))
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;

            SysFreeString(audioInfo.bstrCodecDescription);
        }

        // audio bitrate
        if (audioInfo.lpwcCodecBitrate && *audioInfo.lpwcCodecBitrate)
        {
            item.iItem = itemNumber;
            item.pszText = L"Audio Bitrate:";
            if (-1 != ListView_InsertItem(m_hListView, &item))
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;

            item.iItem = itemNumber;
            item.pszText = audioInfo.lpwcCodecBitrate;
            if (-1 != ListView_InsertItem(m_hListView, &item))
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;

            delete [] audioInfo.lpwcCodecBitrate;
        }

        // What's the frequency, Kenneth?
        WCHAR frequency[8];
        if (audioInfo.wfmtWaveFormat.nSamplesPerSec)
        {
            StringCchPrintf( frequency, 8, L"%2.1fKHz", (float)(audioInfo.wfmtWaveFormat.nSamplesPerSec) / 1000.0);

            item.iItem = itemNumber;
            item.pszText = L"Audio Frequency:";
            if (-1 != ListView_InsertItem(m_hListView, &item))
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;

            item.iItem = itemNumber;
            item.pszText = frequency;
            if (-1 != ListView_InsertItem(m_hListView, &item))
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;
        }

        // stero or mono sound?
        WCHAR stereo[4];
        if (audioInfo.wfmtWaveFormat.nChannels)
        {
            if(audioInfo.wfmtWaveFormat.nChannels > 1)
                StringCchPrintf( stereo, 4, L"Yes");
            else
                StringCchPrintf( stereo, 4, L"No");

            item.iItem = itemNumber;
            item.pszText = L"Stereo:";
            if (-1 != ListView_InsertItem(m_hListView, &item))
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;

            item.iItem = itemNumber;
            item.pszText = stereo;
            if (-1 != ListView_InsertItem(m_hListView, &item))
                itemNumber++;

            if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
                && size.cx > cx)
                cx = size.cx;
        }
    }

    // video info
    BSTR videoCodec = NULL;
    if (SUCCEEDED(VideoInfo(&videoCodec, pnsep)))
    {
        item.iItem = itemNumber;
        item.pszText = L"Video Codec:";
        if (-1 != ListView_InsertItem(m_hListView, &item))
            itemNumber++;

        if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
            && size.cx > cx)
            cx = size.cx;

        item.iItem = itemNumber;
        item.pszText = videoCodec;
        if (-1 != ListView_InsertItem(m_hListView, &item))
            itemNumber++;

        if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
            && size.cx > cx)
            cx = size.cx;

        SysFreeString(videoCodec);
    }

    // video resolution
    long lWidth, lHeight;
    WCHAR resolution[10] = L"";
    if (NULL != pibv
        && SUCCEEDED(pibv->get_VideoWidth(&lWidth))
        && SUCCEEDED(pibv->get_VideoHeight(&lHeight))
        && lWidth && lHeight)
    {
        StringCchPrintf( resolution, 10, L"%d x %d", lWidth, lHeight);

        item.iItem = itemNumber;
        item.pszText = L"Video Size:";
        if (-1 != ListView_InsertItem(m_hListView, &item))
            itemNumber++;

        if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
            && size.cx > cx)
            cx = size.cx;

        item.iItem = itemNumber;
        item.pszText = resolution;
        if (-1 != ListView_InsertItem(m_hListView, &item))
            itemNumber++;

        if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
            && size.cx > cx)
            cx = size.cx;
    }

    // protected
    item.iItem = itemNumber;
    item.pszText = L"Protected:";
    if (-1 != ListView_InsertItem(m_hListView, &item))
        itemNumber++;

    if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
        && size.cx > cx)
        cx = size.cx;

    item.iItem = itemNumber;
    item.pszText = audioInfo.bSecure ? L"Yes" : L"No";
    if (-1 != ListView_InsertItem(m_hListView, &item))
        itemNumber++;

    if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
        && size.cx > cx)
        cx = size.cx;

    if (hdc)
    {
        ::ReleaseDC(m_hListView, hdc);
    }

    // Clean up list view
    RECT rc;
    LONG width;

    GetClientRect(m_hListView, &rc);

    width = rc.right - rc.left;

    if (ListView_GetCountPerPage(m_hListView) < ListView_GetItemCount(m_hListView))
    {
        width -= GetSystemMetrics(SM_CXVSCROLL);
    }

    if (cx < width)
    {
        cx = width;
    }

    ListView_SetColumnWidth(m_hListView, 0, cx);

    SendMessage(m_hListView, WM_SETREDRAW, TRUE, 0);

    InvalidateRect(m_hListView, NULL, TRUE);

    // Release filter interfaces
    if (NULL != pMediaContent)
    {
        pMediaContent->Release();
        pMediaContent = NULL;
    }

    if (NULL != piba)
    {
        piba->Release();
        piba = NULL;
    }

    if (NULL != pibv)
    {
        pibv->Release();
        pibv = NULL;
    }

    if (NULL != pnsep)
    {
        pnsep->Release();
        pnsep = NULL;
    }

    if (NULL != psmc)
    {
        psmc->Release();
        psmc = NULL;
    }

    if (NULL != pims)
    {
        pims->Release();
        pims = NULL;
    }

    // Release IGraphBuilder
    if (NULL != pGraphBuilder)
    {
         pGraphBuilder->Release();
         pGraphBuilder = NULL;
    }
    ::SysFreeString( property );
}
