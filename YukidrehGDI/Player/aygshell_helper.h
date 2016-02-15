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
#include "shellsdk.h"

typedef BOOL (* LPSHINITDIALOG)( PSHINITDLGINFO pshidi );
typedef void (* LPSHINPUTDIALOG)( HWND hWnd, UINT uMsg, WPARAM wParam );
typedef BOOL (* LPSHHANDLEWMSETTINGCHANGE)( HWND hwnd, WPARAM wParam, LPARAM lParam, SHACTIVATEINFO* psai);
typedef BOOL (* LPSHSIPINFO)( UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni );



class CAygshellHelper
{
public:
    CAygshellHelper(void);
    ~CAygshellHelper(void);
    BOOL Loaded(void);
    LPSHINITDIALOG SHInitDialog;
    LPSHINPUTDIALOG SHInputDialog;
    LPSHHANDLEWMSETTINGCHANGE SHHandleWMSettingChange;
    LPSHSIPINFO SHSipInfo;
private:
    HMODULE m_hAygshellDLL;
};

extern CAygshellHelper g_AygshellHelper;
