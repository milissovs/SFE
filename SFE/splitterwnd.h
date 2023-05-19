#ifndef SPLITTERWND_H_INCLUDED
#define SPLITTERWND_H_INCLUDED

#include "mdirame.h"
#include <d2d1.h>

class CSplitterWnd
{
public:
    typedef enum _SPLIT_TYPE
    {
        SPLIT_NONE = 0,
        SPLIT_VERT = 1,
        SPLIT_HORZ = 2
    }SPLIT_TYPE;

    CSplitterWnd();
    ~CSplitterWnd();

    HWND Create(
        HWND hWndParent,
        HINSTANCE hInstance,
        LPVOID lpParam);

    LRESULT WindowProcedure(HWND, UINT, WPARAM, LPARAM);

    HWND                m_hWnd;
    HWND                m_hParent;
    SPLIT_TYPE          m_split_type;
    float               m_fRatio;
    int                 nSplitterH; //Width or height of splitter
    BOOL                m_LBDown;

private:
    BOOL    RegisterClassEx(HINSTANCE hInstance);

    int  OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct);
    void OnPaint(HDC hdc);
    void OnSize(UINT nType, UINT nWidth, UINT nHeight);
    void OnMousemove(DWORD vKeys, POINT& pt);
    void OnLButtonDlclk(DWORD vKeys, POINT& pt);

    HCURSOR    hCursorNS;
    HCURSOR    hCursorEW;

private:
    ID2D1Factory*          m_pDirect2dFactory;
    ID2D1HwndRenderTarget* m_pRenderTarget;
    ID2D1SolidColorBrush*  m_pBr1;
    ID2D1SolidColorBrush*  m_pBr2;

private:
    // Initialize device-independent resources.
    HRESULT CreateDeviceIndependentResources();

    // Initialize device-dependent resources.
    HRESULT CreateDeviceDependentResources();

    HRESULT OnPaint2D();

    // Release device-dependent resource.
    void DiscardDeviceDependentResources();
};


#endif // SPLITTERWND_H_INCLUDED
