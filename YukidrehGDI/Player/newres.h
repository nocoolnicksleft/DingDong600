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
#ifndef __NEWRES_H__
#define __NEWRES_H__

#ifdef UNDER_CE
#define _WIN32_WCE 400
#else
#define UNDER_CE _WIN32_WCE
#endif

#if defined(_WIN32_WCE)
	#if !defined(WCEOLE_ENABLE_DIALOGEX)
		#define DIALOGEX DIALOG DISCARDABLE
	#endif
	#include <commctrl.h>
	#define  SHMENUBAR RCDATA
	#define I_IMAGENONE		(-2)
	#define NOMENU			0xFFFF
	#define IDS_SHNEW		1

	#define IDM_SHAREDNEW        10
	#define IDM_SHAREDNEWDEFAULT 11
	#define AFXCE_IDD_SAVEMODIFIEDDLG 28701
#endif // _WIN32_WCE

#ifdef RC_INVOKED
#ifndef _INC_WINDOWS
#define _INC_WINDOWS
	#include "winuser.h"           // extract from windows header
#endif
#endif

#ifdef IDC_STATIC
#undef IDC_STATIC
#endif
#define IDC_STATIC      (-1)

#endif //__NEWRES_H__
