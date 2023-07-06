#ifndef FOLDERITEM_H_INCLUDED
#define FOLDERITEM_H_INCLUDED

#include <windows.h>
#include <commctrl.h>
#include <vector>
#include <string>

#define DEFAULT_ITEM_HEIGHT 24
#define COLOR_DEFAULT RGB(157, 195, 230)

#define FOLDER_UNLOCKED L" "
#define FOLDER_LOCKED L"*"
#define FOLDER_CONNECTOR_NEXT L"├"
#define FOLDER_CONNECTOR_END L"└"

class CFolderItem;
typedef std::vector <CFolderItem*> folder_items;
typedef struct _SELECTED_ITEM_ SELECTED_ITEM;

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
	LONG   item = -1;
	HANDLE parent_item = NULL;
	HANDLE handle_item = NULL;
	BOOL   plus = FALSE;

public:
	bool operator == (const SELECTED_ITEM& oth)
	{
		if (/*this->item == oth.item &&
			this->level == oth.level &&*/
			this->part        == oth.part &&
			this->parent_item == oth.parent_item &&
			this->handle_item == oth.handle_item &&
			this->plus        == oth.plus)
			return TRUE;
		else
			return FALSE;
	}

	bool operator != (const SELECTED_ITEM& oth)
	{
		if (/*this->item != oth.item ||
			this->level != oth.level ||*/
			this->part        != oth.part ||
			this->parent_item != oth.parent_item ||
			this->handle_item != oth.handle_item ||
			this->plus        != oth.plus)
			return TRUE;
		else
			return FALSE;
	}

} SELECTED_ITEM;

class CFolderItem
{
public:
	HANDLE    hHandle;
	WORD      nLevel;
	HANDLE    hParent;
	HWND      hWndParent;
	LONG_PTR  move_index;
	LPCTSTR   lpszPath;
	LPCTSTR   lpszTitle;
	BOOL      bCollapsed;
	BOOL      bSelected;
	BOOL      bHiden;
	BOOL      bLocked;
	UINT      nColorIndex;
	HANDLE    nLastChildSelected;
	folder_items children;

public:
	CFolderItem();
	~CFolderItem();

	BOOL    HasChildren() { return (BOOL)children.size(); };
	long    GetChildrenHeight();
	HANDLE  IsThereSlectedChild();
	void    SelectChildren();
	void    ClearChildrenSelection();
	BOOL    IsLastChild(CFolderItem* fi);
	void    GetTitle(std::wstring& title);

	void    DeleteChildren();
	void    Select();
};


#endif // FOLDERITEM_H_INCLUDED
