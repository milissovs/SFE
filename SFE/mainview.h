#ifndef MAINVIEW_H_INCLUDED
#define MAINVIEW_H_INCLUDED

#include <d2d1.h>
#include "splitterwnd.h"

class CMainView
{
public:
    CMainView();
    ~CMainView();


    HWND     Create(HWND hWndParent, HINSTANCE hInstance, LPVOID lpParam);
    LRESULT  WindowProcedure(HWND, UINT, WPARAM, LPARAM);

    void     OnSize(UINT nType, UINT nWidth, UINT nHeight);
    void     OnIdViewSplitNone(RECT rect);
    void     OnIdViewSplitVert(RECT rect);
    void     OnIdViewSplitHorz(RECT rect);
    void     OnPaint(HDC hdc, RECT rect);

    void     SetActiveFrame(int nFrame = 0);
    int      GetActiveFrame();

    void     SetFolderPositions(INT nFolderPositions[2]);
    void     GetFolderPositions(INT &nFP0, INT &nFP1);
    void     SetFolderPaneWidth(INT nFolderPaneWidth[2], BOOL bInit = FALSE);
    void     GetFolderPaneWidth(INT& nW0, INT& nW1);

    HWND m_hWnd;

    CSplitterWnd           m_wndSplitter;
    CMDIFrame              m_wndMDIs[2];


private:    
    int     OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct);
    void    OnSplitter(RECT rect, WPARAM wParam, LPARAM lParam);

    RECT GetSplitterPos(RECT rect);
    RECT GetMDIPos(RECT wRect, RECT sRect, int nIndex);
    void CalcSplitterRatio(RECT rect, POINT pt);
    void RepositionChildren(RECT rect);

private:
    ID2D1Factory* m_pDirect2dFactory;
    ID2D1HwndRenderTarget* m_pRenderTarget;
    ID2D1SolidColorBrush* m_pBr1;
    ID2D1SolidColorBrush* m_pBr2;

private:
    // Initialize device-independent resources.
    HRESULT CreateDeviceIndependentResources();

    // Initialize device-dependent resources.
    HRESULT CreateDeviceDependentResources();

    HRESULT OnPaint2D();

    // Release device-dependent resource.
    void DiscardDeviceDependentResources();
};

#endif // MAINVIEW_H_INCLUDED