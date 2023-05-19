#include <windows.h>
#include <windowsx.h>
#include "button.h"
#include "globals.h"
#include "resource.h"

CButton::CButton()
	: m_nID(0)
	, m_rect()
	, m_rrect()
	, m_nState(0)
	, m_sText(L"")
	, m_pBr0(NULL)
	, m_pBr1(NULL)
	, m_bValidDependentResurces(FALSE)
{

}

CButton::CButton(UINT nID)
	: m_nID(nID)
	, m_rect()
	, m_rrect()
	, m_nState(0)
	, m_sText(L"")
	, m_pBr0(NULL)
	, m_pBr1(NULL)
	, m_bValidDependentResurces(FALSE)
{

}

CButton::~CButton()
{

}

void CButton::SetState(DWORD nState)
{
	m_nState = nState;
}

void CButton::SetText(LPCTSTR lpzText)
{
	m_sText = lpzText;
}

void CButton::CrateDependantRecources(ID2D1HwndRenderTarget* pRT)
{
	m_bValidDependentResurces = TRUE;

	pRT->CreateSolidColorBrush(D2D1::ColorF(clrBORDER), &m_pBr0);
	pRT->CreateSolidColorBrush(D2D1::ColorF(clrBORDER_GLOW), &m_pBr1);
}

void CButton::Draw(ID2D1HwndRenderTarget* pRT)
{
	if (!m_bValidDependentResurces)
		CrateDependantRecources(pRT);

	// Draw background
	// ...

	// Draw border
	DrawBorder(pRT);


	// Draw text / image
}

void CButton::SetRect(RECT rc)
{
	m_rect.left = rc.left + 2;
	m_rect.top = rc.top + 2;
	m_rect.right = m_rect.left + 24;
	m_rect.bottom = m_rect.top + 24;

	m_rrect.radiusX = 2;
	m_rrect.radiusY = 2;
	m_rrect.rect = { m_rect.left, m_rect.top, m_rect.right, m_rect.bottom };
}

void CButton::DrawBorder(ID2D1HwndRenderTarget* pRT)
{
	if (!m_nState)
		return;

	if (m_nState & BS_HOVERED)
	{
		pRT->DrawRoundedRectangle(&m_rrect, m_pBr1, 3);
		pRT->DrawRoundedRectangle(&m_rrect, m_pBr0, 1);
	}
}