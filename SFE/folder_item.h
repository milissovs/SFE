#ifndef FOLDERITEM_H_INCLUDED
#define FOLDERITEM_H_INCLUDED

#include <windows.h>
#include <commctrl.h>
#include <vector>

#define DEFAULT_ITEM_HEIGHT 24
#define COLOR_DEFAULT RGB(157, 195, 230)

#define FOLDER_UNLOCKED L" "
#define FOLDER_LOCKED L"*"

enum HOVER_PART
{
	HP_INVALID = -1,
	HP_TOP = 0,
	HP_MIDDLE = 1,
	HP_BOTTOM = 2
};

typedef struct _SELECTED_ITEM_
{
	WORD level = 0;
	WORD part = HOVER_PART::HP_MIDDLE;
	LONG item = -1;
	HANDLE parent_item = NULL;
	HANDLE handle_item = NULL;

public:
	bool operator == (const _SELECTED_ITEM_& oth)
	{
		if (this->item == oth.item &&
			this->level == oth.level &&
			this->part == oth.part &&
			this->parent_item == oth.parent_item &&
			this->handle_item == oth.handle_item)
			return TRUE;
		else
			return FALSE;
	}

	bool operator != (const _SELECTED_ITEM_& oth)
	{
		if (this->item != oth.item ||
			this->level != oth.level ||
			this->part != oth.part ||
			this->parent_item != oth.parent_item ||
			this->handle_item != oth.handle_item)
			return TRUE;
		else
			return FALSE;
	}

} SELECTED_ITEM;

typedef struct _FOLDER_ITEM_
{
	HANDLE    hHandle;
	WORD      nLevel = 0;
	HANDLE    hParent = NULL;
	HWND      hWndParent = NULL;
	LONG_PTR  move_index = -1;
	LPCTSTR   lpszPath{};
	BOOL      bCollapsed = FALSE;
	BOOL      bSelected = FALSE;
	BOOL      bHiden = FALSE;
	BOOL      bLocked = FALSE;
	UINT      nColorIndex = 0;
	std::vector <_FOLDER_ITEM_> children {};
	LONG      nLastChildSelected = -1;

public:
	BOOL    HasChildren() { return (BOOL)children.size(); };
	long    GetChildrenHeight() {
		long lHeight = 0;
		for (_FOLDER_ITEM_& fi : children)
			if (!fi.bHiden)
				lHeight += DEFAULT_ITEM_HEIGHT;
		return lHeight;
	};
	BOOL    IsThereSlectedChild() {
		for (_FOLDER_ITEM_& fi : children)
			if (fi.bSelected)
			{
				return TRUE;
			}
		return FALSE;
	};
	void   SelectChildren() {
		if (HasChildren())
		{
			if (nLastChildSelected < 0)
				nLastChildSelected = 0;

			for (size_t t = 0; t < children.size(); t++)
			{
				if (nLastChildSelected == t)
					children.at(t).bSelected = TRUE;
				else
					children.at(t).bSelected = FALSE;

				if (children.at(t).HasChildren())
					children.at(t).SelectChildren();
			}
		}

	}

} FOLDER_ITEM;


#endif // FOLDERITEM_H_INCLUDED
