#include "folder_item.h"

CFolderItem::CFolderItem()
	: hHandle(NULL)
	, nLevel(0)
	, hParent(NULL)
	, hWndParent(NULL)
	, move_index(-1)
	, lpszPath(L"")
	, lpszTitle(L"")
	, bCollapsed(FALSE)
	, bSelected(FALSE)
	, bHiden(FALSE)
	, bLocked(FALSE)
	, nColorIndex(0)
	, nLastChildSelected(NULL)
	, children()
{

}

CFolderItem::~CFolderItem()
{
	DeleteChildren();
}

long CFolderItem::GetChildrenHeight()
{
	long lHeight = 0;
	for (CFolderItem* fi : children)
		if (!fi->bHiden)
			lHeight += DEFAULT_ITEM_HEIGHT;
	return lHeight;
};

HANDLE CFolderItem::IsThereSlectedChild()
{
	for (CFolderItem* fi : children)
		if (fi->bSelected)
		{
			return fi;
		}
	return NULL;
};

void CFolderItem::SelectChildren()
{
	if (HasChildren())
	{
		if (nLastChildSelected == NULL)
		{
			children.at(0)->bSelected = TRUE;
			return;
		}

		for (size_t t = 0; t < children.size(); t++)
		{
			if (children.at(t) == nLastChildSelected)
				children.at(t)->bSelected = TRUE;
			else
				children.at(t)->bSelected = FALSE;
		}
	}

}

void CFolderItem::DeleteChildren()
{
	for (CFolderItem* fi : children)
	{
		delete fi;
		fi = NULL;
	}
	children.clear();
}

void CFolderItem::ClearChildrenSelection()
{
	for (CFolderItem* fi : children)
	{
		fi->bSelected = FALSE;
		if (fi->HasChildren())
			fi->ClearChildrenSelection();
	}
}

void CFolderItem::Select()
{
	CFolderItem* pfi = (CFolderItem*)hParent;
	if (pfi != NULL)
	{
		if (pfi->hParent != NULL)
		{
			CFolderItem* parentfi = (CFolderItem*)pfi->hParent;
			parentfi->ClearChildrenSelection();
		}

		for (CFolderItem* fi : pfi->children)
		{
			fi->bSelected = FALSE;
			fi->ClearChildrenSelection();
		}
	}
	else
		return;

	if (HasChildren())
	{
		CFolderItem* cfi = NULL;
		if (nLastChildSelected != NULL)
		{
			cfi = (CFolderItem*)nLastChildSelected;
		}
		else
		{
			nLastChildSelected = children.at(0);
			cfi = children.at(0);
		}

		if (cfi != NULL)
			cfi->Select();

		if (bCollapsed)
			bSelected = TRUE;
	}
	else
	{
		pfi->nLastChildSelected = this;
		bSelected = TRUE;
	}
}

BOOL CFolderItem::IsLastChild(CFolderItem* fi)
{
	for (size_t t = 0; t < children.size() - 1; t++)
	{
		CFolderItem* cfi = children.at(t);
		if (cfi == fi)
			return FALSE;
	}

	return TRUE;
}

void CFolderItem::GetTitle(std::wstring& title)
{
	if (wcslen(lpszTitle) > 0)
	{
		title = lpszTitle;
		return;
	}

	std::wstring path = lpszPath;
	size_t p = path.rfind(L"\\");
	if (p != std::string::npos)
	{
		title = path.substr(p + 1);
		return;
	}
}
