#ifndef MDIFRAME_H_INCLUDED
#define MDIFRAME_H_INCLUDED

#include "treefolders.h"
#include "listview.h"
#include "tpane.h"
#include "folderpane.h"

class CMDIFrame
{
public:
	CMDIFrame();
	~CMDIFrame();

    HWND     Create( HWND hWndParent, HINSTANCE hInstance, LPVOID lpParam);
    LRESULT  WindowProcedure(HWND, UINT, WPARAM, LPARAM);

    int      OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct);
    void     OnSize(UINT nType, UINT nWidth, UINT nHeight);

    int      GetFolderPanePosition()              { return m_wndFolderPane.GetPanePosition(); };
    int      GetFolderPaneWidth()                 { return m_wndFolderPane.GetPaneWidth(); };
    void     SetFolderPanePosition(int nPosition) { m_wndFolderPane.SetPanePosition(nPosition); };
    void     SetFolderPaneWidth(int nWidth, BOOL bInit = FALSE)       { m_wndFolderPane.SetPaneWidth(nWidth, bInit); };
    //int      GetFolderPanePosition() { return m_wndFolders.GetPosition(); };
    //int      GetFolderPaneWidth() { return m_wndFolders.GetWidth(); };
    //void     SetFolderPanePosition(int nPosition) { m_wndFolders.SetPosition(nPosition); };
    //void     SetFolderPaneWidth(int nWidth) { m_wndFolders.SetWidth(nWidth); };

    RECT    MoveFolder(RECT rect);

    HWND m_hWnd;
    BOOL m_bActive;
    int  nID;
    float m_fScale;

    CTPane       m_wndTPane;
    CFolderPane  m_wndFolderPane;
    CListView    m_wmdListView;

};


#endif // MDIFRAME_H_INCLUDED
