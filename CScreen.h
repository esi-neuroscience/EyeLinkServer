#pragma once


// CScreen

class CScreen : public CWnd
{
	DECLARE_DYNAMIC(CScreen)

public:
	CScreen();
	virtual ~CScreen();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	CD2DSolidColorBrush* m_pWhiteBrush;
	CD2DSolidColorBrush* m_pGreenBrush;
protected:
	afx_msg LRESULT OnDraw2D(WPARAM wParam, LPARAM lParam);
public:
	void Init();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};


