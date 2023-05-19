#include <windows.h>
#include <windowsx.h>
#include "listview.h"
#include "globals.h"
#include "resource.h"
#include <Uxtheme.h>
#include <vsstyle.h>

LRESULT CALLBACK ListProcEx(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    UNALIGNED CListView* pThis = (CListView*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (Message == WM_NCCREATE)
    {
        LPCREATESTRUCT lpCS = (LPCREATESTRUCT)lParam;
        CListView* pThis = (CListView*)lpCS->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->m_hWnd = hwnd;
    }

    if (!pThis)
        return ::DefWindowProc(hwnd, Message, wParam, lParam);

    return pThis->WindowProcedure(hwnd, Message, wParam, lParam);
}

LRESULT  CListView::WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_ERASEBKGND:
            return 1;

        case WM_PAINT:
        {
            RECT rect;
            BOOL bRes = GetUpdateRect(hWnd, &rect, FALSE);
            if (!bRes)
                return 0;

            OnPaint();

            return 0;
        }

        case WM_SIZE:
        {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
            OnSize((UINT)wParam, width, height);
            break;
        }

        case WM_NCLBUTTONDOWN:
        {
            HWND hParent = ::GetParent(m_hWnd);
            ::PostMessage(hParent, WM_NCLBUTTONDOWN, (WPARAM)0, (LPARAM)0);
            break;
        }

        case WM_LBUTTONDOWN:
        {
            HWND hParent = ::GetParent(m_hWnd);
            ::PostMessage(hParent, WM_LBUTTONDOWN, (WPARAM)0, (LPARAM)0);
            break;
        }


        default:
            return ::CallWindowProc((WNDPROC)oldListWndProc, hWnd, message, wParam, lParam);
            //return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

CListView::CListView()
    : m_hWnd(NULL)
    , oldListWndProc(NULL)
    , m_fScale(NULL)
    , m_pDirect2dFactory(NULL)
    , m_pRenderTarget(NULL)
    , m_pBr0(NULL)
    , m_pBr1(NULL)
{

}

CListView::~CListView()
{
    SafeRelease(&m_pDirect2dFactory);
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pBr0);
    SafeRelease(&m_pBr1);
}

HWND CListView::Create(HWND hWndParent, HINSTANCE hInstance, LPVOID lpParam)
{
    HRESULT hr = CreateDeviceIndependentResources();
    if (!SUCCEEDED(hr))
        return NULL;

    m_hWnd = CreateWindowEx(
        //WS_EX_CLIENTEDGE |
        WS_EX_TRANSPARENT |
        WS_EX_COMPOSITED,
        WC_LISTVIEW, //m_szFolderClassName,
        NULL,
        WS_VISIBLE | WS_CHILD | WS_VSCROLL, // | WS_BORDER,
        0,
        0,
        200,
        200,
        hWndParent,
        NULL,
        hInstance,
        (LPVOID)this);

    if (m_hWnd)
    {
        SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);
        oldListWndProc = (LONG_PTR)SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)ListProcEx);
        SetWindowTheme(m_hWnd, L"Explorer", NULL);
    }

    return m_hWnd;
}

HRESULT CListView::CreateDeviceIndependentResources()
{
    // Create a Direct2D factory.
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);

    return hr;
}

HRESULT CListView::CreateDeviceDependentResources()
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
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::SkyBlue), &m_pBr0);
        if (SUCCEEDED(hr))
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGray), &m_pBr1);

        float dpiX{ 96.0f };
        float dpiY{ 96.0f };
        if (m_pRenderTarget)
            m_pRenderTarget->GetDpi(&dpiX, &dpiY);
        m_fScale = dpiX / 96.0f;

        // Force WM_NCCALCSIZE
        //SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOCOPYBITS);
    }

    return hr;
}

void CListView::DiscardDeviceDependentResources()
{
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pBr0);
    SafeRelease(&m_pBr1);
}

void CListView::OnSize(UINT nType, UINT nWidth, UINT nHeight)
{

}

void CListView::OnPaint()
{
    if (m_hWnd == NULL)
        return;

    HRESULT hr = CreateDeviceDependentResources();

    if (SUCCEEDED(hr))
    {
        m_pRenderTarget->BeginDraw();

        // Begin painting
        //========================================================================

        OnPaint2D(m_pRenderTarget);

        //========================================================================
        // End painting
        hr = m_pRenderTarget->EndDraw();
        if (hr == D2DERR_RECREATE_TARGET)
        {
            hr = S_OK;
            DiscardDeviceDependentResources();
        }
    }
}

HRESULT CListView::OnPaint2D(ID2D1HwndRenderTarget* pRT)
{
    pRT->Clear(D2D1::ColorF(D2D1::ColorF::WhiteSmoke));

    return S_OK;
}