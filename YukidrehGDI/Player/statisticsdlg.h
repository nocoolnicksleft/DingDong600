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
// File: StatisticsDlg.h
//
// Desc: This file declares a class that will house the Statistics dialog box.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _STATISTICSDLG_H_
#define _STATISTICSDLG_H_

#define SD_SHOW     WM_APP+3
#define SD_UPDATE   WM_APP+4
#define SD_CLOSED   WM_APP+5

BOOL CALLBACK StatisticsDialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

class CStatisticsDlg
{
 public:
   struct stats_t
   {
      double dFrameRate;
      double dActualRate;
      long   lFramesDropped;
      long   lBandwidth;
      long   lSourceProtocol;
      long   lReceivedPackets;
      long   lRecoveredPackets;
      long   lLostPackets;
      long   lDroppedAudioPackets;
      TCHAR *szErrorCorrection;
      TCHAR *szFilename;
   };

   CStatisticsDlg();
   ~CStatisticsDlg();

   bool Init(HWND hWnd);
   bool Fini();

   bool Show(int iShowCmd);
   void Update(stats_t *pStats);

   BOOL HandleNotifyMsg(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

 protected:
   void Refresh();
   void RefreshItem(int iIndex, LPCTSTR pszTitle, TCHAR * pszText, HDC hdc, LONG & cx);
   void InsertTwoItems(int iIndex);
 private:
   double m_dFrameRate;
   double m_dActualRate;
   long   m_lFramesDropped;
   long   m_lBandwidth;
   long   m_lSourceProtocol;
   long   m_lReceivedPackets;
   long   m_lRecoveredPackets;
   long   m_lLostPackets;
   long   m_lDroppedAudioPackets;
   TCHAR *m_szErrorCorrection;
   TCHAR *m_szFilename;
   HWND   m_hWnd;
   HWND   m_hWndParent;
   HWND   m_hListView;
   HFONT  m_hFont;
   int    m_iMaxWidth;

   int    m_iFRIndex;
   int    m_iARIndex;
   int    m_iFDIndex;
   int    m_iBWIndex;
   int    m_iSPIndex;
   int    m_iRCIndex;
   int    m_iRPIndex;
   int    m_iLPIndex;
   int    m_iECIndex;
   int    m_iFNIndex;
   int    m_iMIIndex;
   int    m_iADIndex;

   bool   m_bFRChanged;
   bool   m_bARChanged;
   bool   m_bFDChanged;
   bool   m_bBWChanged;
   bool   m_bSPChanged;
   bool   m_bRCChanged;
   bool   m_bRPChanged;
   bool   m_bLPChanged;
   bool   m_bADChanged;
   bool   m_bECChanged;
   bool   m_bFNChanged;
};

#endif /* _STATISTICSDLG_H_ */
