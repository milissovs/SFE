#ifndef MAINFRAME_H_INCLUDED
#define MAINFRAME_H_INCLUDED

#include <vector>
#include <d2d1.h>
#include "mainview.h"

#define SNAP_NONE   0 //0b0000
#define SNAP_TOP    1 //0b0001
#define SNAP_BOTTOM 2 //0b0010
#define SNAP_LEFT   4 //0b0100
#define SNAP_RIGHT  8 //0b1000

class CMainFrame
{
    typedef struct _REGISTRY
    {
        DWORD nSplitType{ 0 };
        FLOAT fSplitRatio{ 0.5f };
        RECT  rect{0, 0, 800, 600};
        BOOL  bMaximaized{FALSE};
        WORD  wSnap{ SNAP_NONE };
        INT   nActiveFrame{ 0 };
        INT   nFolderPosition[2] = { 1, 1 };
        INT   nFolderPaneWidth[2] = {300, 300};
    }REGISTRY;

public:
    CMainFrame();
    ~CMainFrame();

    HWND     Create(DWORD dwStyle,
                    int x,
                    int y,
                    int nWidth,
                    int nHeight,
                    HWND hWndParent,
                    HMENU hMenu,
                    HINSTANCE hInstance,
                    LPVOID lpParam);

    BOOL     CenterWindow();
    HWND     GetSaveHwnd() { return m_hWnd; };
    BOOL     ShowWindow(int nCmdShow);
    BOOL     UpdateWindow();
    BOOL     ReadRegistry(REGISTRY & regs);
    BOOL     ReadRegValue(LPCTSTR pszValueName, void* pData, ULONG nBytes, HKEY hKey = NULL);
    BOOL     WriteRegistry(REGISTRY& regs);
    BOOL     WriteRegValue(LPCTSTR pszValueName, const void *pData, ULONG nBytes, HKEY hKey = NULL);
    void     SnapWindow();
    void     UpdateSettings();

    float    GetScale(HWND hWnd);

    int      OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct);
    void     OnDestroy();
    int      OnCommand(HWND hWnd, WPARAM wp, LPARAM lp);
    void     OnSize(UINT nType, UINT nWidth, UINT nHeight);

    int      MessageLoop();
    LRESULT  WindowProcedure(HWND, UINT, WPARAM, LPARAM);

    HWND      m_hWnd;
    HINSTANCE m_hInstance;
    RECT      m_wndPossition;
    BOOL      m_bMaximized;
    WORD      m_wSnap;

private:
    BOOL RegisterClassEx(HINSTANCE hInstance);


protected:
    //BOOL                   m_bShowMDIFrame;
    //CMDIFrame              m_wndMDIs[2];
    CMainView                m_wndMainView;

//private:
//    // Initialize device-independent resources.
//    HRESULT CreateDeviceIndependentResources();
//
//    // Initialize device-dependent resources.
//    HRESULT CreateDeviceDependentResources();
//
//    HRESULT OnPaint2D();
//
//    // Release device-dependent resource.
//    void DiscardDeviceDependentResources();
//
//private:
//    ID2D1Factory*          m_pDirect2dFactory;
//    ID2D1HwndRenderTarget* m_pRenderTarget;
//    ID2D1SolidColorBrush*  m_pCornflowerBlueBrush;
};

#endif // MAINFRAME_H_INCLUDED
