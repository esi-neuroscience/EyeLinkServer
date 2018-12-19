
// EyeLinkServerView.h : interface of the CEyeLinkServerView class
//

#pragma once


class CEyeLinkServerView : public CFormView
{
protected: // create from serialization only
	CEyeLinkServerView() noexcept;
	DECLARE_DYNCREATE(CEyeLinkServerView)

public:
#ifdef AFX_DESIGN_TIME
	enum{ IDD = IDD_EYELINKSERVER_FORM };
#endif

// Attributes
public:
	CEyeLinkServerDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CEyeLinkServerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in EyeLinkServerView.cpp
inline CEyeLinkServerDoc* CEyeLinkServerView::GetDocument() const
   { return reinterpret_cast<CEyeLinkServerDoc*>(m_pDocument); }
#endif

