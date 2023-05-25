#include <windows.h>
#include <windowsx.h>
#include "button.h"
#include "globals.h"
#include "resource.h"

CButton::CButton()
	: m_nID(0)
	, m_rc()
	, m_rect()
	, m_rrect()
	, m_nState(0)
	, m_sText(L"")
	, m_sTooltip(L"")
	, m_pBrBorder0(NULL)
	, m_pBrBorder1(NULL)
	, m_pBrText0(NULL)
	, m_pBrText1(NULL)
	, m_pTF(NULL)
	, m_bSeparator(FALSE)
{

}

CButton::CButton(UINT nID)
	: m_nID(nID)
	, m_rc()
	, m_rect()
	, m_rrect()
	, m_nState(0)
	, m_sText(L"")
	, m_sTooltip(L"")
	, m_pBrBorder0(NULL)
	, m_pBrBorder1(NULL)
	, m_pBrText0(NULL)
	, m_pBrText1(NULL)
	, m_pTF(NULL)
	, m_bSeparator(FALSE)
{

}

CButton::~CButton()
{

}

void CButton::InvalidateDependantResources(
	ID2D1HwndRenderTarget* pRT,
	ID2D1SolidColorBrush* pBrBorder0,
	ID2D1SolidColorBrush* pBrBorder1,
	ID2D1SolidColorBrush* pBrText0,
	ID2D1SolidColorBrush* pBrText1,
	IDWriteTextFormat* pTF)
{
	m_pBrBorder0 = pBrBorder0;
	m_pBrBorder1 = pBrBorder1;
	m_pBrText0 = pBrText0;
	m_pBrText1 = pBrText1;

	m_pTF = pTF;
}

void CButton::Draw(ID2D1HwndRenderTarget* pRT)
{

	// Draw background
	// ...

	// Draw border
	DrawBorder(pRT);

	// Draw text / image
	DrawText(pRT);
}

void CButton::SetRect(RECT rc)
{
	m_rc = rc;
	m_rect.left = (float)rc.left + 1.5f;
	m_rect.top = (float)rc.top + 1.5f;
	m_rect.right = m_rect.left + 24.0f;
	m_rect.bottom = m_rect.top + 24.0f;

	m_rrect.radiusX = 2;
	m_rrect.radiusY = 2;
	m_rrect.rect = m_rect;
}

void CButton::DrawBorder(ID2D1HwndRenderTarget* pRT)
{
	if (m_bSeparator)
	{
		D2D1_POINT_2F pt0 = { (m_rect.right - m_rect.left) / 2, m_rect.top };
		D2D1_POINT_2F pt1 = { (m_rect.right - m_rect.left) / 2, m_rect.bottom };
		pRT->DrawLine(pt0, pt1, m_pBrText1, 2);
		return;
	}

	if (m_nState & BS_DISABLED && m_nState & BS_HOVERED)
	{
		pRT->DrawRoundedRectangle(&m_rrect, m_pBrText1, 1);
		return;
	}

	if (m_nState & BS_HOVERED)
	{
		//pRT->DrawRoundedRectangle(&m_rrect, m_pBrBorder1, 3);
		pRT->DrawRoundedRectangle(&m_rrect, m_pBrBorder0, 1);
	}
}

void CButton::DrawText(ID2D1HwndRenderTarget* pRT)
{
	if (m_bSeparator)
		return;

	if (m_nState & BS_DISABLED)
	{
		pRT->DrawText(m_sText, lstrlen(m_sText), m_pTF, &m_rect, m_pBrText1);
	}
	else
	{
		pRT->DrawText(m_sText, lstrlen(m_sText), m_pTF, &m_rect, m_pBrText0);
	}
}

BOOL CButton::IsMouseOver(POINT pt)
{
	return PtInRect(&m_rc, pt);
}