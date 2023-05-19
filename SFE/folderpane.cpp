#include <windows.h>
#include <windowsx.h>
#include "folderpane.h"
#include "globals.h"
#include "resource.h"

TCHAR m_szPaneClassName[] = TEXT("FOLDERPANE");


#define CX_BITMAP 16	// Each icon width  (tileset)
#define CY_BITMAP 16	// Each icon height (tileset)

LRESULT CALLBACK PaneProcEx(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    UNALIGNED CFolderPane* pThis = (CFolderPane*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (Message == WM_NCCREATE)
    {
        LPCREATESTRUCT lpCS = (LPCREATESTRUCT)lParam;
        CFolderPane* pThis = (CFolderPane*)lpCS->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->m_hWnd = hwnd;
    }

    if (!pThis)
        return ::DefWindowProc(hwnd, Message, wParam, lParam);

    return pThis->WindowProcedure(hwnd, Message, wParam, lParam);
}

LRESULT  CFolderPane::WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_ERASEBKGND:
        return 1;

    case WM_NCLBUTTONDOWN:
    {
        HWND hParent = ::GetParent(m_hWnd);
        m_bNCLButtonDown = TRUE;
        ::PostMessage(hParent, WM_NCLBUTTONDOWN, (WPARAM)0, (LPARAM)0);

        LRESULT hitTest = ::SendMessage(m_hWnd, WM_NCHITTEST, wParam, lParam);
        if (hitTest == HTLEFT || hitTest == HTRIGHT)
        {
            SetCapture(m_hWnd);
        }
        break;
    }

    case WM_LBUTTONDOWN:
    {
        HWND hParent = ::GetParent(m_hWnd);
        ::PostMessage(hParent, WM_LBUTTONDOWN, (WPARAM)0, (LPARAM)0);

        m_bLButtonDown = TRUE;
        break;
    }

    case WM_LBUTTONUP:
    {
        m_bLButtonDown = FALSE;
        m_bNCLButtonDown = FALSE;
        ReleaseCapture();
        break;
    }

    case WM_NCLBUTTONUP:
    {
        m_bLButtonDown = FALSE;
        m_bNCLButtonDown = FALSE;
        ReleaseCapture();
        break;
    }

    case WM_NCHITTEST:
    {
        POINT pt{ GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) };
        ScreenToClient(hWnd, &pt);
        RECT rect;
        GetClientRect(hWnd, &rect);
        if (pt.x < 0)
            return HTLEFT;
        if (pt.x > (rect.right - rect.left))
            return HTRIGHT;


        return HTCLIENT;
    }

    case WM_MOUSEMOVE:
    {
        POINT pt{ GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) };
        if (m_bNCLButtonDown)
        {
            ClientToScreen(m_hWnd, &pt);
            PostMessage(::GetParent(m_hWnd), WM_FOLDER_RESIZE, (WPARAM)0, MAKELPARAM(pt.x, pt.y));
        }
        break;
    }

    case WM_NCMOUSEMOVE:
    {
        POINT pt{ GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) };
        if (m_bNCLButtonDown)
        {
            ClientToScreen(m_hWnd, &pt);
            PostMessage(::GetParent(m_hWnd), WM_FOLDER_RESIZE, (WPARAM)0, MAKELPARAM(pt.x, pt.y));
        }
        break;
    }

    case WM_CREATE:
    {
        LPCREATESTRUCT lpCS = (LPCREATESTRUCT)lParam;
        OnCreate(hWnd, lpCS);
        return 1;
    }

    case WM_SIZE:
    {
        UINT width = LOWORD(lParam);
        UINT height = HIWORD(lParam);
        OnSize((UINT)wParam, width, height);
        break;
    }

    case WM_PANE_FOLDER:
    {
        switch (wParam)
        {
            case 0:
            {
                SetPanePosition((int)lParam);
                PostMessage(GetParent(m_hWnd), WM_FOLDER_RESIZE, 1, lParam);
                break;
            }
        }
        break;
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

CFolderPane::CFolderPane()
    : m_hWnd(NULL)
    , m_bNCLButtonDown(FALSE)
    , m_bLButtonDown(FALSE)
    , m_nWidth(300)
    , m_nPanePos(0)
{

}

CFolderPane::~CFolderPane()
{
}

HWND CFolderPane::Create(HWND hWndParent, HINSTANCE hInstance, LPVOID lpParam)
{

    WNDCLASSEX wincl;

    /* The Window structure */
    wincl.cbSize = sizeof(WNDCLASSEX);
    wincl.hInstance = hInstance;
    wincl.lpszClassName = m_szPaneClassName;
    wincl.lpfnWndProc = PaneProcEx;
    wincl.style = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;

    /* Use default icon and mouse-pointer */
    wincl.hIcon = NULL;
    wincl.hIconSm = NULL;

    wincl.hCursor = (HCURSOR)LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = sizeof(CFolderPane*); //sizeof(LONG_PTR)
    wincl.cbWndExtra = 0;

    /* Use Windows's default color as the background of the window */
    wincl.hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);

    /* Register the window class, and if it fails quit the program */
    if (!::RegisterClassEx(&wincl))
    {
        DWORD dwError = GetLastError();
        if (dwError != ERROR_CLASS_ALREADY_EXISTS)
            return FALSE;
    }

    m_hWnd = CreateWindowEx(
        WS_EX_COMPOSITED | WS_EX_TRANSPARENT | WS_EX_CLIENTEDGE,
        m_szPaneClassName,
        NULL,
        WS_VISIBLE | WS_CHILD | WS_BORDER,
        0,
        0,
        200,
        200,
        hWndParent,
        NULL,
        hInstance,
        (LPVOID)this);

    return m_hWnd;
}

void CFolderPane::SetPanePosition(int nPosition)
{
    m_nPanePos = nPosition;
    m_wndPaneHeader.SetPosition(nPosition);
}

void CFolderPane::SetPaneWidth(int nWidth, BOOL bInit)
{
    if (bInit)
    {
        m_nWidth = nWidth;
        return;
    }

    if (nWidth < 200)
        return;

    RECT rect;
    GetClientRect(GetParent(m_hWnd), &rect);
    if (nWidth > rect.right - 200)
        return;

    m_nWidth = nWidth;
}

//void CFolderPane::OnNcPaint(WPARAM wParam)
//{
//    RECT rect;
//    GetWindowRect(m_hWnd, &rect);
//
//    HRGN region = NULL;
//    if (wParam == NULLREGION)
//    {
//        region = CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom);
//    }
//    else
//    {
//        HRGN copy = CreateRectRgn(0, 0, 0, 0);
//        if (CombineRgn(copy, (HRGN)wParam, NULL, RGN_COPY))
//            region = copy;
//        else
//            DeleteObject(copy);
//    }
//
//    HDC dc{ GetDCEx(m_hWnd, region, DCX_WINDOW | DCX_CACHE | DCX_INTERSECTRGN | DCX_LOCKWINDOWUPDATE) };
//    if (!dc && region)
//    {
//        DeleteObject(region);
//    }
//    else
//    {
//        HPEN pen0{};
//        HPEN pen1{};
//        HPEN pen2{};
//
//        pen0 = CreatePen(PS_INSIDEFRAME, 1, RGB(127, 127, 127));
//        pen1 = CreatePen(PS_INSIDEFRAME, 1, RGB(165, 165, 165));
//        pen2 = CreatePen(PS_INSIDEFRAME, 1, RGB(222, 222, 222));
//
//        HGDIOBJ old{ SelectObject(dc, pen0) };
//
//        if (old != 0)
//        {
//            if (dc != 0)
//            {
//                SelectObject(dc, pen0);
//                Rectangle(dc, 0, 0, rect.right - rect.left, rect.bottom - rect.top);
//
//                SelectObject(dc, pen1);
//                Rectangle(dc, 1, 0, rect.right - rect.left - 1, rect.bottom - rect.top);
//
//                SelectObject(dc, pen2);
//                Rectangle(dc, 2, 0, rect.right - rect.left - 2, rect.bottom - rect.top);
//            }
//        }
//
//        if (old != 0)
//            SelectObject(dc, old);
//        if (dc != 0)
//            ReleaseDC(m_hWnd, dc);
//        DeleteObject(pen1);
//        DeleteObject(pen2);
//    }
//}

int CFolderPane::OnCreate(HWND hWnd, LPCREATESTRUCT lpCS)
{
    HWND hW = m_wndPaneHeader.Create(hWnd, lpCS->hInstance, &m_wndPaneHeader);
    //ShowWindow(hW, SW_SHOW);

    hW = m_wndTreeForlders.Create(hWnd, lpCS->hInstance, &m_wndPaneHeader);
    //ShowWindow(hW, SW_SHOW);

    return 1;
}

void CFolderPane::OnSize(UINT nType, UINT nWidth, UINT nHeight)
{
    if (nType == SIZE_MINIMIZED)
        return;

    RECT rect;
    GetClientRect(m_hWnd, &rect);

    MoveWindow(m_wndPaneHeader.m_hWnd, rect.left, rect.top, rect.right - rect.left, 30, TRUE);
    MoveWindow(m_wndTreeForlders.m_hWnd, rect.left, 30, rect.right - rect.left, rect.bottom - 30, TRUE);

    InvalidateRect(m_hWnd, NULL, TRUE);
}
