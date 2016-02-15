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
//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
	ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
	PARTICULAR PURPOSE.

Module Name:

	Decibels.h

Abstract:

	Declaration file for routines in Decibels.h

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////

#define MIN_VOLUME_RANGE 0
#define MAX_VOLUME_RANGE 100

#define AX_MIN_VOLUME  -10000
#define AX_HALF_VOLUME   -600
#define AX_MAX_VOLUME       0

DWORD DBToAmpFactor( LONG lDB );
LONG AmpFactorToDB( DWORD dwFactor );
