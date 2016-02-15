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
// File: OpenURLDlg.h
//
// Desc: This file contains the prototype for the OpenURLDialogProc.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _OPENURLDLG_H_
#define _OPENURLDLG_H_

#include <windows.h>

#define MAX_FILEOPEN_HISTORY 5
///////////////////////////////////////////////////////////////////////////////
// This structure is used to pass parameters to the OpenURL dialog box
///////////////////////////////////////////////////////////////////////////////
typedef struct _args
{
   TCHAR **szURL;
   TCHAR  *szFilename;
   TCHAR  *szFilenameHistory[ MAX_FILEOPEN_HISTORY ];
} args_t;

///////////////////////////////////////////////////////////////////////////////
// Name:    OpenURLDialogProc()
// Returns: FALSE - The message was not handled
//          TRUE  - The message was handled
///////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK OpenURLDialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

class COpenURLDlg
{
 public:
   COpenURLDlg();
   ~COpenURLDlg();

   bool Init(HWND hWnd);
   bool Fini();

   bool Show(int iShowCmd);

 private:
   HWND   m_hWnd;
   HWND   m_hWndParent;
};

#define MAX_URL_LENGTH 2048

#endif /* _OPENURLDLG_H_ */
