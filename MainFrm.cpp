
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "EyeLinkServer.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CPersistentFrame)

BEGIN_MESSAGE_MAP(CMainFrame, CPersistentFrame)
	ON_WM_CREATE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction
CMainFrame::CMainFrame() noexcept
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPersistentFrame::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CPersistentFrame::PreCreateWindow(cs) )
		return FALSE;

	cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE
		 | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MINIMIZE | WS_SYSMENU;
	//cs.cx = 840;
	//cs.cy = 525;
//	cs.cx = 875;
//	cs.cy = 686;
	cs.cx = 883;
	cs.cy = 646;

	return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CPersistentFrame::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CPersistentFrame::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

