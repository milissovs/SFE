#include <windows.h>
#include <windowsx.h>
#include "breadcrumb.h"
#include "globals.h"
#include "resource.h"

TCHAR m_szTBCClassName[] = TEXT("BREADCRUMB");

LRESULT CALLBACK TBCProcEx(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    UNALIGNED CBreadCrumb* pThis = (CBreadCrumb*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (Message == WM_NCCREATE)
    {
        LPCREATESTRUCT lpCS = (LPCREATESTRUCT)lParam;
        CBreadCrumb* pThis = (CBreadCrumb*)lpCS->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->m_hWnd = hwnd;
    }

    if (!pThis)
        return ::DefWindowProc(hwnd, Message, wParam, lParam);

    return pThis->WindowProcedure(hwnd, Message, wParam, lParam);
}

CBreadCrumb::CBreadCrumb()
	: m_hWnd(0)
    , m_fScale(1.0f)
    , m_nHeight(24)
    , m_pBr0(NULL)
    , m_pBr1(NULL)
    , m_pDirect2dFactory(NULL)
    , m_pRenderTarget(NULL)
    , m_pDWriteFactory(NULL)
    , m_pTFC(NULL)
    , m_spInlineObjec_TFC(NULL)
    , m_bMouseTracking(FALSE)
{

}

CBreadCrumb::~CBreadCrumb()
{
    SafeRelease(&m_pDirect2dFactory);
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pDWriteFactory);
    SafeRelease(&m_pBr0);
    SafeRelease(&m_pBr1);
}

LRESULT  CBreadCrumb::WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_ERASEBKGND:
            return 1;

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

        case WM_MOUSEMOVE:
        {
            if (!m_bMouseTracking)
            {
                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof(TRACKMOUSEEVENT);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hWnd;
                TrackMouseEvent(&tme);
                m_bMouseTracking = TRUE;
            }

            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            OnMouseMove((UINT)wParam, pt);
            break;
        }

        case WM_MOUSELEAVE:
        {
            m_bMouseTracking = FALSE;
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


        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
return 0;
}

HWND CBreadCrumb::Create(HWND hWndParent, HINSTANCE hInstance, LPVOID lpParam)
{
    HRESULT hr = CreateDeviceIndependentResources();
    if (!SUCCEEDED(hr))
        return NULL;

    WNDCLASSEX wincl;

    /* The Window structure */
    wincl.cbSize = sizeof(WNDCLASSEX);
    wincl.hInstance = hInstance;
    wincl.lpszClassName = m_szTBCClassName;
    wincl.lpfnWndProc = TBCProcEx;
    wincl.style = CS_DBLCLKS;

    /* Use default icon and mouse-pointer */
    wincl.hIcon = NULL;
    wincl.hIconSm = NULL;

    wincl.hCursor = (HCURSOR)LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = sizeof(CBreadCrumb*); //sizeof(LONG_PTR)
    wincl.cbWndExtra = 0;

    /* Use Windows's default color as the background of the window */
    wincl.hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH); //GetSysColorBrush(COLOR_3DFACE); 

    /* Register the window class, and if it fails quit the program */
    if (!::RegisterClassEx(&wincl))
    {
        DWORD dwError = GetLastError();
        if (dwError != ERROR_CLASS_ALREADY_EXISTS)
            return FALSE;
    }

    m_hWnd = CreateWindowEx(
        WS_EX_COMPOSITED | WS_EX_TRANSPARENT | WS_EX_CLIENTEDGE,
        m_szTBCClassName,
        NULL,
        WS_VISIBLE | WS_CHILD,
        0,
        0,
        200,
        200,
        hWndParent,
        NULL,
        hInstance,
        (LPVOID)this);

    HDC hDC = ::GetDC(m_hWnd);
    INT xdpi = ::GetDeviceCaps(hDC, LOGPIXELSX);
    ::ReleaseDC(m_hWnd, hDC);
    m_fScale = xdpi / 96.0f;

    return m_hWnd;
}

int CBreadCrumb::OnCreate(HWND hWnd, LPCREATESTRUCT lpCS)
{
    return 1;
}

HRESULT CBreadCrumb::CreateDeviceIndependentResources()
{
    return D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);
}

void CBreadCrumb::DiscardDeviceDependentResources()
{
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pDWriteFactory);
    SafeRelease(&m_pBr0);
    SafeRelease(&m_pBr1);
}

HRESULT CBreadCrumb::CreateDeviceDependentResources()
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

        // Create Brushes
        if (SUCCEEDED(hr))
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xA0A0A0), &m_pBr0);
        if (SUCCEEDED(hr))
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x696969), &m_pBr1);

        m_pRenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE::D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(m_pDWriteFactory), reinterpret_cast<IUnknown**>(&m_pDWriteFactory));

        // Create Text Foramt
        HRESULT hr = m_pDWriteFactory->CreateTextFormat(
            L"Colibi",
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            12,
            L"",
            &m_pTFC);

        hr = m_pTFC->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        hr = m_pTFC->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        if (m_pTFC != 0)
            hr = m_pDWriteFactory->CreateEllipsisTrimmingSign(m_pTFC, &m_spInlineObjec_TFC);
        hr = m_pTFC->SetTrimming(&trimming, m_spInlineObjec_TFC);

        float dpiX = { 0 };
        float dpiY = { 0 };
        if (m_pRenderTarget)
            m_pRenderTarget->GetDpi(&dpiX, &dpiY);
        m_fScale = dpiX / 96.0f;
    }

    return hr;
}

HRESULT CBreadCrumb::OnPaint()
{
    if (m_hWnd == NULL)
        return S_OK;

    RECT rc;
    GetWindowRect(m_hWnd, &rc);

    HRESULT hr = CreateDeviceDependentResources();

    if (SUCCEEDED(hr))
    {
        m_pRenderTarget->BeginDraw();
        m_pRenderTarget->Clear(D2D1::ColorF(0xFFFFFF)); // GetSysColor(COLOR_WINDOW)));

        //========================================================================
        // Begin painting
        //========================================================================
        hr = OnPaint2D(m_pRenderTarget, NULL);

        //========================================================================
        // End painting
        //========================================================================

        hr = m_pRenderTarget->EndDraw();

        if (hr == D2DERR_RECREATE_TARGET)
        {
            hr = S_OK;
            DiscardDeviceDependentResources();
        }
    }
    return hr;
}

HRESULT CBreadCrumb::OnPaint2D(ID2D1HwndRenderTarget* pRT, HDC hDC)
{
    D2D1_SIZE_F size = pRT->GetSize();

    //D2D1_POINT_2F p0{ 0, size.height - 0.5f };
    //D2D1_POINT_2F p1{ size.width, size.height - 0.5f };
    //pRT->DrawLine(p0, p1, m_pBr1);


    return S_OK;
}

void CBreadCrumb::OnMouseMove(UINT nFlag, POINT pt)
{
    //POINT point = pt;
    //point.x = (int)(pt.x / m_fScale);
    //point.y = (int)(pt.y / m_fScale);

    //InvalidateRect(m_hWnd, NULL, NULL);
}

void CBreadCrumb::OnSize(UINT nType, UINT nWidth, UINT nHeight)
{
    if (nType == SIZE_MINIMIZED)
        return;

    if (m_pRenderTarget)
        m_pRenderTarget->Resize(D2D1::SizeU(nWidth, nHeight));
}
