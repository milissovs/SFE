#include <windows.h>
#include <windowsx.h>
//#include <wingdi.h>
#include "globals.h"
#include "resource.h"
#include "splitterwnd.h"

TCHAR m_szSplitterClassName[] = TEXT("SPLITTER");

LRESULT CALLBACK SplitterProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    CSplitterWnd* pThis = (CSplitterWnd*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    return pThis->WindowProcedure(hwnd, Message, wParam, lParam);
}

LRESULT  CSplitterWnd::WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_ERASEBKGND:
            return 1;

        case WM_CREATE:
        {
            LPCREATESTRUCT lpCS = (LPCREATESTRUCT)lParam;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)lpCS->lpCreateParams);
            return OnCreate(hWnd, (LPCREATESTRUCT)lParam);
            //return 1;
        }

        case WM_SIZE:
        {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
            OnSize((UINT)wParam, width, height);
            break;
        }

        case WM_PAINT:
        {
            RECT rect;
            BOOL bRes = GetUpdateRect(hWnd, &rect, FALSE);
            if (!bRes)
                return 0;

            OnPaint2D();

            return 0;
        }

        case WM_MOUSEMOVE:
        {
            POINT pt{ GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) };
            OnMousemove((DWORD)wParam, pt);
            break;
        }

        case WM_LBUTTONDOWN:
        {
            POINT pt{ GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) };
            m_LBDown = TRUE;
            SetCapture(hWnd);
            break;
        }

        case WM_LBUTTONDBLCLK:
        {
            POINT pt{ GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) };
            OnLButtonDlclk((DWORD)wParam, pt);
            break;
        }

        case WM_LBUTTONUP:
        {
            m_LBDown = FALSE;
            ReleaseCapture();
            break;
        }

        case WM_SETCURSOR:
        {
            if (m_split_type == SPLIT_TYPE::SPLIT_VERT)
                SetCursor(hCursorNS);
            if (m_split_type == SPLIT_TYPE::SPLIT_HORZ)
                SetCursor(hCursorEW);
            return TRUE;
        }

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

CSplitterWnd::CSplitterWnd()
    : m_hWnd(NULL)
    , m_hParent(NULL)
    , nSplitterH(6)
    , m_fRatio(0.5f)
    , hCursorNS(NULL)
    , hCursorEW(NULL)
    , m_LBDown(FALSE)
    , m_split_type(SPLIT_NONE)
    , m_pDirect2dFactory(NULL)
    , m_pRenderTarget(NULL)
    , m_pBr1(NULL)
    , m_pBr2(NULL)
{
    hCursorNS = LoadCursor(NULL, IDC_SIZENS);
    hCursorEW = LoadCursor(NULL, IDC_SIZEWE);
}

CSplitterWnd::~CSplitterWnd()
{
    SafeRelease(&m_pDirect2dFactory);
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pBr1);
    SafeRelease(&m_pBr2);
}

BOOL CSplitterWnd::RegisterClassEx(HINSTANCE hInstance)
{
    WNDCLASSEX wincl;

    /* The Window structure */
    wincl.cbSize = sizeof(WNDCLASSEX);
    wincl.hInstance = hInstance;
    wincl.lpszClassName = m_szSplitterClassName;
    wincl.lpfnWndProc = SplitterProc;
    wincl.style = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;

    /* Use default icon and mouse-pointer */
    wincl.hIcon = NULL;
    wincl.hIconSm = NULL;

    wincl.hCursor = (HCURSOR)LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = sizeof(CSplitterWnd*); //sizeof(LONG_PTR)
    wincl.cbWndExtra = 0;

    /* Use Windows's default color as the background of the window */
    //wincl.hbrBackground = (HBRUSH)COLOR_WINDOW + 1;
    wincl.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    //wincl.hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);

    /* Register the window class, and if it fails quit the program */
    return ::RegisterClassEx(&wincl);
}

HWND CSplitterWnd::Create(
    HWND hWndParent,
    HINSTANCE hInstance,
    LPVOID lpParam)
{
    HRESULT hr = CreateDeviceIndependentResources();
    if (!SUCCEEDED(hr))
        return NULL;

    if (!RegisterClassEx(hInstance))
        return NULL;

    m_hWnd = CreateWindowEx(
        0,                      /* Extended possibilites for variation */
        m_szSplitterClassName,  /* Classname */
        NULL,                   /* Title Text */
        WS_CHILDWINDOW | WS_VISIBLE,         /* child window */
        0,                      /* Windows decides the position */
        0,                      /* where the window ends up on the screen */
        100,                    /* The programs width */
        100,                    /* and height in pixels */
        hWndParent,             /* The window is a child-window to desktop */
        NULL,                   /* No menu */
        hInstance,              /* Program Instance handler */
        this                    /* Window Creation data */
    );

    m_hParent = hWndParent;

    return m_hWnd;
}

void CSplitterWnd::OnPaint(HDC hdc)
{
    if (m_hWnd == NULL)
        return;

    if (m_split_type == SPLIT_TYPE::SPLIT_NONE)
        return;

    //HBRUSH hBrush;
    HPEN   hPen;
    HPEN   hOld{NULL};
    RECT   rect;
    GetClientRect(m_hWnd , &rect);

    if (m_split_type == SPLIT_TYPE::SPLIT_VERT)
    {
        hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
        hOld = (HPEN)SelectObject(hdc, hPen);
        MoveToEx(hdc, 0, 0, NULL);
        LineTo(hdc, rect.right, 0);

        hPen = CreatePen(PS_SOLID, 1, RGB(167, 167, 167));
        SelectObject(hdc, hPen);
        MoveToEx(hdc, 0, rect.bottom - 1, NULL);
        LineTo(hdc, rect.right, rect.bottom - 1);
    }
    else if (m_split_type == SPLIT_TYPE::SPLIT_HORZ)
    {
        hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
        hOld = (HPEN)SelectObject(hdc, hPen);
        MoveToEx(hdc, 0, 0, NULL);
        LineTo(hdc, 0, rect.bottom);

        hPen = CreatePen(PS_SOLID, 1, RGB(167, 167, 167));
        SelectObject(hdc, hPen);
        MoveToEx(hdc, rect.right - 1, 0, NULL);
        LineTo(hdc, rect.right - 1, rect.bottom);
    }
    else
        return;

    SelectObject(hdc, hOld);
    ReleaseDC(m_hWnd, hdc);
}

void CSplitterWnd::OnSize(UINT nType, UINT nWidth, UINT nHeight)
{
    if (this == NULL)
        return;
    if (nType == SIZE_MINIMIZED)
        return;
    if (m_hWnd == NULL)
        return;
    if (m_pRenderTarget)
    {
        m_pRenderTarget->Resize(D2D1::SizeU(nWidth, nHeight));
    }
}

void CSplitterWnd::OnMousemove(DWORD vKeys, POINT& pt)
{
    static RECT rect;
    if (this == NULL)
        return;
    if (m_hWnd == NULL)
        return;

    if (m_LBDown)
    {
        ClientToScreen(m_hWnd, &pt);
        PostMessage(m_hParent, WM_SPLITTER, (WPARAM)1, MAKELPARAM(pt.x, pt.y));
        GetWindowRect(m_hWnd, &rect);
    }
}

void CSplitterWnd::OnLButtonDlclk(DWORD vKeys, POINT& pt)
{
    if (this == NULL)
        return;
    if (m_hWnd == NULL)
        return;

    PostMessage(m_hParent, WM_SPLITTER, (WPARAM)NULL, (LPARAM)NULL);
}

int CSplitterWnd::OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
    CSplitterWnd* pThis = (CSplitterWnd*)lpCreateStruct->lpCreateParams;

    return 1;
}

HRESULT CSplitterWnd::CreateDeviceIndependentResources()
{
    // Create a Direct2D factory.
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);

    return hr;
}

HRESULT CSplitterWnd::CreateDeviceDependentResources()
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
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pBr1);
        if (SUCCEEDED(hr))
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x696969), &m_pBr2);
    }

    return hr;
}

void CSplitterWnd::DiscardDeviceDependentResources()
{
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pBr1);
    SafeRelease(&m_pBr2);
}

HRESULT CSplitterWnd::OnPaint2D()
{
    if (m_hWnd == NULL)
        return S_OK;

    if (m_split_type == SPLIT_TYPE::SPLIT_NONE)
        return S_OK;

    HRESULT hr = CreateDeviceDependentResources();

    if (SUCCEEDED(hr))
    {
        m_pRenderTarget->BeginDraw();

        m_pRenderTarget->Clear(D2D1::ColorF(0xDDDDDD)); // D2D1::ColorF::WhiteSmoke));
        D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

        // Begin painting
        //========================================================================

        D2D1_RECT_F rect{ 0.0f, 0.0f, rtSize.width, rtSize.height };

        if (m_split_type == SPLIT_TYPE::SPLIT_VERT)
        {
            D2D1_POINT_2F pt1 = { 0.0f, 0.5f };
            D2D1_POINT_2F pt2 = { rtSize.width, 0.5f };
            m_pRenderTarget->DrawLine(pt1, pt2, m_pBr1);

            D2D1_POINT_2F pt3 = { 0.0f, rtSize.height - 0.5f };
            D2D1_POINT_2F pt4 = { rtSize.width, rtSize.height - 0.5f };
            m_pRenderTarget->DrawLine(pt3, pt4, m_pBr2);
        }
        if (m_split_type == SPLIT_TYPE::SPLIT_HORZ)
        {
            D2D1_POINT_2F pt1 = { 0.5f, 0.0f };
            D2D1_POINT_2F pt2 = { 0.5f, rtSize.height };
            m_pRenderTarget->DrawLine(pt1, pt2, m_pBr1);

            D2D1_POINT_2F pt3 = { rtSize.width - 0.5f, 0.0f };
            D2D1_POINT_2F pt4 = { rtSize.width - 0.5f, rtSize.height };
            m_pRenderTarget->DrawLine(pt3, pt4, m_pBr2);
        }

        //========================================================================
        // End painting
        hr = m_pRenderTarget->EndDraw();
        if (hr == D2DERR_RECREATE_TARGET)
        {
            hr = S_OK;
            DiscardDeviceDependentResources();
        }
    }
    return hr;
}
