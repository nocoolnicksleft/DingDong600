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
// File: PropertyDlg.h
//
// Desc: This file defines a class that will house the Property Dialog box.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _PROPERTYDLG_H_
#define _PROPERTYDLG_H_

#include <windows.h>
#include <dxmplay.h>
#include <strmif.h>
#include <amvideo.h>
#include <wtypes.h>
#include <qnetwork.h>

#include "playerwindow.h"

#define PD_SHOW     WM_APP
#define PD_UPDATE   WM_APP+1
#define PD_CLOSED   WM_APP+2

BOOL CALLBACK PropertyDialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef struct _AudioInfo
{
    WAVEFORMATEX wfmtWaveFormat;
    BSTR         bstrCodecDescription;
    LPWSTR       lpwcCodecBitrate;
    BOOL         bStereo;   
    VARIANT_BOOL bSecure;
} AUDIOINFO, LPAUDIOINFO;

class CPropertyDlg
{
 public:
    CPropertyDlg();
    ~CPropertyDlg();

    bool Init(HWND hWnd);
    bool Fini();

    bool Show(int iShowCmd);
    void Update(TCHAR * szFilename);
    void UpdateAndRender(TCHAR * szFilename);
    BOOL HandleNotifyMsg( HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

 private:
    HRESULT AudioInfo( LPAUDIOINFO           * audioInfo,
                       IBasicAudio           * piba,
                       IAMNetShowExProps     * pnsep,
                       IAMSecureMediaContent * psmc );
    HRESULT VideoInfo( BSTR              * videoInfo,
                       IAMNetShowExProps * psnep );
    HWND   m_hWnd;
    HWND   m_hWndParent;
    HWND   m_hListView;
    HFONT  m_hFont;
};

#endif /* _PROPERTYDLG_H_ */
