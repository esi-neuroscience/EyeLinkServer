#pragma once
#include <afx.h>
class CTransformation :
	public CObject
{
public:
	CTransformation() {};
//	~CTransformation();
	virtual void Apply(float* x, float* y) = 0;
	virtual void xMsg(char* msg) = 0;
	virtual void yMsg(char* msg) = 0;
};

class CLinearTransform :
	public CTransformation
{
public:
	CLinearTransform(float* message);
//	~CLinearTransform();
	void Apply(float* x, float* y);
	void xMsg(char* msg);
	void yMsg(char* msg);
private:
	float m_x0, m_y0, m_x1, m_y1;
};

class CPolynom2Transform :
	public CTransformation
{
public:
	CPolynom2Transform(float* message);
	//	~CPolynom2Transform();
	void Apply(float* x, float* y);
	void xMsg(char* msg);
	void yMsg(char* msg);
private:
	float m_x0, m_y0, m_x1, m_y1, m_x2, m_y2;
};

class CbiquadTransform :
	public CTransformation
{
public:
	CbiquadTransform(float* message);
	//	~CbiquadTransform();
	void Apply(float* x, float* y);
	void xMsg(char* msg);
	void yMsg(char* msg);
private:
	float m_a0, m_b0, m_a1, m_b1, m_a2, m_b2, m_a3, m_b3, m_a4, m_b4, m_a5, m_b5;
};

