
// EyeLinkServerDoc.h : interface of the CEyeLinkServerDoc class
//


#pragma once
#include "Target.h"
//#include "EyeLinkServerView.h"


class CEyeLinkServerDoc : public CDocument
{
protected: // create from serialization only
	CEyeLinkServerDoc() noexcept;
	DECLARE_DYNCREATE(CEyeLinkServerDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CEyeLinkServerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	short Command(WORD key, unsigned char message[], DWORD messageLength);
	void FixationUpdate(float x, float y);
	void DrawTargets(CHwndRenderTarget* pRenderTarget);

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
private:
	CTarget* m_pCurrent;
	CMapWordToOb m_targets;
	virtual short AddTarget(CTarget* pTarget);
	CRITICAL_SECTION m_criticalSection;	// protect map operations
};
