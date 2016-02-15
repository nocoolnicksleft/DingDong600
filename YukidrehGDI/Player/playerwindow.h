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
// File: PlayerWindow.h
//
// Desc: This header defines a class that encapsulates the window(s) needed
//       for the CEPlayer application.  It also joins the functionality of
//       the MediaPlayer container and Event Sink to provide a means for
//       handling events.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _PLAYERWINDOW_H_
#define _PLAYERWINDOW_H_

#include <windows.h>
#include <commctrl.h>
#include <control.h>

#include "MPContainer.h"
#include "MPEventSink.h"
#include "StatisticsDlg.h"
#include "OpenURLDlg.h"
#include "Plist.h"

// This defines the ID of the timer
#define CEPLAYER_TIMER   100

// This defines the number of milliseconds between timer events
#define CEPLAYER_TIMEOUT 2000

#ifdef CEPLAYER_SKIN
enum EButtonState { UP, DOWN, DISABLED };

#define SKIN_NEXT      0
#define SKIN_PREV      1
#define SKIN_SOUND     2
#define SKIN_STOP      3
#define SKIN_PLAY      4
#define SKIN_PAUSE     5
#define SKIN_VOLUME    6
#define SKIN_SEEK      7
#define SKIN_FF	       8
#define SKIN_FR	       9
#define SKIN_SIZE      10

typedef struct _button_info
{
    EButtonState eState;
    HBITMAP      hUp;
    HBITMAP      hDown;
    HBITMAP      hDisabled;
    POINT        ptPos;
    POINT        ptDim;
    RECT         rc;
//    bool         bHover;
} button_info_t;
#endif /* CEPLAYER_SKIN */

class CPlayerWindow : public CMPContainer,
                             CMPEventSink
{
 public:
   enum EState {BAD, STOP, PLAY, PAUSE, OPENING};

   // Constructor/Destructor
   CPlayerWindow(HWND hWnd, HINSTANCE hInstace);
   ~CPlayerWindow();

   // PlayerWindow methods
   bool Init();
   bool Fini();

   void Show(int iShow);
   bool TranslateAccelerator(LPMSG pMsg);
   bool DialogMessage(LPMSG pMsg);
   bool SaveRegState();
   bool LoadRegState();

   void ApplyPlayCount(void);
   void ApplyZoomLevel(void);
   void SetMinimumRect( RECT &rect );

   bool OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
   void OnPaint(HDC hDC, RECT *rc);
   void OnSize(int x, int y);
   void OnSizeSmall(int x, int y);
   void OnMeasureItem(MEASUREITEMSTRUCT * pmis);
   void OnDrawItem(DRAWITEMSTRUCT * pdis);
   void OnMouseMove(int x, int y);
   void OnMouseDown(int x, int y);
   void OnMouseUp(int x, int y);
   void OnCursorFocusChange();
   bool OnTimer(UINT uID);
   void StatTimer();
   void TrackerTimer();

   bool OnCommand(WORD wID, LPARAM lParam);
   bool OnOpen(LPCTSTR szFilename);
   bool CompleteOpen();				// 10/29/03. Splitting OnOpen function in two to handle asynchronous close of the OCX
   bool OnOpenURL();
   bool OnClose();
   bool OnPlaylist();
   void OnPlaylistClose();
   bool OnAddToFavorites();
   bool OnOrganizeFavorites();
   bool OnFavorite(int iIndex);
   bool OnFullScreen();
   bool OnStatistics();
   bool OnProperties();
   bool OnOptions();
   bool OnPlay();
   bool OnPause();
   bool OnStop();
   bool OnMute();
   bool OnRepeat();
   bool OnShuffle();
   bool OnPlaySong(CPlaylist * pPlaylist);
   bool OnGoWeb();

   void PropertyDlgClosed();
   void StatisticsDlgClosed();
   void PlaylistDelete(CPlaylist * pPlaylist);
   void UpdatePropertyDlg(LPCTSTR szFilename = NULL);

   // CMPContainer methods
   STDMETHOD (OnPosRectChange)(LPCRECT lprcPosRect);

   // CMPEventSink virtual methods
   void DVDNotify(long lEventCode, long lEventParam1, long lEventParam2);
   void EndOfStream(long lResult);
   void KeyDown(short nKeyCode, short nShiftState);
   void KeyUp(short nKeyCode, short nShiftState);
   void KeyPress(short nCharacterCode);
   void MouseMove(short nButton, short nShiftState, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y);
   void MouseDown(short nButton, short nShiftState, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y);
   void MouseUp(short nButton, short nShiftState, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y);
   void Click(short nButton, short nShiftState, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y);
   void DblClick(short nButton, short nShiftState, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y);
   void OpenStateChange(long lOldState, long lNewState);
   void PlayStateChange(long lOldState, long lNewState);
   void ScriptCommand(BSTR strSCType, BSTR strParam);
   void Buffering(VARIANT_BOOL bStart);
   void Error(void);
   void MarkerHit(long lMarkerNum);
   void Warning(long lWarningType, long lParam, BSTR strDescription);
   void NewStream(void);
   void Disconnect(long lResult);
   void PositionChange(double dOldPosition, double dNewPosition);
   void DisplayModeChange(void);
   void ReadyStateChange(ReadyStateConstants eReadyState);
   HRESULT FindInterfaceOnGraph( REFIID id, void **Interface );
   TCHAR                   *m_szPath;
   TCHAR                   *m_szFilter;
   BOOL                     m_bPlayForever;
   BOOL                     m_bShuffle;
   DWORD                    m_dwZoomLevel;
   DWORD                    m_dwMinimumWidth;
   DWORD                    m_dwMinimumHeight;
   int                      m_nFilterIndex;
   HWND                     m_hWndPlay;

 protected:
   void UpdateMenus();
   void SetState(EState eState);
   BOOL CanSeek();

 private:
   UINT                     m_uiStatTimer;
   UINT                     m_uiTrackerTimer;
   HINSTANCE                m_hInstance;
   HWND                     m_hWnd;
   HWND                     m_hWndCB;
   HWND                     m_hWndProp;
   HWND                     m_hWndStat;
   EState                   m_eState;
   TCHAR                   *m_szFilename;
   TCHAR                   *m_szFilenameHistory[ MAX_FILEOPEN_HISTORY ];
   CStatisticsDlg::stats_t  m_stats;
   CPlaylist               *m_pPlaylist;
   HIMAGELIST               m_himgLocationList;
   bool                     m_bSwitchingPlaylists;
   BOOL                     m_fCanSeek;
   BOOL                     m_fCanPause;
   double                   m_dDuration; //   m_pMP->get_Duration(&m_dDuration);
   BOOL                     m_fResumeOpen;
   BOOL                     m_bStop;
   UINT                     m_currentRate;

#ifdef CEPLAYER_SKIN
   HBITMAP                  m_hbmBuffer;
   int                      m_iSkinMargin;
   button_info_t            m_binfo[SKIN_SIZE];
   button_info_t            m_binfoVolThumb;
   button_info_t            m_binfoSeekThumb;
   RECT                     m_rcVolBounds;
   RECT                     m_rcSeekBounds;
   bool                     m_bPlayPause;  // true = play    false = pause
#endif /* CEPLAYER_SKIN */
   public:
       CRITICAL_SECTION     m_csButtonInfoCritSec;
};

extern CPlayerWindow *g_pPlayerWindow;
extern HINSTANCE g_hInst;

#endif /* _PLAYERWINDOW_H_ */
