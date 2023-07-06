#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include "btnpane.h"
#include "globals.h"
#include "resource.h"

//#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

TCHAR m_szBTNPaneClassName[] = TEXT("BTNPANE");

LRESULT CALLBACK BTNPaneProcEx(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    UNALIGNED CBTNPane* pThis = (CBTNPane*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (Message == WM_NCCREATE)
    {
        LPCREATESTRUCT lpCS = (LPCREATESTRUCT)lParam;
        CBTNPane* pThis = (CBTNPane*)lpCS->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->m_hWnd = hwnd;
    }

    if (!pThis)
        return ::DefWindowProc(hwnd, Message, wParam, lParam);

    return pThis->WindowProcedure(hwnd, Message, wParam, lParam);
}

LRESULT  CBTNPane::WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
            POINT pt = { LOWORD(lParam), HIWORD(lParam) };
            HWND hParent = ::GetParent(m_hWnd);
            ::PostMessage(hParent, WM_LBUTTONDOWN, (WPARAM)0, (LPARAM)0);
            OnLButtonDown(pt);
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
            SendMessage(m_hWndTT, TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_TI);
            m_bMouseTracking = FALSE;
            for (int i = 0; i < 3; i++)
            {
                m_btns[i].SetStateHovered(FALSE);
                m_btns[i].SetStateHoveredDrop(FALSE);
            }
            InvalidateRect(m_hWnd, NULL, NULL);
            break;
        }

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

CBTNPane::CBTNPane()
    : m_hWnd(NULL)
    , m_hWndTT(NULL)
    , m_fScale(1.0f)
    , m_btns()
    , m_bMouseTracking(FALSE)
    , m_nHoveredButton(-1)
    , m_TI()
    , m_pDirect2dFactory(NULL)
    , m_pRenderTarget(NULL)
    , m_pDWriteFactory(NULL)
    , m_pBrBorder0(NULL)
    , m_pBrBorder1(NULL)
    , m_pBrText0(NULL)
    , m_pBrText1(NULL)
    , m_pTFC(NULL)
    , m_spInlineObjec_TFC(NULL)
{

}

CBTNPane::~CBTNPane()
{
    SafeRelease(&m_pDirect2dFactory);
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pBrBorder0);
    SafeRelease(&m_pBrBorder1);
    SafeRelease(&m_pBrText0);
    SafeRelease(&m_pBrText1);
    SafeRelease(&m_pTFC);
    SafeRelease(&m_spInlineObjec_TFC);
}

void CBTNPane::DiscardDeviceDependentResources()
{
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pBrBorder0);
    SafeRelease(&m_pBrBorder1);
    SafeRelease(&m_pBrText0);
    SafeRelease(&m_pBrText1);
    SafeRelease(&m_pTFC);
    SafeRelease(&m_spInlineObjec_TFC);
}

HWND CBTNPane::Create(HWND hWndParent, HINSTANCE hInstance, LPVOID lpParam)
{
    HRESULT hr = CreateDeviceIndependentResources();
    if (!SUCCEEDED(hr))
        return NULL;

    WNDCLASSEX wincl;

    /* The Window structure */
    wincl.cbSize = sizeof(WNDCLASSEX);
    wincl.hInstance = hInstance;
    wincl.lpszClassName = m_szBTNPaneClassName;
    wincl.lpfnWndProc = BTNPaneProcEx;
    wincl.style = CS_DBLCLKS;

    /* Use default icon and mouse-pointer */
    wincl.hIcon = NULL;
    wincl.hIconSm = NULL;

    wincl.hCursor = (HCURSOR)LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = sizeof(CBTNPane*); //sizeof(LONG_PTR)
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
        m_szBTNPaneClassName,
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

    m_hWndTT = CreateTrackingToolTip(0, m_hWnd, hInstance);

    HDC hDC = ::GetDC(m_hWnd);
    INT xdpi = ::GetDeviceCaps(hDC, LOGPIXELSX);
    ::ReleaseDC(m_hWnd, hDC);
    m_fScale = xdpi / 96.0f;

    return m_hWnd;
}

HWND CBTNPane::CreateTrackingToolTip(int toolID, HWND hWndParent, HINSTANCE hInst)
{
    // Create a tooltip.
    HWND h = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
        WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        hWndParent, NULL, hInst, NULL);

    if (!h)
    {
        return NULL;
    }

    // Set up the tool information. In this case, the "tool" is the entire parent window.
    memset(&m_TI, 0, sizeof(TOOLINFO));
    m_TI.cbSize = sizeof(TOOLINFO);
    m_TI.uFlags = TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE | TTF_TRANSPARENT | TTF_SUBCLASS;
    m_TI.hwnd = hWndParent;
    m_TI.hinst = hInst;
    m_TI.lpszText = NULL; //LPSTR_TEXTCALLBACK
    m_TI.uId = (UINT_PTR)hWndParent;

    GetClientRect(hWndParent, &m_TI.rect);

    // Associate the tooltip with the tool window.
    if (!SendMessage(h, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&m_TI))
        return NULL;

    return h;
}

HRESULT CBTNPane::CreateDeviceIndependentResources()
{
    return D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);
}

HRESULT CBTNPane::CreateDeviceDependentResources()
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

        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(clrBORDER), &m_pBrBorder0);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(clrBORDER_GLOW), &m_pBrBorder1);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(clrTXT), &m_pBrText0);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(clrTXT_DISABLED), &m_pBrText1);

        m_pRenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE::D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(m_pDWriteFactory), reinterpret_cast<IUnknown**>(&m_pDWriteFactory));

        HRESULT hr = m_pDWriteFactory->CreateTextFormat(
            L"Segoe UI",
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

        //----------------------------------------------------------------
        for (int i = 0; i < 3; i++)
        {
            m_btns[i].InvalidateDependantResources(
                m_pRenderTarget,
                m_pBrBorder0,
                m_pBrBorder1,
                m_pBrText0,
                m_pBrText1,
                m_pTFC);
        }
    }

    return hr;
}

HRESULT CBTNPane::OnPaint()
{
    if (m_hWnd == NULL)
        return S_OK;

    RECT rc;
    GetWindowRect(m_hWnd, &rc);

    HRESULT hr = CreateDeviceDependentResources();

    if (SUCCEEDED(hr))
    {
        m_pRenderTarget->BeginDraw();
        m_pRenderTarget->Clear(D2D1::ColorF(GetSysColor(COLOR_WINDOW)));

        //========================================================================
        // Begin painting
        //========================================================================
        hr = OnPaint2D(m_pRenderTarget);

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

HRESULT CBTNPane::OnPaint2D(ID2D1HwndRenderTarget* pRT)
{
    m_btns[0].Draw(pRT);
    m_btns[1].Draw(pRT);
    m_btns[2].Draw(pRT);

    return S_OK;
}

int CBTNPane::OnCreate(HWND hWnd, LPCREATESTRUCT lpCS)
{
    m_btns[0].SetText(L"<"); //🡠🡨🡰🡸🢀
    m_btns[0].SetTooltip(L"Back");
    m_btns[1].SetText(L">"); //🡢🡪🡲🡺🢂
    m_btns[1].SetTooltip(L"Forward");
    m_btns[2].SetText(L"△"); //🡣🡫🡳🡻🢃
    m_btns[2].SetTooltip(L"Up to parent");
    m_btns[1].SetStateDisabled(TRUE);

    return 1;
}

void CBTNPane::OnSize(UINT nType, UINT nWidth, UINT nHeight)
{
    if (nType == SIZE_MINIMIZED)
        return;

    if (m_pRenderTarget)
        m_pRenderTarget->Resize(D2D1::SizeU(nWidth, nHeight));

    RECT rc = { 0, 0, 28, (int)nHeight };

    m_btns[0].SetRect(rc);

    rc.left += (int)m_btns[0].GetWidth();
    rc.right += (int)m_btns[0].GetWidth();
    m_btns[1].SetRect(rc);

    rc.left += (int)m_btns[1].GetWidth();
    rc.right += (int)m_btns[1].GetWidth();
    m_btns[2].SetRect(rc);
}

int CBTNPane::GetHoveredPutton()
{
    for (int i = 0; i < 3; i++)
    {
        if (m_btns[i].GetState() & BS_HOVERED)
            return i;
    }
    return -1;
}

void CBTNPane::OnLButtonDown(POINT pt)
{

}

void CBTNPane::OnMouseMove(UINT nFlag, POINT pt)
{
    POINT point = pt;
    point.x = (int)(pt.x / m_fScale);
    point.y = (int)(pt.y / m_fScale);

    for (int i = 0; i < 3; i++)
    {
        if (m_btns[i].IsMouseOver(point))
        {
            BOOL b = m_btns[i].GetState() & BS_HOVERED;
            if (!b)
                m_btns[i].SetStateHovered(TRUE);
        }
        else
        {
            m_btns[i].SetStateHovered(FALSE);
            m_btns[i].SetStateHoveredDrop(FALSE);
        }
    }

    int nHovered = GetHoveredPutton();
    if (nHovered >= 0)
    {
        if (m_nHoveredButton != nHovered)
        {
            m_nHoveredButton = nHovered;

            SendMessage(m_hWndTT, TTM_GETTOOLINFO, 0, (LPARAM)&m_TI);
            TCHAR txt[80];
            swprintf_s(txt, ARRAYSIZE(txt), L"%ls", m_btns[nHovered].GetTooltip());
            m_TI.lpszText = txt;
            SendMessage(m_hWndTT, TTM_SETTOOLINFO, 0, (LPARAM)&m_TI);

            SendMessage(m_hWndTT, TTM_SETDELAYTIME, TTDT_INITIAL, MAKELPARAM(250, 0));
            SendMessage(m_hWndTT, TTM_TRACKACTIVATE, TRUE, (LPARAM)&m_TI);

            RECT rc = m_btns[nHovered].GetRect();
            POINT btpt = { (int)(rc.left * m_fScale) + 2, (int)(rc.bottom * m_fScale) + 2 };
            ClientToScreen(m_hWnd, &btpt);
            SendMessage(m_hWndTT, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(btpt.x, btpt.y));
        }
    }
    else
        m_nHoveredButton = -1;
}

int CBTNPane::GetWidth()
{
    int nWidth = 0;
    for (int i = 0; i < 3; i++)
    {
        nWidth += (int)(m_btns[i].GetWidth());
    }
    return nWidth;
}
