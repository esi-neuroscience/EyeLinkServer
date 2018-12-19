
// EyeLinkServer.h : main header file for the EyeLinkServer application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CEyeLinkServerApp:
// See EyeLinkServer.cpp for the implementation of this class
//

class CEyeLinkServerApp : public CWinApp
{
public:
	CEyeLinkServerApp() noexcept;


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CEyeLinkServerApp theApp;
