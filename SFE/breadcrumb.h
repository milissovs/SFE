#ifndef BREADCRUMB_H_INCLUDED
#define BREADCRUMB_H_INCLUDED

#include <commctrl.h>
#include <d2d1.h>
#include <dwrite.h>
#include <uxtheme.h>
#include "button.h"
#include <vector>

class CBreadCrumb
{
public:
	CBreadCrumb();
	~CBreadCrumb();

	virtual LRESULT WindowProcedure(HWND, UINT, WPARAM, LPARAM);
	virtual HWND Create(HWND hWndParent, HINSTANCE hInstance, LPVOID lpParam);

	virtual int      OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct);
	virtual void     OnSize(UINT nType, UINT nWidth, UINT nHeight);
	virtual void     OnMouseMove(UINT nFlag, POINT pt);

	HWND m_hWnd;

protected:
	virtual HRESULT OnPaint();
	virtual HRESULT OnPaint2D(ID2D1HwndRenderTarget* pRT, HDC hDC);
	virtual HRESULT CreateDeviceIndependentResources();
	virtual HRESULT CreateDeviceDependentResources();
	virtual void    DiscardDeviceDependentResources();

	float m_fScale;
	INT  m_nHeight;
	ID2D1SolidColorBrush* m_pBr0;
	ID2D1SolidColorBrush* m_pBr1;

protected:
	ID2D1Factory*          m_pDirect2dFactory;
	ID2D1HwndRenderTarget* m_pRenderTarget;
	IDWriteFactory*        m_pDWriteFactory;
	IDWriteTextFormat*     m_pTFC;
	IDWriteInlineObject*   m_spInlineObjec_TFC;
	DWRITE_TRIMMING trimming = { DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0 };

private:
	BOOL     m_bMouseTracking;
};

#endif // BUTTON_H_INCLUDED
