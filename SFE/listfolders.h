#ifndef LISTFOLDERS_H_INCLUDED
#define LISTFOLDERS_H_INCLUDED

#include <commctrl.h>
#include <d2d1.h>
#include <dwrite.h>
#include <vector>
#include "drag_drop.h"
#include "folder_item.h"

class CListFolders
{
public:
	CListFolders();
	~CListFolders();

	LRESULT  WindowProcedure(HWND, UINT, WPARAM, LPARAM);
	HWND     Create(HWND hWndParent, HINSTANCE hInstance, LPVOID lpParam);

	int  GetWidth() { return m_nWidth; };
	void SetWidth(int nWidth) { m_nWidth = nWidth; };
	int  GetPosition() { return m_nPosition; };
	void SetPosition(int nPosition) { m_nPosition = nPosition; };

	void OnSize(UINT nType, UINT nWidth, UINT nHeight);
	void OnVScroll(UINT nSBCode, UINT nPos, HWND hScroll);
	void OnMouseWheel(UINT nFlags, short zDelta, POINT pt);
	void OnLButtonDown(UINT nFlags, POINT point);
	void OnLButtonDblClk(UINT nFlags, POINT point);
	void OnMouseMove(UINT nFlags, POINT point);

	HANDLE AddFolder(LPCTSTR lpszText, HANDLE lParent = NULL);
	void InsertFolder(FOLDER_ITEM* pfi, size_t at);
	void RemoveFolder(size_t at);

	void GetSelectedItems(std::vector<size_t>& selected);

	long GetVirtualHeight();
	long GetTopItem();
	int  GetScrollPos32(int nBar, BOOL bGetTrackPos = FALSE);
	BOOL SetScrollPos32(int nBar, int nPos, BOOL bRedraw = TRUE);
	void ResetScrollBar();
	UINT AddColor(ID2D1HwndRenderTarget* pRT, COLORREF color);
	SELECTED_ITEM HitTest(POINT point);
	void          HiTestItem(long nItem, std::vector <FOLDER_ITEM> items, float& fOffset, D2D1_POINT_2F& fpt, SELECTED_ITEM &si, WORD nLevel = 0);
	HGLOBAL       CopyItem(LONG nItem);
	void          SelectItem(SELECTED_ITEM item, std::vector <FOLDER_ITEM>& items);

	HWND m_hWnd;
	int  m_nWidth;
	BOOL m_bLButtonDown;
	BOOL m_bDraging;
	BOOL m_bNCLButtonDown;

	float m_fScale;
	int   m_nPosition;
	//long  m_nLastItemHover;
	//WORD  m_nLastPartHover;
	//WORD  m_nLastLevelHover;
	BOOL  m_bMouseTracking;
	POINT m_pointLbuttonDown;
	SELECTED_ITEM m_hovered;

	//LONG_PTR oldListWndProc;

private:
	ID2D1Factory*             m_pDirect2dFactory;
	ID2D1HwndRenderTarget*    m_pRenderTarget;
	IDWriteFactory*           m_pDWriteFactory;
	ID2D1SolidColorBrush*     m_pBr1;
	ID2D1SolidColorBrush*     m_pBr2;
	ID2D1SolidColorBrush*     m_pBrRed;
	IDWriteTextFormat*        m_pTFL;
	IDWriteTextFormat*        m_pTFLB;
	IDWriteInlineObject*      m_spInlineObjec_TFL;
	IDWriteInlineObject*      m_spInlineObjec_TFLB;
	ID2D1LinearGradientBrush* m_plgbBorder;
	ID2D1LinearGradientBrush* m_plgbHover;


	std::vector<ID2D1LinearGradientBrush*> m_colors_normal;
	std::vector<ID2D1LinearGradientBrush*> m_colors_pressed;

private:
	// Initialize device-independent resources.
	HRESULT CreateDeviceIndependentResources();

	// Initialize device-dependent resources.
	HRESULT CreateDeviceDependentResources();

	HRESULT OnPaint2D();
	void    DrawItems(ID2D1HwndRenderTarget *pRT, D2D1_RECT_F & rect);
	void    DrawItem(ID2D1HwndRenderTarget* pRT, D2D1_RECT_F& rect, D2D1_RECT_F& rc, std::vector <FOLDER_ITEM> m_items, SIZE_T nItem, D2D1_POINT_2F &ptMark0, D2D1_POINT_2F &ptMark1, UINT nLevel = 0);

	// Release device-dependent resource.
	void DiscardDeviceDependentResources();

private:
	std::vector <FOLDER_ITEM> m_items;
	int m_nVScrollMax;
	IDropTarget* pDropTarget;
};

#endif // LISTFOLDERS_H_INCLUDED
