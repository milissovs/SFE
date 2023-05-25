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
#define clrTXT_DISABLED   0xAAAAAA
#define clrTXT_CHECKED    0x72D2EE

class CButton
{
public:
	CButton();
	CButton(UINT nID);
	~CButton();

	void Draw(ID2D1HwndRenderTarget * pRT);

	DWORD GetState() { return m_nState; };
	FLOAT GetWidth() { return m_rect.right - m_rect.left + 4; };
	FLOAT GetHeight() { return m_rect.bottom - m_rect.top + 4; };
	RECT  GetRect() { return m_rc; };
	LPCTSTR GetTooltip() { return m_sTooltip; };
	BOOL  IsMouseOver(POINT pt);

	void SetStateNormal(BOOL bEnable = TRUE) { if (bEnable) m_nState = BS_NORMAL; };
	void SetStateDisabled(BOOL bEnable = TRUE) { if (bEnable) m_nState |= BS_DISABLED; else m_nState &= ~BS_DISABLED; };
	void SetStateHovered(BOOL bEnable = TRUE) { if (bEnable) m_nState |= BS_HOVERED; else m_nState &= ~BS_HOVERED; };
	void SetStateHoveredDrop(BOOL bEnable = TRUE) { if (bEnable) m_nState |= BS_HOVERED_DROP; else m_nState &= ~BS_HOVERED_DROP; };
	void SetStateChecked(BOOL bEnable = TRUE) { if (bEnable) m_nState |= BS_CHECKED; else m_nState &= ~BS_CHECKED; };

	void SetTooltip(LPCTSTR lpzTooltipText) { m_sTooltip = lpzTooltipText; };
	void SetText(LPCTSTR lpzText) { m_sText = lpzText; };
	void SetRect(RECT rc);
	void InvalidateDependantResources(
		ID2D1HwndRenderTarget* pRT,
		ID2D1SolidColorBrush* pBrBorder0,
		ID2D1SolidColorBrush* pBrBorder1,
		ID2D1SolidColorBrush* pBrText0,
		ID2D1SolidColorBrush* pBrText1,
		IDWriteTextFormat* pTF);

	void SetSeparator() { m_bSeparator = TRUE; };

private:
	UINT m_nID;
	RECT m_rc;
	D2D1_RECT_F m_rect;
	D2D1_ROUNDED_RECT m_rrect;
	DWORD m_nState;
	const TCHAR* m_sText;
	const TCHAR* m_sTooltip;
	ID2D1SolidColorBrush* m_pBrBorder0;
	ID2D1SolidColorBrush* m_pBrBorder1;
	ID2D1SolidColorBrush* m_pBrText0;
	ID2D1SolidColorBrush* m_pBrText1;
	IDWriteTextFormat*    m_pTF;
	BOOL m_bSeparator;

	void DrawBorder(ID2D1HwndRenderTarget* pRT);
	void DrawText(ID2D1HwndRenderTarget* pRT);

};

#endif // BUTTON_H_INCLUDED
