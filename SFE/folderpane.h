#ifndef FOLDERPANE_H_INCLUDED
#define FOLDERPANE_H_INCLUDED

#include <commctrl.h>
#include <d2d1.h>
#include "paneheader.h"
#include "treefolders.h"
#include "listfolders.h"
#include <string>

class CFolderPane
{
public:
	CFolderPane();
	~CFolderPane();

	LRESULT WindowProcedure(HWND, UINT, WPARAM, LPARAM);

	HWND Create(HWND hWndParent, HINSTANCE hInstance, LPVOID lpParam);

	void SetPanePosition(int nPosition);
	int  GetPanePosition() { return m_nPanePos; };
	void SetPaneWidth(int nWidth, BOOL bInit = FALSE);
	int  GetPaneWidth() { return m_nWidth; };
	BOOL GetCollapsible() { return m_bCollapsible; };
	BOOL GetCollapsed() { return m_bCollapsed; };

	void SetCollapsed(BOOL bCollapsed = TRUE) { m_bCollapsed = bCollapsed; };

	//void     OnNcPaint(WPARAM wParam);
	int      OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct);
	void     OnSize(UINT nType, UINT nWidth, UINT nHeight);
	void     OnNCMouseMove(POINT pt);
	BOOL     OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

	HWND m_hWnd;
	BOOL m_bNCLButtonDown;
	BOOL m_bLButtonDown;

	CPaneHeader  m_wndPaneHeader;
	//CTreeFolders m_wndTreeForlders;
	CListFolders m_wndListFolders;

private:
	int  m_nPanePos;
	INT  m_nWidth;
	BOOL m_bCollapsible;
	BOOL m_bCollapsed;
	BOOL m_bMouseTracking;

};

#endif // FOLDERPANE_H_INCLUDED
