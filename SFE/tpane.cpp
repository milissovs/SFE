#include <windows.h>
#include <windowsx.h>
#include "tpane.h"
#include "globals.h"
#include "resource.h"
//#include <vsstyle.h>

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

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            OnPaint(hdc, ps.rcPaint);

            EndPaint(hWnd, &ps);

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
            POINT pt = {LOWORD(lParam), HIWORD(lParam)};
            HWND hParent = ::GetParent(m_hWnd);
            ::PostMessage(hParent, WM_LBUTTONDOWN, (WPARAM)0, (LPARAM)0);
            //OnLButtonDown(pt);
            break;
        }

        //case WM_MOUSEMOVE:
        //{
        //    if (!m_bMouseTracking)
        //    {
        //        TRACKMOUSEEVENT tme;
        //        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        //        tme.dwFlags = TME_LEAVE;
        //        tme.hwndTrack = hWnd;
        //        TrackMouseEvent(&tme);
        //        m_bMouseTracking = TRUE;
        //    }

        //    POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        //    OnMouseMove((UINT)wParam, pt);
        //    break;
        //}

        case WM_MOUSELEAVE:
        {
            //SendMessage(m_hWndTT, TTM_TRACKACTIVATE, FALSE, (LPARAM)&toolTipInfo);
            //m_bMouseTracking = FALSE;
            //for (int i = 0; i < 3; i++)
            //{
            //    m_btns[i].SetStateHovered(FALSE);
            //    m_btns[i].SetStateHoveredDrop(FALSE);
            //}
            //InvalidateRect(m_hWnd, NULL, NULL);
            break;
        }

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;
            switch (pnmh->code)
            {
                case NM_CUSTOMDRAW:
                {
                    const auto& lpNMCustomDraw = (LPNMTTCUSTOMDRAW)lParam;
                    //if (pnmh->hwndFrom == m_hWndTT)
                    //{
                    //    //NMTTCUSTOMDRAW* pnmtt = (NMTTCUSTOMDRAW*)pnmh;
                    //    return  OnToolTipCustomDraw(lpNMCustomDraw);
                    //}
                    break;
                }
                case TTN_GETDISPINFO:
                {
                    LPNMTTDISPINFO pInfo = (LPNMTTDISPINFO)lParam;
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

CTPane::CTPane()
    : m_hWnd(NULL)
    , m_hWndTT(NULL)
    , m_fScale(1.0f)
    , m_nHeight(28)
    , m_wndBtnPane()
    //, m_pDirect2dFactory(NULL)
    //, m_pRenderTarget(NULL)
    //, m_pDWriteFactory(NULL)
    //, m_spInlineObjec_TFC(NULL)
    //, m_spInlineObjec_TFL(NULL)
    //, m_pTFC(NULL)
    //, m_pTFL(NULL)
    //, m_pBr0(NULL)
    //, m_pBr1(NULL)
    //, m_bMouseTracking(FALSE)
    //, toolTipInfo()
    //, m_nHoveredButton(-1)
    //, m_pBrBorder0(NULL)
    //, m_pBrBorder1(NULL)
    //, m_pBrText0(NULL)
    //, m_pBrText1(NULL)
    //, m_rcBC()
    //, m_bEditMode(FALSE)
{

}

CTPane::~CTPane()
{
    //SafeRelease(&m_pDirect2dFactory);
    //SafeRelease(&m_pRenderTarget);
    //SafeRelease(&m_pBr0);
    //SafeRelease(&m_pBr1);
    //SafeRelease(&m_pBrBorder0);
    //SafeRelease(&m_pBrBorder1);
    //SafeRelease(&m_pBrText0);
    //SafeRelease(&m_pBrText1);
    //SafeRelease(&m_pTFC);
    //SafeRelease(&m_pTFL);
}

HWND CTPane::Create(HWND hWndParent, HINSTANCE hInstance, LPVOID lpParam)
{
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
        WS_VISIBLE | WS_CHILD,// | WS_BORDER, | WS_CLIPCHILDREN
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

int CTPane::OnCreate(HWND hWnd, LPCREATESTRUCT lpCS)
{
    m_wndBtnPane.Create(hWnd, lpCS->hInstance, &m_wndBtnPane);
    m_wndBC.Create(hWnd, lpCS->hInstance, &m_wndBC);

    m_wndBC.SetPath(TEST_PATH);

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

    RECT rc = { 1, 1, (int)(m_wndBtnPane.GetWidth() * m_fScale), (int)(28 * m_fScale) };  //
    MoveWindow(m_wndBtnPane.m_hWnd, rc.left, rc.top, rc.right, rc.bottom - 2, TRUE);

    MoveWindow(m_wndBC.m_hWnd, rc.right + 1, rc.top - 1, nWidth - rc.right - 1, nHeight - 1, TRUE);
}

HRESULT CTPane::OnPaint(HDC hdc, RECT rect)
{
    if (m_hWnd == NULL)
        return S_OK;

    RECT rc;
    GetClientRect(m_hWnd, &rc);

    HRESULT hr = S_OK;

    HPEN pen0 = CreatePen(PS_INSIDEFRAME, 1, RGB(227, 227, 227));
    HPEN pen1 = CreatePen(PS_INSIDEFRAME, 1, RGB(160, 160, 160));
    HPEN pen2 = CreatePen(PS_INSIDEFRAME, 1, RGB(255, 255, 255));
    HPEN pen3 = CreatePen(PS_INSIDEFRAME, 1, RGB(105, 105, 105));

    //HBRUSH hbr = CreateSolidBrush(0xFF0000);

    HGDIOBJ old = SelectObject(hdc, pen2);

    //FillRect(hdc, &rect, hbr);
    MoveToEx(hdc, rect.left, rect.top, NULL);
    LineTo(hdc, rect.right, rect.top);
    MoveToEx(hdc, rect.left, rect.top, NULL);
    LineTo(hdc, rect.left, rect.bottom - 1);

    SelectObject(hdc, pen1);
    LineTo(hdc, rect.right - 1, rect.bottom - 1);
    LineTo(hdc, rect.right - 1, rect.top);

    //
    int nOffset = (int)(m_wndBtnPane.GetWidth() * m_fScale);
    MoveToEx(hdc, nOffset + 1, rect.bottom - 1, NULL);
    LineTo(hdc, nOffset + 1, rect.top);
    LineTo(hdc, rect.right - 1, rect.top);

    SelectObject(hdc, pen3);
    MoveToEx(hdc, nOffset + 2, rect.bottom - 2, NULL);
    LineTo(hdc, nOffset + 2, rect.top + 1);
    LineTo(hdc, rect.right - 2, rect.top + 1);

    SelectObject(hdc, pen2);
    MoveToEx(hdc, nOffset + 1, rect.bottom - 2, NULL);
    LineTo(hdc, rect.right - 1, rect.bottom - 2);
    LineTo(hdc, rect.right - 1, rect.top);

    SelectObject(hdc, pen0);
    MoveToEx(hdc, nOffset + 2, rect.bottom - 3, NULL);
    LineTo(hdc, rect.right - 2, rect.bottom - 3);
    LineTo(hdc, rect.right - 2, rect.top);

    SelectObject(hdc, old);
    DeleteObject(pen0);
    DeleteObject(pen1);
    DeleteObject(pen2);
    DeleteObject(pen3);

    //HRESULT hr = CreateDeviceDependentResources();

    //if (SUCCEEDED(hr))
    //{
    //    m_pRenderTarget->BeginDraw();
    //    m_pRenderTarget->Clear(D2D1::ColorF(GetSysColor(COLOR_WINDOW)));
    //
    //    //========================================================================
    //    // Begin painting
    //    //========================================================================
    //    hr = OnPaint2D(m_pRenderTarget, NULL);
    //
    //    //========================================================================
    //    // End painting
    //    //========================================================================
    //
    //    hr = m_pRenderTarget->EndDraw();
    //
    //    if (hr == D2DERR_RECREATE_TARGET)
    //    {
    //        hr = S_OK;
    //        DiscardDeviceDependentResources();
    //    }
    //}
    return hr;
}
