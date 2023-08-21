#ifndef LISTFOLDERS_H_INCLUDED
#define LISTFOLDERS_H_INCLUDED

#include <commctrl.h>
#include <d2d1.h>
#include <dwrite.h>
#include <vector>
#include "drag_drop.h"
#include "folder_item.h"

typedef struct _DLG_LRESULT_INIT_
{
	HWND hParent{ NULL };
	std::wstring sTitle{};
	COLORREF clrBack{ 0 };
	COLORREF clrText{ 0 };
	RECT     rect{ 0, 0 };
} DLG_LRESULT_INIT;

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
	void OnRButtonDown(UINT nFlags, POINT point);
	void OnMouseMove(UINT nFlags, POINT point);
	void OnMouseHover(UINT nFlags, POINT point);

	void OnFolderProperties(HWND hWnd);
	void OnFolderRemove(HWND hWnd);
	void OnFolderAdd(HWND hWnd);

	HANDLE        AddFolder(LPCTSTR lpszText, HANDLE lParent = NULL);
	void          InsertFolder(CFolderItem* pfi, size_t at);
	void          RemoveFolder(size_t at);

	void          GetSelectedItems(std::vector<size_t>& selected);
	void          SetSelectedItem(CFolderItem* fi);
	HANDLE        GetFirstItem() { if (!m_root.HasChildren()) return NULL; return m_root.children.at(0); };
	HANDLE        GetSelectedItem();

	long          GetVirtualHeight();
	HANDLE        GetTopItem();
	int           GetScrollPos32(int nBar, BOOL bGetTrackPos = FALSE);
	BOOL          SetScrollPos32(int nBar, int nPos, BOOL bRedraw = TRUE);
	void          ResetScrollBar();
	UINT          AddColor(ID2D1HwndRenderTarget* pRT, COLORREF color, COLORREF tcolor = 0, size_t index = 0); // index = 0 - add new; index > 0 - update
	SELECTED_ITEM HitTest(POINT point);
	HANDLE        HiTestItem(CFolderItem* fi, float& fOffset, D2D1_POINT_2F& fpt, SELECTED_ITEM &si, WORD nLevel = 0);
	HGLOBAL       CopyItem(LONG nItem);
	BOOL          SetFolderPath(LPCTSTR lpszText);
	BOOL          Navigate(CFolderItem* fi);

	HWND CreateTrackingToolTip(int toolID, HWND hWndParent, HINSTANCE hInst);

	HWND m_hWnd;
	HWND m_hWndTT;
	HWND m_hWndProperties;
	HINSTANCE m_hInstance;

	int  m_nWidth;
	BOOL m_bLButtonDown;
	BOOL m_bDraging;
	BOOL m_bNCLButtonDown;

	float m_fScale;
	int   m_nPosition;
	BOOL  m_bMouseTracking;
	POINT m_pointLbuttonDown;
	SELECTED_ITEM m_hovered;
	SELECTED_ITEM m_selected;

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
	ID2D1LinearGradientBrush* m_plgbSelected;

	std::vector<ID2D1LinearGradientBrush*> m_colors_normal;
	std::vector<ID2D1LinearGradientBrush*> m_colors_pressed;
	std::vector<ID2D1SolidColorBrush*>     m_colors_brtext;
	std::vector<COLORREF>                  m_colors_back;
	std::vector<COLORREF>                  m_colors_text;

	DLG_LRESULT_INIT m_dlg_init;

private:
	// Initialize device-independent resources.
	HRESULT CreateDeviceIndependentResources();

	// Initialize device-dependent resources.
	HRESULT CreateDeviceDependentResources();

	HRESULT OnPaint2D();
	void    DrawItems(ID2D1HwndRenderTarget *pRT, D2D1_RECT_F & rect);
	void    DrawItem(ID2D1HwndRenderTarget* pRT, D2D1_RECT_F& rect, D2D1_RECT_F& rc, CFolderItem* Item, D2D1_POINT_2F &ptMark0, D2D1_POINT_2F &ptMark1);
	void    DrawItem_Text(ID2D1HwndRenderTarget* pRT, CFolderItem* fi, D2D1_RECT_F& rcBorder, D2D1_RECT_F& rc);
	void    DrawItem_LeftMark(ID2D1HwndRenderTarget* pRT, CFolderItem* fi, D2D1_RECT_F& rcBorder);
	void    DrawItem_RightMark(ID2D1HwndRenderTarget* pRT, CFolderItem* fi, D2D1_RECT_F& rcBorder);

	// Release device-dependent resource.
	void    DiscardDeviceDependentResources();

private:
	//folder_items m_items;
	int          m_nVScrollMax;
	IDropTarget* pDropTarget;
	CFolderItem  m_root;
	TOOLINFO     m_TI;
};

#endif // LISTFOLDERS_H_INCLUDED
