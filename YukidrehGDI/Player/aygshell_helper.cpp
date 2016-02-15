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
#include "aygshell_helper.h"

CAygshellHelper g_AygshellHelper;

CAygshellHelper::CAygshellHelper( void )
{
    if( m_hAygshellDLL = LoadLibrary( L"aygshell.dll" ) )
    {
        BOOL (*SHInitExtraControls)() = NULL;
        SHInitExtraControls = (BOOL (*)())GetProcAddress( m_hAygshellDLL, L"SHInitExtraControls" );
        SHInitDialog = (LPSHINITDIALOG)GetProcAddress( m_hAygshellDLL, L"SHInitDialog" );
        SHInputDialog = (LPSHINPUTDIALOG)GetProcAddress( m_hAygshellDLL, L"SHInputDialog" );
        SHSipInfo = (LPSHSIPINFO)GetProcAddress( m_hAygshellDLL, L"SHSipInfo" );
        // SHHandleWMSettingChange isn't exported by name, so reference its ordinal.
        // OEMs shouldn't have to do this since they won't conditionally include aygshell.
        SHHandleWMSettingChange = (LPSHHANDLEWMSETTINGCHANGE)GetProcAddress( m_hAygshellDLL, (LPWSTR)MAKELONG(83, 0) );
        if( !(SHInitDialog && SHInputDialog
        	&& SHInitExtraControls && SHSipInfo
        	&& SHHandleWMSettingChange) )
        {
            FreeLibrary( m_hAygshellDLL );
            m_hAygshellDLL = NULL;
        }
        else
        {
            SHInitExtraControls();
        }
    }
}

CAygshellHelper::~CAygshellHelper( void )
{
    if( m_hAygshellDLL )
    {
        FreeLibrary( m_hAygshellDLL );
#ifdef DEBUG
        m_hAygshellDLL = (HMODULE)0xDEADBEEF;
#endif
    }
}

BOOL CAygshellHelper::Loaded( void )
{
    return m_hAygshellDLL ? TRUE : FALSE;
}
