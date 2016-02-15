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
// File: StatisticsDlg.cpp
//
// Desc: This file defines the member functions of the Statistics Dialog class
//       as well as the DialogProc for the dialog.
//
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <tchar.h>
#include <nsplay.h>

#include "StatisticsDlg.h"
#include "PlayerWindow.h"
#include "resource.h"

#include "aygshell_helper.h"

static const int REGKEY_SIZE = 80;

extern bool g_bSmallScreen;

///////////////////////////////////////////////////////////////////////////////
// Name: StatisticsDialogProc()
// Desc: This function handles dialog messages for the Statistics dialog box.
///////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK StatisticsDialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static CStatisticsDlg *pStatisticsDlg = NULL;
    
    switch (uMsg)
    {
    case WM_INITDIALOG:
        pStatisticsDlg = new CStatisticsDlg;
        
        if (NULL == pStatisticsDlg || false == pStatisticsDlg->Init(hWndDlg))
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
        if( pStatisticsDlg )
        {
            return pStatisticsDlg->HandleNotifyMsg(hWndDlg, uMsg, wParam, lParam);
        }
        break;
        
    // The WM_CLOSE message is sent when the "X" button is pressed.
    case WM_CLOSE:
        DestroyWindow(hWndDlg);
        return 0;

    case WM_DESTROY:
        if (NULL != pStatisticsDlg)
        {
            delete pStatisticsDlg;
            pStatisticsDlg = NULL;
        }
        return TRUE;
        
    // This message is sent from the parent window to tell the dialog to
    // show itself.
    case SD_SHOW:
        if( pStatisticsDlg )
        {
            pStatisticsDlg->Show(static_cast<int>(wParam));
        }
        return TRUE;
        
    // This message is sent from the parent window to force the dialog to
    // update the fields in the display.
    case SD_UPDATE:
        if( pStatisticsDlg )
        {
            pStatisticsDlg->Update(reinterpret_cast<CStatisticsDlg::stats_t*>(wParam));
            return TRUE;
        }
        break;
    }
    
    return FALSE;
}

CStatisticsDlg::CStatisticsDlg() : m_dFrameRate(0.0),
    m_dActualRate(0.0),
    m_lFramesDropped(0),
    m_lBandwidth(0),
    m_lSourceProtocol(0),
    m_lReceivedPackets(0),
    m_lRecoveredPackets(0),
    m_lLostPackets(0),
    m_lDroppedAudioPackets(0),
    m_szErrorCorrection(NULL),
    m_szFilename(NULL),
    m_hWnd(NULL),
    m_hWndParent(NULL),
    m_hListView(NULL),
    m_hFont(NULL),
    m_iMaxWidth(0),
    m_iFRIndex(-1),
    m_iARIndex(-1),
    m_iFDIndex(-1),
    m_iBWIndex(-1),
    m_iSPIndex(-1),
    m_iRCIndex(-1),
    m_iRPIndex(-1),
    m_iLPIndex(-1),
    m_iECIndex(-1),
    m_iFNIndex(-1),
    m_iMIIndex(-1),
    m_bFRChanged(true),
    m_bARChanged(true),
    m_bFDChanged(true),
    m_bBWChanged(true),
    m_bSPChanged(true),
    m_bRCChanged(true),
    m_bRPChanged(true),
    m_bLPChanged(true),
    m_bECChanged(true),
    m_bFNChanged(true)
{
    LOGFONT logfont;
    memset(&logfont, 0, sizeof (logfont));
    logfont.lfWeight = FW_SEMIBOLD;
    logfont.lfHeight = -11;
    m_hFont = CreateFontIndirect(&logfont);
}

CStatisticsDlg::~CStatisticsDlg()
{
    (void)Fini();

    if (m_hFont)
        DeleteObject(m_hFont);
}

BOOL CStatisticsDlg::HandleNotifyMsg(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fHandled = FALSE;
    NMHDR * pnmh = (NMHDR*)lParam;
    NMLISTVIEW *pnmlv = (NMLISTVIEW*)lParam;

    if (NM_CUSTOMDRAW == pnmh->code
        && m_hListView == pnmh->hwndFrom)
    {
        NMLVCUSTOMDRAW *pcd = (NMLVCUSTOMDRAW*)lParam;

        if (CDDS_PREPAINT == pcd->nmcd.dwDrawStage)
        {
            SetWindowLong(hWndDlg, DWL_MSGRESULT, CDRF_NOTIFYITEMDRAW);
            fHandled = TRUE;
        }
        else if (CDDS_ITEMPREPAINT == pcd->nmcd.dwDrawStage)
        {
            if (0 == (pcd->nmcd.dwItemSpec % 2) && m_hFont)
            {
                SelectObject(pcd->nmcd.hdc, m_hFont);
                SetWindowLong(hWndDlg, DWL_MSGRESULT, CDRF_NEWFONT);
            }

            fHandled = TRUE;
        }
    }
    else if (LVN_DELETEALLITEMS == pnmh->code)
    {
        //
        // Suppress LVN_DELETEITEM notifications
        //
        SetWindowLong(hWndDlg, DWL_MSGRESULT, TRUE);
        fHandled = TRUE;
    }

    return fHandled;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CStatisticsDlg::Init()
// Desc: This function intializes the StatisticsDlg class.
///////////////////////////////////////////////////////////////////////////////
bool CStatisticsDlg::Init(HWND hWnd)
{
    m_hWnd       = hWnd;
    m_hWndParent = ::GetParent(m_hWnd);
    m_hListView = GetDlgItem(m_hWnd, IDC_STATISTICS_LIST);

    if (NULL == m_hWnd || NULL == m_hWndParent)
    {
        return false;
    }

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
    GetClientRect(m_hListView, &rect);
    column.mask = LVCF_FMT | LVCF_WIDTH;
    column.fmt  = LVCFMT_LEFT;
    column.cx   = rect.right - rect.left;
    column.iSubItem = 0;
    column.iOrder   = 0;
    column.iImage   = 0;
    int result = ListView_InsertColumn(m_hListView, 0, &column);

    ListView_SetExtendedListViewStyle(m_hListView, LVS_EX_GRIDLINES);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CStatisticsDlg::Fini()
// Desc: This function closes the Statistics dialog (if necessary) and signals
//       the parent window that it has closed.
///////////////////////////////////////////////////////////////////////////////
bool CStatisticsDlg::Fini()
{
    bool bResult = false;
    
    if (NULL != m_hWndParent)
    {
        PostMessage(m_hWndParent, SD_CLOSED, NULL, NULL);
        m_hWndParent = NULL;
    }

    if (NULL != m_szErrorCorrection)
    {
        delete[] m_szErrorCorrection;
        m_szErrorCorrection = NULL;
    }

    if (NULL != m_szFilename)
    {
        delete [] m_szFilename;
        m_szFilename = NULL;
    }

    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CStatisticsDlg::Show()
// Desc: This function will cause the dialog to be shown or hidden.
///////////////////////////////////////////////////////////////////////////////
bool CStatisticsDlg::Show(int iShowCmd)
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

    m_bFNChanged = true;
    m_bFRChanged = true;
    m_bARChanged = true;
    m_bFDChanged = true;
    m_bBWChanged = true;
    m_bSPChanged = true;
    m_bRCChanged = true;
    m_bRPChanged = true;
    m_bLPChanged = true;
    m_bECChanged = true;
    m_bADChanged = true;

    m_iFRIndex = -1;
    m_iARIndex = -1;
    m_iFDIndex = -1;
    m_iBWIndex = -1;
    m_iSPIndex = -1;
    m_iRCIndex = -1;
    m_iRPIndex = -1;
    m_iLPIndex = -1;
    m_iECIndex = -1;
    m_iFNIndex = -1;
    m_iMIIndex = -1;
    m_iADIndex = -1;

    Refresh();
    
    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CStatisticsDlg::Update()
// Desc: This function is called when a SD_UPDATE message is received by the
//       dialog.  It update the member variables associated with the edit
//       boxes in the dialog.
///////////////////////////////////////////////////////////////////////////////
void CStatisticsDlg::Update(stats_t *pStats)
{
    // Save this property set
    if (m_dFrameRate != pStats->dFrameRate)
    {
        m_dFrameRate = pStats->dFrameRate;
        m_bFRChanged = true;
    }
    
    if (m_dActualRate != pStats->dActualRate)
    {
        m_dActualRate = pStats->dActualRate;
        m_bARChanged  = true;
    }
    
    if (m_lFramesDropped != pStats->lFramesDropped)
    {
        m_lFramesDropped = pStats->lFramesDropped;
        m_bFDChanged     = true;
    }
    
    if (m_lBandwidth != pStats->lBandwidth)
    {
        m_lBandwidth = pStats->lBandwidth;
        m_bBWChanged = true;
    }
    
    if (m_lSourceProtocol != pStats->lSourceProtocol)
    {
        m_lSourceProtocol = pStats->lSourceProtocol;
        m_bSPChanged      = true;
    }
    
    if (m_lReceivedPackets != pStats->lReceivedPackets)
    {
        m_lReceivedPackets = pStats->lReceivedPackets;
        m_bRCChanged       = true;
    }
    
    if (m_lRecoveredPackets != pStats->lRecoveredPackets)
    {
        m_lRecoveredPackets = pStats->lRecoveredPackets;
        m_bRPChanged        = true;
    }
    
    if (m_lLostPackets != pStats->lLostPackets)
    {
        m_lLostPackets = pStats->lLostPackets;
        m_bLPChanged   = true;
    }
    
    if (m_lDroppedAudioPackets != pStats->lDroppedAudioPackets)
    {
        m_lDroppedAudioPackets = pStats->lDroppedAudioPackets;
        m_bADChanged   = true;
    }

    if (!(NULL != m_szErrorCorrection && NULL != pStats->szErrorCorrection
        && 0 == _tcscmp(m_szErrorCorrection, pStats->szErrorCorrection)))
    {
        if (NULL != m_szErrorCorrection)
        {
            delete[] m_szErrorCorrection;
            m_szErrorCorrection = NULL;
        }
        
        if (NULL != pStats->szErrorCorrection)
        {
            size_t BufLen = _tcslen(pStats->szErrorCorrection) + 1;
            m_szErrorCorrection = new TCHAR[BufLen];

            if (NULL != m_szErrorCorrection)
            {
                StringCchCopy(m_szErrorCorrection, BufLen, pStats->szErrorCorrection);
            }
        }

        m_bECChanged = true;
    }

    if (!(NULL != m_szFilename && NULL != pStats->szFilename
          && 0 == _tcscmp(m_szFilename, pStats->szFilename)))
    {
        if (NULL != m_szFilename)
        {
            delete [] m_szFilename;
            m_szFilename = NULL;
        }

        if (NULL != pStats->szFilename)
        {
            size_t BufLen = _tcslen(pStats->szFilename) + 1;
            m_szFilename = new TCHAR[BufLen];

            if (NULL != m_szFilename)
            {
                StringCchCopy(m_szFilename, BufLen, pStats->szFilename);
            }
        }

        m_bFNChanged = true;
    }

    Refresh();

    return;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CStatisticsDlg::Refresh()
// Desc: This function copies what is currently in member variables into the
//       associated edit box on the dialog.
///////////////////////////////////////////////////////////////////////////////
void CStatisticsDlg::Refresh()
{
    LONG  cx;
    TCHAR szText[REGKEY_SIZE];
    TCHAR szBuf[20];
    HDC   hdc = NULL;
    RECT rc;
    int width;

    int   itemNumber = 2;

    hdc = ::GetDC(m_hListView);

    cx = m_iMaxWidth;

    if (m_bFNChanged || -1 == m_iFNIndex)
    {
        if (-1 == m_iFNIndex)
        {
            ListView_DeleteAllItems(m_hListView);

            InsertTwoItems(0);

            m_iMaxWidth = 0;
            m_iFNIndex = 0;
            m_iFRIndex = -1;
            m_iARIndex = -1;
            m_iFDIndex = -1;
            m_iBWIndex = -1;
            m_iSPIndex = -1;
            m_iRCIndex = -1;
            m_iRPIndex = -1;
            m_iLPIndex = -1;
            m_iECIndex = -1;
            m_iMIIndex = -1;
            m_iADIndex = -1;
        }

        TCHAR * pszFilename = (m_szFilename ? m_szFilename : (TCHAR *)L"");
        TCHAR * pszSlash    = wcsrchr(pszFilename, L'\\');

        RefreshItem(m_iFNIndex, L"Filename:", (pszSlash ? ++pszSlash : pszFilename), hdc, cx);
    }

    if (-1 == m_iFRIndex && m_bFRChanged && 0.0 != m_dFrameRate)
    {
        m_iFRIndex = itemNumber;

        InsertTwoItems(m_iFRIndex);

        if (-1 != m_iARIndex) m_iARIndex += 2;
        if (-1 != m_iFDIndex) m_iFDIndex += 2;
        if (-1 != m_iBWIndex) m_iBWIndex += 2;
        if (-1 != m_iSPIndex) m_iSPIndex += 2;
        if (-1 != m_iRCIndex) m_iRCIndex += 2;
        if (-1 != m_iRPIndex) m_iRPIndex += 2;
        if (-1 != m_iLPIndex) m_iLPIndex += 2;
        if (-1 != m_iECIndex) m_iECIndex += 2;
        if (-1 != m_iMIIndex) m_iMIIndex += 2;
        if (-1 != m_iADIndex) m_iMIIndex += 2;

        itemNumber += 2;
    }
    else if (-1 != m_iFRIndex)
    {
        itemNumber = m_iFRIndex + 2;
    }

    if (0.0 != m_dFrameRate && m_bFRChanged)
    {
        StringCchPrintf(szBuf, 20, TEXT("%.2f"), m_dFrameRate);

        RefreshItem(m_iFRIndex, L"Desired Frame Rate:", szBuf, hdc, cx);
    }

    if (-1 == m_iARIndex && m_bARChanged && 0.0 != m_dActualRate)
    {
        m_iARIndex = itemNumber;

        InsertTwoItems(m_iARIndex);

        if (-1 != m_iFDIndex) m_iFDIndex += 2;
        if (-1 != m_iBWIndex) m_iBWIndex += 2;
        if (-1 != m_iSPIndex) m_iSPIndex += 2;
        if (-1 != m_iRCIndex) m_iRCIndex += 2;
        if (-1 != m_iRPIndex) m_iRPIndex += 2;
        if (-1 != m_iLPIndex) m_iLPIndex += 2;
        if (-1 != m_iECIndex) m_iECIndex += 2;
        if (-1 != m_iMIIndex) m_iMIIndex += 2;
        if (-1 != m_iADIndex) m_iMIIndex += 2;

        itemNumber += 2;
    }
    else if (-1 != m_iARIndex)
    {
        itemNumber = m_iARIndex + 2;
    }

    if (0.0 != m_dActualRate && m_bARChanged)
    {
        StringCchPrintf(szBuf, 20, L"%.2f", m_dActualRate);

        RefreshItem(m_iARIndex, L"Frame Rate:", szBuf, hdc, cx);
    }

    // 10/15/03 - meirab - adding audio glitches counts
    if( -1 == m_iADIndex )
    {
        m_iADIndex = itemNumber;

        InsertTwoItems(m_iADIndex);

        if (-1 != m_iBWIndex) m_iBWIndex += 2;
        if (-1 != m_iSPIndex) m_iSPIndex += 2;
        if (-1 != m_iRCIndex) m_iRCIndex += 2;
        if (-1 != m_iRPIndex) m_iRPIndex += 2;
        if (-1 != m_iLPIndex) m_iLPIndex += 2;
        if (-1 != m_iECIndex) m_iECIndex += 2;
        if (-1 != m_iMIIndex) m_iMIIndex += 2;

        itemNumber += 2;
        StringCchPrintf(szBuf, 20, L"%ld", m_lDroppedAudioPackets);
        RefreshItem( m_iADIndex, L"Audio Glitches", szBuf, hdc, cx);
    }


    if (-1 == m_iFDIndex && m_bFDChanged && 0 != m_lFramesDropped)
    {
        m_iFDIndex = itemNumber;

        InsertTwoItems(m_iFDIndex);

        if (-1 != m_iBWIndex) m_iBWIndex += 2;
        if (-1 != m_iSPIndex) m_iSPIndex += 2;
        if (-1 != m_iRCIndex) m_iRCIndex += 2;
        if (-1 != m_iRPIndex) m_iRPIndex += 2;
        if (-1 != m_iLPIndex) m_iLPIndex += 2;
        if (-1 != m_iECIndex) m_iECIndex += 2;
        if (-1 != m_iMIIndex) m_iMIIndex += 2;

        itemNumber += 2;
    }
    else if (-1 != m_iFDIndex)
    {
        itemNumber = m_iFDIndex + 2;
    }

    if (0 != m_lFramesDropped && m_bFDChanged)
    {
        StringCchPrintf(szBuf, 20, L"%ld", m_lFramesDropped);

        RefreshItem(m_iFDIndex, L"Frames Dropped:", szBuf, hdc, cx);
    }

    if (-1 == m_iBWIndex && m_bBWChanged && 0 != m_lBandwidth)
    {
        m_iBWIndex = itemNumber;

        InsertTwoItems(m_iBWIndex);

        if (-1 != m_iSPIndex) m_iSPIndex += 2;
        if (-1 != m_iRCIndex) m_iRCIndex += 2;
        if (-1 != m_iRPIndex) m_iRPIndex += 2;
        if (-1 != m_iLPIndex) m_iLPIndex += 2;
        if (-1 != m_iECIndex) m_iECIndex += 2;
        if (-1 != m_iMIIndex) m_iMIIndex += 2;

        itemNumber += 2;
    }
    else if (-1 != m_iBWIndex)
    {
        itemNumber = m_iBWIndex + 2;
    }

    if (0 != m_lBandwidth && m_bBWChanged)
    {
        StringCchPrintf(szBuf, 20, L"%ld", m_lBandwidth);

        RefreshItem(m_iBWIndex, L"Bandwidth:", szBuf, hdc, cx);
    }

    szText[0] = L'\0';
    switch (m_lSourceProtocol)
    {
    case nsMulticast:
        LoadString( g_hInst, IDS_STREAM_MULTICAST_NO_CONNECTION, szText, REGKEY_SIZE );
        break;

    case nsMulticastPlus:
        LoadString( g_hInst, IDS_STREAM_MULTICAST, szText, REGKEY_SIZE );
        break;

    case nsUDP:
        LoadString( g_hInst, IDS_STREAM_UNICAST_UDP, szText, REGKEY_SIZE );
        break;

    case nsTCP:
        LoadString( g_hInst, IDS_STREAM_UNICAST_TCP, szText, REGKEY_SIZE );
        break;

    case nsDistribution:
        LoadString( g_hInst, IDS_STREAM_UNICAST_DISTRIBUTION, szText, REGKEY_SIZE );
        break;

    case nsHTTP:
        LoadString( g_hInst, IDS_STREAM_UNICAST_HTTP, szText, REGKEY_SIZE );
        break;

    case nsFile:
        LoadString( g_hInst, IDS_STREAM_NETWORKED_FILE, szText, REGKEY_SIZE );
        break;

    default:
        StringCchCopy(szText, REGKEY_SIZE, L"Local File");
        break;
    }

    if (-1 == m_iSPIndex && m_bSPChanged && L'\0' != szText[0])
    {
        m_iSPIndex = itemNumber;

        InsertTwoItems(m_iSPIndex);

        if (-1 != m_iRCIndex) m_iRCIndex += 2;
        if (-1 != m_iRPIndex) m_iRPIndex += 2;
        if (-1 != m_iLPIndex) m_iLPIndex += 2;
        if (-1 != m_iECIndex) m_iECIndex += 2;
        if (-1 != m_iMIIndex) m_iMIIndex += 2;

        itemNumber += 2;
    }
    else if (-1 != m_iSPIndex)
    {
        itemNumber = m_iSPIndex + 2;
    }

    if (L'\0' != szText[0] && m_bSPChanged)
    {
        RefreshItem(m_iSPIndex, L"Source Protocol:", szText, hdc, cx);
    }

    if (-1 == m_iECIndex && m_bECChanged && NULL != m_szErrorCorrection && L'\0' != m_szErrorCorrection[0])
    {
        m_iECIndex = itemNumber;

        InsertTwoItems(m_iECIndex);

        if (-1 != m_iRCIndex) m_iRCIndex += 2;
        if (-1 != m_iRPIndex) m_iRPIndex += 2;
        if (-1 != m_iLPIndex) m_iLPIndex += 2;
        if (-1 != m_iMIIndex) m_iMIIndex += 2;

        itemNumber += 2;
    }
    else if (-1 != m_iECIndex)
    {
        itemNumber = m_iECIndex + 2;
    }

    if (NULL != m_szErrorCorrection
        && L'\0' != m_szErrorCorrection[0])
    {
        RefreshItem(m_iECIndex, L"Error Correction:", m_szErrorCorrection, hdc, cx);
    }

    if (-1 == m_iRCIndex && (m_bRCChanged || m_bRPChanged || m_bLPChanged)
        && (0 != m_lReceivedPackets || 0 != m_lRecoveredPackets || 0 != m_lLostPackets))
    {
        m_iRCIndex = itemNumber;

        InsertTwoItems(m_iRCIndex);

        if (-1 != m_iRPIndex) m_iRPIndex += 2;
        if (-1 != m_iLPIndex) m_iLPIndex += 2;
        if (-1 != m_iMIIndex) m_iMIIndex += 2;

        itemNumber += 2;
    }
    else if (-1 != m_iRCIndex)
    {
        itemNumber = m_iRCIndex + 2;
    }

    if (-1 == m_iRPIndex && (m_bRCChanged || m_bRPChanged || m_bLPChanged)
        && (0 != m_lReceivedPackets || 0 != m_lRecoveredPackets || 0 != m_lLostPackets))
    {
        m_iRPIndex = itemNumber;

        InsertTwoItems(m_iRPIndex);

        if (-1 != m_iLPIndex) m_iLPIndex += 2;
        if (-1 != m_iMIIndex) m_iMIIndex += 2;

        itemNumber += 2;
    }
    else if (-1 != m_iRPIndex)
    {
        itemNumber = m_iRPIndex + 2;
    }

    if (-1 == m_iLPIndex && (m_bRCChanged || m_bRPChanged || m_bLPChanged)
        && (0 != m_lReceivedPackets || 0 != m_lRecoveredPackets || 0 != m_lLostPackets))
    {
        m_iLPIndex = itemNumber;

        InsertTwoItems(m_iLPIndex);

        if (-1 != m_iMIIndex) m_iMIIndex += 2;

        itemNumber += 2;
    }
    else if (-1 != m_iLPIndex)
    {
        itemNumber = m_iLPIndex + 2;
    }

    if ( (0 != m_lReceivedPackets
          || 0 != m_lRecoveredPackets
          || 0 != m_lLostPackets)
        && (m_bRCChanged || m_bRPChanged || m_bLPChanged))
    {
        StringCchPrintf(szBuf, 20, TEXT("%ld"), m_lReceivedPackets);

        RefreshItem(m_iRCIndex, L"Received Packets:", szBuf, hdc, cx);

        StringCchPrintf(szBuf, 20, TEXT("%ld"), m_lRecoveredPackets);

        RefreshItem(m_iRPIndex, L"Recovered Packets:", szBuf, hdc, cx);

        StringCchPrintf(szBuf, 20, TEXT("%ld"), m_lLostPackets);

        RefreshItem(m_iLPIndex, L"Lost Packets:", szBuf, hdc, cx);
    }

    //
    // Get memory information
    //
    MEMORYSTATUS memstat;

    memstat.dwLength = sizeof (MEMORYSTATUS);
    GlobalMemoryStatus(&memstat);

    if (-1 == m_iMIIndex)
    {
        m_iMIIndex = ListView_GetItemCount(m_hListView);

        InsertTwoItems(m_iMIIndex);
    }

    StringCchPrintf(szText, REGKEY_SIZE, L"%d%% Used (%d KB Available)", memstat.dwMemoryLoad, memstat.dwAvailPhys/1024);

    RefreshItem(m_iMIIndex, L"Memory Usage", szText, hdc, cx);

    m_bFRChanged = false;
    m_bARChanged = false;
    m_bFDChanged = false;
    m_bBWChanged = false;
    m_bSPChanged = false;
    m_bRCChanged = false;
    m_bRPChanged = false;
    m_bLPChanged = false;
    m_bECChanged = false;
    m_bFNChanged = false;

    if (hdc)
    {
        ::ReleaseDC(m_hListView, hdc);
    }

    // clean up the list view
    GetClientRect(m_hListView, &rc);

    width = rc.right - rc.left;

    if (cx < width)
    {
        cx = width;
    }

    ListView_SetColumnWidth(m_hListView, 0, cx);

    return;
}

void CStatisticsDlg::RefreshItem(int     iIndex,
                                 LPCTSTR pszTitle,
                                 TCHAR * pszText,
                                 HDC     hdc,
                                 LONG  & cx)
{
    LVITEM item;
    SIZE   size;
    int    itemNumber = 0;

    item.mask      = LVIF_TEXT;
    item.iSubItem  = 0;
    item.state     = 0;
    item.stateMask = 0;

    item.iItem   = iIndex;
    item.pszText = (TCHAR *)pszTitle;
    if (-1 != ListView_SetItem(m_hListView, &item))
        itemNumber++;

    if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
        && size.cx > cx)
        cx = size.cx;

    item.iItem   = iIndex + 1;
    item.pszText = pszText;
    if (-1 != ListView_SetItem(m_hListView, &item))
        itemNumber++;

    if (GetTextExtentExPoint(hdc, item.pszText, _tcslen(item.pszText), 0, NULL, NULL, &size)
        && size.cx > cx)
        cx = size.cx;

    if (cx > m_iMaxWidth)
        m_iMaxWidth = cx;
}

void CStatisticsDlg::InsertTwoItems(int iIndex)
{
    LVITEM item;

    item.mask      = LVIF_TEXT;
    item.pszText   = L"";
    item.iSubItem  = 0;
    item.state     = 0;
    item.stateMask = 0;
    item.iItem     = iIndex;

    ListView_InsertItem(m_hListView, &item);
    ListView_InsertItem(m_hListView, &item);
}
