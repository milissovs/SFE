#ifndef PANEHEADER_H_INCLUDED
#define PANEHEADER_H_INCLUDED

#include <commctrl.h>
#include <d2d1.h>
//#include <d2d1_1.h>
#include <dwrite.h>

class CPaneHeader
{
private:
	typedef struct _BUTTON_
	{
		const TCHAR* title = L"";
		DWORD state = 0;
		RECT rect = {};
		D2D1_ROUNDED_RECT rr_outer;
		D2D1_ROUNDED_RECT rr_middle;
		D2D1_ROUNDED_RECT rr_innner;

		void SetRect(RECT rc)
		{
			rect = rc;

			rr_middle.rect.left = (float)rect.left;
			rr_middle.rect.top = (float)rect.top;
			rr_middle.rect.right = (float)rect.right;
			rr_middle.rect.bottom = (float)rect.bottom;
			rr_middle.radiusX = 2;
			rr_middle.radiusY = 2;

			rr_outer = rr_middle;
			rr_outer.rect.left += 0.75f;
			rr_outer.rect.top += 0.75f;
			rr_outer.rect.right -= 0.75f;
			rr_outer.rect.bottom -= 0.75f;

			rr_innner = rr_middle;
			rr_innner.rect.left -= 0.75f;
			rr_innner.rect.top -= 0.75f;
			rr_innner.rect.right += 0.75f;
			rr_innner.rect.bottom += 0.75f;
		};

	}BUTTON;

public:
	CPaneHeader();
	~CPaneHeader();

	LRESULT WindowProcedure(HWND, UINT, WPARAM, LPARAM);
	HWND    Create(HWND hWndParent, HINSTANCE hInstance, LPVOID lpParam);

	void CalculateLayout(RECT rc);
	void SetPosition(int nPosition);

	void     OnMouseMove(UINT flag, POINT pt);
	void     OnSize(UINT nType, UINT nWidth, UINT nHeight);
	void     OnLButtonDown(UINT nType, UINT nX, UINT nY);

	HWND m_hWnd;
	int  m_nPanePos;

private:
	// Initialize device-independent resources.
	HRESULT CreateDeviceIndependentResources();
	
	// Initialize device-dependent resources.
	HRESULT CreateDeviceDependentResources();
	
	HRESULT OnPaint2D();
	
	// Release device-dependent resource.
	void DiscardDeviceDependentResources();
	
private:
	ID2D1Factory*          m_pDirect2dFactory;
	ID2D1HwndRenderTarget* m_pRenderTarget;
	ID2D1SolidColorBrush*  m_pBrBlack;
	ID2D1SolidColorBrush*  m_pBrGray127;
	ID2D1SolidColorBrush*  m_pBrGray033;
	ID2D1SolidColorBrush*  m_pBrGray196;
	ID2D1SolidColorBrush*  m_pBr1;
	ID2D1SolidColorBrush*  m_pBr2;
	IDWriteTextFormat*     m_pTFC;
	IDWriteTextFormat*     m_pTFL;
	IDWriteFactory*        m_pDWriteFactory;
	IDWriteInlineObject*   m_spInlineObjec_TFC;
	DWRITE_TRIMMING trimming = { DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0 };


private:
	float m_fScale;
	RECT rButtons[3];
	BUTTON m_buttons[3];
	int bMouseOverBtn;
	BOOL m_bMouseTracking;
};

#endif // PANEHEADER_H_INCLUDED
#pragma once
