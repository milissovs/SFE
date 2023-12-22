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
	, m_fWidth(24.0f)
	, m_fHeight(24.0f)
	, m_bHasChevron(FALSE)
{

}

CButton::CButton(UINT nID, BOOL bHasChevron)
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
	, m_fWidth(24.0f)
	, m_fHeight(24.0f)
	, m_bHasChevron(bHasChevron)
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
	m_rect.right = m_rect.left + m_fWidth;
	m_rect.bottom = m_rect.top + m_fHeight;

	m_rrect.radiusX = 2;
	m_rrect.radiusY = 2;
	m_rrect.rect = m_rect;
}


FLOAT CButton::GetWidth()
{
	if (m_bHasChevron)
		return m_rect.right - m_rect.left + 4 + DROP_WIDTH;
	return m_rect.right - m_rect.left + 4;
}

void CButton::SetChevron(BOOL bHasCevron)
{
	m_bHasChevron = bHasCevron;
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
		D2D1_ROUNDED_RECT rr = m_rrect;
		if (m_bHasChevron)
			rr.rect.right += DROP_WIDTH;

		pRT->DrawRoundedRectangle(&rr, m_pBrText1, 1);

		D2D1_POINT_2F pt0 = { m_rect.right, m_rect.top + 3 };
		D2D1_POINT_2F pt1 = { m_rect.right, m_rect.bottom - 3 };
		pRT->DrawLine(pt0, pt1, m_pBrText1, 1);

		return;
	}

	if (m_nState & BS_HOVERED)
	{
		D2D1_ROUNDED_RECT rr = m_rrect;
		if (m_bHasChevron)
			rr.rect.right += DROP_WIDTH;

		//pRT->DrawRoundedRectangle(&m_rrect, m_pBrBorder1, 3);
		pRT->DrawRoundedRectangle(&rr, m_pBrBorder0, 1);

		D2D1_POINT_2F pt0 = { m_rect.right, m_rect.top + 3 };
		D2D1_POINT_2F pt1 = { m_rect.right, m_rect.bottom - 3 };
		pRT->DrawLine(pt0, pt1, m_pBrText1, 1);
	}
}

void CButton::DrawText(ID2D1HwndRenderTarget* pRT)
{
	if (m_bSeparator)
		return;

	if (m_nState & BS_DISABLED)
	{
		pRT->DrawText(m_sText, lstrlen(m_sText), m_pTF, &m_rect, m_pBrText1);
		if (m_bHasChevron)
		{
			D2D1_RECT_F rr = m_rect;
			rr.left = m_rect.right;
			rr.right = rr.left + DROP_WIDTH;
			pRT->DrawText(L"▾", lstrlen(L"▾"), m_pTF, &rr, m_pBrText1);
		}
	}
	else
	{
		pRT->DrawText(m_sText, lstrlen(m_sText), m_pTF, &m_rect, m_pBrText0);
		if (m_bHasChevron)
		{
			D2D1_RECT_F rr = m_rect;
			rr.left = m_rect.right;
			rr.right = rr.left + DROP_WIDTH;
			pRT->DrawText(L"▾", lstrlen(L"▾"), m_pTF, &rr, m_pBrText1);
		}
	}
}

BOOL CButton::IsMouseOver(POINT pt)
{
	RECT rc = m_rc;
	if (m_bHasChevron)
		rc.right += 10;
	return PtInRect(&rc, pt);
}

BOOL CButton::IsMouseOverDrop(POINT pt)
{
	RECT rc = m_rc;
	rc.left = rc.right;
	if (m_bHasChevron)
		rc.right += 10;
	return PtInRect(&rc, pt);
}