#ifndef FOLDERITEM_H_INCLUDED
#define FOLDERITEM_H_INCLUDED

#include <windows.h>
#include <commctrl.h>
#include <vector>
#include <string>
#include <d2d1.h>

#define DEFAULT_ITEM_HEIGHT 24
#define COLOR_DEFAULT RGB(224, 224, 224)
//(RGB(157, 195, 230)

#define FOLDER_UNLOCKED L" "
#define FOLDER_LOCKED L"*"
#define FOLDER_CONNECTOR_NEXT L"├"
#define FOLDER_CONNECTOR_END L"└"

class CFolderItem;
typedef std::vector <CFolderItem*> folder_items;
typedef struct _SELECTED_ITEM_ SELECTED_ITEM;
typedef struct _BTN_COLORS_ BTN_COLORS;

enum HOVER_PART
{
	HP_INVALID = -1,
	HP_TOP = 0,
	HP_MIDDLE = 1,
	HP_BOTTOM = 2
};

typedef struct _SELECTED_ITEM_
{
	WORD   level = 0;
	WORD   part = HOVER_PART::HP_MIDDLE;
	HANDLE parent_item = NULL;
	HANDLE handle_item = NULL;
	BOOL   plus = FALSE;

public:
	bool operator == (const SELECTED_ITEM& oth)
	{
		if (this->part        == oth.part &&
			this->parent_item == oth.parent_item &&
			this->handle_item == oth.handle_item &&
			this->plus        == oth.plus)
			return TRUE;
		else
			return FALSE;
	}

	bool operator != (const SELECTED_ITEM& oth)
	{
		if (this->part        != oth.part ||
			this->parent_item != oth.parent_item ||
			this->handle_item != oth.handle_item ||
			this->plus        != oth.plus)
			return TRUE;
		else
			return FALSE;
	}

} SELECTED_ITEM;

typedef struct _BTN_COLORS_
{
	COLORREF                  txt;
	COLORREF                  btn;

public:
	_BTN_COLORS_(COLORREF txt, COLORREF btn)
	{
		this->txt = txt;
		this->btn = btn;
	}

	bool operator == (const BTN_COLORS& oth)
	{
		if (this->txt == oth.txt &&
			this->btn == oth.btn)
			return TRUE;
		return FALSE;
	}

	bool operator != (const BTN_COLORS& oth)
	{
		if (this->txt == oth.txt &&
			this->btn == oth.btn)
			return FALSE;
		return TRUE;
	}

} BTN_COLORS;

typedef struct _DRAW_RES_
{
	ID2D1SolidColorBrush*     scb_txt;
	ID2D1LinearGradientBrush* lgb_btn_normal;
	ID2D1LinearGradientBrush* lgb_btn_pressed;
} DRAW_RES;

class CFolderItem
{
public:
	HANDLE       hHandle;
	HANDLE       hParent;
	HANDLE       hPrev;
	HANDLE       hNext;
	INT          nLevel;
	HWND         hWndParent;
	LONG_PTR     move_index;
	std::wstring sPath;
	std::wstring sTitle;
	BOOL         bCollapsed;
	BOOL         bSelected;
	BOOL         bHiden;
	BOOL         bLocked;
	//UINT         nColorIndex;
	HANDLE       nLastChildSelected;
	folder_items children;
	BTN_COLORS   clrs;
	DRAW_RES     draw_res;

public:
	CFolderItem();
	~CFolderItem();

	BOOL       HasChildren() { return (BOOL)children.size(); };
	long       GetItemHeight();
	HANDLE     GetTopItem(int &nTop, int& nVertScroll);
	HANDLE     GetNextItem();
	HANDLE     GetPrevItem();

	HANDLE     IsThereSlectedChild();
	void       SelectChildren();
	void       ClearChildrenSelection();
	BOOL       IsLastChild(CFolderItem* fi);
	void       GetTitle(std::wstring& title);
	void       SetColors(COLORREF txt = RGB(0, 0 ,0), COLORREF bnt = COLOR_DEFAULT);
	void       SetColors(BTN_COLORS clrs);
	BTN_COLORS GetColors() { return clrs; };
	void       CreateDPColors(ID2D1HwndRenderTarget* pRT);
	void       Select();
	int        GetLevel() { return nLevel - 1; };
	int        FindChildPos(HANDLE hItem);
	void       DeleteChildren();
	void       Remove();
	HANDLE     GetSelectedItem();
	BOOL       SetFolderPath(LPCTSTR lpszText);
};


#endif // FOLDERITEM_H_INCLUDED
