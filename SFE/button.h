#ifndef BUTTON_H_INCLUDED
#define BUTTON_H_INCLUDED

#include <commctrl.h>
#include <d2d1.h>
#include <dwrite.h>

#define BS_NORMAL       0x0000
#define BS_HOVERED      0x0001
#define BS_HOVERED_DROP 0x0002
#define BS_CHECKED      0x0004
#define BS_DISABLED     0x0008

#define clrBORDER         0x0078D4
#define clrBORDER_GLOW    0xACD3F1
#define clrBORDER_CHECKED 0x005499
#define clrBKG            0xE0EEF9
#define clrBKG_CHECKED    0xCCE4F7
#define clrTXT            0x000000
#define clrTXT_DISABLED   0x212121
#define clrTXT_CHECKED    0x72D2EE

class CButton
{
	CButton();
	CButton(UINT nID);
	~CButton();

	void Draw(ID2D1HwndRenderTarget * pRT);

	DWORD GetState() { return m_nState; };
	FLOAT GetWidth() { return m_rect.right - m_rect.left + 4; };
	FLOAT GetHeight() { return m_rect.bottom - m_rect.top + 4; };

	void SetState(DWORD nState);
	void SetText(LPCTSTR lpzText);
	void SetRect(RECT rc);
	void InvalidateDependantResources() { m_bValidDependentResurces = FALSE; };

private:
	UINT m_nID;
	D2D1_RECT_F m_rect;
	D2D1_ROUNDED_RECT m_rrect;
	DWORD m_nState;
	const TCHAR* m_sText;
	ID2D1SolidColorBrush* m_pBr0;
	ID2D1SolidColorBrush* m_pBr1;
	BOOL m_bValidDependentResurces;

	void DrawBorder(ID2D1HwndRenderTarget* pRT);
	void CrateDependantRecources(ID2D1HwndRenderTarget* pRT);

};

#endif // BUTTON_H_INCLUDED
