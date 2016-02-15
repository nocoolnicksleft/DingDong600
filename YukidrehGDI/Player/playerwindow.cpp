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
// File: PlayerWindow.cpp
//
// Desc: This file contains the implementation of the PlayerWindow class,
//       which combines the MediaPlayer container and Event Sink class into
//       one class to facilitate event handling.
//
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <tchar.h>
#include <shellapi.h>
#include "PlayerWindow.h"

#define _COMCTL32_
#include <commctrl.h>
#undef  _COMCTL32_

#include <commdlg.h>
#include <uuids.h>
#include <strsafe.h>

#include "PropertyDlg.h"

#include "resource.h"
#include "StatisticsDlg.h"
#include "OpenURLDlg.h"
#include "OptionsDlg.h"
#include "PlaylistDlg.h"
#include "CEPlayerUtil.h"
#include "PlaylistMgr.h"
#include "decibels.h"

#define HIMETRIC_PER_INCH 2540
#define MAP_LOGHIM_TO_PIX(x, ppli) ((ppli) * (x) / HIMETRIC_PER_INCH)

#define MINIMUM_PLAYER_WIDTH 239
#define MINIMUM_PLAYER_HEIGHT 88

#define SKIN_HEIGHT 40
#define SKIN_WIDTH  210
#define SKIN_MARGIN 10

#define TRACKER_ID       123
#define TRACKER_TIMEOUT  750

#define MENU_ICON_WIDTH           16
#define MENU_ITEM_HEIGHT          20
#define MENU_ITEM_HORZ_MARGIN      2
#define MAX_MENU_ITEM_WIDTH      200

static const int FILTER_SIZE = 300;
static const int REGKEY_SIZE = 80;

extern TCHAR **g_ppszAudioTypes;
extern TCHAR **g_ppszVideoTypes;
extern TCHAR **g_ppszPlaylistTypes;
extern TCHAR  *g_szHomePage;
extern DWORD   g_cAudioTypes;
extern DWORD   g_cVideoTypes;
extern DWORD   g_cPlaylistTypes;
extern bool    g_bSmallScreen;
extern bool    g_bFullscreenToWindowedOnPause;

static const int ratesArray[] = { -600, -60, -30, -15, -4, -2, -1, 1, 2, 4, 15, 30, 60, 600 };

CPlayerWindow::CPlayerWindow(HWND hWnd, HINSTANCE hInstance) :
m_uiStatTimer(0),
m_uiTrackerTimer(0),
m_hInstance(hInstance),
m_hWnd(hWnd),
m_hWndCB(NULL),
m_hWndProp(NULL),
m_hWndStat(NULL),
m_hWndPlay(NULL),
m_eState(BAD),
m_szFilename(NULL),
m_szPath(NULL),
m_szFilter(NULL),
m_bPlayForever(FALSE),
m_bShuffle(FALSE),
m_dwZoomLevel(1),
m_dwMinimumWidth(MINIMUM_PLAYER_WIDTH),
m_dwMinimumHeight(MINIMUM_PLAYER_HEIGHT),
m_nFilterIndex(1),
m_pPlaylist(NULL),
m_himgLocationList(NULL),
m_bSwitchingPlaylists(false),
#ifdef CEPLAYER_SKIN
m_hbmBuffer(NULL),
m_iSkinMargin(0),
#endif
m_fResumeOpen( FALSE ),
m_bStop( FALSE )
{
    int i;
    
    // Intialize the properties structure to invalid properties
    m_stats.dFrameRate        = 0.0;
    m_stats.dActualRate       = 0.0;
    m_stats.lFramesDropped    = 0;
    m_stats.lBandwidth        = 0;
    m_stats.lSourceProtocol   = 0;
    m_stats.lReceivedPackets  = 0;
    m_stats.lRecoveredPackets = 0;
    m_stats.lLostPackets      = 0;
    m_stats.lDroppedAudioPackets = 0;
    m_stats.szErrorCorrection = NULL;
    m_stats.szFilename        = NULL;
    m_currentRate = sizeof( ratesArray ) / sizeof( ratesArray[0] ) / 2;

    InitializeCriticalSection( &m_csButtonInfoCritSec );

    m_himgLocationList = ImageList_LoadBitmap(g_hInst,
                                              MAKEINTRESOURCE(IDB_LOCATIONLIST),
                                              MENU_ICON_WIDTH,
                                              1,
                                              CLR_DEFAULT);
    
    for( i = 0; i < MAX_FILEOPEN_HISTORY; i++ )
    {
        m_szFilenameHistory[i] = NULL;
    }

    SetState(BAD);

    CMPContainer::AddRef();
    CMPEventSink::AddRef();
}

CPlayerWindow::~CPlayerWindow()
{
    // Incase someone forgot to call this method, we'll do it now
    Fini();

    // don't call Release for CMPContainer and CMPEventSink, because we don't
    // want their ref count going to zero...

    if (NULL != m_himgLocationList)
    {
        ImageList_Destroy(m_himgLocationList);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::Init()
// Desc: This function is used to initialize the PlayerWindow class.
//       It creates the Media Player control, and hooks up the Event sink.
///////////////////////////////////////////////////////////////////////////////
bool CPlayerWindow::Init()
{
    IConnectionPointContainer *pCPC     = NULL;
    IConnectionPoint          *pCP      = NULL;

    //
    // Load skin bitmaps
    //
#ifdef CEPLAYER_SKIN
    BITMAP                     bm;

    m_binfo[SKIN_PREV].eState      = DISABLED;
    m_binfo[SKIN_PREV].hUp         = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_PREV_UP));
    m_binfo[SKIN_PREV].hDown       = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_PREV_DOWN));
    m_binfo[SKIN_PREV].hDisabled   = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_PREV_DISABLED));
    m_binfo[SKIN_PREV].ptPos.x     = 49;
    m_binfo[SKIN_PREV].ptPos.y     = 18;
    memset(&bm, 0, sizeof (BITMAP));
    GetObject(m_binfo[SKIN_PREV].hUp, sizeof (BITMAP), &bm);
    m_binfo[SKIN_PREV].ptDim.x     = bm.bmWidth;
    m_binfo[SKIN_PREV].ptDim.y     = bm.bmHeight;

    m_binfo[SKIN_FR].eState      = DISABLED;
    m_binfo[SKIN_FR].hUp         = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_FR_UP));
    m_binfo[SKIN_FR].hDown       = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_FR_DOWN));
    m_binfo[SKIN_FR].hDisabled   = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_FR_DISABLED));
    m_binfo[SKIN_FR].ptPos.x     = 71;
    m_binfo[SKIN_FR].ptPos.y     = 18;
    memset(&bm, 0, sizeof (BITMAP));
    GetObject(m_binfo[SKIN_FR].hUp, sizeof (BITMAP), &bm);
    m_binfo[SKIN_FR].ptDim.x     = bm.bmWidth;
    m_binfo[SKIN_FR].ptDim.y     = bm.bmHeight;

    m_binfo[SKIN_FF].eState      = DISABLED;
    m_binfo[SKIN_FF].hUp         = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_FF_UP));
    m_binfo[SKIN_FF].hDown       = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_FF_DOWN));
    m_binfo[SKIN_FF].hDisabled   = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_FF_DISABLED));
    m_binfo[SKIN_FF].ptPos.x     = 93;
    m_binfo[SKIN_FF].ptPos.y     = 18;
    memset(&bm, 0, sizeof (BITMAP));
    GetObject(m_binfo[SKIN_FF].hUp, sizeof (BITMAP), &bm);
    m_binfo[SKIN_FF].ptDim.x     = bm.bmWidth;
    m_binfo[SKIN_FF].ptDim.y     = bm.bmHeight;

    m_binfo[SKIN_NEXT].eState      = DISABLED;
    m_binfo[SKIN_NEXT].hUp         = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_NEXT_UP));
    m_binfo[SKIN_NEXT].hDown       = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_NEXT_DOWN));
    m_binfo[SKIN_NEXT].hDisabled   = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_NEXT_DISABLED));
    m_binfo[SKIN_NEXT].ptPos.x     = 115;
    m_binfo[SKIN_NEXT].ptPos.y     = 18;
    memset(&bm, 0, sizeof (BITMAP));
    GetObject(m_binfo[SKIN_NEXT].hUp, sizeof (BITMAP), &bm);
    m_binfo[SKIN_NEXT].ptDim.x     = bm.bmWidth;
    m_binfo[SKIN_NEXT].ptDim.y     = bm.bmHeight;

    m_binfo[SKIN_SOUND].eState     = UP;
    m_binfo[SKIN_SOUND].hUp        = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_SOUND_UP));
    m_binfo[SKIN_SOUND].hDown      = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_SOUND_DOWN));
    m_binfo[SKIN_SOUND].hDisabled  = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_SOUND_DISABLED));
    m_binfo[SKIN_SOUND].ptPos.x    = 137;
    m_binfo[SKIN_SOUND].ptPos.y    = 18;
    memset(&bm, 0, sizeof (BITMAP));
    GetObject(m_binfo[SKIN_SOUND].hUp, sizeof (BITMAP), &bm);
    m_binfo[SKIN_SOUND].ptDim.x     = bm.bmWidth;
    m_binfo[SKIN_SOUND].ptDim.y     = bm.bmHeight;

    m_binfo[SKIN_STOP].eState      = DISABLED;
    m_binfo[SKIN_STOP].hUp         = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_STOP_UP));
    m_binfo[SKIN_STOP].hDown       = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_STOP_DOWN));
    m_binfo[SKIN_STOP].hDisabled   = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_STOP_DISABLED));
    m_binfo[SKIN_STOP].ptPos.x     = 30;
    m_binfo[SKIN_STOP].ptPos.y     = 6;
    memset(&bm, 0, sizeof (BITMAP));
    GetObject(m_binfo[SKIN_STOP].hUp, sizeof (BITMAP), &bm);
    m_binfo[SKIN_STOP].ptDim.x     = bm.bmWidth;
    m_binfo[SKIN_STOP].ptDim.y     = bm.bmHeight;

    m_binfo[SKIN_PLAY].eState      = DISABLED;
    m_binfo[SKIN_PLAY].hUp         = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_PLAY_UP));
    m_binfo[SKIN_PLAY].hDown       = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_PLAY_DOWN));
    m_binfo[SKIN_PLAY].hDisabled   = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_PLAY_DISABLED));
    m_binfo[SKIN_PLAY].ptPos.x     = 0;
    m_binfo[SKIN_PLAY].ptPos.y     = 0;
    memset(&bm, 0, sizeof (BITMAP));
    GetObject(m_binfo[SKIN_PLAY].hUp, sizeof (BITMAP), &bm);
    m_binfo[SKIN_PLAY].ptDim.x     = bm.bmWidth;
    m_binfo[SKIN_PLAY].ptDim.y     = bm.bmHeight;

    m_binfo[SKIN_PAUSE].eState     = UP;
    m_binfo[SKIN_PAUSE].hUp        = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_PAUSE_UP));
    m_binfo[SKIN_PAUSE].hDown      = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_PAUSE_DOWN));
    m_binfo[SKIN_PAUSE].hDisabled  = NULL;
    m_binfo[SKIN_PAUSE].ptPos.x    = 0;
    m_binfo[SKIN_PAUSE].ptPos.y    = 0;
    memset(&bm, 0, sizeof (BITMAP));
    GetObject(m_binfo[SKIN_PAUSE].hUp, sizeof (BITMAP), &bm);
    m_binfo[SKIN_PAUSE].ptDim.x    = bm.bmWidth;
    m_binfo[SKIN_PAUSE].ptDim.y    = bm.bmHeight;

    m_binfo[SKIN_VOLUME].eState    = UP;
    m_binfo[SKIN_VOLUME].hUp       = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_VOLUME_FOREGROUND));
    m_binfo[SKIN_VOLUME].hDown     = NULL;
    m_binfo[SKIN_VOLUME].hDisabled = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_VOLUME_BACKGROUND));
    m_binfo[SKIN_VOLUME].ptPos.x   = 159;
    m_binfo[SKIN_VOLUME].ptPos.y   = 19;
    memset(&bm, 0, sizeof (BITMAP));
    GetObject(m_binfo[SKIN_VOLUME].hUp, sizeof (BITMAP), &bm);
    m_binfo[SKIN_VOLUME].ptDim.x     = bm.bmWidth;
    m_binfo[SKIN_VOLUME].ptDim.y     = bm.bmHeight;

    m_binfo[SKIN_SEEK].eState      = DISABLED;
    m_binfo[SKIN_SEEK].hUp         = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_SEEK_FOREGROUND));
    m_binfo[SKIN_SEEK].hDown       = NULL;
    m_binfo[SKIN_SEEK].hDisabled   = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_SEEK_BACKGROUND));
    m_binfo[SKIN_SEEK].ptPos.x     = 49;
    m_binfo[SKIN_SEEK].ptPos.y     = 0;
    memset(&bm, 0, sizeof (BITMAP));
    GetObject(m_binfo[SKIN_SEEK].hUp, sizeof (BITMAP), &bm);
    m_binfo[SKIN_SEEK].ptDim.x     = bm.bmWidth;
    m_binfo[SKIN_SEEK].ptDim.y     = bm.bmHeight;

    m_binfoVolThumb.eState    = UP;
    m_binfoVolThumb.hUp       = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_THUMB_UP));
    m_binfoVolThumb.hDown     = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_THUMB_DOWN));
    m_binfoVolThumb.hDisabled = NULL;
    m_binfoVolThumb.ptPos.x   = 159;
    m_binfoVolThumb.ptPos.y   = 19;
    memset(&bm, 0, sizeof (BITMAP));
    GetObject(m_binfoVolThumb.hUp, sizeof (BITMAP), &bm);
    m_binfoVolThumb.ptDim.x   = bm.bmWidth;
    m_binfoVolThumb.ptDim.y   = bm.bmHeight;

    m_binfoSeekThumb.eState    = UP;
    m_binfoSeekThumb.hUp       = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_THUMB_UP));
    m_binfoSeekThumb.hDown     = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_THUMB_DOWN));
    m_binfoSeekThumb.hDisabled = NULL;
    m_binfoSeekThumb.ptPos.x   = 49;
    m_binfoSeekThumb.ptPos.y   = 0;
    memset(&bm, 0, sizeof (BITMAP));
    GetObject(m_binfoSeekThumb.hUp, sizeof (BITMAP), &bm);
    m_binfoSeekThumb.ptDim.x   = bm.bmWidth;
    m_binfoSeekThumb.ptDim.y   = bm.bmHeight;

    memset(&m_rcVolBounds,  0, sizeof (m_rcVolBounds));
    memset(&m_rcSeekBounds, 0, sizeof (m_rcSeekBounds));

    m_rcVolBounds.left  = m_binfo[SKIN_VOLUME].ptPos.x;
    m_rcVolBounds.right = m_binfo[SKIN_VOLUME].ptPos.x + m_binfo[SKIN_VOLUME].ptDim.x - m_binfoVolThumb.ptDim.x;

    m_rcSeekBounds.left  = m_binfo[SKIN_SEEK].ptPos.x;
    m_rcSeekBounds.right = m_binfo[SKIN_SEEK].ptPos.x + m_binfo[SKIN_SEEK].ptDim.x - m_binfoSeekThumb.ptDim.x;

    m_bPlayPause = true;
#endif /* CEPLAYER_SKIN */

    SetState(BAD);

    if (NULL != m_szFilter)
    {
        delete[] m_szFilter;
        m_szFilter = NULL;
    }

    // Figure out how big this string will be.
    int   iTotalSize = 0;
    int   iStrLen    = 0;
    DWORD i;

    iTotalSize += _tcslen(TEXT("All Media Files")) + 1;

    for (i = 0; i < g_cAudioTypes; i++)
    {
        iTotalSize += _tcslen(g_ppszAudioTypes[i]) + 2;
    }

    for (i = 0; i < g_cVideoTypes; i++)
    {
        iTotalSize += _tcslen(g_ppszVideoTypes[i]) + 2;
    }

    for (i = 0; i < g_cPlaylistTypes; i++)
    {
        iTotalSize += _tcslen(g_ppszPlaylistTypes[i]) + 2;
    }

    iTotalSize++;

    iTotalSize += _tcslen(TEXT("Video Files")) + 1;

    for (i = 0; i < g_cVideoTypes; i++)
    {
        iTotalSize += _tcslen(g_ppszVideoTypes[i]) + 2;
    }

    iTotalSize++;

    iTotalSize += _tcslen(TEXT("Playlists")) + 1;

    for (i = 0; i < g_cPlaylistTypes; i++)
    {
        iTotalSize += _tcslen(g_ppszPlaylistTypes[i]) + 2;
    }

    iTotalSize++;

    iTotalSize += _tcslen(TEXT("Audio Files")) + 1;

    for (i = 0; i < g_cAudioTypes; i++)
    {
        iTotalSize += _tcslen(g_ppszAudioTypes[i]) + 2;
    }

    iTotalSize++;

    iTotalSize += _tcslen(TEXT("All Files")) + 1;
    iTotalSize += _tcslen(TEXT("*.*")) + 2;

    // Allocate the filter string
    m_szFilter = new TCHAR[iTotalSize + 1];

    // Build the filter string.

    if( !m_szFilter )
    {
        return false;
    }
    StringCchCopy(m_szFilter, iTotalSize + 1, TEXT("All Media Files"));
    iStrLen = _tcslen(m_szFilter);
    iStrLen++;

    for (i = 0; i < g_cAudioTypes; i++)
    {
        _stprintf(m_szFilter + iStrLen, TEXT("*%s;"), g_ppszAudioTypes[i]);
        iStrLen += _tcslen(g_ppszAudioTypes[i]) + 2;
    }

    for (i = 0; i < g_cVideoTypes; i++)
    {
        _stprintf(m_szFilter + iStrLen, TEXT("*%s;"), g_ppszVideoTypes[i]);
        iStrLen += _tcslen(g_ppszVideoTypes[i]) + 2;
    }

    for (i = 0; i < g_cPlaylistTypes; i++)
    {
        _stprintf(m_szFilter + iStrLen, TEXT("*%s;"), g_ppszPlaylistTypes[i]);
        iStrLen += _tcslen(g_ppszPlaylistTypes[i]) + 2;
    }

    iStrLen++;

    StringCchCopy(m_szFilter + iStrLen, (iTotalSize + 1 - iStrLen), TEXT("Video Files"));
    iStrLen += _tcslen(TEXT("Video Files"));
    iStrLen++;

    for (i = 0; i < g_cVideoTypes; i++)
    {
        _stprintf(m_szFilter + iStrLen, TEXT("*%s;"), g_ppszVideoTypes[i]);
        iStrLen += _tcslen(g_ppszVideoTypes[i]) + 2;
    }

    iStrLen++;

    StringCchCopy(m_szFilter + iStrLen, (iTotalSize + 1 - iStrLen), TEXT("Playlists"));
    iStrLen += _tcslen(TEXT("Playlists"));
    iStrLen++;

    for (i = 0; i < g_cPlaylistTypes; i++)
    {
        _stprintf(m_szFilter + iStrLen, TEXT("*%s;"), g_ppszPlaylistTypes[i]);
        iStrLen += _tcslen(g_ppszPlaylistTypes[i]) + 2;
    }

    iStrLen++;

    StringCchCopy(m_szFilter + iStrLen, (iTotalSize + 1 - iStrLen), TEXT("Audio Files"));
    iStrLen += _tcslen(TEXT("Audio Files"));
    iStrLen++;

    for (i = 0; i < g_cAudioTypes; i++)
    {
        _stprintf(m_szFilter + iStrLen, TEXT("*%s;"), g_ppszAudioTypes[i]);
        iStrLen += _tcslen(g_ppszAudioTypes[i]) + 2;
    }

    iStrLen++;

    StringCchCopy(m_szFilter + iStrLen, (iTotalSize + 1 - iStrLen), TEXT("All Files"));
    iStrLen += _tcslen(TEXT("All Files"));
    iStrLen++;

    StringCchCopy(m_szFilter + iStrLen, (iTotalSize + 1 - iStrLen), TEXT("*.*"));
    iStrLen += _tcslen(TEXT("*.*"));
    iStrLen++;
    m_szFilter[iStrLen] = TEXT('\0');

#ifdef UNDER_CE
	/*
    // Create the command bar
    m_hWndCB = ::CommandBar_Create(m_hInstance, m_hWnd, 1);
    if (NULL != m_hWndCB)
    {
        HKEY hkResult = NULL;
        bool bHasHttp = false;

        // Add a menu bar
        ::CommandBar_InsertMenubar(m_hWndCB, m_hInstance, IDR_MENUBAR, 0);

        // Check to see if there is as way to open an http stream, and assume
        // it will render the media homepage.
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, L"http\\Shell\\Open\\Command", 0, 0, &hkResult))
        {
            bHasHttp = true;

            RegCloseKey(hkResult);
        }

        if (bHasHttp && NULL != g_szHomePage)
        {
            ::CommandBar_AddBitmap(m_hWndCB, m_hInstance, IDB_GOWEB, 1, 16, 16);

            TBBUTTON tbb[2];

            tbb[0].iBitmap   = 0;
            tbb[0].idCommand = ID_DEAD_SPACE;
            tbb[0].fsState   = 0;
            tbb[0].fsStyle   = TBSTYLE_SEP;
            tbb[0].dwData    = 0;
            tbb[0].iString   = 0;

            tbb[1].iBitmap   = 0;
            tbb[1].idCommand = ID_GOWEB;
            tbb[1].fsState   = TBSTATE_ENABLED;
            tbb[1].fsStyle   = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;
            tbb[1].dwData    = 0;
            tbb[1].iString   = (int)TEXT("Web");

            ::CommandBar_AddButtons(m_hWndCB, 2, tbb);
        }

        if (g_bSmallScreen)
        {
            ::CommandBar_AddAdornments(m_hWndCB, 0, 0);
        }
    }
	*/
#endif // UNDER_CE
    
    // Set up the menus based on the current state (m_eState)
    UpdateMenus();
    
    // Save the window handle
    CMPContainer::SetWindow(m_hWnd);
    
    // Create the WMP control
    if (FAILED(CMPContainer::CreateControl(CLSID_MediaPlayer)))
    {
        return false;
    }
    
    // Attach this object as an event handler
    // Get a connection point container interface
    if (SUCCEEDED(m_pMP->QueryInterface(IID_IConnectionPointContainer, reinterpret_cast<LPVOID*>(&pCPC))))
    {
        // Get a connection point
        if (SUCCEEDED(pCPC->FindConnectionPoint(DIID__MediaPlayerEvents, &pCP)))
        {
            // Register this object as a MediaPlayerEvent handler
            CMPEventSink::Connect(pCP);
            
            if (FAILED(pCP->Advise(static_cast<CMPEventSink*>(this), &m_dwCookie)))
            {
                CMPEventSink::Disconnect();
            }
            
            pCP->Release();
        }
        
        pCPC->Release();
    }

    LoadRegState();
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::Fini()
// Desc: This function is used to release all interfaces acquired by the
//       by the PlayerWindow class, as well as stopping any currently playing
//       media clip and closing all windows.
///////////////////////////////////////////////////////////////////////////////
bool CPlayerWindow::Fini()
{
    int   i;
    bool  bResult = true;

    SaveRegState();

    // If there is a properties dialog, close it
    if (NULL != m_hWndProp)
    {
        DestroyWindow(m_hWndProp);
        m_hWndProp = NULL;
    }
    
    // If there is a statistics dialog, close it
    if (NULL != m_hWndStat)
    {
        DestroyWindow(m_hWndStat);
        m_hWndStat = NULL;
    }
    
    // If there is a command bar, close it
    if (NULL != m_hWndCB)
    {
        DestroyWindow(m_hWndCB);
        m_hWndCB = NULL;
    }
    
    // Hide our window (if we have one)
    if (NULL != m_hWnd)
    {
        Show(SW_HIDE);
    }
    
    // Shutdown the ActiveX container and the EventSink
    CMPEventSink::Disconnect();
    CMPContainer::DestroyControl();
    
    // Deallocate the stored filename if needed
    if (NULL != m_szFilename)
    {
        delete[] m_szFilename;
        m_szFilename = NULL;
    }
    
    for( i = 0; i < MAX_FILEOPEN_HISTORY; i++ )
    {
        if( m_szFilenameHistory[i] )
        {
            delete [] m_szFilenameHistory[i];
            m_szFilenameHistory[i] = NULL;
        }
    }
    
    // Deallocate the stored path if needed
    if (NULL != m_szPath)
    {
        delete[] m_szPath;
        m_szPath = NULL;
    }
    
    // Deallocate the filter string if needed
    if (NULL != m_szFilter)
    {
        delete[] m_szFilter;
        m_szFilter = NULL;
    }
    
    // Make sure to free up memory if it was allocated
    if (NULL != m_stats.szErrorCorrection)
    {
        delete[] m_stats.szErrorCorrection;
        m_stats.szErrorCorrection = NULL;
    }

    if (NULL != m_stats.szFilename)
    {
        delete [] m_stats.szFilename;
        m_stats.szFilename = NULL;
    }

#ifdef CEPLAYER_SKIN
    for (i = 0; i < SKIN_SIZE; i++)
    {
        if (NULL != m_binfo[i].hUp)
            DeleteObject(m_binfo[i].hUp);
        if (NULL != m_binfo[i].hDown)
            DeleteObject(m_binfo[i].hDown);
        if (NULL != m_binfo[i].hDisabled)
            DeleteObject(m_binfo[i].hDisabled);
    }

    if (NULL != m_hbmBuffer)
    {
        DeleteObject(m_hbmBuffer);
    }

    DeleteCriticalSection( &m_csButtonInfoCritSec );
#endif /* CEPLAYER_SKIN */

    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::Show()
// Desc: This function allows the PlayerWindow to show or hide itself.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::Show(int iShow)
{
    // Tell the ActiveX container to tell the WMP control to show itself
    CMPContainer::ShowObject();
    
    // Cause our window (command bar and all) to get repainted
    ::ShowWindow(m_hWnd, iShow);
    ::UpdateWindow(m_hWnd);
    
    return;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::TranslateAccelerator()
// Desc: This function give the MediaPlayer a chance to handle any accelerator
//       keys that may have been pressed.
///////////////////////////////////////////////////////////////////////////////
bool CPlayerWindow::TranslateAccelerator(LPMSG pMsg)
{
    IOleInPlaceActiveObject *pIPAO     = NULL;
    IOleControl             *pIControl = NULL;
    bool                     bResult   = false;

    if (m_eState == BAD
        && NULL != pMsg
        && (    L' '     == (int)pMsg->wParam
             || L'.'     == (int)pMsg->wParam
             || VK_LEFT  == (int)pMsg->wParam
             || VK_RIGHT == (int)pMsg->wParam
             || VK_NEXT  == (int)pMsg->wParam
             || VK_PRIOR == (int)pMsg->wParam
             || L'V'     == (int)pMsg->wParam
             || L'G'     == (int)pMsg->wParam ))
    {
        return bResult;
    }

    // First, give the MediaPlayer a chance to catch this message
    if (NULL != m_pUnk)
    {
        if (SUCCEEDED(m_pUnk->QueryInterface(IID_IOleInPlaceActiveObject, reinterpret_cast<LPVOID*>(&pIPAO))))
        {
            if (SUCCEEDED(pIPAO->TranslateAccelerator(pMsg)))
            {
                // If the MediaPlayer can accept this accelerator key, get the
                // IOleControl interface and give it the message.   Otherwise
                // no action as a result of the key event will occur.
                if (SUCCEEDED(m_pUnk->QueryInterface(IID_IOleControl, reinterpret_cast<LPVOID*>(&pIControl))))
                {
                    if (SUCCEEDED(pIControl->OnMnemonic(pMsg)))
                    {
                        bResult = true;
                    }
                    
                    pIControl->Release();
                }
            }
            
            pIPAO->Release();
        }
    }
    
    return bResult;
}

bool CPlayerWindow::DialogMessage(LPMSG pMsg)
{
    bool bResult = false;

    if (NULL != m_hWndStat && IsDialogMessage(m_hWndStat, pMsg))
    {
        bResult = true;
    }

    if (false == bResult
        && NULL != m_hWndProp && IsDialogMessage(m_hWndProp, pMsg))
    {
        bResult = true;
    }

    return bResult;
}

bool CPlayerWindow::SaveRegState()
{
    RECT  rc;
    LONG  lVolume = 0;
    int   i;
    HKEY  hkResult;
    TCHAR szRegKey[REGKEY_SIZE];
    VARIANT_BOOL fMuted = FALSE;
    DWORD dwTemp    = 0;
    bool  bResult   = false;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                      TEXT("SOFTWARE\\Microsoft\\CEPlayer"),
                                      0, 0, &hkResult))
    {
        if (FALSE != GetWindowRect(m_hWnd, &rc))
        {
            RegSetValueEx(hkResult, TEXT("XPos"), 0, REG_DWORD,
                          (BYTE*)&rc.left, sizeof (DWORD));
            RegSetValueEx(hkResult, TEXT("YPos"), 0, REG_DWORD,
                          (BYTE*)&rc.top, sizeof (DWORD));
        }

        if (NULL != m_szFilename)
        {
            DWORD dwStrLength;

            szRegKey[0] = TEXT('\0');

            for (i = 0; i < MAX_FILEOPEN_HISTORY; i++)
            {
                if (m_szFilenameHistory[i])
                {
                    LoadString(m_hInstance, ID_FILENAME1_REGKEY + i, szRegKey, REGKEY_SIZE);
                    dwStrLength = MAX_URL_LENGTH;
                    RegSetValueEx(hkResult, szRegKey, 0, REG_SZ,
                                  (BYTE*)m_szFilenameHistory[i],
                                  (_tcslen(m_szFilenameHistory[i]) + 1) * sizeof (TCHAR));
                }
            }
        }

        if (NULL != m_pMP)
        {
            m_pMP->get_Volume(&lVolume);
            lVolume = VolumeLogToLin(lVolume);
            m_pMP->get_Mute(&fMuted);
        }

        dwTemp = (DWORD)fMuted;

        RegSetValueEx(hkResult,
                      TEXT("PlayForever"), 0,
                      REG_BINARY, (BYTE*)&m_bPlayForever,
                      sizeof (BOOL));
        RegSetValueEx(hkResult,
                      TEXT("Shuffle"), 0,
                      REG_BINARY, (BYTE*)&m_bShuffle,
                      sizeof (BOOL));
        RegSetValueEx(hkResult,
                      TEXT("ZoomLevel"), 0,
                      REG_DWORD, (BYTE*)&m_dwZoomLevel,
                      sizeof (DWORD));
        RegSetValueEx(hkResult,
                      TEXT("Muted"), 0,
                      REG_DWORD, (BYTE*)&dwTemp,
                      sizeof (DWORD));
        RegSetValueEx(hkResult,
                      TEXT("Volume"), 0,
                      REG_DWORD, (BYTE*)&lVolume,
                      sizeof (DWORD));

        RegCloseKey(hkResult);

        bResult = true;
    }

    return bResult;
}

bool CPlayerWindow::LoadRegState()
{
    HKEY  hkResult;
    DWORD dwDisp,
          dwType,
          dwSize,
          dwKeyType;
    int   i;
    TCHAR szRegKey[REGKEY_SIZE];
    bool  bResult   = false;

    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                        TEXT("SOFTWARE\\Microsoft\\CEPlayer"),
                                        0, NULL, 0, 0, NULL, &hkResult, &dwDisp))
    {
        if (NULL != m_szFilename)
        {
            delete [] m_szFilename;
            m_szFilename = NULL;
        }

        for (i = 0; i < MAX_FILEOPEN_HISTORY; i++)
        {
            m_szFilenameHistory[i] = new TCHAR[MAX_URL_LENGTH];

            szRegKey[0] = TEXT('\0');
            LoadString(m_hInstance, ID_FILENAME1_REGKEY + i, szRegKey, REGKEY_SIZE);

            dwSize = MAX_URL_LENGTH;

            // Don't try to load a value from the registry if we weren't able to allocate memory
            if (NULL == m_szFilenameHistory[i]
                || ERROR_SUCCESS != RegQueryValueEx(hkResult, szRegKey, 0, &dwKeyType,
                                                 (BYTE*)m_szFilenameHistory[i], &dwSize)
                || REG_SZ != dwKeyType)
            {
                delete [] m_szFilenameHistory[i];
                m_szFilenameHistory[i] = NULL;
            }
        }

        if (m_szFilenameHistory[0])
        {
            size_t BufLen = _tcslen(m_szFilenameHistory[0]) + 1;
            if (m_szFilename = new TCHAR[BufLen])
            {
                StringCchCopy(m_szFilename, BufLen, m_szFilenameHistory[0]);
            }
        }

        if (REG_OPENED_EXISTING_KEY == dwDisp)
        {
            LONG         lVolume = 0;
            DWORD        dwTemp  = 0;
            VARIANT_BOOL fMuted  = FALSE;

            dwSize = sizeof (m_bPlayForever);
            RegQueryValueEx(hkResult,
                            TEXT("PlayForever"), NULL,
                            &dwType, (BYTE*)&m_bPlayForever,
                            &dwSize);
            dwSize = sizeof (m_bShuffle);
            RegQueryValueEx(hkResult,
                            TEXT("Shuffle"), NULL,
                            &dwType, (BYTE*)&m_bShuffle,
                            &dwSize);
            dwSize = sizeof (m_dwZoomLevel);
            RegQueryValueEx(hkResult,
                            TEXT("ZoomLevel"), NULL,
                            &dwType, (BYTE*)&m_dwZoomLevel,
                            &dwSize);
            dwSize = sizeof (dwTemp);
            RegQueryValueEx(hkResult,
                            TEXT("Muted"), NULL,
                            &dwType, (BYTE*)&dwTemp,
                            &dwSize);
            if( m_pMP )
            {
                fMuted = (VARIANT_BOOL)dwTemp;
                m_pMP->put_Mute(fMuted);
            }

            dwSize = sizeof (lVolume);
            if (ERROR_SUCCESS == RegQueryValueEx(hkResult,
                                                 TEXT("Volume"), NULL,
                                                 &dwType, (BYTE*)&lVolume,
                                                 &dwSize)
                && sizeof (LONG) == dwSize
                && NULL != m_pMP)
            {
                m_pMP->put_Volume(VolumeLinToLog((short)lVolume));
            }
            else if (NULL != m_pMP)
            {
                // If a value couldn't be found in the registry, use a
                // reasonable default value.
                m_pMP->put_Volume(AX_HALF_VOLUME);
            }
        }

        RegCloseKey(hkResult);

        bResult = true;

        ApplyPlayCount();
        ApplyZoomLevel();
    }

    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnCommand()
// Desc: This function is used then a WM_COMMAND message is received by the
//       main WinProc.  It will figure out which function or action to take
//       based on the type of command received.
///////////////////////////////////////////////////////////////////////////////
bool CPlayerWindow::OnCommand(WORD wID, LPARAM lParam)
{
    bool         bResult = false;

    if (ID_FAVORITE_MIN <= wID && wID <= ID_FAVORITE_MAX)
    {
        return OnFavorite((int)wID - ID_FAVORITE_MIN);
    }

    switch (wID)
    {
    case ID_FILE_OPEN_URL:
        bResult = OnOpenURL();
        break;
        
    case ID_FILE_CLOSE:
        bResult = OnClose();
        break;

    case ID_FILE_PLAYLISTS:
        bResult = OnPlaylist();
        break;

    case ID_ADD_TO_FAVORITES:
        bResult = OnAddToFavorites();
        break;

    case ID_ORGANIZE_FAVORITES:
        bResult = OnOrganizeFavorites();
        break;
        
    case ID_FILE_EXIT:
        ::PostMessage(m_hWnd, WM_CLOSE, NULL, NULL);
        
        bResult = true;
        break;
    
#ifndef CEPLAYER_SKIN
    case ID_VIEW_STANDARD:
        m_dwLevelOfControls = 2;
        UpdateMenus();
        UpdateDisplay();
        bResult = true;
        break;
        
    case ID_VIEW_MINIMAL:
        m_dwLevelOfControls = 0;
        UpdateMenus();
        UpdateDisplay();
        bResult = true;
        break;
#endif /* CEPLAYER_SKIN */

    case ID_VIEW_ZOOM_50:
        m_dwZoomLevel = 0;
        ApplyZoomLevel();
        bResult = true;
        break;

    case ID_VIEW_ZOOM_100:
        m_dwZoomLevel = 1;
        ApplyZoomLevel();
        bResult = true;
        break;

    case ID_VIEW_ZOOM_200:
        m_dwZoomLevel = 2;
        ApplyZoomLevel();
        bResult = true;
        break;

    case ID_VIEW_FULLSCREEN:
        bResult = OnFullScreen();
        break;

    case ID_VIEW_STATISTICS:
        bResult = OnStatistics();
        break;
        
    case ID_VIEW_PROPERTIES:
        bResult = OnProperties();
        break;
        
    case ID_VIEW_OPTIONS:
        bResult = OnOptions();
        break;
        
    case ID_PLAYBACK_PLAY:
        bResult = OnPlay();
        break;
        
    case ID_PLAYBACK_PAUSE:
        bResult = OnPause();
        break;
        
    case ID_PLAYBACK_STOP:
        bResult = OnStop();
        break;

    case ID_PLAYBACK_MUTE:
        bResult = OnMute();
        break;
        
    case ID_PLAYBACK_REPEAT:
        bResult = OnRepeat();
        break;

    case ID_PLAYBACK_SHUFFLE:
        bResult = OnShuffle();
        break;

    case ID_PLAY_SONG:
        if (NULL != (CPlaylist*)lParam)
        {
            ((CPlaylist*)lParam)->ResetPlayed();
        }

        bResult = OnPlaySong((CPlaylist*)lParam);
        break;

    case ID_GOWEB:
        bResult = OnGoWeb();
        break;

    case ID_PLAYLIST_NEXT:
        OnStop();
        EndOfStream(0);
        break;
        
    }
    
    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnWindowMessage()
// Desc: This function is called if a message is received by the main WinProc
//       and no other function has handled the message.  Essentially it allows
//       the MediaPlayer control to respond to mouse events.
///////////////////////////////////////////////////////////////////////////////
bool CPlayerWindow::OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lr      = 0;
    bool    bResult = false;

	if (uMsg == WM_KEYUP) {
		if (VK_ESCAPE == wParam) {

			::PostMessage(m_hWnd, WM_CLOSE, NULL, NULL);

			bResult = true;
		}
	}

	if (false == bResult) {

		// Forward any window messages to the control so that the control can
		// take appropriate action.

		if (NULL != m_pIPOW)
		{
			if (SUCCEEDED(m_pIPOW->OnWindowMessage(uMsg, wParam, lParam, &lr)))
			{
				bResult = true;
			}
		}

	}
    
    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnPaint()
// Desc: This function is called when a WM_PAINT message is received.  It
//       just hands the message to the MPContainer class.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::OnPaint(HDC hdc, RECT *prc)
{
#ifdef CEPLAYER_SKIN
    RECT         rc;
    LONG         lVolume           = 0;
    VARIANT_BOOL fHasMultipleItems = VARIANT_FALSE;

    GetClientRect(m_hWnd, &rc);

    if (NULL == m_pMP)
    {
        FillRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));

        return;
    }

    m_pMP->get_Volume(&lVolume);
    lVolume = VolumeLogToLin(lVolume);
    m_pMP->get_HasMultipleItems(&fHasMultipleItems);

    if (prc->bottom == rc.bottom)
    {
        prc->bottom -= SKIN_HEIGHT;
    }

    if (NULL == m_hbmBuffer)
    {
        m_hbmBuffer = CreateCompatibleBitmap(hdc, SKIN_WIDTH, SKIN_HEIGHT);
    }
#endif /* CEPLAYER_SKIN */

    CMPContainer::Paint(hdc, prc);

#ifdef CEPLAYER_SKIN
    //
    // draw in the last 40 pixels of the window
    //

    COLORREF cTransColor = RGB(255,   0, 255);
    COLORREF cBkColor    = RGB(100, 125, 192);
    HBRUSH   hbrush;
    HDC      hdcBuf;
    RECT     rcFill;
    int      x,y;

    // Center the controls
    x = rc.left + (rc.right - rc.left - SKIN_WIDTH)/2;
    y = rc.bottom - SKIN_HEIGHT + 2;

    m_iSkinMargin = x - rc.left;

    hdcBuf = CreateCompatibleDC(hdc);

    SelectObject(hdcBuf, m_hbmBuffer);

    //
    // Fill in the back buffer
    //

    rcFill.left   = 0;
    rcFill.right  = SKIN_WIDTH;
    rcFill.top    = 0;
    rcFill.bottom = SKIN_HEIGHT;

    hbrush = CreateSolidBrush(cBkColor);

    FillRect(hdcBuf, &rcFill, hbrush);

    DeleteObject(hbrush);

    for (int i = 0; i < SKIN_SIZE; i++)
    {
        HBITMAP hbitmap = NULL;
        HDC     hdcBmp  = NULL;

        // Only draw either the play button, or the pause button.  Never both
        if (m_bPlayPause && SKIN_PAUSE == i) continue;
        if (!m_bPlayPause && SKIN_PLAY == i) continue;

        EnterCriticalSection( &m_csButtonInfoCritSec );

        if (UP == m_binfo[i].eState)
        {
            hbitmap = m_binfo[i].hUp;
        }
        else if (DOWN == m_binfo[i].eState)
        {
            hbitmap = m_binfo[i].hDown;

            if (SKIN_VOLUME == i || SKIN_SEEK == i)
            {
                hbitmap = m_binfo[i].hUp;
            }
        }
        else if (DISABLED == m_binfo[i].eState)
        {
            if (SKIN_PAUSE == i)
            {
#ifdef UNDER_CE
                ASSERT(0);
#endif /* UNDER_CE */
            }
            else
            {
                hbitmap = m_binfo[i].hDisabled;
            }
        }
        else
        {
            DebugBreak();
        }

        m_binfo[i].rc.left   = x + m_binfo[i].ptPos.x;
        m_binfo[i].rc.right  = m_binfo[i].rc.left + m_binfo[i].ptDim.x;
        m_binfo[i].rc.top    = y + m_binfo[i].ptPos.y;
        m_binfo[i].rc.bottom = m_binfo[i].rc.top + m_binfo[i].ptDim.y;

        hdcBmp = CreateCompatibleDC(hdc);
        SelectObject(hdcBmp, hbitmap);

        BitBlt(hdcBuf,
               m_binfo[i].rc.left - x,
               m_binfo[i].rc.top  - y,
               m_binfo[i].ptDim.x,
               m_binfo[i].ptDim.y,
               hdcBmp,
               0,
               0,
               SRCCOPY);

        LeaveCriticalSection( &m_csButtonInfoCritSec );

        DeleteDC(hdcBmp);
    }

    // Let's add the speed if we're fast forwarding or fast rewarding.
    if( ratesArray[ m_currentRate ] != 1 )
    {
        WCHAR       wzBuffer[10];
        COLORREF    oldTextColor, oldBgColor;
        RECT        rcText = { 0 };
        SIZE        textSize = { 0 };

        StringCchPrintf(wzBuffer, 10, L"x%d", ratesArray[ m_currentRate ] );
        GetTextExtentPoint( hdcBuf, wzBuffer, wcslen( wzBuffer ), &textSize );
            
        rcText.left = m_rcSeekBounds.right + 10;
        rcText.right = rcText.left + textSize.cx;
        rcText.top = m_rcSeekBounds.top;
        rcText.bottom = rcText.top + textSize.cy;

        oldBgColor = SetBkColor( hdcBuf, cBkColor );
        oldTextColor = SetTextColor( hdcBuf, RGB( 0, 0, 0 ));       

        DrawText( hdcBuf, wzBuffer, wcslen( wzBuffer ), &rcText, DT_LEFT );

        SetBkColor( hdcBuf, oldBgColor );
        SetTextColor( hdcBuf, oldTextColor );
    }

    //
    // Draw the Seek Slider Thumb
    //
    if (m_fCanSeek)
    {
        HBITMAP hbitmap = NULL;

        if (UP == m_binfoSeekThumb.eState)
        {
            hbitmap = m_binfoSeekThumb.hUp;
        }
        else
        {
            hbitmap = m_binfoSeekThumb.hDown;
        }

        m_binfoSeekThumb.rc.left   = x + m_binfoSeekThumb.ptPos.x;
        m_binfoSeekThumb.rc.right  = m_binfoSeekThumb.rc.left + m_binfoSeekThumb.ptDim.x;
        m_binfoSeekThumb.rc.top    = y + m_binfoSeekThumb.ptPos.y;
        m_binfoSeekThumb.rc.bottom = m_binfoSeekThumb.rc.top + m_binfoSeekThumb.ptDim.y;

        TransparentImage(hdcBuf,
                         m_binfoSeekThumb.rc.left - x,
                         m_binfoSeekThumb.rc.top  - y,
                         m_binfoSeekThumb.ptDim.x,
                         m_binfoSeekThumb.ptDim.y,
                         hbitmap,
                         0, 0,
                         m_binfoSeekThumb.ptDim.x,
                         m_binfoSeekThumb.ptDim.y,
                         cTransColor);
    }

    //
    // Draw the Volume Slider Thumb
    //
    if (DISABLED != m_binfo[SKIN_VOLUME].eState)
    {
        HBITMAP hbitmap = NULL;

        if (UP == m_binfoVolThumb.eState)
        {
            hbitmap = m_binfoVolThumb.hUp;
        }
        else
        {
            hbitmap = m_binfoVolThumb.hDown;
        }

        if (UP == m_binfoVolThumb.eState)
        {
            if (MIN_VOLUME_RANGE < lVolume && lVolume < MAX_VOLUME_RANGE)
            {
                lVolume = (m_rcVolBounds.right - m_rcVolBounds.left) * lVolume / (MAX_VOLUME_RANGE - MIN_VOLUME_RANGE);
            }
            else if (MAX_VOLUME_RANGE == lVolume)
            {
                lVolume = m_rcVolBounds.right - m_rcVolBounds.left;
            }
            else
            {
                lVolume = 0;
            }

            m_binfoVolThumb.rc.left   = x + m_rcVolBounds.left;
            m_binfoVolThumb.rc.left   += lVolume;

            m_binfoVolThumb.ptPos.x = m_binfoVolThumb.rc.left - x;
        }
        else
        {
            m_binfoVolThumb.rc.left   = x + m_binfoVolThumb.ptPos.x;
        }

        m_binfoVolThumb.rc.right  = m_binfoVolThumb.rc.left + m_binfoVolThumb.ptDim.x;
        m_binfoVolThumb.rc.top    = y + m_binfoVolThumb.ptPos.y;
        m_binfoVolThumb.rc.bottom = m_binfoVolThumb.rc.top + m_binfoVolThumb.ptDim.y;

        TransparentImage(hdcBuf,
                         m_binfoVolThumb.rc.left - x,
                         m_binfoVolThumb.rc.top  - y,
                         m_binfoVolThumb.ptDim.x,
                         m_binfoVolThumb.ptDim.y,
                         hbitmap,
                         0, 0,
                         m_binfoVolThumb.ptDim.x,
                         m_binfoVolThumb.ptDim.y,
                         cTransColor);
    }

    //
    // Fill the background of the real window
    //

    rcFill.left   = 0;
    rcFill.right  = rc.right;
    rcFill.top    = y - 2;
    rcFill.bottom = rc.bottom;

    hbrush = CreateSolidBrush(cBkColor);

    FillRect(hdc, &rcFill, hbrush);

    DeleteObject(hbrush);

    //
    // Blt in the the buffered image
    //

    BitBlt(hdc,
           x,
           y,
           SKIN_WIDTH,
           SKIN_HEIGHT,
           hdcBuf,
           0,
           0,
           SRCCOPY);



    DeleteDC(hdcBuf);
#endif /* CEPLAYER_SKIN */

    return;
}

void CPlayerWindow::ApplyPlayCount( void )
{
    if( m_pMP )
    {
        if( m_bPlayForever )
        {
            m_pMP->put_PlayCount( 0 );
        }
        else
        {
            m_pMP->put_PlayCount( 1 );
        }
    }
}

void CPlayerWindow::ApplyZoomLevel( void )
{
    if( m_pMP )
    {
        m_pMP->put_AutoSize( TRUE );

        switch( m_dwZoomLevel )
        {
            case 0:
                m_pMP->put_DisplaySize( mpHalfSize );
            break;
            case 2:
                m_pMP->put_DisplaySize( mpDoubleSize );
            break;
            case 1:
            default:
                m_pMP->put_DisplaySize( mpDefaultSize );
            break;
        }
    }

    UpdateMenus();
}

void CPlayerWindow::SetMinimumRect( RECT &rect )
{
    DWORD width = rect.right - rect.left;
    DWORD height = rect.bottom - rect.top;

    if( width < m_dwMinimumWidth )
        m_dwMinimumWidth = (width < SKIN_WIDTH ? SKIN_WIDTH : width);
    if( height < m_dwMinimumHeight )
        m_dwMinimumHeight = height + SKIN_HEIGHT;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnSize()
// Desc: This function is called when a WM_SIZE message is received.  It
//       calculates how much area is available for the MediaPlayer control.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::OnSize(int x, int y)
{
    RECT rcPos, rcWnd;
    bool bTooLarge  = false;
    bool bTooSmall  = false;
    bool bOffScreen = false;
    
    if( !m_pMP )
    {
        GetWindowRect(m_hWnd, &rcPos);

        x = (int)m_dwMinimumWidth;
        y = (int)m_dwMinimumHeight;

        if ( (x != rcPos.right - rcPos.left)
            || (y != rcPos.bottom - rcPos.top) )
        {
            MoveWindow(m_hWnd ,rcPos.left, rcPos.top, x, y, true);
        }
        return;
    }

#ifdef UNDER_CE
    // Refresh the command bar
    SendMessage(m_hWndCB, TB_AUTOSIZE, 0, 0);
    CommandBar_AlignAdornments(m_hWndCB);
#endif // UNDER_CE
    
    // Check this new size against the maximum display size of our device
    int iWidth, iHeight;
    RECT rcWorkArea;
    if( SystemParametersInfo( SPI_GETWORKAREA, 0, &rcWorkArea, 0 ) )
    {
        iWidth = rcWorkArea.right - rcWorkArea.left;
        iHeight = rcWorkArea.bottom - rcWorkArea.top;
    }
    else
    {
        HDC hDC = ::GetDC(NULL);
        iWidth = GetDeviceCaps(hDC, HORZRES);
        iHeight = GetDeviceCaps(hDC, VERTRES) - GetSystemMetrics(SM_CYMENU);
        ::ReleaseDC(NULL, hDC);

        rcWorkArea.left   = 0;
        rcWorkArea.top    = 0;
        rcWorkArea.right  = iWidth;
        rcWorkArea.bottom = iHeight;
    }

    if( y > iHeight )
    {
        y         = iHeight;
        bTooLarge = true;
    }
    if( x > iWidth )
    {
        x         = iWidth;
        bTooLarge = true;
    }

    if( y < (LONG)m_dwMinimumHeight
       && (LONG)m_dwMinimumHeight <= iHeight )
    {
        y         = (int)m_dwMinimumHeight;
        bTooSmall = true;
    }
    if( x < (LONG)m_dwMinimumWidth
       && (LONG)m_dwMinimumWidth <= iWidth )
    {
        x         = (int)m_dwMinimumWidth;
        bTooSmall = true;
    }

    // Factor in the height of the command bar when telling the control how
    // much area it has to deal with.
#ifdef UNDER_CE
    rcPos.top    = CommandBar_Height(m_hWndCB);
#else
    rcPos.top    = 0;
#endif // UNDER_CE
    rcPos.left   = 0;
    rcPos.right  = x;
#ifdef CEPLAYER_SKIN
    rcPos.bottom = y - SKIN_HEIGHT;
#else /* !CEPLAYER_SKIN */
    rcPos.bottom = y;
#endif /* CEPLAYER_SKIN */

    // Save the control's window position
    m_rcPos = rcPos;

    // Attempt the call the windowless control, and if it is not available
    // use a windowed control
    if (NULL != m_pIPOW)
    {
        m_pIPOW->SetObjectRects(&m_rcPos, &m_rcPos);
        InvalidateRect(&m_rcPos, FALSE);
    }
    else if (NULL != m_pIPO)
    {
        m_pIPO->SetObjectRects(&m_rcPos, &m_rcPos);
        InvalidateRect(&m_rcPos, FALSE);
    }

    GetWindowRect(m_hWnd, &rcWnd);

    if (rcWnd.right < rcWorkArea.left
        || rcWnd.left > rcWorkArea.right
        || rcWnd.top + GetSystemMetrics(SM_CYMENU) < rcWorkArea.top
        || rcWnd.top > rcWorkArea.bottom)
    {
        bOffScreen = true;
    }

    // If we're too big or small, resize the window
    if ( bTooLarge || bTooSmall || bOffScreen)
    {
        RECT rcClient;

        GetClientRect(m_hWnd, &rcClient);

        iWidth  = (rcWnd.right - rcWnd.left) - (rcClient.right - rcClient.left);
        iHeight = (rcWnd.bottom - rcWnd.top) - (rcClient.bottom - rcClient.top);

        iWidth += rcPos.right;
#ifdef CEPLAYER_SKIN
        iHeight += rcPos.bottom + SKIN_HEIGHT;
#else /* !CEPLAYER_SKIN */
        iHeight += rcPos.bottom;
#endif /* CEPLAYER_SKIN */

        //
        // Check for the maximum size of the player
        //
        if (iWidth > rcWorkArea.right - rcWorkArea.left)
            iWidth = rcWorkArea.right - rcWorkArea.left;
        if (iHeight > rcWorkArea.bottom - rcWorkArea.top)
            iHeight = rcWorkArea.bottom - rcWorkArea.top;

        //
        // It's possible that the player can resize itself to be completely
        // off the screen, so if it looks like it won't be on the screen, move
        // it somewhere else
        //

        if (rcWnd.left + iWidth < rcWorkArea.left)
        {
            rcWnd.left = rcWorkArea.left;
        }
        else if (rcWnd.left > rcWorkArea.right)
        {
            rcWnd.left = rcWorkArea.right - iWidth;
        }

        if (rcWnd.top + GetSystemMetrics(SM_CYMENU) < rcWorkArea.top)
        {
            rcWnd.top = rcWorkArea.top;
        }
        else if (rcWnd.top > rcWorkArea.bottom)
        {
            rcWnd.top = rcWorkArea.bottom - iHeight;
        }

        MoveWindow(m_hWnd,
            rcWnd.left,
            rcWnd.top,
            iWidth,
            iHeight,
            true);
    }

    return;
}

void CPlayerWindow::OnMeasureItem(MEASUREITEMSTRUCT * pmis)
{
    DrawMenuStruct * pMenuStruct;

    pMenuStruct = (DrawMenuStruct *)pmis->itemData;

    // we have fixed height menu items
    pmis->itemHeight = MENU_ITEM_HEIGHT;

    // calculate the required width for this item's text
    HDC hdc = ::GetDC(NULL);

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

    if (NULL != szMenuItem)
    {
        GetTextExtentPoint32(hdc, szMenuItem, _tcslen(szMenuItem), &size);
    }
    else
    {
        size.cx = 0;
    }

    ::ReleaseDC(NULL, hdc);

    pmis->itemWidth = size.cx + MENU_ICON_WIDTH + 3*MENU_ITEM_HORZ_MARGIN;
    pmis->itemWidth = min(pmis->itemWidth, MAX_MENU_ITEM_WIDTH);
}

void CPlayerWindow::OnDrawItem(DRAWITEMSTRUCT * pdis)
{
    HDC              hdc         = pdis->hDC;
    RECT             rcItem      = pdis->rcItem;
    DrawMenuStruct * pMenuStruct = NULL;

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

    // Draw the appropriate item
    int iImage = 0;

    if (pMenuStruct)
    {
        iImage = pMenuStruct->iImage;
    }

    ImageList_DrawEx(
        m_himgLocationList,
        iImage,
        hdc,
        rcItem.left + MENU_ITEM_HORZ_MARGIN,
        rcItem.top + 1,
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

    if (NULL != szMenuItem)
    {
        cchMenuItem = _tcslen(szMenuItem);
    }
    else
    {
        szMenuItem = TEXT("");
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

void CPlayerWindow::OnSizeSmall(int x, int y)
{
    RECT rcPos, rcWorkArea;
    int iWidth, iHeight;

    if (!m_pMP)
        return;

#ifdef UNDER_CE
    SendMessage(m_hWndCB, TB_AUTOSIZE, 0, 0);
    CommandBar_AlignAdornments(m_hWndCB);
#endif // UNDER_CE

    if (SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, 0))
    {
        iWidth  = rcWorkArea.right - rcWorkArea.left;
        iHeight = rcWorkArea.bottom - rcWorkArea.top;
    }
    else
    {
        HDC hDC = ::GetDC(NULL);
        iWidth  = GetDeviceCaps(hDC, HORZRES);
        iHeight = GetDeviceCaps(hDC, VERTRES) - GetSystemMetrics(SM_CYMENU);
        ::ReleaseDC(NULL, hDC);
    }

    m_dwMinimumWidth  = (DWORD)iWidth;
    m_dwMinimumHeight = (DWORD)iHeight;

    if (x != iWidth || y != iHeight)
    {
        MoveWindow(m_hWnd, 0, 0, iWidth, iHeight, true);
    }
    else
    {
        // factor in the command bar
#ifdef UNDER_CE
        rcPos.top    = CommandBar_Height(m_hWndCB);
#else
        rcPos.top    = 0;
#endif // UNDER_CE
        rcPos.left   = 0;
        rcPos.right  = iWidth;
#ifdef CEPLAYER_SKIN
        rcPos.bottom = iHeight - SKIN_HEIGHT;
#else
        rcPos.bottom = iHeight;
#endif // CEPLAYER_SKIN

        // Save the control's window position.
        m_rcPos = rcPos;

        if (NULL != m_pIPOW)
        {
            m_pIPOW->SetObjectRects(&m_rcPos, &m_rcPos);
            InvalidateRect(&m_rcPos, FALSE);
        }
        else if (NULL != m_pIPO)
        {
            m_pIPO->SetObjectRects(&m_rcPos, &m_rcPos);
            InvalidateRect(&m_rcPos, FALSE);
        }
    }

    return;
}

void CPlayerWindow::OnMouseMove(int x, int y)
{
    POINT pt;
    bool  bStateChange = false;

    pt.x = x;
    pt.y = y;

#ifdef CEPLAYER_SKIN
    for (int i = 0; i < SKIN_SIZE; i++)
    {
        EnterCriticalSection( &m_csButtonInfoCritSec );
        if (TRUE != PtInRect(&m_binfo[i].rc, pt)
            && DOWN == m_binfo[i].eState
            && SKIN_SOUND != i)
        {
            m_binfo[i].eState = UP;
            bStateChange      = true;
        }
        LeaveCriticalSection( &m_csButtonInfoCritSec );
    }

    if (DOWN == m_binfoVolThumb.eState
        && m_rcVolBounds.left <= (x - m_iSkinMargin - m_binfoVolThumb.ptDim.x/2)
        && (x - m_iSkinMargin - m_binfoVolThumb.ptDim.x/2) <= m_rcVolBounds.right)
    {
        LONG  lVolume;

        m_binfoVolThumb.ptPos.x = x - m_iSkinMargin - m_binfoVolThumb.ptDim.x/2;
        bStateChange = true;

        if (m_binfoVolThumb.ptPos.x < m_rcVolBounds.left)
        {
            m_binfoVolThumb.ptPos.x = m_rcVolBounds.left;
        }
        else if (m_binfoVolThumb.ptPos.x > m_rcVolBounds.right)
        {
            m_binfoVolThumb.ptPos.x = m_rcVolBounds.right;
        }

        if (m_binfoVolThumb.ptPos.x <= m_rcVolBounds.left)
        {
            lVolume = MIN_VOLUME_RANGE;
        }
        else if (m_binfoVolThumb.ptPos.x >= m_rcVolBounds.right)
        {
            lVolume = MAX_VOLUME_RANGE;
        }
        else
        {
            lVolume = (m_binfoVolThumb.ptPos.x - m_rcVolBounds.left) * (MAX_VOLUME_RANGE - MIN_VOLUME_RANGE) / (m_rcVolBounds.right - m_rcVolBounds.left);
        }

        m_pMP->put_Volume(VolumeLinToLog((short)lVolume));

        //
        // If the volume is being changed, make sure mute isn't set
        //
        VARIANT_BOOL fMute = FALSE;

        m_pMP->get_Mute(&fMute);

        if (fMute)
        {
            m_pMP->put_Mute(FALSE);

            UpdateMenus();
        }
    }

    if (DOWN == m_binfoSeekThumb.eState
        && m_rcSeekBounds.left <= (x - m_iSkinMargin - m_binfoSeekThumb.ptDim.x/2)
        && (x - m_iSkinMargin - m_binfoSeekThumb.ptDim.x/2) <= m_rcSeekBounds.right)
    {
        m_binfoSeekThumb.ptPos.x = x - m_iSkinMargin - m_binfoSeekThumb.ptDim.x/2;
        bStateChange = true;

        if (m_binfoSeekThumb.ptPos.x < m_rcSeekBounds.left)
        {
            m_binfoSeekThumb.ptPos.x = m_rcSeekBounds.left;
        }
        else if (m_binfoSeekThumb.ptPos.x > m_rcSeekBounds.right)
        {
            m_binfoSeekThumb.ptPos.x = m_rcSeekBounds.right;
        }
    }
#endif /* CEPLAYER_SKIN */

    if (bStateChange)
    {
        ::InvalidateRect(m_hWnd, NULL, TRUE);
    }
}

void CPlayerWindow::OnMouseDown(int x, int y)
{
    POINT pt;
    bool  bStateChange = false;

    pt.x = x;
    pt.y = y;

#ifdef CEPLAYER_SKIN
    for (int i = 0; i < SKIN_SIZE; i++)
    {
        EnterCriticalSection( &m_csButtonInfoCritSec );
        if (SKIN_VOLUME == i
                 && UP == m_binfo[SKIN_VOLUME].eState
                 && TRUE == PtInRect(&m_binfo[i].rc, pt))
        {
            if (DISABLED != m_binfoVolThumb.eState)
            {
                m_binfoVolThumb.eState = DOWN;
            }
        }
        else if (SKIN_SEEK == i
                 && UP == m_binfo[SKIN_SEEK].eState
                 && TRUE == PtInRect(&m_binfo[i].rc, pt))
        {
            if (DISABLED != m_binfoSeekThumb.eState)
            {
                m_binfoSeekThumb.eState = DOWN;
            }
        }
        else if (TRUE == PtInRect(&m_binfo[i].rc, pt))
        {
            if (UP == m_binfo[i].eState)
            {
                m_binfo[i].eState = DOWN;
                bStateChange      = true;
            }
        }
        else if (DOWN == m_binfo[i].eState && SKIN_SOUND != i)
        {
            m_binfo[i].eState = UP;
            bStateChange      = true;
        }
        LeaveCriticalSection( &m_csButtonInfoCritSec );
    }

    if (DOWN == m_binfoVolThumb.eState)
    {
        m_binfoVolThumb.ptPos.x = x - m_iSkinMargin - m_binfoVolThumb.ptDim.x/2;
        bStateChange = true;

        if (m_binfoVolThumb.ptPos.x < m_rcVolBounds.left)
        {
            m_binfoVolThumb.ptPos.x = m_rcVolBounds.left;
        }
        else if (m_binfoVolThumb.ptPos.x > m_rcVolBounds.right)
        {
            m_binfoVolThumb.ptPos.x = m_rcVolBounds.right;
        }
    }
    else if (UP == m_binfoVolThumb.eState
             && TRUE == PtInRect(&m_binfoVolThumb.rc, pt))
    {
        m_binfoVolThumb.eState = DOWN;
        bStateChange           = true;
    }

    if (DOWN == m_binfoSeekThumb.eState)
    {
        m_binfoSeekThumb.ptPos.x = x - m_iSkinMargin - m_binfoSeekThumb.ptDim.x/2;
        bStateChange = true;

        if (m_binfoSeekThumb.ptPos.x < m_rcSeekBounds.left)
        {
            m_binfoSeekThumb.ptPos.x = m_rcSeekBounds.left;
        }
        else if (m_binfoSeekThumb.ptPos.x > m_rcSeekBounds.right)
        {
            m_binfoSeekThumb.ptPos.x = m_rcSeekBounds.right;
        }
    }
    else if (UP == m_binfoSeekThumb.eState
             && m_fCanSeek
             && TRUE == PtInRect(&m_binfoSeekThumb.rc, pt))
    {
        m_binfoSeekThumb.eState = DOWN;
        bStateChange            = true;
    }
#endif /* CEPLAYER_SKIN */

    if (bStateChange)
    {
        ::InvalidateRect(m_hWnd, NULL, TRUE);
    }
}

void CPlayerWindow::OnMouseUp(int x, int y)
{
    POINT pt;
    bool  bStateChange = false;

    pt.x = x;
    pt.y = y;

#ifdef CEPLAYER_SKIN
    for (int i = 0; i < SKIN_SIZE; i++)
    {
        EnterCriticalSection( &m_csButtonInfoCritSec );
        if (TRUE == PtInRect(&m_binfo[i].rc, pt))
        {
            if (DOWN == m_binfo[i].eState)
            {
                m_binfo[i].eState = UP;
                bStateChange      = true;

                // Call the appropriate function for a button press
                switch (i)
                {
                    case SKIN_NEXT:
                        if (m_pPlaylist)
                        {
                            m_pPlaylist->CurrentTrackDonePlaying();
                            m_pPlaylist->NextTrack(
                                m_bShuffle ? true : false, true);
                            OnPlaySong(m_pPlaylist);
                        }
                        else
                        {
                            if (PAUSE == m_eState)
                            {
                                SetState(PLAY);
                                UpdateMenus();
                            }

                            if (m_pMP) m_pMP->Next();
                        }
                    break;

                    case SKIN_PREV:
                        if (m_pPlaylist)
                        {
                            m_pPlaylist->CurrentTrackDonePlaying();
                            m_pPlaylist->PrevTrack(
                                m_bShuffle ? true : false, true);
                            OnPlaySong(m_pPlaylist);
                        }
                        else
                        {
                            if (PAUSE == m_eState)
                            {
                                SetState(PLAY);
                                UpdateMenus();
                            }

                            if (m_pMP) m_pMP->Previous();
                        }
                    break;

                    case SKIN_FF:
                        if( m_currentRate < sizeof( ratesArray ) / sizeof( ratesArray[0] ) - 1 )
                        {
                            m_currentRate++;
                            if( m_pMP )
                            {
                                WCHAR wzBuffer[1024];
                                StringCchPrintf(wzBuffer, 1024, L"WMP Debug: Setting playback rate %d", ratesArray[ m_currentRate ] );
                                m_pMP->put_Rate( ratesArray[ m_currentRate ] );

                                // Update the UI
                                ::InvalidateRect(m_hWnd, NULL, TRUE);
                            }
                        }
                        break;

                    case SKIN_FR:
                        if( m_currentRate > 0 )
                        {
                            m_currentRate--;
                            if( m_pMP )
                            {
                                WCHAR wzBuffer[1024];
                                StringCchPrintf(wzBuffer, 1024, L"WMP Debug: Setting playback rate %d", ratesArray[ m_currentRate ] );
                                m_pMP->put_Rate( ratesArray[ m_currentRate ] );
                            }
                        }
                        break;

                    case SKIN_SOUND:
                        OnMute();
                    break;

                    case SKIN_STOP:
                        if (OPENING == m_eState)
                        {
                            if (m_pPlaylist)
                            {
                                m_pPlaylist->CurrentTrackDonePlaying();
                            }

                            if (NULL != m_pMP)
                            {
                                m_pMP->Cancel();
                            }
                        }
                        else
                        {
                            OnStop();
                        }
                    break;

                    case SKIN_PLAY:
                        OnPlay();
                    break;

                    case SKIN_PAUSE:
                        OnPause();
                    break;

                    case SKIN_VOLUME:
                    break;

                    case SKIN_SEEK:
                    break;
                }
            }
            else if (i == SKIN_SOUND)
            {
                OnMute();
                bStateChange = true;
            }
        }
        else if (DOWN == m_binfo[i].eState && SKIN_SOUND != i)
        {
            m_binfo[i].eState = UP;
            bStateChange      = true;
        }
        LeaveCriticalSection( &m_csButtonInfoCritSec );
    }

    if (DOWN == m_binfoVolThumb.eState)
    {
        LONG  lVolume;

        m_binfoVolThumb.eState = UP;

        m_binfoVolThumb.ptPos.x = x - m_iSkinMargin - m_binfoVolThumb.ptDim.x/2;
        bStateChange = true;

        if (m_binfoVolThumb.ptPos.x <= m_rcVolBounds.left)
        {
              lVolume = MIN_VOLUME_RANGE;
        }
        else if (m_binfoVolThumb.ptPos.x >= m_rcVolBounds.right)
        {
              lVolume = MAX_VOLUME_RANGE;
        }
        else
        {
            lVolume = (m_binfoVolThumb.ptPos.x - m_rcVolBounds.left) * (MAX_VOLUME_RANGE - MIN_VOLUME_RANGE) / (m_rcVolBounds.right - m_rcVolBounds.left);
        }

        m_pMP->put_Volume(VolumeLinToLog((short)lVolume));

        //
        // If the volume is being changed, make sure mute isn't set
        //
        VARIANT_BOOL fMute = FALSE;

        m_pMP->get_Mute(&fMute);

        if (fMute)
        {
            m_pMP->put_Mute(FALSE);

            UpdateMenus();
        }
    }

    if (DOWN == m_binfoSeekThumb.eState)
    {
        double dPosition = 0.0;

        m_binfoSeekThumb.eState = UP;

        m_binfoSeekThumb.ptPos.x = x - m_iSkinMargin - m_binfoVolThumb.ptDim.x/2;
        bStateChange = true;

        if (m_binfoSeekThumb.ptPos.x < m_rcSeekBounds.left)
        {
            m_binfoSeekThumb.ptPos.x = m_rcSeekBounds.left;
        }
        else if (m_binfoSeekThumb.ptPos.x > m_rcSeekBounds.right)
        {
            m_binfoSeekThumb.ptPos.x = m_rcSeekBounds.right;
        }

        dPosition = (m_binfoSeekThumb.ptPos.x - m_rcSeekBounds.left)
                    / (double) (m_rcSeekBounds.right - m_rcSeekBounds.left);

        m_pMP->put_CurrentPosition(dPosition*m_dDuration);
    }
#endif /* CEPLAYER_SKIN */

    if (bStateChange)
    {
        ::InvalidateRect(m_hWnd, NULL, TRUE);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnTimer()
// Desc: This function is called when a WM_TIMER message is received.  It
//       allows the Statistics dialog to be updated.
///////////////////////////////////////////////////////////////////////////////
bool CPlayerWindow::OnTimer(UINT uID)
{
    bool bResult = false;

    if (m_uiStatTimer == uID)
    {
        StatTimer();

        bResult = true;
    }
    else if (m_uiTrackerTimer == uID)
    {
        TrackerTimer();

        bResult = true;
    }

    return bResult;
}

void CPlayerWindow::StatTimer()
{
    IBasicVideo *pBV                = NULL;
    IAMAudioRendererStats* pRS      = NULL;
    BSTR         strErrorCorrection = NULL;

    // If the window doesn't exist and we're still getting timer messages
    // kill the timer
    if( m_hWndStat )
    {
        m_stats.dFrameRate     = 0.0;
        m_stats.dActualRate    = 0.0;
        m_stats.lFramesDropped = 0;
        m_stats.lDroppedAudioPackets = 0;
        
        // 10/15/03 - meirab - Adding audio glitch count to the statistics displayed
        if (SUCCEEDED(FindInterfaceOnGraph( IID_IAMAudioRendererStats, (void**)&pRS)) && ( NULL != pRS ))
        {
            DWORD dwParam1, dwParam2;
            if( SUCCEEDED( pRS->GetStatParam( AM_AUDREND_STAT_PARAM_BREAK_COUNT, &dwParam1, &dwParam2 )))
            {
                m_stats.lDroppedAudioPackets += dwParam1;
            }

            if( SUCCEEDED( pRS->GetStatParam( AM_AUDREND_STAT_PARAM_DISCONTINUITIES, &dwParam1, &dwParam2 )))
            {
                m_stats.lDroppedAudioPackets += dwParam1;
            }

            pRS->Release();
        }

        // If an IBasicVideo filter can be found on the filter graph, attempt
        // to figure out the frame rate, etc.  Otherwise set them to 0
        // It is possible for FindBasicVideo to succeed and return a NULL
        // pointer, so pBV must be check for being non-NULL
        if (SUCCEEDED(FindInterfaceOnGraph( IID_IBasicVideo, (void **)&pBV)) && NULL != pBV)
        {
            REFTIME    time;
            IQualProp *pQP;
            
            pBV->get_AvgTimePerFrame(&time);
            if (time > 0)
            {
                m_stats.dFrameRate = (1.0/time);
            }
            
            if (SUCCEEDED(pBV->QueryInterface(IID_IQualProp, reinterpret_cast<LPVOID*>(&pQP))))
            {
                int iTemp;
                
                pQP->get_FramesDroppedInRenderer(&iTemp);
                m_stats.lFramesDropped = iTemp;
                
                pQP->get_AvgFrameRate(&iTemp);
                m_stats.dActualRate = iTemp / 100.0;
                
                pQP->Release();
            }
            
            pBV->Release();
        }
        
        // Make sure there is no memory leaked
        if (NULL != m_stats.szErrorCorrection)
        {
            delete[] m_stats.szErrorCorrection;
            m_stats.szErrorCorrection = NULL;
        }

        // Get all the current properties from the media player
        m_pMP->get_ErrorCorrection(&strErrorCorrection);
        size_t BufLen = _tcslen(strErrorCorrection) + 1;
        if (m_stats.szErrorCorrection = new TCHAR[BufLen])
        {
            StringCchCopy(m_stats.szErrorCorrection, BufLen, strErrorCorrection);
        }
        
        SysFreeString(strErrorCorrection);
        
        m_pMP->get_Bandwidth(&m_stats.lBandwidth);
        m_pMP->get_SourceProtocol(&m_stats.lSourceProtocol);
        m_pMP->get_ReceivedPackets(&m_stats.lReceivedPackets);
        m_pMP->get_RecoveredPackets(&m_stats.lRecoveredPackets);
        m_pMP->get_LostPackets(&m_stats.lLostPackets);

        if (NULL != m_stats.szFilename)
        {
            delete [] m_stats.szFilename;
            m_stats.szFilename = NULL;
        }

        if (NULL != m_szFilename)
        {
            size_t BufLen = _tcslen(m_szFilename) + 1;
            if (m_stats.szFilename = new TCHAR[BufLen])
            {
                StringCchCopy(m_stats.szFilename, BufLen, m_szFilename);
            }
        }

        // Notify the Statistics Dialog that the properties have been updated
        PostMessage(m_hWndStat, SD_UPDATE, reinterpret_cast<WPARAM>(&m_stats), NULL);
    }
    else
    {
        KillTimer(m_hWnd, m_uiStatTimer);
        m_uiStatTimer = 0;
    }
    
    return;
}

void CPlayerWindow::TrackerTimer()
{
    double dCurrent  = 0.0;
    double dPercent  = 0.0;

    MPPlayStateConstants mpPlayState;

    if (NULL != m_pMP)
    {
        // 08/15/03 - meirab
        // if the OCX is not playing, the seek thumb should not be displayed at all (bug 58424)
        //
        m_pMP->get_PlayState(&mpPlayState);
        if ((mpPlayState == mpStopped) || (mpPlayState == mpClosed))
        {
            if (m_fCanSeek == TRUE)
            {
                m_fCanSeek = FALSE;
#ifdef CEPLAYER_SKIN
                m_binfoSeekThumb.ptPos.x = m_rcSeekBounds.left;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_SEEK].rc, FALSE);
#endif
            }
            return;
        }

        m_pMP->get_CurrentPosition(&dCurrent);

        if (0.0 != m_dDuration)
        {
            dPercent = dCurrent / m_dDuration;
        }
        else
        {
            dPercent = 0.0;
        }

#ifdef CEPLAYER_SKIN
        if (UP == m_binfoSeekThumb.eState)
        {
            m_binfoSeekThumb.ptPos.x = (LONG)(dPercent * (m_rcSeekBounds.right - m_rcSeekBounds.left));
            m_binfoSeekThumb.ptPos.x += m_rcSeekBounds.left;

            ::InvalidateRect(m_hWnd, &m_binfo[SKIN_SEEK].rc, FALSE);
        }
#endif /* CEPLAYER_SKIN */
    }

    return;
}

void CPlayerWindow::PlaylistDelete(CPlaylist * pPlaylist)
{
    if (NULL != m_pPlaylist && pPlaylist == m_pPlaylist)
    {
        OnClose();
    }
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnOpen()
// Desc: This function is called when the File->Open menu item is selected.
//       it is responsible for opening the file and storing the filename.
///////////////////////////////////////////////////////////////////////////////
bool CPlayerWindow::OnOpen(LPCTSTR szFilename)
{
    bool bResult = true;
    bool bInHistory = false;
    int  i;

    // if a file is already open, close it so that the m_szFilename variable
    // may have the proper state.  If a file is already open and the user just
    // opened another file, the control closes the first file, which triggers
    // an event and then causes the m_szFilename variable to be deleted, and
    // then the control reads the m_szFilename variable to figure out what
    // to open.  ie. the chuck of memory holding the filename can become
    // corrupted if the previously open file is not closed.
    if (BAD != m_eState)
    {
        OnClose();
        m_fResumeOpen = TRUE;
    }
    else
    {
        m_fResumeOpen = FALSE;
    }

    // While closing, the state might have changed. Let's check again 
    if( BAD == m_eState )
    {
        m_fResumeOpen = FALSE;
    }


    // Store the file name in m_szFilename
    if (NULL != m_szFilename)
    {
        delete [] m_szFilename;
        m_szFilename = NULL;
    }
    
    // Also, update our fileopen history list if its not already in the list
    for( i = 0; i < MAX_FILEOPEN_HISTORY; i++ )
    {
        if( m_szFilenameHistory[i] && 
            (_tcscmp( m_szFilenameHistory[i], szFilename ) == 0 ) )
        {
            bInHistory = true;
            break;
        }
    }
    if( !bInHistory )
    {
        if( m_szFilenameHistory[ MAX_FILEOPEN_HISTORY - 1 ] )
            delete [] m_szFilenameHistory[ MAX_FILEOPEN_HISTORY - 1 ];
        for( i = MAX_FILEOPEN_HISTORY - 1; i > 0; i-- )
        {
            m_szFilenameHistory[i] = m_szFilenameHistory[i-1];
        }
        size_t BufLen = _tcslen(szFilename) + 1;
        if (m_szFilenameHistory[0] = new TCHAR[BufLen])
        {
            StringCchCopy(m_szFilenameHistory[0], BufLen, szFilename);
        }
    }

    // Allocate new space and make a copy of the file name for the containers
    // use.
    size_t BufLen = _tcslen(szFilename) + 1;
    if (m_szFilename = new TCHAR[BufLen])
    {
        StringCchCopy(m_szFilename, BufLen, szFilename);
    }

    if( !m_fResumeOpen )
    {
        bResult =  CompleteOpen();
    }

    return bResult;
}

    
    


bool CPlayerWindow::CompleteOpen()
{
    bool bResult = false;

    if (m_szFilename && IsLocalPlaylist(m_szFilename))
    {
        CPlaylistMgr * pManager = CPlaylistMgr::GetInstance();

        if (pManager)
        {
            pManager->SetCurrentPlaylist(m_szFilename);

            m_pPlaylist = pManager->CurrentPlaylist();

            OnPlaylist();
        }
    }
    else if (m_szFilename)
    {

        // Have the media player open the file
        if (SUCCEEDED(CMPContainer::OpenFile(m_szFilename)))
        {
            bResult = true;
        }
    }
    
    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnOpenURL()
// Desc: This function creates a dialog box to use when opening a URL.  It
//       then allows OnOpen() to handle the actual opening of the file.
///////////////////////////////////////////////////////////////////////////////
bool CPlayerWindow::OnOpenURL()
{
    args_t  args;
    TCHAR  *szURL   = NULL;
    bool    bResult = false;
    int     i;

    // Initialize the argument structure which will be passed to the dialog
    args.szURL      = &szURL;
    args.szFilename = m_szFilename;
    for( i = 0; i < MAX_FILEOPEN_HISTORY; i++ )
        args.szFilenameHistory[i] = m_szFilenameHistory[i];
    
    // DialogBoxParam can return 0 or -1 on an error and
    // The result returned via EndDialog will be ID_DLG_CANCEL if the cancel
    // button was pressed and ID_DLG_OKAY if the okay button was pressed
    if (ID_DLG_OKAY == DialogBoxParam(m_hInstance, MAKEINTRESOURCE(IDD_OPENURL), m_hWnd, OpenURLDialogProc, reinterpret_cast<LPARAM>(&args)))
    {
        if (NULL != szURL)
        {
            bResult = OnOpen(szURL);
            
            delete[] szURL;
        }
    }
    
    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnOptions()
// Desc: This function creates a dialog box for setting player options.
///////////////////////////////////////////////////////////////////////////////
bool CPlayerWindow::OnOptions()
{
    bool    bResult = false;
    
    // DialogBoxParam can return 0 or -1 on an error and
    // The result returned via EndDialog will be ID_DLG_CANCEL if the cancel
    // button was pressed and ID_DLG_OKAY if the okay button was pressed
    if (ID_DLG_OKAY == DialogBoxParam(m_hInstance, MAKEINTRESOURCE(IDD_OPTIONS), m_hWnd, OptionsDialogProc, NULL))
    {
        bResult = true;
    }
    
    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnClose()
// Desc: This function is called with the File->Close menu item is activated.
//       It is responsible for stopping playback, freeing the stored filename,
//       and closing the file.
///////////////////////////////////////////////////////////////////////////////
bool CPlayerWindow::OnClose()
{
    bool bResult = false;
    
    // Stop any playback
    OnStop();

    if (m_pPlaylist)
    {
        OnPlaylistClose();
    }
    
    // Tell the media player to close the file
    if (SUCCEEDED(CMPContainer::Close()))
    {
        bResult = true;
    }
    
    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnPlaylist()
// Desc: Called whenever we open/create a local playlist
///////////////////////////////////////////////////////////////////////////////
bool CPlayerWindow::OnPlaylist()
{
    bool bResult = true;
    long lCount = 1;
    HRESULT hr;

    // We handle local playlist functionality ourselves,
    // whereas we allow the media player control to
    // handle network playlists.
    // One thing we need to do to fully control this
    // local playlist is to disable 'repeat' at the
    // media player control.  So I'm going to
    // unconditionally set IMediaPlayer::PlayCount = 1
    // and logically decouple our local m_bPlayForever
    // variable from PlayCount.  OnPlayListClose() will
    // re-link our variable to PlayCount.

    if (m_pMP)
    {
        hr = m_pMP->get_PlayCount(&lCount);
        ASSERT(SUCCEEDED(hr));
    }

    // should be in sync to begin with...
    ASSERT(m_pPlaylist ? (lCount == 1)
            : (!lCount && m_bPlayForever)
                || (lCount && !m_bPlayForever));

    // ..but sync just in case
    if (!m_pPlaylist)
    {
        m_bPlayForever = (lCount ? FALSE : TRUE);

        // decouple local var from PlayCount
        if (1 != lCount && m_pMP)
        {
            hr = m_pMP->put_PlayCount(1);
            ASSERT(SUCCEEDED(hr));
        }
    }

    CPlaylistDialog playlistDlg(m_hWnd);
    playlistDlg.DoModal();
    m_hWndPlay = NULL;

    // meirab (02/18/04). If the user clicks on "Ok" instead of play, we return to the previous 
    // mode. Hence, we need to check if there's a playlist, and if not, restore the repeat to be
    // handled by the OCX, not by the player. 

    if(( m_pPlaylist == NULL ) && ( m_bPlayForever ) && ( m_pMP ))
    {
        m_pMP->put_PlayCount(0);
    }

    return bResult;
}

void CPlayerWindow::OnPlaylistClose()
{
    // propagate local var back to PlayCount
    if (m_pMP)
    {
        long lCount = (m_bPlayForever ? 0 : 1);
        HRESULT hr;

        hr = m_pMP->put_PlayCount(lCount);
        ASSERT(SUCCEEDED(hr));
    }

    m_pPlaylist = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnAddToFavorites()
// Desc:
///////////////////////////////////////////////////////////////////////////////
bool CPlayerWindow::OnAddToFavorites()
{
    bool        bResult = false;
    CPlaylist * pFavorites;

    CPlaylistMgr * pManager = CPlaylistMgr::GetInstance();

    if (NULL != m_szFilename && BAD != m_eState && pManager)
    {
        pFavorites = pManager->GetFavorites();

        if (pFavorites)
        {
            bResult = pFavorites->InsertTrack(pFavorites->GetCount(), m_szFilename);
        }

        if (bResult)
        {
            pFavorites->Save();
            UpdateMenus();
        }
    }

    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnOrganizeFavorites()
// Desc:
///////////////////////////////////////////////////////////////////////////////
bool CPlayerWindow::OnOrganizeFavorites()
{
    bool bResult = false;

    CPlaylistDialog playlistDlg(m_hWnd, true);
    playlistDlg.DoModal();

    UpdateMenus();

    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnFavorite()
// Desc:
///////////////////////////////////////////////////////////////////////////////
bool CPlayerWindow::OnFavorite(int iIndex)
{
    bool         bResult    = false;
    CPlaylist  * pFavorites = NULL;
    CMediaClip * pClip      = NULL;

    CPlaylistMgr * pManager = CPlaylistMgr::GetInstance();

    if (pManager)
    {
        pFavorites = pManager->GetFavorites();

        if (pFavorites)
        {
            pClip = pFavorites->GetTrack(iIndex);
        }

        if (pClip)
        {
            OnOpen((TCHAR*)pClip->GetPath());
        }
    }

    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnProperties()
// Desc: This function is called when the File->Properties menu item is
//       activated.  It will create a properties dialog.
///////////////////////////////////////////////////////////////////////////////
bool CPlayerWindow::OnProperties()
{
    // Create a new dialog or set it to active if it is already open.
    if (NULL == m_hWndProp)
    {
        m_hWndProp = CreateDialog(m_hInstance, MAKEINTRESOURCE(IDD_PROPERTIES), m_hWnd, PropertyDialogProc);
    }
    else
    {
        SetActiveWindow(m_hWndProp);
    }
    
    // Show the properties window
    PostMessage(m_hWndProp, PD_SHOW, SW_SHOW, NULL);
    
    UpdatePropertyDlg();
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnStatistics()
// Desc: This function is called when the File->Statistics menu item is
//       activated.  It will create the Statistics dialog and start a timer
//       so that the Statistics dialog may be updated.
///////////////////////////////////////////////////////////////////////////////
bool CPlayerWindow::OnStatistics()
{
    // Create an instance of the CStatisticsDlg class
    if (NULL == m_hWndStat)
    {
        m_hWndStat = CreateDialog(m_hInstance, MAKEINTRESOURCE(IDD_STATISTICS), m_hWnd, StatisticsDialogProc);
        
        // Create a timer to update the window
        m_uiStatTimer = SetTimer(m_hWnd, CEPLAYER_TIMER, CEPLAYER_TIMEOUT, NULL);
    }
    else
    {
        SetActiveWindow(m_hWndStat);
    }
    
    // Show the Statistics window
    PostMessage(m_hWndStat, SD_SHOW, SW_SHOW, NULL);
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnFullScreen()
// Desc: This function is called when the View->Fullscreen menu item is
//       activated.  It allows the control to go to fullscreen mode.
///////////////////////////////////////////////////////////////////////////////
bool CPlayerWindow::OnFullScreen()
{
    bool                   bResult       = false;
    MPDisplaySizeConstants mpDisplaySize;

    // Get the current display size
    if (FAILED(m_pMP->get_DisplaySize(&mpDisplaySize)))
    {
        return false;
    }
    
    // If full screen mode is currently enabled, then switch back to the
    // default display size, otherwise enable full screen mode
    if (mpFullScreen == mpDisplaySize)
    {
        ApplyZoomLevel();

        bResult = true;
    }
    else if (SUCCEEDED(m_pMP->put_DisplaySize(mpFullScreen)))
    {
        bResult = true;
    }

    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnPlay()
// Desc: This function is called when the Playback->Play menu item is
//       activated.  It will play the currently open media clip and update
//       the menus so that Pause and Stop become available.
///////////////////////////////////////////////////////////////////////////////
bool CPlayerWindow::OnPlay()
{
    bool bResult = false;

    if (m_pPlaylist && PAUSE != m_eState)
    {
        bResult = OnPlaySong(m_pPlaylist);
    }
    else if (SUCCEEDED(Play()))
    {
        bResult = true;
    }

    UpdateMenus();
    
    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnPause()
// Desc: This function is called when the Playback->Pause menu item is
//       activated.  It will pause the currently playing media clip and
//       deactivate the Pause menu item.
///////////////////////////////////////////////////////////////////////////////
bool CPlayerWindow::OnPause()
{
    bool bResult = false;

    if (SUCCEEDED(Pause()))
    {
        bResult = true;
    }
    
    // Reset the trickmodes counter
    m_currentRate = sizeof( ratesArray ) / sizeof( ratesArray[0] ) / 2;

    UpdateMenus();

    
    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnStop()
// Desc: This function is called when the Playback->Stop menu item is
//       activated.  The Stop menu item is deactivated.
///////////////////////////////////////////////////////////////////////////////
bool CPlayerWindow::OnStop()
{
    bool bResult = false;
    if( m_bStop )
    {
        return false;
    }

    m_bStop = TRUE;
    
    if (SUCCEEDED(CMPContainer::Stop()))
    {
        bResult = true;
    }

    if (m_pPlaylist)
        m_pPlaylist->CurrentTrackDonePlaying();
    
    UpdateMenus();
    
    m_bStop = FALSE;

    // Reset the trickmodes counter
    m_currentRate = sizeof( ratesArray ) / sizeof( ratesArray[0] ) / 2;

    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnMute()
// Desc: This function is called when the Playback->Mute menu item is
//       toggled.  It calls CMPContainer::Mute().
///////////////////////////////////////////////////////////////////////////////
bool CPlayerWindow::OnMute()
{
    bool bResult = false;
    
    if (SUCCEEDED(Mute()))
    {
        bResult = true;
    }
    
    UpdateMenus();
    
    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnRepeat()
// Desc: This function is called when the Playback->Repeat button is toggled.
//       It causes the media player to repeat (or play only once) a media
//       clip.
///////////////////////////////////////////////////////////////////////////////
bool CPlayerWindow::OnRepeat()
{
    long lCount  = 0;
    bool bResult = false;

    // if we have a local playlist, then we're going
    // to handle repeat functionality; we don't want
    // to pass it down to the media player control
    //
    // otherwise (network playlist or single clip),
    // call Repeat() to let the control do the work
    if (m_pPlaylist || SUCCEEDED(CMPContainer::Repeat()))
    {
        bResult = true;
    }

    if (m_pPlaylist)
    {
        m_bPlayForever = !m_bPlayForever;
    }
    else if (NULL != m_pMP && SUCCEEDED(m_pMP->get_PlayCount(&lCount)))
    {
        m_bPlayForever = (lCount ? FALSE : TRUE);
    }
    
    UpdateMenus();
    
    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnShuffle()
// Desc:
///////////////////////////////////////////////////////////////////////////////
bool CPlayerWindow::OnShuffle()
{
    bool bResult = false;

    if (m_bShuffle)
    {
        m_bShuffle = FALSE;
    }
    else
    {
        m_bShuffle = TRUE;
    }

    UpdateMenus();

    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnPlaySong()
// Desc:
///////////////////////////////////////////////////////////////////////////////
bool CPlayerWindow::OnPlaySong(CPlaylist * pPlaylist)
{
    CMediaClip * pClip;
    HRESULT      hr        = E_FAIL;
    bool         bPlaylist = true;
    bool         bStatWnd  = false;
    bool         bPropWnd  = false;
    HANDLE         hFile;

    m_bSwitchingPlaylists = true;

    //
    // Are the Statistics or Properties dialogs open?
    //
    if (NULL != m_hWndStat)
    { 
        bStatWnd = true;
    }

    if (NULL != m_hWndProp)
    { 
        bPropWnd = true;
    }

    //
    // If the playlist is "Local Content" of "Favorites" we need to not
    // add that name to the playlists
    //
    if (pPlaylist
        && (pPlaylist->IsHidden() || pPlaylist->IsTransient()))
    {
        bPlaylist = false;
    }

    if (m_pPlaylist)
    {
        m_pPlaylist = NULL;
    }

    if (BAD != m_eState)
    {
        CMPContainer::Close();
        SetState(BAD);
    }

    if (NULL != m_szFilename)
    {
        delete [] m_szFilename;
        m_szFilename = NULL;
    }

    if (NULL != pPlaylist)
    {
        LPCTSTR pszFilename = NULL;

        pClip  = pPlaylist->PlayCurrentTrack();

        if (bPlaylist)
        {
            pszFilename = pPlaylist->GetPath();

            size_t BufLen = _tcslen(pszFilename) + 1;
            if (m_szFilename = new TCHAR[BufLen])
            {
                StringCchCopy(m_szFilename, BufLen, pszFilename);
            }
        }
        else if (pClip)
        {
            pszFilename = pClip->GetPath();

            size_t BufLen = _tcslen(pszFilename) + 1;
            if (m_szFilename = new TCHAR[BufLen])
            {
                StringCchCopy(m_szFilename, BufLen, pszFilename);
            }
        }

        // We're playing some content. If this is a file, we want to make sure that it's still available (ie CF-Card removed ...)
        if(( wcsstr( (LPTSTR)pClip->GetPath(), L"://" ) == 0 ) || wcsstr( (LPTSTR)pClip->GetPath(), L"file://" ))
        {
            // Let's attempt to open the file. If this fails, we return false.
            hFile = CreateFile( (LPTSTR)pClip->GetPath(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0 );
            if( hFile == INVALID_HANDLE_VALUE )
            {
                if( pPlaylist )
                {
                    pPlaylist->IsTransient( true );
                }
                return FALSE;
            }
            else
            {
                CloseHandle( hFile );
            }
        }


        if (pClip)
        {
            hr = CMPContainer::OpenFile((LPTSTR)pClip->GetPath());
        }

        SendMessage( m_hWndPlay, WM_COMMAND, ID_UPDATE_INFO, 0 );
    }

    
    

    m_bSwitchingPlaylists = false;

    if (SUCCEEDED(hr))
    {
        if (bPlaylist)
        {
            m_pPlaylist = pPlaylist;
        }

        if (bStatWnd)
        {
            PostMessage(m_hWndStat, SD_SHOW, SW_SHOW, NULL);
            StatTimer();
        }

        if (bPropWnd)
        {
            UpdatePropertyDlg();
        }

        return true;
    }
    else
    {
        // Close any dialogs if they were open
        if (NULL != m_hWndProp)
        {
            DestroyWindow(m_hWndProp);
        }

        if (NULL != m_hWndStat)
        {
            DestroyWindow(m_hWndStat);
        }

        return false;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnGoWeb()
// Desc: Opens the media homepage
///////////////////////////////////////////////////////////////////////////////
bool CPlayerWindow::OnGoWeb()
{
    if (NULL != g_szHomePage)
    {
        SHELLEXECUTEINFO sei;

        memset(&sei, 0, sizeof (sei));

        sei.cbSize   = sizeof (SHELLEXECUTEINFO);
        sei.fMask    = SEE_MASK_NOCLOSEPROCESS;
        sei.lpVerb   = L"open";
        sei.lpFile   = g_szHomePage;
        sei.nShow    = SW_SHOW;
        sei.hInstApp = g_hInst;

        ShellExecuteEx(&sei);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::PropertyDlgClosed()
// Desc: This function is called when the Property dialog box closes.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::PropertyDlgClosed()
{
    m_hWndProp = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::StatisticsDlgClosed()
// Desc: This function is called when the Statistics dialog box closes.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::StatisticsDlgClosed()
{
    if (NULL != m_uiStatTimer)
    {
        KillTimer(m_hWnd, m_uiStatTimer);
        m_uiStatTimer = NULL;
    }
    
    m_hWndStat = NULL;
}

//////
// Methods overridden from CMPContainer

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnPosRectChange()
// Desc: This function overrides the function from the MPContainer class.
//       It takes into account the command bar when calculating the area
//       available to the MediaPlayer control.
///////////////////////////////////////////////////////////////////////////////
HRESULT CPlayerWindow::OnPosRectChange(LPCRECT lprcPosRect)
{
    RECT    rcClient,
        rcPos;
    long    lNewClientWidth,
        lNewClientHeight,
        lCommandBarHeight = 0,
        lSkinHeight       = 0;
    HRESULT hr = E_FAIL;
    
    GetClientRect(m_hWnd, &rcClient);
    GetWindowRect(m_hWnd, &rcPos);
    
#ifdef UNDER_CE
    // Get the height of the command bar height
    if (NULL != m_hWndCB)
    {
        lCommandBarHeight = CommandBar_Height(m_hWndCB);
    }
#endif // UNDER_CE

#ifdef CEPLAYER_SKIN
    lSkinHeight = SKIN_HEIGHT;
#endif /* CEPLAYER_SKIN */
    
    // See how much to change the size of the window
    lNewClientWidth  = lprcPosRect->right - lprcPosRect->left;
    lNewClientHeight = lprcPosRect->bottom - lprcPosRect->top + lCommandBarHeight + lSkinHeight;
    
    rcPos.bottom += (lNewClientHeight - rcClient.bottom);
    rcPos.right  += (lNewClientWidth  - rcClient.right);
    
    MoveWindow(m_hWnd,
        rcPos.left,
        rcPos.top,
        rcPos.right - rcPos.left,
        rcPos.bottom - rcPos.top, TRUE);
    
    // Store the new value for the window size
    GetClientRect(m_hWnd, &m_rcPos);
    
    // Make an adjustment for the command bar
    m_rcPos.top += lCommandBarHeight;

    // Make an adjustment for the skin controls
    m_rcPos.bottom -= lSkinHeight;

    // Have these settings take effect
    if (NULL != m_pIPOW)
    {
        hr = m_pIPOW->SetObjectRects(&m_rcPos, &m_rcPos);
    }
    else if (NULL != m_pIPO)
    {
        hr = m_pIPO->SetObjectRects(&m_rcPos, &m_rcPos);
    }
    
    return hr;
}

//////
// pure virtual methods left over from CMPEventSink

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::DVDNotify()
// Desc: Not implemented.  This method is not supported on CE 3.0 and is not
//       available through the Dispatch method in MPEventSink.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::DVDNotify(long, long, long)
{
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::EndOfStream()
// Desc: This function is called when the MediaPlayer encounters the end of
//       a media stream.  It adjusts the state of the PlayerWindow class
//       appropriately, and returns to normal display size if needed.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::EndOfStream(long lValue)
{
    //
    // If the media player control is set to repeat, the EndOfStream event
    // occurs AFTER the PlayStateChange(PLAY) event is fired.  So if
    // SetState(STOP) is unconditionally set here, it is impossible to
    // pause/stop a clip that is being repeated.
    //
    if (!(PLAY == m_eState && CMPContainer::IsRepeating()))
    {
        MPDisplaySizeConstants mpDisplaySize;
        SetState(STOP);

        // If we're in full size mode, we need to return in normal size mode
        if( SUCCEEDED( m_pMP->get_DisplaySize( &mpDisplaySize )) && ( mpFullScreen == mpDisplaySize ))
        {
            ApplyZoomLevel();
        }    

#ifdef CEPLAYER_SKIN // [BS]
		m_binfoSeekThumb.ptPos.x = m_rcSeekBounds.left;
        ::InvalidateRect(m_hWnd, &m_binfo[SKIN_SEEK].rc, FALSE);
#endif
    }
    
    if (m_pPlaylist)
    {
        m_pPlaylist->CurrentTrackDonePlaying();
        BOOL fNext = false;
        while( !fNext && m_pPlaylist->NextTrack( m_bShuffle ? true : false, m_bPlayForever ? true : false) )
        {
            fNext = OnPlaySong(m_pPlaylist);
        }
        if( !fNext )
        {
            m_pPlaylist->ResetPlayed();
        }

    }

    // Let's reset the playback rate
    if( m_currentRate != sizeof( ratesArray ) / sizeof( ratesArray[0] ) / 2 )
    {
        m_currentRate = sizeof( ratesArray ) / sizeof( ratesArray[0] ) / 2;
        ::InvalidateRect( m_hWnd, NULL, TRUE );
    }

    

    UpdateMenus();
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::KeyDown()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::KeyDown(short, short)
{
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::KeyUp()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::KeyUp(short, short)
{
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::KeyPress()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::KeyPress(short)
{
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::MouseMove()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::MouseMove(short, short, OLE_XPOS_PIXELS, OLE_YPOS_PIXELS)
{
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::MouseDown()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::MouseDown(short, short, OLE_XPOS_PIXELS, OLE_YPOS_PIXELS)
{
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::MouseUp()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::MouseUp(short, short, OLE_XPOS_PIXELS, OLE_YPOS_PIXELS)
{
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::Click()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::Click(short, short, OLE_XPOS_PIXELS, OLE_YPOS_PIXELS)
{
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::DblClick()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::DblClick(short, short, OLE_XPOS_PIXELS, OLE_YPOS_PIXELS)
{
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OpenStateChange()
// Desc: This function is called when the MediaPlayer's open state changes.
//       It adjusts the PlayerWindow class's state appropriately.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::OpenStateChange(long lOldState, long lNewState)
{
    MPPlayStateConstants mpPlayState = mpClosed;

    // This is where the state of playback gets changed from opening or closing
    // a file
    
    switch (lNewState)
    {
    case nsClosed:
        SetState(BAD);
        
        // In order for the properties dialog to display the right filename,
        // it is necessary to make sure it is deleted before calling
        // UpdatePropertyDlg().
        //      if (NULL != m_szFilename)
        //      {
        //         delete[] m_szFilename;
        //         m_szFilename = NULL;
        //      }
        
        UpdatePropertyDlg();
        break;
        
    case nsLoadingASX:
        break;
        
    case nsLoadingNSC:
        break;
        
    case nsLocating:
        break;
        
    case nsConnecting:
        break;
        
    case nsOpening:
        break;
        
    case nsOpen:
        if (BAD == m_eState || OPENING == m_eState) SetState(STOP);

        UpdatePropertyDlg();
        break;

    default:
        break;
    }

#if 0
    if (SUCCEEDED(m_pMP->get_PlayState(&mpPlayState)))
    {
        PlayStateChange(mpClosed, mpPlayState);
    }
#endif

    UpdateMenus();
    
    return;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::PlayStateChange()
// Desc: This function is called when the MediaPlayer's play state changes.
//       It will adjust the state of the PlayerWindow class appropriately.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::PlayStateChange(long lOldState, long lNewState)
{
    // This is where the state of playback gets changed from using any of the
    // playback buttons or menu items.
    
    switch (lNewState)
    {
    case mpStopped:
        SetState(STOP);
		::PostMessage(m_hWnd, WM_CLOSE, NULL, NULL);
        break;
        
    case mpPaused:
        SetState(PAUSE);
        break;
        
    case mpPlaying:
        SetState(PLAY);
        break;
        
    case mpWaiting:
        break;
        
    case mpScanForward:
        break;
        
    case mpScanReverse:
        break;
        
    case mpClosed:
        SetState(BAD);
        break;

    default:
        break;
    }

    UpdateMenus();
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::ScriptCommand()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::ScriptCommand(BSTR, BSTR)
{
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::Buffering()
// Desc: This function is called when the MediaPlayer is buffering the
//       media clip it is about to play.  It ensures that the current state
//       is set to PLAY.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::Buffering(VARIANT_BOOL)
{
    // If buffering is occuring, make sure the state is set to at least OPENING
    if (BAD == m_eState) SetState(OPENING);
    
    UpdateMenus();
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::Warning()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::Warning(long lWarningType, long lParam, BSTR strDescription)
{
    LPCTSTR szMessage;
    
    // If a warning occurs, display a dialog box with the message
    // retrieved from the control
    
    if( strDescription )
    {
        szMessage = strDescription;
    }
    else
    {
        szMessage = TEXT("Unknown warning");
    }
    
    ::MessageBox( m_hWnd, szMessage, TEXT("Warning..."), MB_OK | MB_ICONINFORMATION | MB_APPLMODAL );
    
    if( strDescription )
    {
        SysFreeString(strDescription);
    }
    
    return;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::Error()
// Desc: This function is called when the MediaPlayer encounters an error.
//       A dialog is opened displaying the error.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::Error(void)
{
    BSTR     strError = NULL;
    HRESULT  hr;
    TCHAR   *szMessage = NULL;
    long     dwError = 0;
    
    // If an error occurs, display a dialog box with the error message
    // retrieved from the control
    
    hr = m_pMP->get_ErrorDescription(&strError);
    
    if( FAILED(hr) || !strError || !wcslen(strError) )
    {
        strError = SysAllocString( L"Unknown error" );
    }

    m_pMP->get_ErrorCode(&dwError);
    if( dwError )
    {
    DWORD dwSize = 1024;
        if( szMessage = new TCHAR [dwSize] )
        {
            StringCchPrintf( szMessage, dwSize - 1, L"%s (0x%x)", strError, dwError );
        }
    }
    else
    {
        szMessage = strError;
    }
    
    if (szMessage)
    {
        ::MessageBox( m_hWnd, szMessage, NULL, MB_OK | MB_ICONERROR | MB_APPLMODAL ); // NULL title gives "Error"
    }
    
    if (SUCCEEDED(hr))
    {
        SysFreeString(strError);
    }

    if( szMessage )
        delete [] szMessage;
    
    return;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::MarkerHit()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::MarkerHit(long)
{
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::NewStream()
// Desc: Update the property dialog (so that it may receive the new filename).
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::NewStream(void)
{
    // This function can be called when switching files in an ASX without any
    // calls to PlayStateChange or OpenStateChange.  So if we're playing, get
    // the new streams state.

    if (PLAY == m_eState) SetState(PLAY);

    UpdatePropertyDlg();
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::Disconnect()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::Disconnect(long)
{
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::PositionChange()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::PositionChange(double, double)
{
    double dCurrent  = 0.0;

    m_pMP->get_CurrentPosition(&dCurrent);

    if (0.0 != m_dDuration)
    {
        dCurrent = dCurrent / m_dDuration;
    }
    else
    {
        dCurrent = 0.0;
    }

#ifdef CEPLAYER_SKIN
    if (UP == m_binfoSeekThumb.eState)
    {
        m_binfoSeekThumb.ptPos.x = (LONG)(dCurrent * (m_rcSeekBounds.right - m_rcSeekBounds.left));
        m_binfoSeekThumb.ptPos.x += m_rcSeekBounds.left;

        ::InvalidateRect(m_hWnd, &m_binfo[SKIN_SEEK].rc, FALSE);
    }
#endif /* CEPLAYER_SKIN */
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::DisplayModeChange()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::DisplayModeChange(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::ReadyStateChange()
// Desc: This function is called when the ReadyState changes on for the
//       MediaPlayer.  It updates the current state if needed.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::ReadyStateChange(ReadyStateConstants eReadyState)
{
    switch (eReadyState)
    {
    case mpReadyStateUninitialized:
        break;
        
    case mpReadyStateLoading:
        SetState(OPENING);
        break;
        
    case mpReadyStateInteractive:
        break;
        
    case mpReadyStateComplete:
        if (OPENING == m_eState)
        {
            SetState(BAD);
        }

        break;

    default:
        break;
    }
    
    UpdateMenus();

    return;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::UpdateMenus()
// Desc: This function updates the enabled/disabled state and checked/unchecked
//       state of the menu items.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::UpdateMenus()
{
    UINT  uCloseEnable,
        uFullScreenEnable,
        uMuteCheck,
        uPauseEnable,
        uPlayEnable,
        uRepeatCheck,
        uShuffleCheck,
        uShuffleEnable,
        uStopEnable,
        uMinimalCheck,
        uStandardCheck,
        uZoom50Check,
        uZoom100Check,
        uZoom200Check;
    HMENU hMenu   = NULL;
    HMENU hCBMenu = NULL;
    VARIANT_BOOL fMute;

    uCloseEnable = MF_GRAYED;

    if( !m_pMP )
    {
        m_dDuration = 0.0;
        m_fCanPause = FALSE;
    }
#ifdef UNDER_CE
    // Some platforms can't handle a NULL window handle, so we have to do the
    // check here.
    if (NULL != m_hWndCB)
    {
        hCBMenu = ::CommandBar_GetMenu(m_hWndCB, 0);
    }

    if (NULL != hCBMenu)
    {
        hMenu = ::GetSubMenu(hCBMenu, 0);
    }
#else
    hMenu = ::GetSubMenu(::GetMenu(m_hWnd), 0);
#endif // UNDER_CE
    
    if (NULL != hMenu)
    {
        CPlaylist * pFavorites = NULL;

        // If the state is bad, then we don't need to close a file
        if (BAD == m_eState)
        {
            uCloseEnable = MF_GRAYED;
        }
        else
        {
            uCloseEnable = MF_ENABLED;
        }
        
        ::EnableMenuItem(hMenu, ID_FILE_CLOSE,      uCloseEnable);

        hMenu = ::GetSubMenu(hMenu, 4);

        int iPos = 3;

        // remove all the items in the menu
        while (0 != ::RemoveMenu(hMenu, iPos, MF_BYPOSITION))
        {
        }

        CPlaylistMgr * pManager = CPlaylistMgr::GetInstance();

        if (pManager)
        {
            // Enable the Favorites stuff
            pFavorites = pManager->GetFavorites();
        }


        if (NULL != pFavorites)
        {
            int              iCount     = pFavorites->GetCount();
            DrawMenuStruct * pMenuStruct;
            MENUITEMINFO     mii = {0};

            for (int i = 0; i < iCount; i++)
            {
                CMediaClip * pClip    = pFavorites->GetTrack(i);
                LPCTSTR      pszTitle = pClip->GetTitle();

                pMenuStruct = new DrawMenuStruct;
                if( !pMenuStruct )
                {
                    return;
                }

                pMenuStruct->iImage = pClip->GetLocation();
                StringCchCopy(pMenuStruct->szText, 50, pszTitle);
                mii.cbSize     = sizeof(MENUITEMINFO);
                mii.fMask      = MIIM_TYPE | MIIM_DATA;
                mii.fType      = MFT_OWNERDRAW;
                mii.dwTypeData = (LPTSTR)pMenuStruct;
                mii.dwItemData = (DWORD)pMenuStruct;

                if (InsertMenu(hMenu, i + 3, MF_BYPOSITION | MF_OWNERDRAW, ID_FAVORITE_MIN + i, pszTitle))
                {
                    SetMenuItemInfo(hMenu, i + 3, TRUE, &mii);
                }
            }
        }
    }
    
#ifdef UNDER_CE
    if (NULL != hCBMenu)
    {
        hMenu = ::GetSubMenu(hCBMenu, 1);
    }
#else
    hMenu = ::GetSubMenu(::GetMenu(m_hWnd), 1);
#endif // UNDER_CE

    if (NULL != hMenu)
    {
        if( m_dwLevelOfControls == 0 )
        {
            uMinimalCheck = MF_CHECKED;
            uStandardCheck = MF_UNCHECKED;
        }
        else
        {
            uMinimalCheck = MF_UNCHECKED;
            uStandardCheck = MF_CHECKED;
        }

        uZoom50Check = MF_UNCHECKED;
        uZoom100Check = MF_UNCHECKED;
        uZoom200Check = MF_UNCHECKED;

        switch( m_dwZoomLevel )
        {
        case 0: // 50%
            uZoom50Check = MF_CHECKED;
            break;
        case 2: // 200%
            uZoom200Check = MF_CHECKED;
            break;
        case 1: // 100%
        default:
            uZoom100Check = MF_CHECKED;
            break;
        }

        uFullScreenEnable = MF_GRAYED;
        
        // This assumes that no video window will have a zero width or height.
        // If the hight and width of a video window is non-zero, assume it
        // may be possible for full screen to be used.
        if (NULL != m_pMP)
        {
            long lWidth, lHeight;
            
            if (SUCCEEDED(m_pMP->get_ImageSourceWidth(&lWidth))
                && SUCCEEDED(m_pMP->get_ImageSourceHeight(&lHeight))
                && (0 != lWidth || 0 != lHeight))
            {
                uFullScreenEnable = MF_ENABLED;
            }
        }
        
        ::CheckMenuItem(hMenu, ID_VIEW_MINIMAL,       uMinimalCheck);
        ::CheckMenuItem(hMenu, ID_VIEW_STANDARD,      uStandardCheck);
        ::CheckMenuItem(hMenu, ID_VIEW_ZOOM_50,       uZoom50Check);
        ::CheckMenuItem(hMenu, ID_VIEW_ZOOM_100,      uZoom100Check);
        ::CheckMenuItem(hMenu, ID_VIEW_ZOOM_200,      uZoom200Check);
        ::EnableMenuItem(hMenu, ID_VIEW_FULLSCREEN,   uFullScreenEnable);
        // Enable the Properties and Statistics dialogs based on whether or
        // not Close is enabled.
        ::EnableMenuItem(hMenu, ID_VIEW_PROPERTIES, uCloseEnable);
        ::EnableMenuItem(hMenu, ID_VIEW_STATISTICS, uCloseEnable);
    }
    
#ifdef UNDER_CE
    if (NULL != hCBMenu)
    {
        hMenu = ::GetSubMenu(hCBMenu, 2);
    }
#else
    hMenu = ::GetSubMenu(::GetMenu(m_hWnd), 2);
#endif // UNDER_CE
    
    if (NULL != hMenu)
    {
        if (STOP == m_eState)
        {
            uPlayEnable  = MF_ENABLED;
            uStopEnable  = MF_GRAYED;
            uPauseEnable = MF_GRAYED;
        }
        else if (PLAY == m_eState)
        {
            uPlayEnable  = MF_GRAYED;
            uStopEnable  = MF_ENABLED;
            uPauseEnable = MF_ENABLED;
        }
        else if (PAUSE == m_eState)
        {
            uPlayEnable  = MF_ENABLED;
            uStopEnable  = MF_ENABLED;
            uPauseEnable = MF_GRAYED;
        }
        else
        {
            uPlayEnable  = MF_GRAYED;
            uStopEnable  = MF_GRAYED;
            uPauseEnable = MF_GRAYED;
        }
        
        // Set the defaults for mute and repeat state
        uMuteCheck    = MF_UNCHECKED;
        uRepeatCheck  = MF_UNCHECKED;

        // If the current media type does not support seeking, don't enable pause
        if (MF_ENABLED == uPauseEnable)
        {
            uPauseEnable = m_fCanPause ? MF_ENABLED : MF_GRAYED;
        }

        // Update the mute state
        if (NULL != m_pMP
            && SUCCEEDED(m_pMP->get_Mute(&fMute)))
        {
            uMuteCheck = fMute ? MF_CHECKED : MF_UNCHECKED;

#ifdef CEPLAYER_SKIN
            if (fMute)
            {
                if (DOWN != m_binfo[SKIN_SOUND].eState)
                {
                    m_binfo[SKIN_SOUND].eState = DOWN;
                    ::InvalidateRect(m_hWnd, &m_binfo[SKIN_SOUND].rc, FALSE);
                }
            }
            else
            {
                if (UP != m_binfo[SKIN_SOUND].eState)
                {
                    m_binfo[SKIN_SOUND].eState = UP;
                    ::InvalidateRect(m_hWnd, &m_binfo[SKIN_SOUND].rc, FALSE);
                }
            }
#endif /* CEPLAYER_SKIN */
        }

#ifdef CEPLAYER_SKIN
        VARIANT_BOOL fHasMultipleItems = VARIANT_FALSE;
        VARIANT_BOOL fCanScan = VARIANT_FALSE;

        if (NULL != m_pMP)
        {
            m_pMP->get_HasMultipleItems(&fHasMultipleItems);

            if ((VARIANT_TRUE == fHasMultipleItems && m_eState != BAD) || NULL != m_pPlaylist)
            {
                m_binfo[SKIN_NEXT].eState = UP;
                m_binfo[SKIN_PREV].eState = UP;

                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_NEXT].rc, FALSE);
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_PREV].rc, FALSE);
            }
            else
            {
                m_binfo[SKIN_NEXT].eState = DISABLED;
                m_binfo[SKIN_PREV].eState = DISABLED;

                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_NEXT].rc, FALSE);
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_PREV].rc, FALSE);
            }

            m_pMP->get_CanScan( &fCanScan );
            
            if( VARIANT_TRUE == fCanScan  )
            {
                m_binfo[SKIN_FF].eState = UP;
                m_binfo[SKIN_FR].eState = UP;

                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_FF].rc, FALSE);
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_FR].rc, FALSE);
            }
        }
#endif /* CEPLAYER_SKIN */

        // Update the repeat state
        uRepeatCheck = (m_bPlayForever) ? MF_CHECKED : MF_UNCHECKED;

        // Update the shuffle state
        uShuffleCheck  = m_bShuffle ? MF_CHECKED : MF_UNCHECKED;
        uShuffleEnable = m_pPlaylist ? MF_ENABLED : MF_GRAYED;

        ::CheckMenuItem(hMenu, ID_PLAYBACK_MUTE, uMuteCheck);
        ::CheckMenuItem(hMenu, ID_PLAYBACK_REPEAT, uRepeatCheck);
        ::CheckMenuItem(hMenu, ID_PLAYBACK_SHUFFLE, uShuffleCheck);
        ::EnableMenuItem(hMenu, ID_PLAYBACK_PLAY,  uPlayEnable);
        ::EnableMenuItem(hMenu, ID_PLAYBACK_STOP,  uStopEnable);
        ::EnableMenuItem(hMenu, ID_PLAYBACK_PAUSE, uPauseEnable);
        ::EnableMenuItem(hMenu, ID_PLAYBACK_SHUFFLE, uShuffleEnable);
    }
    
    return;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::FindInterfaceOnGraph()
// Desc: This function traverses the FilterGraph looking for the specified
//       interface.
///////////////////////////////////////////////////////////////////////////////
HRESULT CPlayerWindow::FindInterfaceOnGraph( REFIID riid, void **ppInterface )
{
    IFilterGraph *pFG     = NULL;
    IActiveMovie *pAM     = NULL;
    IBaseFilter  *pFilter = NULL;
    IEnumFilters *pEnum   = NULL;
    HRESULT       hResult = S_FALSE;
    
    if (SUCCEEDED(m_pUnk->QueryInterface(IID_IActiveMovie, reinterpret_cast<LPVOID*>(&pAM))))
    {
        if (SUCCEEDED(pAM->get_FilterGraph(reinterpret_cast<IUnknown**>(&pFG))))
        {
            hResult = pFG->EnumFilters(&pEnum);
        }
        
        pAM->Release();
        pAM = NULL;
    }
    
    if (NULL != pEnum)
    {
        hResult = E_NOINTERFACE;
        
        // S_OK must be used instead of the SUCCEEDED macro to prevent an
        // infinite loop from resulting if there is no IBasicVideo interface
        while (NULL == *ppInterface && S_OK == pEnum->Next(1, &pFilter, NULL))
        {
            hResult = pFilter->QueryInterface( riid, reinterpret_cast<LPVOID*>(ppInterface));
            
            pFilter->Release();
        }
        
        pEnum->Release();
    }
    
    if (NULL != pFG)
    {
        pFG->Release();
        pFG = NULL;
    }
    
    return hResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::UpdatePropertyDlg()
// Desc: This function is responsible for sending a message to the property
//       dialog to get the dialog to update its display.
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::UpdatePropertyDlg(LPCTSTR szClip)
{
    TCHAR  * szFilename = NULL;
    LPARAM   lParam     = NULL;

    // If no file is open, don't display a file name
    if (BAD != m_eState)
    {
        szFilename = m_szFilename;
    }

    if (NULL != szClip)
    {
        // Create the dialog if necessary
        if (NULL == m_hWndProp)
        {
            m_hWndProp = CreateDialog(m_hInstance, MAKEINTRESOURCE(IDD_PROPERTIES), m_hWnd, PropertyDialogProc);
        }
        else
        {
            SetActiveWindow(m_hWndProp);
        }

        PostMessage(m_hWndProp, PD_SHOW, SW_SHOW, NULL);

        // Set the filename to the value passed in
        szFilename = (TCHAR*)szClip;
        lParam     = TRUE;
    }

    // Force the properties dialog to update
    if (NULL != m_hWndProp)
    {
        PostMessage(m_hWndProp, PD_UPDATE, (WPARAM)szFilename, lParam);
    }
    
    return;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::SetState(EState eState)
// Desc: This function sets the player state and enables the appropriate
//       buttons and menu options
///////////////////////////////////////////////////////////////////////////////
void CPlayerWindow::SetState(EState eState)
{
    if (NULL != m_pMP)
    {
        LPCWSTR lpwszExtension;
        VARIANT_BOOL fDurationValid;

        m_pMP->get_Duration(&m_dDuration);
        m_pMP->get_IsDurationValid(&fDurationValid);
        if (!fDurationValid) m_dDuration = 0.0;

        m_fCanSeek = (0.0 != m_dDuration && CanSeek());

        // Work around seeking issue with MIDI files
        if( m_fCanSeek && m_szFilename && ( lpwszExtension = wcsrchr( m_szFilename, L'.' ) ) )
        {
            lpwszExtension++;
            if( !wcsicmp( lpwszExtension, L"mid" ) ||
                !wcsicmp( lpwszExtension, L"midi" )||
                !wcsicmp( lpwszExtension, L"rmi" ) )
            {
                m_fCanSeek = FALSE;
                m_dDuration = 0.0; // This will cause m_fCanPause to be FALSE below...
            }
        }
        m_fCanPause = (BOOL)(0.0 != m_dDuration ? TRUE : FALSE);
    }
    else
    {
        m_dDuration = 0.0;
        m_fCanSeek = FALSE;
        m_fCanPause = FALSE;
    }

    switch (eState)
    {
        case STOP:
            m_eState = STOP;

            if (NULL != m_hWnd
                && NULL != m_uiTrackerTimer)
            {
                KillTimer(m_hWnd, m_uiTrackerTimer);
                m_uiTrackerTimer = NULL;
            }

            if(NULL != m_hWndStat
               && NULL != m_uiStatTimer)
            {
                KillTimer(m_hWnd, m_uiStatTimer);
                m_uiStatTimer = NULL;
            }
        
#ifdef CEPLAYER_SKIN
            m_bPlayPause = true;   // enable the play button

            EnterCriticalSection( &m_csButtonInfoCritSec );
            if (DISABLED != m_binfo[SKIN_STOP].eState)
            {
                m_binfo[SKIN_STOP].eState = DISABLED;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_STOP].rc, FALSE);
            }

            if (UP != m_binfo[SKIN_PLAY].eState)
            {
                m_binfo[SKIN_PLAY].eState = UP;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_PLAY].rc, FALSE);
            }

            if (DISABLED != m_binfo[SKIN_PAUSE].eState)
            {
                m_binfo[SKIN_PAUSE].eState = DISABLED;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_PAUSE].rc, FALSE);
            }

            if (TRUE == m_fCanSeek
                && UP != m_binfo[SKIN_SEEK].eState)
            {
                m_binfo[SKIN_SEEK].eState = UP;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_SEEK].rc, FALSE);
            }

            if (FALSE == m_fCanSeek
                && DISABLED != m_binfo[SKIN_SEEK].eState)
            {
                m_binfo[SKIN_SEEK].eState = DISABLED;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_SEEK].rc, FALSE);
            }
            LeaveCriticalSection( &m_csButtonInfoCritSec );
#endif /* CEPLAYER_SKIN */
        break;

        case PLAY:
            m_eState     = PLAY;

            if (NULL != m_hWnd
                && NULL == m_uiTrackerTimer)
            {
                m_uiTrackerTimer = SetTimer(m_hWnd, TRACKER_ID, TRACKER_TIMEOUT, NULL);
            }

            if(NULL != m_hWndStat
               && NULL == m_uiStatTimer)
            {
                m_uiStatTimer = SetTimer(m_hWnd, CEPLAYER_TIMER, CEPLAYER_TIMEOUT, NULL);
            }

#ifdef CEPLAYER_SKIN
            if (TRUE == m_fCanPause)
            {
                m_bPlayPause = false;  // enable the pause button
            }
            else
            {
                m_bPlayPause = true;
            }

            EnterCriticalSection( &m_csButtonInfoCritSec );
            if (UP != m_binfo[SKIN_STOP].eState)
            {
                m_binfo[SKIN_STOP].eState = UP;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_STOP].rc, FALSE);
            }

            if (DISABLED != m_binfo[SKIN_PLAY].eState)
            {
                m_binfo[SKIN_PLAY].eState = DISABLED;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_PLAY].rc, FALSE);
            }

            if (TRUE == m_fCanPause
                && UP != m_binfo[SKIN_PAUSE].eState)
            {
                m_binfo[SKIN_PAUSE].eState = UP;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_PAUSE].rc, FALSE);
            }

            if (FALSE == m_fCanPause
                && DISABLED != m_binfo[SKIN_PAUSE].eState)
            {
                m_binfo[SKIN_PAUSE].eState = DISABLED;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_PAUSE].rc, FALSE);
            }

            if (TRUE == m_fCanSeek
                && UP != m_binfo[SKIN_SEEK].eState)
            {
                m_binfo[SKIN_SEEK].eState = UP;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_SEEK].rc, FALSE);
            }

            if (FALSE == m_fCanSeek
                && DISABLED != m_binfo[SKIN_SEEK].eState)
            {
                m_binfo[SKIN_SEEK].eState = DISABLED;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_SEEK].rc, FALSE);
            }
            LeaveCriticalSection( &m_csButtonInfoCritSec );
#endif /* CEPLAYER_SKIN */
        break;

        case PAUSE:
            m_eState     = PAUSE;

            if (NULL != m_hWnd
                && NULL != m_uiTrackerTimer)
            {
                KillTimer(m_hWnd, m_uiTrackerTimer);
                m_uiTrackerTimer = NULL;
            }

            if(NULL != m_hWndStat
               && NULL != m_uiStatTimer)
            {
                KillTimer(m_hWnd, m_uiStatTimer);
                m_uiStatTimer = NULL;
            }

#ifdef CEPLAYER_SKIN
            m_bPlayPause = true;   // enable the play button

            EnterCriticalSection( &m_csButtonInfoCritSec );
            if (UP != m_binfo[SKIN_STOP].eState)
            {
                m_binfo[SKIN_STOP].eState = UP;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_STOP].rc, FALSE);
            }

            if (UP != m_binfo[SKIN_PLAY].eState)
            {
                m_binfo[SKIN_PLAY].eState = UP;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_PLAY].rc, FALSE);
            }

            if (DISABLED != m_binfo[SKIN_PAUSE].eState)
            {
                m_binfo[SKIN_PAUSE].eState = DISABLED;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_PAUSE].rc, FALSE);
            }

            if (TRUE == m_fCanSeek
                && UP != m_binfo[SKIN_SEEK].eState)
            {
                m_binfo[SKIN_SEEK].eState = UP;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_SEEK].rc, FALSE);
            }

            if (FALSE == m_fCanSeek
                && DISABLED != m_binfo[SKIN_SEEK].eState)
            {
                m_binfo[SKIN_SEEK].eState = DISABLED;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_SEEK].rc, FALSE);
            }
            LeaveCriticalSection( &m_csButtonInfoCritSec );
#endif /* CEPLAYER_SKIN */
            MPDisplaySizeConstants mpDisplaySize;

            // If full screen mode is currently enabled, then switch back to the
            // default display size on the pause.
            if( g_bFullscreenToWindowedOnPause && 
                m_pMP &&
                SUCCEEDED( m_pMP->get_DisplaySize( &mpDisplaySize ) ) &&
                ( mpFullScreen == mpDisplaySize ) )
            {
                ApplyZoomLevel();
            }
        break;

        case OPENING:
            m_eState = OPENING;
            m_currentRate = sizeof( ratesArray ) / sizeof( ratesArray[0] ) / 2;

            if (NULL != m_hWnd
                && NULL != m_uiTrackerTimer)
            {
                KillTimer(m_hWnd, m_uiTrackerTimer);
                m_uiTrackerTimer = NULL;
            }

            if(NULL != m_hWndStat
               && NULL != m_uiStatTimer)
            {
                KillTimer(m_hWnd, m_uiStatTimer);
                m_uiStatTimer = NULL;
            }
        
#ifdef CEPLAYER_SKIN
            m_bPlayPause = true;   // enable the play button

            EnterCriticalSection( &m_csButtonInfoCritSec );
            if (UP != m_binfo[SKIN_STOP].eState)
            {
                m_binfo[SKIN_STOP].eState = UP;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_STOP].rc, FALSE);
            }

            if (DISABLED != m_binfo[SKIN_PLAY].eState)
            {
                m_binfo[SKIN_PLAY].eState = DISABLED;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_PLAY].rc, FALSE);
            }

            if (DISABLED != m_binfo[SKIN_PAUSE].eState)
            {
                m_binfo[SKIN_PAUSE].eState = DISABLED;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_PAUSE].rc, FALSE);
            }

            if (TRUE == m_fCanSeek
                && UP != m_binfo[SKIN_SEEK].eState)
            {
                m_binfo[SKIN_SEEK].eState = UP;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_SEEK].rc, FALSE);
            }

            if (FALSE == m_fCanSeek
                && DISABLED != m_binfo[SKIN_SEEK].eState)
            {
                m_binfo[SKIN_SEEK].eState = DISABLED;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_SEEK].rc, FALSE);
            }
            LeaveCriticalSection( &m_csButtonInfoCritSec );
#endif /* CEPLAYER_SKIN */
        break;

        case BAD:
        default:
            m_eState = BAD;

            if (NULL != m_hWnd
                && NULL != m_uiTrackerTimer
                && !m_bSwitchingPlaylists)
            {
                KillTimer(m_hWnd, m_uiTrackerTimer);
                m_uiTrackerTimer = NULL;
            }

            if(NULL != m_hWndStat
               && NULL != m_uiStatTimer)
            {
                KillTimer(m_hWnd, m_uiStatTimer);
                m_uiStatTimer = NULL;
            }
        
            //
            // If the Properties or Statistics dialogs are open, close them
            //
            if (NULL != m_hWndProp && !m_bSwitchingPlaylists)
            {
                DestroyWindow(m_hWndProp);
            }

            if (NULL != m_hWndStat && !m_bSwitchingPlaylists)
            {
                DestroyWindow(m_hWndStat);
            }

#ifdef CEPLAYER_SKIN
            m_bPlayPause = true;   // enable the play button

            EnterCriticalSection( &m_csButtonInfoCritSec );
            if (DISABLED != m_binfo[SKIN_STOP].eState)
            {
                m_binfo[SKIN_STOP].eState = DISABLED;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_STOP].rc, FALSE);
            }

            if (DISABLED != m_binfo[SKIN_PLAY].eState)
            {
                m_binfo[SKIN_PLAY].eState = DISABLED;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_PLAY].rc, FALSE);
            }

            if (DISABLED != m_binfo[SKIN_PAUSE].eState)
            {
                m_binfo[SKIN_PAUSE].eState = DISABLED;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_PAUSE].rc, FALSE);
            }

            if (DISABLED != m_binfo[SKIN_SEEK].eState)
            {
                m_binfo[SKIN_SEEK].eState = DISABLED;
                ::InvalidateRect(m_hWnd, &m_binfo[SKIN_SEEK].rc, FALSE);
            }
            LeaveCriticalSection( &m_csButtonInfoCritSec );
#endif /* CEPLAYER_SKIN */

            if( m_fResumeOpen )
            {
                m_fResumeOpen = FALSE;
                CompleteOpen();
            }

        break;
    }
}


BOOL CPlayerWindow::CanSeek()
{
    VARIANT_BOOL fCanSeek;

    m_pMP->get_CanSeek(&fCanSeek);

    // The CanSeek property relies on an extended interface that
    // some filters don't support, so we need to optionally check
    // the IMediaSeeking interface as well.
    if (!fCanSeek)
    {
        HRESULT hr;
        DWORD dwCaps = 0;
        IFilterGraph *pFG = NULL;
        IActiveMovie *pAM = NULL;
        IMediaSeeking* pMS = NULL;

        do
        {
            hr = m_pUnk->QueryInterface(IID_IActiveMovie,
                        reinterpret_cast<LPVOID*>(&pAM));
            if (FAILED(hr)) break;

            hr = pAM->get_FilterGraph(reinterpret_cast<IUnknown**>(&pFG));
            if (FAILED(hr)) break;

            hr = pFG->QueryInterface(IID_IMediaSeeking,
                        reinterpret_cast<LPVOID*>(&pMS));
            if (FAILED(hr)) break;

            pMS->GetCapabilities(&dwCaps);
            fCanSeek = ((dwCaps &
                        (AM_SEEKING_CanSeekAbsolute
                        | AM_SEEKING_CanSeekForwards
                        | AM_SEEKING_CanSeekBackwards))
                        ? VARIANT_TRUE : VARIANT_FALSE);
        } while (FALSE);

        if (pMS) pMS->Release();
        if (pFG) pFG->Release();
        if (pAM) pAM->Release();
    }

    return (fCanSeek ? TRUE : FALSE);
}

///////////////////////////////////////////////////////////////////////////////
// Name: CPlayerWindow::OnCursorFocusChange()
// Desc: This function ensures that the skin items ( volume &  seek bar) are 
//		 in the right state
///////////////////////////////////////////////////////////////////////////////

void CPlayerWindow::OnCursorFocusChange()
{
    bool  bStateChange = false;

#ifdef CEPLAYER_SKIN
    if (DOWN == m_binfoVolThumb.eState)
    {
        m_binfoVolThumb.eState = UP;
        bStateChange = true;

        //
        // If the volume is being changed, make sure mute isn't set
        //
        VARIANT_BOOL fMute = FALSE;

        m_pMP->get_Mute(&fMute);

        if (fMute)
        {
            m_pMP->put_Mute(FALSE);

            UpdateMenus();
        }
    }

    if (DOWN == m_binfoSeekThumb.eState)
    {
        m_binfoSeekThumb.eState = UP;
        bStateChange = true;
    }
#endif /* CEPLAYER_SKIN */

    if (bStateChange)
    {
		::InvalidateRect(m_hWnd, NULL, TRUE);
    }
}

