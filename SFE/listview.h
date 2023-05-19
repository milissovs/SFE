#ifndef LISTVIEW_H_INCLUDED
#define LISTVIEW_H_INCLUDED

#include <commctrl.h>
#include <d2d1.h>
#include <dwrite.h>

class CListView
{
public:
	CListView();
	~CListView();

	HWND     Create(HWND hWndParent, HINSTANCE hInstance, LPVOID lpParam);
	LRESULT  WindowProcedure(HWND, UINT, WPARAM, LPARAM);

	void     OnSize(UINT nType, UINT nWidth, UINT nHeight);

	HWND m_hWnd;

protected:
	void OnPaint();
	virtual HRESULT OnPaint2D(ID2D1HwndRenderTarget* pRT);
	virtual HRESULT CreateDeviceIndependentResources();
	virtual HRESULT CreateDeviceDependentResources();
	virtual void    DiscardDeviceDependentResources();

private:
	LONG_PTR oldListWndProc;
	float m_fScale;

protected:
	ID2D1Factory* m_pDirect2dFactory;
	ID2D1HwndRenderTarget* m_pRenderTarget;

	ID2D1SolidColorBrush* m_pBr0;
	ID2D1SolidColorBrush* m_pBr1;

};

#endif // LISTVIEW_H_INCLUDED
