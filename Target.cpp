// Target.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "EyeLinkServer.h"
#include "Target.h"
#include <eyelink.h>
#define _USE_MATH_DEFINES TRUE
#include <math.h>


// CTarget

CTarget::CTarget(void)
	: m_OutOnBlink(false)
	, m_wasInside(false)
	, m_isDrawn(false)
	, m_inclusive(false)
{
}

void CTarget::Init(CString name, float position[2])
{
	m_name = name;
	m_position[0] = position[0];
	m_position[1] = position[1];
	VERIFY(m_hEnter = CreateEvent(
		NULL,
		TRUE,	// the former Matlab EyeServer used manual reset events
		FALSE,
		name+L"In"));
	VERIFY(m_hLeave = CreateEvent(
		NULL,
		TRUE,	// the former Matlab EyeServer used manual reset events
		FALSE,
		name+L"Out"));
}

CTarget::~CTarget()
{
	VERIFY(CloseHandle(m_hLeave));
	VERIFY(CloseHandle(m_hEnter));
}


// CTarget-Memberfunktionen

void CTarget::FixationUpdate(float x, float y)
{
	bool isInside = Inside(x, y);
	if (isInside != m_wasInside)
	{
		if (!m_inclusive) ResetEvent(isInside ? m_hLeave : m_hEnter);
		SetEvent(isInside ? m_hEnter : m_hLeave);
		m_wasInside = isInside;
	}
}


// CCircle

CCircle::CCircle(CString name, float position[2], float radius)
{
	CTarget::Init(name, position);
	m_radius = radius;
	if (CEyeLinkServerApp::m_eyeLinkIsConnected && (eyelink_current_mode() == IN_IDLE_MODE))
	{
		DrawEyelink(15);
		m_isDrawn = true;
	}
}

CCircle::~CCircle()
{
	TRACE("CCircle Destruktor\n");
	if (CEyeLinkServerApp::m_eyeLinkIsConnected && (eyelink_current_mode() == IN_IDLE_MODE) && m_isDrawn) DrawEyelink(7);
}


// CCircle-Memberfunktionen
bool CCircle::Inside(float x, float y)
{
	return sqrt((x-m_position[0])*(x-m_position[0]) + (y-m_position[1])*(y-m_position[1])) <= m_radius;
}

/*
void CCircle::DrawEyelink(BYTE color)
{
	short x1 = (short)(m_position[0] - m_radius);
	short y1 = (short)(m_position[1] - m_radius);
	short x2 = (short)(m_position[0] + m_radius);
	short y2 = (short)(m_position[1] + m_radius);
	char cmd[36];
	VERIFY(-1 != sprintf(&cmd[0], "draw_box %d %d %d %d %u", x1, y1, x2, y2, color));
	theApp.EyelinkCommand(cmd);
}
*/

void CCircle::DrawEyelink(BYTE color)
{
	short x1 = (short)(m_position[0] + m_radius);
	short y1 = (short)(m_position[1]);
	short x2;
	short y2;
	char cmd[37];
	for (BYTE i = 1; i <= 8; i++)
	{
		float phi = i * M_PI_4;
		x2 = (short) m_position[0] + m_radius * cosf(phi);
		y2 = (short) m_position[1] + m_radius * sinf(phi);
		VERIFY(-1 != sprintf(&cmd[0], "draw_line %d %d %d %d %u", x1, y1, x2, y2, color));
		theApp.EyelinkCommand(cmd);
		x1 = x2;
		y1 = y2;
	}
}


void CCircle::Draw(CHwndRenderTarget* pRenderTarget, CD2DSolidColorBrush* pBrush)
{
	pRenderTarget->DrawEllipse(CD2DEllipse(D2D1::Point2F(m_position[0], m_position[1]),
		D2D1::SizeF(m_radius, m_radius)),
		pBrush, 2.5);
}
