#include "folder_item.h"
#include "shlwapi.h"
#include "globals.h"

CFolderItem::CFolderItem()
	: hHandle(NULL)
	//, nLevel(0)
	, hParent(NULL)
	, hPrev(NULL)
	, hNext(NULL)
	, nLevel(0)
	, hWndParent(NULL)
	, move_index(-1)
	, sPath(L"")
	, sTitle(L"")
	, bCollapsed(FALSE)
	, bSelected(FALSE)
	, bHiden(FALSE)
	, bLocked(FALSE)
	, nColorIndex(0)
	, nLastChildSelected(NULL)
	, children()
	, clrs(RGB(0, 0, 0), COLOR_DEFAULT)
	, draw_res()
{

}

CFolderItem::~CFolderItem()
{
	DeleteChildren();
}

long CFolderItem::GetItemHeight()
{
	long lHeight = 0;

	if (bHiden)
		return lHeight;

	if (hParent)
		lHeight += DEFAULT_ITEM_HEIGHT;

	if (bCollapsed)
		return lHeight;

	for (CFolderItem* fi : children)
	{
		lHeight += fi->GetItemHeight();
	}
	return lHeight;
}

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
	if (sTitle.length() > 0)
	{
		title = sTitle;
		return;
	}

	std::wstring path = sPath;
	size_t p = path.rfind(L"\\");
	if (p != std::string::npos)
	{
		title = path.substr(p + 1);
		return;
	}
	else
	{
		title = path;
	}
}

void CFolderItem::CreateDPColors(ID2D1HwndRenderTarget* pRT)
{
	if (!pRT)
		return;

	WORD wH = 0;
	WORD wL = 0;
	WORD wS = 0;

	COLORREF color = clrs.btn;
	ColorRGBToHLS(color, &wH, &wL, &wS);

	WORD wL1 = wL + 40 >= 240 ? 240 : wL + 40;
	WORD wL2 = wL - 40 <= 0 ? 0 : wL - 40;

	COLORREF clr1 = ColorHLSToRGB(wH, wL1, wS);
	COLORREF clr2 = ColorHLSToRGB(wH, wL2, wS);

	D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES lgbp{ D2D1::Point2F(0, 0) , D2D1::Point2F(0, 0) };

	D2D1_GRADIENT_STOP gpt4[4];
	gpt4[0].position = 0.0f;
	gpt4[1].position = 0.25f;
	gpt4[2].position = 0.75f;
	gpt4[3].position = 1.0f;

	// ------------------------------------------------------------------------------------------------------------
	gpt4[0].color = D2D1::ColorF(GetRValue(clr1) / 255.0f, GetGValue(clr1) / 255.0f, GetBValue(clr1) / 255.0f);
	gpt4[1].color = D2D1::ColorF(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);
	gpt4[2].color = D2D1::ColorF(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);
	gpt4[3].color = D2D1::ColorF(GetRValue(clr2) / 255.0f, GetGValue(clr2) / 255.0f, GetBValue(clr2) / 255.0f);

	ID2D1GradientStopCollection* gsc0 = NULL;
	pRT->CreateGradientStopCollection(&gpt4[0], ARRAYSIZE(gpt4), &gsc0);
	if (gsc0 != NULL)
		pRT->CreateLinearGradientBrush(lgbp, gsc0, &draw_res.lgb_btn_normal);

	D2D1_POINT_2F pt0{ 1000.0f, 0.0f };
	D2D1_POINT_2F pt1{ 1000.0f, DEFAULT_ITEM_HEIGHT };
	draw_res.lgb_btn_normal->SetStartPoint(pt0);
	draw_res.lgb_btn_normal->SetEndPoint(pt1);

	// ------------------------------------------------------------------------------------------------------------
	gpt4[0].color = D2D1::ColorF(GetRValue(clr2) / 255.0f, GetGValue(clr2) / 255.0f, GetBValue(clr2) / 255.0f);
	gpt4[1].color = D2D1::ColorF(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);
	gpt4[2].color = D2D1::ColorF(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);
	gpt4[3].color = D2D1::ColorF(GetRValue(clr1) / 255.0f, GetGValue(clr1) / 255.0f, GetBValue(clr1) / 255.0f);

	ID2D1GradientStopCollection* gsc1 = NULL;
	pRT->CreateGradientStopCollection(&gpt4[0], ARRAYSIZE(gpt4), &gsc1);
	if (gsc1 != NULL)
		pRT->CreateLinearGradientBrush(lgbp, gsc1, &draw_res.lgb_btn_pressed);
	draw_res.lgb_btn_pressed->SetStartPoint(pt0);
	draw_res.lgb_btn_pressed->SetEndPoint(pt1);

	// ------------------------------------------------------------------------------------------------------------
	pRT->CreateSolidColorBrush(
		D2D1::ColorF(GetRValue(clrs.txt) / 255.0f,
					 GetGValue(clrs.txt) / 255.0f,
					 GetBValue(clrs.txt) / 255.0f), &draw_res.scb_txt);

	for (CFolderItem* pfi : children)
	{
		pfi->CreateDPColors(pRT);
	}
}

void CFolderItem::SetColors(COLORREF txt, COLORREF bnt)
{
	clrs.txt = txt;
	clrs.btn = bnt;
}

void CFolderItem::SetColors(BTN_COLORS clrs)
{
	this->clrs = clrs;
}

int CFolderItem::FindChildPos(HANDLE hItem)
{
	for (size_t t = 0; t < children.size(); t++)
	{
		if (children.at(t)->hHandle == hItem)
			return (int)t;
	}
	return -1;
}

HANDLE CFolderItem::GetNextItem()
{
	if (!hNext && hParent)
	{
		return ((CFolderItem*)hParent)->GetNextItem();
	}
	else
		return hNext;
}

HANDLE CFolderItem::GetPrevItem()
{
	if (!hPrev && hParent)
	{
		return ((CFolderItem*)hParent)->GetPrevItem();
	}
	else
		return hPrev;
}

HANDLE CFolderItem::GetTopItem(int& nTop, int& nVertScroll)
{
	nTop += DEFAULT_ITEM_HEIGHT;
	if (nTop > nVertScroll)
		return this;

	if (HasChildren() && !bCollapsed)
	{
		for (CFolderItem* pfi : children)
		{
			HANDLE hTop = pfi->GetTopItem(nTop, nVertScroll);
			if (hTop)
				return hTop;
		}
	}
	return NULL;
}

void CFolderItem::Remove()
{
	if (hParent == NULL)
		return;

	CFolderItem* fiParent = (CFolderItem*)hParent;
	CFolderItem* fiPrev = (CFolderItem *) GetPrevItem();
	CFolderItem* fiNext = (CFolderItem *) GetNextItem();

	if (fiPrev)
		fiPrev->hNext = fiNext;
	if (fiNext)
		fiNext->hPrev = fiPrev;
	int nPos = fiParent->FindChildPos(this);
	fiParent->children.erase(fiParent->children.begin() + nPos);
}

HANDLE CFolderItem::GetSelectedItem()
{
	HANDLE hSelectedItem = NULL;
	for (CFolderItem* pfi : children)
	{
		hSelectedItem = pfi->GetSelectedItem();
		if (hSelectedItem)
			return hSelectedItem;
	}
	if (bSelected)
		return this;
	return hSelectedItem;
}
