#pragma once

// CTarget-Befehlsziel

class CTarget : public CObject
{
public:
	CTarget(void);
	void Init(CString name, float position[2]);
	void FixationUpdate(float x, float y);
	short Command(unsigned char message[], DWORD messageLength);
	void OutOnBlink();
	virtual ~CTarget();
	virtual void Draw(CHwndRenderTarget* pRenderTarget, CD2DSolidColorBrush* pBrush) {};
	HANDLE m_hLeave;
	bool m_wasInside;	// keeps track of the current state
protected:
	virtual bool Inside(float x, float y) = 0;
	float m_position[2];
	bool m_isDrawn;
private:
	CString m_name;
	HANDLE m_hEnter;
	bool m_inclusive;
	bool m_OutOnBlink;	// signal "Out" ("leave") event on blink
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


