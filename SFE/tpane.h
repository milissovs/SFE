#ifndef TPANE_H_INCLUDED
#define TPANE_H_INCLUDED

#include <commctrl.h>
#include <d2d1.h>
#include <dwrite.h>
#include <uxtheme.h>

class CTPane
{
public:
	CTPane();
	~CTPane();

	virtual LRESULT WindowProcedure(HWND, UINT, WPARAM, LPARAM);

	virtual HWND Create(HWND hWndParent, HINSTANCE hInstance, LPVOID lpParam);

	void SetHeight(int nHeight);
	int  GetHeight() { return (int)(m_nHeight); };

	virtual int      OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct);
	virtual void     OnSize(UINT nType, UINT nWidth, UINT nHeight);

	HWND m_hWnd;
	HTHEME m_hTheme;

protected:
	virtual HRESULT OnPaint();
	virtual HRESULT OnPaint2D(ID2D1HwndRenderTarget *pRT, HDC hDC);
	virtual HRESULT CreateDeviceIndependentResources();
	virtual HRESULT CreateDeviceDependentResources();
	virtual void    DiscardDeviceDependentResources();

private:
	float m_fScale;
	INT  m_nHeight;
	ID2D1SolidColorBrush* m_pBr0;
	ID2D1SolidColorBrush* m_pBr1;

protected:
	ID2D1Factory* m_pDirect2dFactory;
	ID2D1HwndRenderTarget* m_pRenderTarget;

};

#endif // TPANE_H_INCLUDED
