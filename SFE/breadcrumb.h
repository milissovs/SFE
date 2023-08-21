#ifndef BREADCRUMB_H_INCLUDED
#define BREADCRUMB_H_INCLUDED

#include <commctrl.h>
#include <d2d1.h>
#include <dwrite.h>
//#include <uxtheme.h>
#include "button.h"
#include <vector>
#include <string>

#define DELEMITER L"\\"
#define CHEVRON_WIDTH 15
#define MIN_CRUMB_WIDTH 20
#define MODE_CRUMB 0
#define MODE_EDIT  1

struct SELECT_TEXT_RANGE
{
	UINT32 startPosition = 0;
	INT32  length = 0;
};

typedef struct _BREAD_CRUMB_ITEM_
{
	int nID = 0;
	float fWidth = 0;
	std::wstring text = TEXT("");
	IDWriteTextLayout* m_pDWriteLayout = NULL;
	DWRITE_TEXT_METRICS tm{};
	std::vector<std::wstring> txt_special;
	BOOL bHasChevron{ TRUE };
	int nHoveredType = 0;
	BOOL bInvalidate = FALSE;
} BREAD_CRUMB_ITEM;

class CBreadCrumb
{
public:
	CBreadCrumb();
	~CBreadCrumb();

	HWND m_hWnd;
	HCURSOR hCursorIBeam;
	HCURSOR hCursorArrow;

	virtual LRESULT  WindowProcedure(HWND, UINT, WPARAM, LPARAM);
	virtual HWND     Create(HWND hWndParent, HINSTANCE hInstance, LPVOID lpParam);

	virtual int      OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct);
	virtual void     OnSize(UINT nType, UINT nWidth, UINT nHeight);
	virtual void     OnMouseMove(UINT nFlag, POINT pt);
	virtual void     OnLButtonDown(UINT nFlag, POINT pt);
	virtual void     OnRButtonDown(UINT nFlag, POINT pt);
	virtual void     OnContextMenu(POINT pt);
	virtual void     OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void     OnKeyDown(WPARAM wParam, LPARAM lParam);

	void             SetPath(LPCTSTR lpszPath);
	int              FindNextCrumBarPos(const std::wstring& path, int iStart);
	void             InitVisibleCrumbs();
	void             ClearVisbleCrumbs();
	void             ClearHoverCrumbs();
	BREAD_CRUMB_ITEM InitCrumb(std::wstring text, int nID, BOOL bHasChevron = TRUE);

	void             DrawCrumb(ID2D1HwndRenderTarget* pRT, BREAD_CRUMB_ITEM& bci, float fStart, float fHeight);
	void             DrawEdit(ID2D1HwndRenderTarget* pRT, POINT pt);
	void             DrawSelection(ID2D1HwndRenderTarget* pRT, IDWriteTextLayout* pTL);
	void             DrawCaret(ID2D1HwndRenderTarget* pRT, IDWriteTextLayout* pTL);

	int              HiTest(D2D1_POINT_2F point);
	void             SetInvalidVisibleCrumb(int nID);

	void             SetMode(BOOL bMode);
	BOOL             GetMode() { return m_bMode; };

private:
	void             SetTxtRange(UINT nFlag, POINT pt);
	void             GreateTextLayout();
	SELECT_TEXT_RANGE NormalizeSelection();

protected:
	virtual HRESULT OnPaint();
	virtual HRESULT OnPaint2D(ID2D1HwndRenderTarget* pRT, HDC hDC);
	virtual HRESULT CreateDeviceIndependentResources();
	virtual HRESULT CreateDeviceDependentResources();
	virtual void    DiscardDeviceDependentResources();

	float m_fScale;
	INT  m_nHeight;
	ID2D1SolidColorBrush*  m_pBr0;
	ID2D1SolidColorBrush*  m_pBr1;
	ID2D1SolidColorBrush*  m_pBrBlack;
	ID2D1SolidColorBrush*  m_pBrBorder;
	ID2D1SolidColorBrush*  m_pBrFill;
	ID2D1SolidColorBrush*  m_pBrFill2;
	ID2D1SolidColorBrush*  m_pBrSelection;

protected:
	ID2D1Factory*          m_pDirect2dFactory;
	ID2D1HwndRenderTarget* m_pRenderTarget;
	IDWriteFactory*        m_pDWriteFactory;
	IDWriteTextFormat*     m_pTFC;
	IDWriteTextFormat*     m_pTFL;
	IDWriteInlineObject*   m_spInlineObjec_TFC;
	IDWriteTextLayout*     m_txtLayout;
	DWRITE_TRIMMING trimming = { DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0 };
	SELECT_TEXT_RANGE      m_txt_range;

private:
	std::wstring                  m_path;
	std::wstring                  m_path_backup;
	std::vector<std::wstring>     m_path_undo;
	BOOL                          m_bMouseTracking;
	std::vector<std::wstring>     m_crumbs;
	std::vector<BREAD_CRUMB_ITEM> m_visible_crumbs;
	int                           m_nLastHoverdCrumb;
	BOOL                          m_bMode;
	BOOL                          m_bLButtonDown;
	BOOL                          m_bDragging;
	HWND                          m_hOldFocus;
	int                           m_nCaretPos;
};

#endif // BUTTON_H_INCLUDED
