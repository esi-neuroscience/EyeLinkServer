// CScreen.cpp : implementation file
//

#include "stdafx.h"
#include "EyeLinkServer.h"
#include "CScreen.h"
#include "EyeLinkServerDoc.h"
#include <core_expt.h>

extern float fux, fuy;
extern float tx, ty;	// transformed eye position
extern CEyeLinkServerDoc* pDoc;

// CScreen

IMPLEMENT_DYNAMIC(CScreen, CWnd)

CScreen::CScreen()
{
	WNDCLASS wndcls;
	wndcls.style = CS_PARENTDC;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.cbClsExtra = 0;
	wndcls.cbWndExtra = 0;
	wndcls.hInstance = AfxGetInstanceHandle();
	wndcls.hIcon = NULL;
	wndcls.hCursor = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
//	wndcls.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
	wndcls.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wndcls.lpszClassName = _T("MFCScreenCtrl");
	wndcls.lpszMenuName = NULL;
	VERIFY(AfxRegisterClass(&wndcls));
	TRACE("CScreen Konstruktor\n");
	EnableD2DSupport();
}

CScreen::~CScreen()
{
//	m_pWhiteBrush->Destroy();
//	m_pGreenBrush->Destroy();
}


BEGIN_MESSAGE_MAP(CScreen, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_REGISTERED_MESSAGE(AFX_WM_DRAW2D, &CScreen::OnDraw2D)
	ON_WM_CREATE()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()



// CScreen message handlers


void CScreen::OnLButtonDown(UINT nFlags, CPoint point)
{
	fux = (point.x - 420.0f) * 2;
	fuy = (262.5f - point.y) * 2;
	TRACE("Left Button Down at %u,%u / %f,%f\n", point.x, point.y, fux, fuy);
	CEyeLinkServerApp::FixationUpdate(fux, fuy);
	CWnd::OnLButtonDown(nFlags, point);
}


afx_msg LRESULT CScreen::OnDraw2D(WPARAM wParam, LPARAM lParam)
{
//	TRACE("CScreen::OnDraw2D\n");
//	CHwndRenderTarget* pRenderTarget = (CHwndRenderTarget*)lParam;
	CHwndRenderTarget* pRenderTarget = GetRenderTarget();
	ASSERT_VALID(pRenderTarget);
	pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
	pRenderTarget->SetTransform(D2D1::Matrix3x2F::Scale(D2D1::Size(0.5f, -0.5f),
		D2D1::Point2F(840.0f, 175.0f)));
	pDoc->DrawTargets(pRenderTarget);
	pRenderTarget->FillEllipse(
		CD2DEllipse(D2D1::Point2F(tx, ty), D2D1::SizeF(6.0f, 6.0f)),
		m_pWhiteBrush);
	return 0;
}


void CScreen::Init()
{
	VERIFY(IsD2DSupportEnabled());
	CHwndRenderTarget* pRenderTarget = GetRenderTarget();
	m_pWhiteBrush = new CD2DSolidColorBrush(
		pRenderTarget,
		D2D1::ColorF(D2D1::ColorF::White));
	m_pGreenBrush = new CD2DSolidColorBrush(
		GetRenderTarget(),
		D2D1::ColorF(D2D1::ColorF::LightGreen));
}


int CScreen::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	TRACE("CScreen::OnCreate\n");	// never called
	//	HRESULT hr = ((CHwndRenderTarget*) m_pRenderTarget)->Create(m_hWnd);
	return 0;
//	return SUCCEEDED(hr) ? 0 : -1;
}


void CScreen::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CWnd::OnLButtonUp(nFlags, point);
}

void CScreen::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (nFlags & MK_LBUTTON)
	{
		fux = (point.x - 420.0f) * 2;
		fuy = (262.5f - point.y) * 2;
		TRACE("Mouse move to %u,%u / %f,%f\n", point.x, point.y, fux, fuy);
		CEyeLinkServerApp::FixationUpdate(fux, fuy);
	}
	CWnd::OnMouseMove(nFlags, point);
}
