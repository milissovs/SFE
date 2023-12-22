#ifndef BTNPANE_H_INCLUDED
#define BTNPANE_H_INCLUDED

#include <commctrl.h>
#include <d2d1.h>
#include <dwrite.h>
#include "button.h"

class CBTNPane
{
public:
	CBTNPane();
	~CBTNPane();

	LRESULT WindowProcedure(HWND, UINT, WPARAM, LPARAM);
	HWND Create(HWND hWndParent, HINSTANCE hInstance, LPVOID lpParam);
	HWND CreateTrackingToolTip(int toolID, HWND hWndParent, HINSTANCE hInst);

	int  OnCreate(HWND hWnd, LPCREATESTRUCT lpCS);
	void OnSize(UINT nType, UINT nWidth, UINT nHeight);
	void OnLButtonDown(POINT pt);
	void OnMouseMove(UINT nFlag, POINT pt);

	int GetHoveredButton();
	int GetWidth();

	HWND m_hWnd;
	HWND m_hWndTT;

private:
	FLOAT    m_fScale;
	CButton  m_btns[3];
	BOOL     m_bMouseTracking;
	int      m_nHoveredButton;
	TOOLINFO m_TI;

private:
	HRESULT OnPaint();
	HRESULT OnPaint2D(ID2D1HwndRenderTarget* pRT);
	HRESULT CreateDeviceIndependentResources();
	HRESULT CreateDeviceDependentResources();
	void    DiscardDeviceDependentResources();

	ID2D1Factory*          m_pDirect2dFactory;
	ID2D1HwndRenderTarget* m_pRenderTarget;
	IDWriteFactory*        m_pDWriteFactory;
	ID2D1SolidColorBrush*  m_pBrBorder0;
	ID2D1SolidColorBrush*  m_pBrBorder1;
	ID2D1SolidColorBrush*  m_pBrText0;
	ID2D1SolidColorBrush*  m_pBrText1;
	IDWriteTextFormat*     m_pTFC;
	IDWriteInlineObject*   m_spInlineObjec_TFC;
	DWRITE_TRIMMING trimming = { DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0 };
};

#endif // BTNPANE_H_INCLUDED
