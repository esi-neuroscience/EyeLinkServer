
// EyeLinkServerDoc.cpp : implementation of the CEyeLinkServerDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "EyeLinkServer.h"
#endif

#include "EyeLinkServerDoc.h"
#include "EyeLinkServerView.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern CTransformation* g_pTransformation;
extern float fux, fuy;

CMapWordToOb CEyeLinkServerDoc::m_targets;
CRITICAL_SECTION CEyeLinkServerDoc::m_criticalSection;	// protect map operations

											// CTarget* CEyeLinkServerDoc::m_pCurrent = NULL;

CEyeLinkServerView* g_pView;

// CEyeLinkServerDoc

IMPLEMENT_DYNCREATE(CEyeLinkServerDoc, CDocument)

BEGIN_MESSAGE_MAP(CEyeLinkServerDoc, CDocument)
END_MESSAGE_MAP()


// CEyeLinkServerDoc construction/destruction

CEyeLinkServerDoc::CEyeLinkServerDoc() noexcept
{
	// TODO: add one-time construction code here
	InitializeCriticalSection(&m_criticalSection);
}

CEyeLinkServerDoc::~CEyeLinkServerDoc()
{
	if (g_pTransformation)
	{
		delete g_pTransformation;
		g_pTransformation = NULL;
	}
	DeleteCriticalSection(&m_criticalSection);
}

BOOL CEyeLinkServerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	return TRUE;
}




// CEyeLinkServerDoc serialization

void CEyeLinkServerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CEyeLinkServerDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CEyeLinkServerDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data.
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CEyeLinkServerDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = nullptr;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != nullptr)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CEyeLinkServerDoc diagnostics

#ifdef _DEBUG
void CEyeLinkServerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CEyeLinkServerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CEyeLinkServerDoc commands

#ifdef noOverlap
void CEyeLinkServerDoc::FixationUpdate(float x, float y)
{
	//	if ((x == m_x) && (y == m_y)) return;
	//	m_x = x;
	//	m_y = y;
	if (m_pCurrent)
	{
		if (!m_pCurrent->Inside(x, y))
		{
			SetEvent(m_pCurrent->m_hLeave);
			m_pCurrent = NULL;
		}
	}
	else
	{
		EnterCriticalSection(&m_criticalSection);
		POSITION pos = m_targets.GetStartPosition();
		if (pos)
		{
			WORD key;
			CTarget* pTarget;
			do
			{
				m_targets.GetNextAssoc(pos, key, (CObject*&)pTarget);
				if (pTarget->Inside(x, y))
				{
					SetEvent(pTarget->m_hEnter);
					m_pCurrent = pTarget;
					break;
				}
			} while (pos);
		}
		LeaveCriticalSection(&m_criticalSection);
	}
	//	g_pView->m_screen.Draw();
	g_pView->m_screen.Invalidate();
}
#endif

void CEyeLinkServerDoc::FixationUpdate(float x, float y)
{
	EnterCriticalSection(&m_criticalSection);
	POSITION pos = m_targets.GetStartPosition();
	WORD key;
	CTarget* pTarget;
	while (pos)
	{
		m_targets.GetNextAssoc(pos, key, (CObject*&)pTarget);
		pTarget->FixationUpdate(x, y);
	}
	LeaveCriticalSection(&m_criticalSection);
	g_pView->m_screen.Invalidate();
}

short CEyeLinkServerDoc::Command(WORD key, unsigned char message[], DWORD messageLength)
{
	if (key == 0)
	{
		switch (message[0])
		{
		case 0:
			TRACE("switch code: %u\n", message[1]);
			switch (message[1])
			{
			case 0:	// [00 0 0 filename] stop recording (and send EDF)
				if (messageLength == 2) message[2] = 0;
				theApp.EyelinkStop(&message[2], messageLength - 2);
				return -1;	// success -- no key to return
				break;
			case 1:	// [00 0 1 width height] set screen size
				theApp.m_screenWidth = *((WORD*)&message[2]);
				theApp.m_screenHeight = *((WORD*)&message[4]);
				return -1;	// success -- no key to return
				break;
			case 2:	// [00 0 2 filename] start recording
				theApp.EyelinkStart(&message[2], messageLength - 2);
				return -1;	// success -- no key to return
				break;
			case 3:
				// [00 0 3]						remove transformation
				// [00 0 3 x0 y0 x1 y1]			coefficients for linear transformation
				// [00 0 3 x0 y0 x1 y1 x2 y2]	coefficients for quadratic transformation
				// [00 0 3 a0 b0 a1 b1 a2 b2 a3 b3 a4 b4 a5 b5]	coefficients for biquadratic transformation
				TRACE("Tansform length: %u\n", messageLength);
				//+
				// delete any existing transformation first
				if (g_pTransformation)
				{
					delete g_pTransformation;
					g_pTransformation = NULL;
				}
				//-
				switch (messageLength)
				{
				case  2:	// remove
					CEyeLinkServerApp::TransformClearMsg();
					break;
				case 18:	// linear
					g_pTransformation = new CLinearTransform((float*)&message[2]);
					VERIFY(g_pTransformation);
					break;
				case 26:	// quadratic
					g_pTransformation = new CPolynom2Transform((float*)&message[2]);
					VERIFY(g_pTransformation);
					break;
				case 50:	// biquadratic
					g_pTransformation = new CbiquadTransform((float*)&message[2]);
					VERIFY(g_pTransformation);
					break;
				default:
					TRACE("Invalid length of transformation command\n");
					ASSERT(false);
					//... AddToLog ...
				}
				CEyeLinkServerApp::TransformMsg();
				break;
			case 4:	// [00 0 4 message] pass message to EyeLink
				theApp.EyelinkPassMessage(&message[2], messageLength - 2);
				break;
			case 5:	// [00 0 5] switch to "sample mode"
				theApp.m_sampleMode = true;
				TRACE("Switched to Sample Mode\n");
				break;
			case 6:	// [00 0 6] enable saccade detection
				theApp.m_reportSaccades = true;
				TRACE("enable saccade detection\n");
				break;
			case 7:	// [00 0 7] signal next start saccade
				theApp.m_signalSaccade = true;
				break;
			}
			return -1;	// success -- no key to return
			break;
		case 1:	// [00 1 x y r name]	create circular target
			CCircle* pTarget;
			pTarget = new CCircle(CString(&message[13]), (float*)&message[1], *((float*)&message[9]));
			return AddTarget(pTarget);
			break;
		default:
			ASSERT(false);
		}
	}
	else	// Target command (key != 0)
	{
		CTarget* pTarget;
		VERIFY(m_targets.Lookup(key, (CObject*&)pTarget));
		switch (message[0])
		{
		case 0:	// remove
			{
			EnterCriticalSection(&m_criticalSection);
			VERIFY(m_targets.RemoveKey(key));
//			if (m_pCurrent == pTarget) m_pCurrent = NULL;
			delete pTarget;
			LeaveCriticalSection(&m_criticalSection);
			g_pView->m_screen.Invalidate();
			return -1;	// success -- no key to return
			}
			break;
		default:
			return pTarget->Command(message, messageLength);
		}
	}
	return 0;
}


short CEyeLinkServerDoc::AddTarget(CTarget* pTarget)
{
	WORD key;
	CTarget* pObject;
	for (key = 1; NULL != m_targets.Lookup(key, (CObject*&)pObject); key++);
	EnterCriticalSection(&m_criticalSection);
	m_targets[key] = pTarget;
	LeaveCriticalSection(&m_criticalSection);
	FixationUpdate(fux, fuy);
	return key;
}


void CEyeLinkServerDoc::DrawTargets(CHwndRenderTarget* pRenderTarget)
{
	if (!this) return;
	POSITION pos = m_viewList.GetHeadPosition();
	CEyeLinkServerView* pView = (CEyeLinkServerView*)this->m_viewList.GetAt(pos);
	CScreen* pScreen = &pView->m_screen;
	EnterCriticalSection(&m_criticalSection);
	pos = m_targets.GetStartPosition();
	WORD key;
	CTarget* pTarget;
	while (pos)
	{
		m_targets.GetNextAssoc(pos, key, (CObject*&)pTarget);
//		CD2DSolidColorBrush* pBrush = (pTarget == m_pCurrent) ?
		CD2DSolidColorBrush* pBrush = (pTarget->m_wasInside) ?
			pScreen->m_pGreenBrush : pScreen->m_pWhiteBrush;
		pTarget->Draw(pRenderTarget, pBrush);
	}
	LeaveCriticalSection(&m_criticalSection);
}

#ifdef noOverlap
void CEyeLinkServerDoc::OnStartBlink()
{
	if (!m_pCurrent) return;
	if (!m_pCurrent->m_OutOnBlink) return;
	SetEvent(m_pCurrent->m_hLeave);
}
#endif // noOverlap

void CEyeLinkServerDoc::OnStartBlink()
{
	EnterCriticalSection(&m_criticalSection);
	POSITION pos = m_targets.GetStartPosition();
	WORD key;
	CTarget* pTarget;
	while (pos)
	{
		m_targets.GetNextAssoc(pos, key, (CObject*&)pTarget);
		pTarget->OutOnBlink();
	}
	LeaveCriticalSection(&m_criticalSection);
}