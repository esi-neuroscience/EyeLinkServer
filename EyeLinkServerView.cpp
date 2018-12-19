
// EyeLinkServerView.cpp : implementation of the CEyeLinkServerView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "EyeLinkServer.h"
#endif

#include "EyeLinkServerDoc.h"
#include "EyeLinkServerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CEyeLinkServerView

IMPLEMENT_DYNCREATE(CEyeLinkServerView, CFormView)

BEGIN_MESSAGE_MAP(CEyeLinkServerView, CFormView)
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
	ResizeParentToFit();

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
