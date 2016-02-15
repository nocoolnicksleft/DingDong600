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
// File: OptionsDlg.h
//
// Desc: This file contains the prototype for the OptionsDialogProc.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _OptionsDLG_H_
#define _OptionsDLG_H_

#include <windows.h>

///////////////////////////////////////////////////////////////////////////////
// Name:    OptionsDialogProc()
// Returns: FALSE - The message was not handled
//          TRUE  - The message was handled
///////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK OptionsDialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif /* _OptionsDLG_H_ */
