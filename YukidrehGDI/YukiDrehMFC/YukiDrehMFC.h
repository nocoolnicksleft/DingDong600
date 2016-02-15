// YukiDrehMFC.h : main header file for the YukiDrehMFC application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CYukiDrehMFCApp:
// See YukiDrehMFC.cpp for the implementation of this class
//

class CYukiDrehMFCApp : public CWinApp
{
public:
	CYukiDrehMFCApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CYukiDrehMFCApp theApp;