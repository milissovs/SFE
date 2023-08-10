#include <windows.h>
#include "globals.h"
#include "resource.h"
#include "mainview.h"

TCHAR m_szMainViewClassName[] = TEXT("MAINVIEW");

LRESULT CALLBACK MainViewProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    UNALIGNED CMainView* pThis = (CMainView*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (Message == WM_NCCREATE)
    {
        LPCREATESTRUCT lpCS = (LPCREATESTRUCT)lParam;
        CMainView* pThis = (CMainView*)lpCS->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->m_hWnd = hwnd;
    }

    if (!pThis)
        return ::DefWindowProc(hwnd, Message, wParam, lParam);

    return pThis->WindowProcedure(hwnd, Message, wParam, lParam);
}

LRESULT  CMainView::WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    RECT rect;
    GetClientRect(hWnd, &rect);

    switch (message) 
    {
        case WM_ERASEBKGND:
        {
            return 1;
        }

        case WM_CREATE:
        {
            LPCREATESTRUCT lpCS = (LPCREATESTRUCT)lParam;
            return OnCreate(hWnd, (LPCREATESTRUCT)lParam);
        }

        case WM_SIZE:
        {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
            OnSize((UINT)wParam, width, height);
            break;
        }

        case WM_COMMAND:
        {
            WORD id = LOWORD(wParam);

            switch (id)
            {
                case ID_VIEW_SPLIT_NONE:
                {
                    OnIdViewSplitNone(rect);
                    break;
                }
                case ID_VIEW_SPLIT_VERT:
                {
                    OnIdViewSplitVert(rect);
                    break;
                }
                case ID_VIEW_SPLIT_HORZ:
                {
                    OnIdViewSplitHorz(rect);
                    break;
                }
            }
            break;
        }

        case WM_SPLITTER:
        {
            OnSplitter(rect, wParam, lParam);
            break;
        }

        case WM_FRAME_ACTIVE:
        {
            switch(wParam)
            {
                case 0:
                {
                    if (m_wndSplitter.m_split_type == CSplitterWnd::SPLIT_TYPE::SPLIT_NONE)
                    {

                    }
                    else
                    {
                        for (int i = 0; i < 2; i++)
                        {
                            CMDIFrame& mdi = m_wndMDIs[i];
                            if (mdi.m_hWnd == (HWND)lParam)
                                mdi.m_bActive = TRUE;
                            else
                                mdi.m_bActive = FALSE;

                            InvalidateRect(hWnd, NULL, TRUE);
                        }
                    }
                    break;
                }
                case 1:
                {
                    if (m_wndSplitter.m_split_type == CSplitterWnd::SPLIT_TYPE::SPLIT_NONE)
                        return 1;
                    else
                        return 0;
                    break;
                }
            }

            break;
        }

        case WM_UPDATE_SETTINGS:
        {
            SendMessage(GetParent(hWnd), WM_UPDATE_SETTINGS, 0, 0);
            break;
        }

        case WM_PAINT:
        {
            RECT rect;
            BOOL bRes = GetUpdateRect(hWnd, &rect, FALSE);
            if (!bRes)
                return 0;

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(m_hWnd, &ps);
            //OnPaint(hdc, ps.rcPaint);
            EndPaint(m_hWnd, &ps);

            return 0;
        }

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

CMainView::CMainView()
    : m_hWnd(NULL)
    , m_pDirect2dFactory(NULL)
    , m_pRenderTarget(NULL)
    , m_pBr1(NULL)
    , m_pBr2(NULL)
{

}

CMainView::~CMainView()
{
    SafeRelease(&m_pDirect2dFactory);
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pBr1);
    SafeRelease(&m_pBr2);
}

HWND CMainView::Create(HWND hWndParent, HINSTANCE hInstance, LPVOID lpParam)
{
    HRESULT hr = CreateDeviceIndependentResources();
    if (!SUCCEEDED(hr))
        return NULL;

    WNDCLASSEX wincl;

    /* The Window structure */
    wincl.cbSize = sizeof(WNDCLASSEX);
    wincl.hInstance = hInstance;
    wincl.lpszClassName = m_szMainViewClassName;
    wincl.lpfnWndProc = MainViewProc;
    wincl.style = CS_DBLCLKS; // | CS_VREDRAW | CS_HREDRAW;

    /* Use default icon and mouse-pointer */
    wincl.hIcon = NULL;
    wincl.hIconSm = NULL;

    wincl.hCursor = (HCURSOR)LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = sizeof(CMainView*);
    wincl.cbWndExtra = 0;

    /* Use Windows's default color as the background of the window */
    wincl.hbrBackground = (HBRUSH)COLOR_WINDOW + 1;

    /* Register the window class, and if it fails quit the program */
    if (!::RegisterClassEx(&wincl))
    {
        DWORD dwError = GetLastError();
        if (dwError != ERROR_CLASS_ALREADY_EXISTS)
            return FALSE;
    }

    m_hWnd = CreateWindowEx(
        WS_EX_COMPOSITED | WS_EX_TRANSPARENT | WS_EX_CONTROLPARENT, //,       /* Extended possibilites for variation */
        m_szMainViewClassName,  /* Classname */
        NULL,                   /* Title Text */
        WS_CHILDWINDOW | WS_VISIBLE,  /* child window */
        0,                      /* Windows decides the position */
        0,                      /* where the window ends up on the screen */
        100,                    /* The programs width */
        100,                    /* and height in pixels */
        hWndParent,             /* The window is a child-window to desktop */
        NULL,                   /* No menu */
        hInstance,              /* Program Instance handler */
        (LPVOID)this            /* Window Creation data */
    );

    return m_hWnd;
}

int CMainView::OnCreate(HWND hWnd, LPCREATESTRUCT lpCS)
{
    CMainView* pThis = (CMainView*)lpCS->lpCreateParams;

    HWND hW = pThis->m_wndSplitter.Create(hWnd, lpCS->hInstance, NULL);
    ::ShowWindow(hW, SW_SHOW);

    for (int i = 0; i < 2; i++)
    {
        m_wndMDIs[i].m_hWnd = m_wndMDIs[i].Create(hWnd, lpCS->hInstance, &m_wndMDIs[i]);
        m_wndMDIs[i].nID = i;
    }

    //for (int i = 0; i < 2; i++)
    //{
    //    hW = m_wndFolders[i].Create(hWnd, lpCS->hInstance, &m_wndMDIs[i]);
    //    ::ShowWindow(hW, SW_SHOW);
    //}

    return 1;
}

void CMainView::OnSize(UINT nType, UINT nWidth, UINT nHeight)
{
    if (nType == SIZE_MINIMIZED)
        return;

    RECT rect;
    GetClientRect(m_hWnd, &rect);

    RepositionChildren(rect);
}

RECT CMainView::GetSplitterPos(RECT rect)
{
    RECT r {};
    CSplitterWnd::SPLIT_TYPE st = m_wndSplitter.m_split_type;
    float ratio = m_wndSplitter.m_fRatio;

    switch (st)
    {
        case CSplitterWnd::SPLIT_TYPE::SPLIT_NONE:
        {
            ::ShowWindow(m_wndSplitter.m_hWnd, SW_HIDE);
            //::ShowWindow(m_wndFolders[1].m_hWnd, SW_HIDE);
            //::ShowWindow(m_wndFolders[0].m_hWnd, SW_SHOW);
            ::ShowWindow(m_wndMDIs[1].m_hWnd, SW_HIDE);
            ::ShowWindow(m_wndMDIs[0].m_hWnd, SW_SHOW);
            break;
        }
        case CSplitterWnd::SPLIT_TYPE::SPLIT_VERT:
        {
            r.left = rect.left;
            r.right = rect.right;
            r.top = (int)((rect.bottom - rect.top - m_wndSplitter.nSplitterH) * ratio);
            r.bottom = r.top + m_wndSplitter.nSplitterH;
            ::ShowWindow(m_wndSplitter.m_hWnd, SW_SHOW);
            //::ShowWindow(m_wndFolders[0].m_hWnd, SW_SHOW);
            //::ShowWindow(m_wndFolders[1].m_hWnd, SW_SHOW);
            ::ShowWindow(m_wndMDIs[0].m_hWnd, SW_SHOW);
            ::ShowWindow(m_wndMDIs[1].m_hWnd, SW_SHOW);
            break;
        }
        case CSplitterWnd::SPLIT_TYPE::SPLIT_HORZ:
        {
            r.top = rect.top;
            r.bottom = rect.bottom;
            r.left = (int)((rect.right - rect.left - m_wndSplitter.nSplitterH) * ratio);
            r.right = r.left + m_wndSplitter.nSplitterH;
            ::ShowWindow(m_wndSplitter.m_hWnd, SW_SHOW);
            //::ShowWindow(m_wndFolders[0].m_hWnd, SW_SHOW);
            //::ShowWindow(m_wndFolders[1].m_hWnd, SW_SHOW);
            ::ShowWindow(m_wndMDIs[0].m_hWnd, SW_SHOW);
            ::ShowWindow(m_wndMDIs[1].m_hWnd, SW_SHOW);
            break;
        }
    }
    return r;
}

void CMainView::OnIdViewSplitNone(RECT client_rect)
{
    if (m_wndSplitter.m_split_type == CSplitterWnd::SPLIT_TYPE::SPLIT_NONE)
        return;

    m_wndSplitter.m_split_type = CSplitterWnd::SPLIT_TYPE::SPLIT_NONE;
    ::ShowWindow(m_wndSplitter.m_hWnd, SW_HIDE);

    RepositionChildren(client_rect);
}

void CMainView::OnIdViewSplitVert(RECT client_rect)
{
    if (m_wndSplitter.m_split_type == CSplitterWnd::SPLIT_TYPE::SPLIT_VERT)
        return;

    m_wndSplitter.m_split_type = CSplitterWnd::SPLIT_TYPE::SPLIT_VERT;
    ::ShowWindow(m_wndSplitter.m_hWnd, SW_SHOW);

    RepositionChildren(client_rect);
}

void CMainView::OnIdViewSplitHorz(RECT client_rect)
{
    if (m_wndSplitter.m_split_type == CSplitterWnd::SPLIT_TYPE::SPLIT_HORZ)
        return;

    m_wndSplitter.m_split_type = CSplitterWnd::SPLIT_TYPE::SPLIT_HORZ;
    ::ShowWindow(m_wndSplitter.m_hWnd, SW_SHOW);

    RepositionChildren(client_rect);
}

void CMainView::OnSplitter(RECT rect, WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
        case 0: // Double click - center the splitter
        {
            m_wndSplitter.m_fRatio = 0.5f;
            RECT r = GetSplitterPos(rect);
            RepositionChildren(rect);

            return;
        }
        case 1: // Mouse move
        {
            if (m_wndSplitter.m_split_type != CSplitterWnd::SPLIT_TYPE::SPLIT_NONE)
            {
                POINT pt = { LOWORD(lParam), HIWORD(lParam) };
                ScreenToClient(m_hWnd, &pt);
                CalcSplitterRatio(rect, pt);
                RepositionChildren(rect);

                return;
            }
        }
    }
}

void CMainView::CalcSplitterRatio(RECT rect, POINT pt)
{
    if (m_wndSplitter.m_split_type == CSplitterWnd::SPLIT_TYPE::SPLIT_VERT)
    {
        m_wndSplitter.m_fRatio = (float)pt.y / (rect.bottom - rect.top - m_wndSplitter.nSplitterH);
    }
    if (m_wndSplitter.m_split_type == CSplitterWnd::SPLIT_TYPE::SPLIT_HORZ)
    {
        m_wndSplitter.m_fRatio = (float)pt.x / (rect.right - rect.left - m_wndSplitter.nSplitterH);
    }

    if (m_wndSplitter.m_fRatio > 1)
        m_wndSplitter.m_fRatio = 1;
    if (m_wndSplitter.m_fRatio < 0)
        m_wndSplitter.m_fRatio = 0;
}

void CMainView::RepositionChildren(RECT rect)
{
    RECT rSplit = GetSplitterPos(rect);

    HDWP defStruct = BeginDeferWindowPos(3);

    DeferWindowPos(defStruct, m_wndSplitter.m_hWnd, NULL, rSplit.left, rSplit.top, rSplit.right - rSplit.left, rSplit.bottom - rSplit.top, SWP_NOZORDER | SWP_NOCOPYBITS);

    RECT r0 = GetMDIPos(rect, rSplit, 0);
    ::ShowWindow(m_wndMDIs[0].m_hWnd, SW_SHOW);
    DeferWindowPos(defStruct, m_wndMDIs[0].m_hWnd, NULL, r0.left, r0.top, r0.right - r0.left, r0.bottom - r0.top, SWP_NOZORDER | SWP_NOCOPYBITS);

    if (m_wndSplitter.m_split_type != CSplitterWnd::SPLIT_TYPE::SPLIT_NONE)
    {
        RECT r1 = GetMDIPos(rect, rSplit, 1);
        ::ShowWindow(m_wndMDIs[1].m_hWnd, SW_SHOW);
        DeferWindowPos(defStruct, m_wndMDIs[1].m_hWnd, NULL, r1.left, r1.top, r1.right - r1.left, r1.bottom - r1.top, SWP_NOZORDER | SWP_NOCOPYBITS);
    }

    EndDeferWindowPos(defStruct);

    InvalidateRect(m_hWnd, NULL, FALSE);
}

RECT CMainView::GetMDIPos(RECT wRect, RECT sRect, int nIndex)
{
    RECT r{};
    CSplitterWnd::SPLIT_TYPE st = m_wndSplitter.m_split_type;

    switch (st)
    {
        case CSplitterWnd::SPLIT_TYPE::SPLIT_NONE:
        {
            r = wRect;
            break;
        }
        case CSplitterWnd::SPLIT_TYPE::SPLIT_VERT:
        {
            if (nIndex == 0)
            {
                r.left   = wRect.left;
                r.right  = wRect.right;
                r.top    = wRect.top;
                r.bottom = sRect.top;
            }
            if (nIndex == 1)
            {
                r.left = wRect.left;
                r.right = wRect.right;
                r.top = sRect.bottom;
                r.bottom = wRect.bottom;
            }
            break;
        }
        case CSplitterWnd::SPLIT_TYPE::SPLIT_HORZ:
        {
            if (nIndex == 0)
            {
                r.left = wRect.left;
                r.right = sRect.left;
                r.top = wRect.top;
                r.bottom = sRect.bottom;
            }
            if (nIndex == 1)
            {
                r.left = sRect.right;
                r.right = wRect.right;
                r.top = wRect.top;
                r.bottom = sRect.bottom;
            }
        }
    }
    return r;
}

void CMainView::SetActiveFrame(int nFrame)
{
    if (nFrame == 0)
    {
        m_wndMDIs[0].m_bActive = TRUE;
        m_wndMDIs[1].m_bActive = FALSE;
    }
    else
    {
        m_wndMDIs[0].m_bActive = FALSE;
        m_wndMDIs[1].m_bActive = TRUE;
    }
}

int CMainView::GetActiveFrame()
{
    for (int i = 0; i < 2; i++)
    {
        CMDIFrame& mdi = m_wndMDIs[i];
        if (mdi.m_bActive == TRUE)
            return i;
    }
    return -1;
}

void CMainView::SetFolderPositions(INT nFolderPositions[2])
{
    m_wndMDIs[0].SetFolderPanePosition(nFolderPositions[0]);
    m_wndMDIs[1].SetFolderPanePosition(nFolderPositions[1]);
}

void CMainView::SetFolderPaneWidth(INT nFolderPaneWidth[2], BOOL bInit)
{
    m_wndMDIs[0].SetFolderPaneWidth(nFolderPaneWidth[0], bInit);
    m_wndMDIs[1].SetFolderPaneWidth(nFolderPaneWidth[1], bInit);
}

void CMainView::GetFolderPositions(INT& nFP0, INT& nFP1)
{
    nFP0 = m_wndMDIs[0].GetFolderPanePosition();
    nFP1 = m_wndMDIs[1].GetFolderPanePosition();
}

void CMainView::GetFolderPaneWidth(INT& nW0, INT& nW1)
{
    nW0 = m_wndMDIs[0].GetFolderPaneWidth();
    nW1 = m_wndMDIs[1].GetFolderPaneWidth();
}

HRESULT CMainView::CreateDeviceIndependentResources()
{
    // Create a Direct2D factory.
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);

    return hr;
}

HRESULT CMainView::CreateDeviceDependentResources()
{
    HRESULT hr = S_OK;

    if (!m_pRenderTarget)
    {
        RECT rc;
        GetClientRect(m_hWnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(
            rc.right - rc.left,
            rc.bottom - rc.top);

        // Create a Direct2D render target.
        hr = m_pDirect2dFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hWnd, size),
            &m_pRenderTarget);

        if (SUCCEEDED(hr))
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::SkyBlue), &m_pBr1);
        if (SUCCEEDED(hr))
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGray), &m_pBr2);

        // Force WM_NCCALCSIZE
        SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOCOPYBITS);
    }

    return hr;
}

void CMainView::DiscardDeviceDependentResources()
{
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pBr1);
    SafeRelease(&m_pBr2);
}

HRESULT CMainView::OnPaint2D()
{
    return S_OK;
}

void CMainView::OnPaint(HDC hdc, RECT rect)
{
    RECT rSplitter = GetSplitterPos(rect);
    RECT r0 = GetMDIPos(rect, rSplitter, 0);
    RECT r1 = GetMDIPos(rect, rSplitter, 1);

    r0.left -= 3;
    r0.top -= 3;
    r0.right += 3;
    r0.bottom += 3;

    r1.left -= 3;
    r1.top -= 3;
    r1.right += 3;
    r1.bottom += 3;

    HPEN pen1{};
    HPEN pen2{};
    HPEN pen3{};
    HPEN pen4{};

    BOOL bSingleView = (BOOL)SendMessage(m_hWnd, WM_FRAME_ACTIVE, 1, 0);
    if (bSingleView)
    {
        pen1 = CreatePen(PS_INSIDEFRAME, 1, RGB(96, 96, 96));
        pen2 = CreatePen(PS_INSIDEFRAME, 1, RGB(165, 165, 165));

        HGDIOBJ old{ SelectObject(hdc, pen1) };

        Rectangle(hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top);

        SelectObject(hdc, pen2);
        Rectangle(hdc, 1, 1, rect.right - rect.left - 1, rect.bottom - rect.top - 1);

        SelectObject(hdc, pen1);
        Rectangle(hdc, 2, 2, rect.right - rect.left - 2, rect.bottom - rect.top - 2);

        SelectObject(hdc, old);
        ReleaseDC(m_hWnd, hdc);
        DeleteObject(pen1);
        DeleteObject(pen2);
    }
    else
    {
        pen1 = CreatePen(PS_INSIDEFRAME, 1, RGB(18, 184, 221));
        pen2 = CreatePen(PS_INSIDEFRAME, 1, RGB(13, 250, 255));
        pen3 = CreatePen(PS_INSIDEFRAME, 1, RGB(96, 96, 96));
        pen4 = CreatePen(PS_INSIDEFRAME, 1, RGB(165, 165, 165));

        HGDIOBJ old{ SelectObject(hdc, pen1) };

        if (GetActiveFrame() == 0)
        {
            SelectObject(hdc, pen1);
            Rectangle(hdc, 0, 0, r0.right - r0.left, r0.bottom - r0.top);
            SelectObject(hdc, pen2);
            Rectangle(hdc, 1, 1, r0.right - r0.left - 1, r0.bottom - r0.top - 1);
            SelectObject(hdc, pen1);
            Rectangle(hdc, 2, 2, r0.right - r0.left - 2, r0.bottom - r0.top - 2);

            SelectObject(hdc, pen3);
            Rectangle(hdc, 0, 0, r1.right - r1.left, r1.bottom - r1.top);
            SelectObject(hdc, pen4);
            Rectangle(hdc, 1, 1, r1.right - r1.left - 1, r1.bottom - r1.top - 1);
            SelectObject(hdc, pen3);
            Rectangle(hdc, 2, 2, r1.right - r1.left - 2, r1.bottom - r1.top - 2);
        }
        if (GetActiveFrame() == 1)
        {
            SelectObject(hdc, pen3);
            Rectangle(hdc, 0, 0, r0.right - r0.left, r0.bottom - r0.top);
            SelectObject(hdc, pen4);
            Rectangle(hdc, 1, 1, r0.right - r0.left - 1, r0.bottom - r0.top - 1);
            SelectObject(hdc, pen3);
            Rectangle(hdc, 2, 2, r0.right - r0.left - 2, r0.bottom - r0.top - 2);

            SelectObject(hdc, pen1);
            Rectangle(hdc, 0, 0, r1.right - r1.left, r1.bottom - r1.top);
            SelectObject(hdc, pen2);
            Rectangle(hdc, 1, 1, r1.right - r1.left - 1, r1.bottom - r1.top - 1);
            SelectObject(hdc, pen1);
            Rectangle(hdc, 2, 2, r1.right - r1.left - 2, r1.bottom - r1.top - 2);
        }

        SelectObject(hdc, old);
        ReleaseDC(m_hWnd, hdc);
        DeleteObject(pen1);
        DeleteObject(pen2);
    }
}