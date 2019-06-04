#pragma once

// CTarget-Befehlsziel

class CTarget : public CObject
{
public:
	CTarget(void);
	void Init(CString name, float position[2]);
	virtual ~CTarget();
	virtual bool Inside(float x, float y) = 0;
	virtual void Draw(CHwndRenderTarget* pRenderTarget, CD2DSolidColorBrush* pBrush) {};
	HANDLE m_hEnter;
	HANDLE m_hLeave;
	bool m_wasInside;	// keeps track of the current state
	bool m_OutOnBlink;	// signal "Out" ("leave") event on blink
protected:
	float m_position[2];
	bool m_isDrawn;
private:
	CString m_name;
};


#pragma once

// CCircle-Befehlsziel

class CCircle : public CTarget
{
public:
	CCircle(CString name, float position[2], float radius);
	virtual ~CCircle();
	bool Inside(float x, float y);
	void Draw(CHwndRenderTarget* pRenderTarget, CD2DSolidColorBrush* pBrush);
private:
	float m_radius;
	void DrawEyelink(BYTE color);
};


