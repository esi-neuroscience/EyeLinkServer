#include "stdafx.h"
#include "CTransformation.h"

/*
CTransformation::CTransformation()
{
}


CTransformation::~CTransformation()
{
}
*/


CLinearTransform::CLinearTransform(float* message)
{
	m_x0 = *message++;
	m_y0 = *message++;
	m_x1 = *message++;
	m_y1 = *message;
}

/*
CLinearTransform::~CLinearTransform()
{
}
*/


void CLinearTransform::Apply(float* x, float* y)
{
	TRACE("Apply (pre): %f, %f\n", *x, *y);
	*x = m_x0 + m_x1 * *x;
	*y = m_y0 + m_y1 * *y;
	TRACE("Apply (post): %f, %f\n", *x, *y);
}

void CLinearTransform::xMsg(char* msg)
{
	VERIFY(sprintf(msg, "x-transform: x' = %f + %f*x", m_x0, m_x1) > 0);
}


void CLinearTransform::yMsg(char* msg)
{
	VERIFY(sprintf(msg, "y-transform: y' = %f + %f*y", m_y0, m_y1) > 0);
}


CPolynom2Transform::CPolynom2Transform(float* message)
{
	m_x0 = *message++;
	m_y0 = *message++;
	m_x1 = *message++;
	m_y1 = *message++;
	m_x2 = *message++;
	m_y2 = *message;
}

/*
CPolynom2Transform::~CPolynom2Transform()
{
}
*/


void CPolynom2Transform::Apply(float* x, float* y)
{
	*x = m_x0 + m_x1 * *x + m_x2 * *x * *x;
	*y = m_y0 + m_y1 * *y + m_y2 * *y * *y;
}


void CPolynom2Transform::xMsg(char* msg)
{
	VERIFY(sprintf(msg, "x-transform: x' = %f + %f*x + %f*x^2", m_x0, m_x1, m_x2) > 0);
}


void CPolynom2Transform::yMsg(char* msg)
{
	VERIFY(sprintf(msg, "y-transform: y' = %f + %f*y + %f*y^2", m_y0, m_y1, m_y2) > 0);
}


CbiquadTransform::CbiquadTransform(float* message)
{
	m_a0 = *message++;
	m_b0 = *message++;
	m_a1 = *message++;
	m_b1 = *message++;
	m_a2 = *message++;
	m_b2 = *message++;
	m_a3 = *message++;
	m_b3 = *message++;
	m_a4 = *message++;
	m_b4 = *message++;
	m_a5 = *message++;
	m_b5 = *message;
}

/*
CPolynom2Transform::~CPolynom2Transform()
{
}
*/


void CbiquadTransform::Apply(float* x, float* y)
{
	float xa, ya;
	xa = m_a0 + m_a1 * *x + m_a2 * *y + m_a3 * *x * *y + m_a4 * *x * *x + m_a5 * *y * *y;
	ya = m_b0 + m_b1 * *x + m_b2 * *y + m_b3 * *x * *y + m_b4 * *x * *x + m_b5 * *y * *y;
	*x = xa;
	*y = ya;
}


void CbiquadTransform::xMsg(char* msg)
{
	VERIFY(sprintf(msg, "x-transform: x' = %f + %f*x + %f*y + %f*x*y + %f*x^2 + %f*y^2", m_a0, m_a1, m_a2, m_a3, m_a4, m_a5) > 0);
}


void CbiquadTransform::yMsg(char* msg)
{
	VERIFY(sprintf(msg, "y-transform: y' = %f + %f*x + %f*y + %f*x*y + %f*x^2 + %f*y^2", m_b0, m_b1, m_b2, m_b3, m_b4, m_b5) > 0);
}
