
// EyeLinkServer.h : main header file for the EyeLinkServer application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include "CTransformation.h"


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
//	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	static void EyelinkDisconnect(void);
	static void FixationUpdate(float x, float y);
	void EyelinkCommand(const char* command);
	void EyelinkStart(unsigned char message[], DWORD messageLength);
	void EyelinkStop(unsigned char message[], DWORD messageLength);
	static void EyelinkPassMessage(unsigned char message[], DWORD messageLength);
	static void TransformMsg();
	static void TransformClearMsg();
	unsigned short m_screenWidth;
	unsigned short m_screenHeight;
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnEyelinkConnect();
private:
	void EyelinkOffline();
	static CRITICAL_SECTION m_criticalUpdate;
	HANDLE m_hLinkTimer;
	HANDLE m_hShare;
	static INT16 m_useEDF;
	static float* m_pShare;
	static HANDLE m_hDone;
	INT16 m_eyeUsedConf;
public:
	afx_msg void OnUpdateEyelinkConnect(CCmdUI *pCmdUI);
	afx_msg void OnEyelinkClearscreen();
	afx_msg void OnUpdateEyelinkClearscreen(CCmdUI *pCmdUI);
	static bool m_eyeLinkIsConnected;
	static INT16 m_eyeUsed;
	afx_msg void OnCbnSelchangeCombo1();
private:
	void EyeUsedMessage();
};

extern CEyeLinkServerApp theApp;
