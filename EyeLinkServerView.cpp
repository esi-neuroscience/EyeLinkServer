
// EyeLinkServerView.cpp : implementation of the CEyeLinkServerView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "EyeLinkServer.h"
#endif

#include "EyeLinkServerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern CEyeLinkServerView* g_pView;

CEdit CEyeLinkServerView::m_stateCtrl;

// CEyeLinkServerView

IMPLEMENT_DYNCREATE(CEyeLinkServerView, CFormView)

BEGIN_MESSAGE_MAP(CEyeLinkServerView, CFormView)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CEyeLinkServerApp::OnCbnSelchangeCombo1)
END_MESSAGE_MAP()

// CEyeLinkServerView construction/destruction

CEyeLinkServerView::CEyeLinkServerView() noexcept
	: CFormView(IDD_EYELINKSERVER_FORM)
{
	// TODO: add construction code here
}

CEyeLinkServerView::~CEyeLinkServerView()
{
}

void CEyeLinkServerView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEyeLinkServerView)
	DDX_Control(pDX, IDC_CUSTOM1, m_screen);
	DDX_Control(pDX, IDC_STATE, m_stateCtrl);
	DDX_Control(pDX, IDC_COMBO1, m_eyeSelID);
	//}}AFX_DATA_MAP
}

BOOL CEyeLinkServerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

void CEyeLinkServerView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit(FALSE);
	SetScaleToFitSize({ 1680, 1050 });	// dosen't care about numbers
	m_screen.Init();
	g_pView = this;
}


// CEyeLinkServerView diagnostics

#ifdef _DEBUG
void CEyeLinkServerView::AssertValid() const
{
	CFormView::AssertValid();
}

void CEyeLinkServerView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CEyeLinkServerDoc* CEyeLinkServerView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CEyeLinkServerDoc)));
	return (CEyeLinkServerDoc*)m_pDocument;
}
#endif //_DEBUG


// CEyeLinkServerView message handlers


void CEyeLinkServerView::SetStateString(LPCTSTR StateString)
{
	m_stateCtrl.SetWindowTextW(StateString);
}
