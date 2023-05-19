#ifndef FOLDERPANE_H_INCLUDED
#define FOLDERPANE_H_INCLUDED

#include <commctrl.h>
#include <d2d1.h>
#include "paneheader.h"
#include "treefolders.h"

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

	//void     OnNcPaint(WPARAM wParam);
	int      OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct);
	void     OnSize(UINT nType, UINT nWidth, UINT nHeight);

	HWND m_hWnd;
	BOOL m_bNCLButtonDown;
	BOOL m_bLButtonDown;

	CPaneHeader m_wndPaneHeader;
	CTreeFolders m_wndTreeForlders;

private:
	int  m_nPanePos;
	INT  m_nWidth;


};

#endif // FOLDERPANE_H_INCLUDED
