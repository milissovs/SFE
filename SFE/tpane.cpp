#include <windows.h>
#include <windowsx.h>
#include "tpane.h"
#include "globals.h"
#include "resource.h"
#include <vsstyle.h>

TCHAR m_szTPaneClassName[] = TEXT("TPANE");

LRESULT CALLBACK TPaneProcEx(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    UNALIGNED CTPane* pThis = (CTPane*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (Message == WM_NCCREATE)
    {
        LPCREATESTRUCT lpCS = (LPCREATESTRUCT)lParam;
        CTPane* pThis = (CTPane*)lpCS->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->m_hWnd = hwnd;
    }

    if (!pThis)
        return ::DefWindowProc(hwnd, Message, wParam, lParam);

    return pThis->WindowProcedure(hwnd, Message, wParam, lParam);
}

LRESULT  CTPane::WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_ERASEBKGND:
            return 1;

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

        case WM_PAINT:
        {
            RECT rect;
            BOOL bRes = GetUpdateRect(hWnd, &rect, FALSE);
            if (!bRes)
                return 0;

            OnPaint();

            return 0;
        }

        case WM_NCLBUTTONDOWN:
        {
            HWND hParent = ::GetParent(m_hWnd);
            ::PostMessage(hParent, WM_NCLBUTTONDOWN, (WPARAM)0, (LPARAM)0);
            break;
        }

        case WM_LBUTTONDOWN:
        {
            UINT nX = LOWORD(lParam);
            UINT nY = HIWORD(lParam);
            HWND hParent = ::GetParent(m_hWnd);
            ::PostMessage(hParent, WM_LBUTTONDOWN, (WPARAM)0, (LPARAM)0);
            break;
        }

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

CTPane::CTPane()
    : m_hWnd(NULL)
    , m_hTheme(NULL)
    , m_fScale(1.0f)
    , m_nHeight(28 * 2)
    , m_pDirect2dFactory(NULL)
    , m_pRenderTarget(NULL)
    , m_pBr0(NULL)
    , m_pBr1(NULL)
{

}

CTPane::~CTPane()
{
    SafeRelease(&m_pDirect2dFactory);
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pBr1);
    SafeRelease(&m_pBr1);
    CloseThemeData(m_hTheme);
}

HWND CTPane::Create(HWND hWndParent, HINSTANCE hInstance, LPVOID lpParam)
{
    HRESULT hr = CreateDeviceIndependentResources();
    if (!SUCCEEDED(hr))
        return NULL;

    WNDCLASSEX wincl;

    /* The Window structure */
    wincl.cbSize = sizeof(WNDCLASSEX);
    wincl.hInstance = hInstance;
    wincl.lpszClassName = m_szTPaneClassName;
    wincl.lpfnWndProc = TPaneProcEx;
    wincl.style = CS_DBLCLKS;

    /* Use default icon and mouse-pointer */
    wincl.hIcon = NULL;
    wincl.hIconSm = NULL;

    wincl.hCursor = (HCURSOR)LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = sizeof(CTPane*); //sizeof(LONG_PTR)
    wincl.cbWndExtra = 0;

    /* Use Windows's default color as the background of the window */
    wincl.hbrBackground = (HBRUSH)GetSysColorBrush(COLOR_3DFACE); // GetStockObject(LTGRAY_BRUSH);

    /* Register the window class, and if it fails quit the program */
    if (!::RegisterClassEx(&wincl))
    {
        DWORD dwError = GetLastError();
        if (dwError != ERROR_CLASS_ALREADY_EXISTS)
            return FALSE;
    }

    m_hWnd = CreateWindowEx(
        WS_EX_COMPOSITED | WS_EX_TRANSPARENT,
        m_szTPaneClassName,
        NULL,
        WS_VISIBLE | WS_CHILD,// | WS_BORDER,
        0,
        0,
        200,
        200,
        hWndParent,
        NULL,
        hInstance,
        (LPVOID)this);

    BOOL bThemed = IsAppThemed();
    m_hTheme = OpenThemeData(m_hWnd, L"Button");

    HDC hDC = ::GetDC(m_hWnd);
    INT xdpi = ::GetDeviceCaps(hDC, LOGPIXELSX);
    ::ReleaseDC(m_hWnd, hDC);
    float m_fScale = xdpi / 96.0f;

    return m_hWnd;
}

int CTPane::OnCreate(HWND hWnd, LPCREATESTRUCT lpCS)
{
    return 1;
}

void CTPane::SetHeight(int nHeight)
{
    m_nHeight = nHeight;
}

void CTPane::OnSize(UINT nType, UINT nWidth, UINT nHeight)
{
    if (nType == SIZE_MINIMIZED)
        return;

    if (m_pRenderTarget)
        m_pRenderTarget->Resize(D2D1::SizeU(nWidth, nHeight));
}

HRESULT CTPane::CreateDeviceIndependentResources()
{
    return D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);
}

HRESULT CTPane::CreateDeviceDependentResources()
{
    HRESULT hr = S_OK;

    if (m_hWnd == NULL)
        return S_OK;

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
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xA0A0A0), &m_pBr0);
        if (SUCCEEDED(hr))
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x696969), &m_pBr1);

        float dpiX = { 0 };
        float dpiY = { 0 };
        if (m_pRenderTarget)
            m_pRenderTarget->GetDpi(&dpiX, &dpiY);
        m_fScale = dpiX / 96.0f;
    }

    return hr;
}

void CTPane::DiscardDeviceDependentResources()
{
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pBr0);
    SafeRelease(&m_pBr1);
}

HRESULT CTPane::OnPaint()
{
    if (m_hWnd == NULL)
        return S_OK;

    RECT rc;
    GetWindowRect(m_hWnd, &rc);

    HRESULT hr = CreateDeviceDependentResources();

    if (SUCCEEDED(hr))
    {
        PAINTSTRUCT ps;
        HDC hDC = BeginPaint(m_hWnd, &ps);

        m_pRenderTarget->BeginDraw();
        m_pRenderTarget->Clear(D2D1::ColorF(GetSysColor(COLOR_WINDOW)));

        //========================================================================
        // Begin painting
        //========================================================================
        hr = OnPaint2D(m_pRenderTarget, hDC);
        //========================================================================
        // End painting
        //========================================================================

        hr = m_pRenderTarget->EndDraw();

        RECT rc;
        rc.left = 3;
        rc.top = 3;
        rc.right = rc.left + 36;
        rc.bottom = rc.top + 36;
        HRESULT hr = DrawThemeBackground(m_hTheme, hDC, TP_BUTTON, TS_PRESSED, &rc, 0);
        rc.top = rc.bottom + 3;
        rc.bottom = rc.top + 36;
        hr = DrawThemeBackground(m_hTheme, hDC, BP_PUSHBUTTON, PBS_HOT, &rc, 0);

        EndPaint(m_hWnd, &ps);

        if (hr == D2DERR_RECREATE_TARGET)
        {
            hr = S_OK;
            DiscardDeviceDependentResources();
        }
    }
    return hr;
}

HRESULT CTPane::OnPaint2D(ID2D1HwndRenderTarget* pRT, HDC hDC)
{
    D2D1_SIZE_F size = pRT->GetSize();

    D2D1_POINT_2F p0{ 0, size.height - 0.5f };
    D2D1_POINT_2F p1{ size.width, size.height - 0.5f };
    pRT->DrawLine(p0, p1, m_pBr1);

    //ID2D1GdiInteropRenderTarget * m_pGDIRT;
    //D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
    //rtProps.usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;

    //D2D1_SIZE_F fs = pRT->GetSize();
    //D2D1_SIZE_U us = { (UINT)fs.width, (UINT)fs.height };

    //// Create a GDI compatible Hwnd render target.
    //HRESULT hr = m_pDirect2dFactory->CreateHwndRenderTarget(
    //    rtProps,
    //    D2D1::HwndRenderTargetProperties(m_hWnd, us),
    //    &pRT);


    //if (SUCCEEDED(hr))
    //{
    //    hr = m_pRenderTarget->QueryInterface(__uuidof(ID2D1GdiInteropRenderTarget), (void**)&m_pGDIRT);

    //    HDC hDC = NULL;
    //    hr = m_pGDIRT->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hDC);

    //    if (SUCCEEDED(hr))
    //    {
    //    }
    //}


    return S_OK;
}