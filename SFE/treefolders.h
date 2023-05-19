#ifndef TREEFOLDERS_H_INCLUDED
#define TREEFOLDERS_H_INCLUDED

#include <commctrl.h>
#include <d2d1.h>

class CTreeFolders
{
public:
	CTreeFolders();
	~CTreeFolders();

	LRESULT WindowProcedure(HWND, UINT, WPARAM, LPARAM);

	HWND      Create(HWND hWndParent, HINSTANCE hInstance, LPVOID lpParam);
	HTREEITEM AddFolder(HWND hWndTV, LPCTSTR lpszItem, int nLevel);
	BOOL      InitTreeViewImageLists(HWND hwnd);

	int GetWidth() { return m_nWidth; };
	void SetWidth(int nWidth) { m_nWidth = nWidth; };
	int GetPosition() { return m_nPosition; };
	void SetPosition(int nPosition) { m_nPosition = nPosition; };

	void     OnSize(UINT nType, UINT nWidth, UINT nHeight);
	void     OnNcPaint(WPARAM wParam);

	HWND m_hWnd;
	int  m_nWidth;
	BOOL m_bLButtonDown;
	BOOL m_bNCLButtonDown;

	LONG_PTR oldTreeWndProc;

private:
	ID2D1Factory*          m_pDirect2dFactory;
	ID2D1HwndRenderTarget* m_pRenderTarget;
	ID2D1SolidColorBrush*  m_pBr1;
	ID2D1SolidColorBrush*  m_pBr2;

	float m_fScale;
	int m_nPosition;

private:
	// Initialize device-independent resources.
	HRESULT CreateDeviceIndependentResources();

	// Initialize device-dependent resources.
	HRESULT CreateDeviceDependentResources();

	HRESULT OnPaint2D();

	// Release device-dependent resource.
	void DiscardDeviceDependentResources();
};

#endif // TREEFOLDERS_H_INCLUDED
