#include <windows.h>
#include <windowsx.h>
#include "paneheader.h"
#include "globals.h"
#include "resource.h"

TCHAR m_szPaneHeaderClassName[] = TEXT("PANEHEADER");

#define IDM_CONTEXT_LINE   1000
#define IDM_CONTEXT_RECTAN 1001
#define IDM_CONTEXT_CIRCLE 1002
#define IDM_MOVE_PANE      1003 

#define CX_BITMAP 16	// Each icon width  (tileset)
#define CY_BITMAP 16	// Each icon height (tileset)

LRESULT CALLBACK PaneHeaderProcEx(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    UNALIGNED CPaneHeader* pThis = (CPaneHeader*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (Message == WM_NCCREATE)
    {
        LPCREATESTRUCT lpCS = (LPCREATESTRUCT)lParam;
        CPaneHeader* pThis = (CPaneHeader*)lpCS->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->m_hWnd = hwnd;
    }

    if (!pThis)
        return ::DefWindowProc(hwnd, Message, wParam, lParam);

    return pThis->WindowProcedure(hwnd, Message, wParam, lParam);
}

LRESULT  CPaneHeader::WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    //case WM_ERASEBKGND:
    //    return 1;

        case WM_PAINT:
        {
            RECT rect;
            BOOL bRes = GetUpdateRect(hWnd, &rect, FALSE);
            if (!bRes)
                return 0;

            OnPaint2D();

            return 0;
        }

        case WM_COMMAND:
        {
            switch (wParam)
            {
                case IDM_MOVE_PANE:
                {
                    if (m_nPanePos == 0)
                        SetPosition(1);
                    else
                        SetPosition(0);

                    //Inform parent that need to change entire pane position
                    PostMessage(GetParent(m_hWnd), WM_PANE_FOLDER, 0, m_nPanePos);

                    break;
                }
            }
            break;
        }

        case WM_SIZE:
        {
            UINT width = GET_X_LPARAM(lParam);
            UINT height = GET_Y_LPARAM(lParam);
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
            UINT nX = GET_X_LPARAM(lParam);
            UINT nY = GET_Y_LPARAM(lParam);
            OnLButtonDown((UINT)wParam, nX, nY);

            HWND hParent = ::GetParent(m_hWnd);
            ::PostMessage(hParent, WM_LBUTTONDOWN, (WPARAM)0, (LPARAM)0);
            break;
        }

        case WM_MOUSELEAVE:
        {
            m_bMouseTracking = FALSE;
            bMouseOverBtn = -1;
            break;
        }

        case WM_RBUTTONDOWN:
        {
            //if (bMouseOverBtn == 0)
            //{
            //    POINT pt;
            //    HMENU hMenuPopup = CreatePopupMenu();
            //    AppendMenu(hMenuPopup, MF_STRING, IDM_CONTEXT_LINE, L"Line");
            //    AppendMenu(hMenuPopup, MF_STRING, IDM_CONTEXT_RECTAN, L"Rectangle");
            //    AppendMenu(hMenuPopup, MF_STRING, IDM_CONTEXT_CIRCLE, L"Circle");
            //    AppendMenu(hMenuPopup, MF_SEPARATOR, 0, NULL);
            //    AppendMenu(hMenuPopup, MF_STRING, IDM_CONTEXT_HELP, L"Help");
            //    pt.x = GET_X_LPARAM(lParam);
            //    pt.y = GET_Y_LPARAM(lParam);
            //    ClientToScreen(hWnd, &pt);
            //    TrackPopupMenu(hMenuPopup,
            //        TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON,
            //        pt.x, pt.y, 0, hWnd, NULL);
            //    DestroyMenu(hMenuPopup);
            //}
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
            POINT pt{ GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) };
            OnMouseMove((UINT)wParam, pt);
            break;
        }

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    return 0;
}

CPaneHeader::CPaneHeader()
    : m_hWnd(NULL)
    , m_pDirect2dFactory(NULL)
    , m_pRenderTarget(NULL)
    , m_pBrGray127(NULL)
    , m_pBrGray033(NULL)
    , m_pBrGray196(NULL)
    , m_pBr1(NULL)
    , m_pBr2(NULL)
    , m_pTFC(NULL)
    , m_pTFL(NULL)
    , m_pDWriteFactory(NULL)
    , m_spInlineObjec_TFC(NULL)
    , rButtons()
    , bMouseOverBtn(-1)
    , m_fScale(1.0f)
    , m_bMouseTracking(FALSE)
    , m_buttons()
    , m_pBrBlack(NULL)
    , m_nPanePos(1)
{
    m_buttons[0].title = L"☰"; //✈✜✴✶❖⭗📍🛧🛨🛪
    m_buttons[1].title = L">";
}

CPaneHeader::~CPaneHeader()
{
    SafeRelease(&m_pDirect2dFactory);
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pBrGray127);
    SafeRelease(&m_pBrGray033);
    SafeRelease(&m_pBrGray196);
    SafeRelease(&m_pBr1);
    SafeRelease(&m_pBr2);
    SafeRelease(&m_pBrBlack);
    SafeRelease(&m_pTFC);
    SafeRelease(&m_pDWriteFactory);
    SafeRelease(&m_spInlineObjec_TFC);
}

HWND CPaneHeader::Create(HWND hWndParent, HINSTANCE hInstance, LPVOID lpParam)
{
    HRESULT hr = CreateDeviceIndependentResources();
    if (!SUCCEEDED(hr))
        return NULL;

    WNDCLASSEX wincl;

    /* The Window structure */
    wincl.cbSize = sizeof(WNDCLASSEX);
    wincl.hInstance = hInstance;
    wincl.lpszClassName = m_szPaneHeaderClassName;
    wincl.lpfnWndProc = PaneHeaderProcEx;
    wincl.style = CS_DBLCLKS;

    /* Use default icon and mouse-pointer */
    wincl.hIcon = NULL;
    wincl.hIconSm = NULL;

    wincl.hCursor = (HCURSOR)LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = sizeof(CPaneHeader*); //sizeof(LONG_PTR)
    wincl.cbWndExtra = 0;

    /* Use Windows's default color as the background of the window */
    wincl.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
    //wincl.hbrBackground = (HBRUSH)::GetSysColorBrush(COLOR_ACTIVECAPTION);

    /* Register the window class, and if it fails quit the program */
    if (!::RegisterClassEx(&wincl))
    {
        DWORD dwError = GetLastError();
        if (dwError != ERROR_CLASS_ALREADY_EXISTS)
            return FALSE;
    }

    m_hWnd = CreateWindowEx(
        0,
        m_szPaneHeaderClassName,
        NULL,
        WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN,
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

HRESULT CPaneHeader::CreateDeviceIndependentResources()
{
    // Create a Direct2D factory.
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);

    return hr;
}

HRESULT CPaneHeader::CreateDeviceDependentResources()
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
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x7F7F7F), &m_pBrGray127);
        if (SUCCEEDED(hr))
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x333333), &m_pBrGray033);
        if (SUCCEEDED(hr))
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xC4C4C4), &m_pBrGray196);
        if (SUCCEEDED(hr))
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x24FBFD), &m_pBr1);
        if (SUCCEEDED(hr))
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x72D2EE), &m_pBr2);
        if (SUCCEEDED(hr))
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x000000), &m_pBrBlack);

        m_pRenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE::D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

        // Get IDWriteFactory
        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(m_pDWriteFactory), reinterpret_cast<IUnknown**>(&m_pDWriteFactory));
        hr = m_pDWriteFactory->CreateTextFormat(
            L"Calibri",
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            13,
            L"",
            &m_pTFL);

        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(m_pDWriteFactory), reinterpret_cast<IUnknown**>(&m_pDWriteFactory));
        hr = m_pDWriteFactory->CreateTextFormat(
            L"Consolas",
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            10,
            L"",
            &m_pTFC);

        // Text Formats
        hr = m_pTFL->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        hr = m_pTFL->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        if (m_pTFL != 0)
            hr = m_pDWriteFactory->CreateEllipsisTrimmingSign(m_pTFL, &m_spInlineObjec_TFC);
        hr = m_pTFL->SetTrimming(&trimming, m_spInlineObjec_TFC);

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
        CalculateLayout(rc);
    }

    return hr;
}

void CPaneHeader::DiscardDeviceDependentResources()
{
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pBrGray127);
    SafeRelease(&m_pBrGray033);
    SafeRelease(&m_pBrGray196);
    SafeRelease(&m_pBr1);
    SafeRelease(&m_pBr2);
    SafeRelease(&m_pTFC);
    SafeRelease(&m_pDWriteFactory);
    SafeRelease(&m_spInlineObjec_TFC);
    SafeRelease(&m_pBrBlack);
}

HRESULT CPaneHeader::OnPaint2D()
{
    if (m_hWnd == NULL)
        return S_OK;

    HRESULT hr = CreateDeviceDependentResources();

    if (SUCCEEDED(hr))
    {
        m_pRenderTarget->BeginDraw();

        HWND hFP = GetParent(m_hWnd);
        HWND hMF = GetParent(hFP);
        BOOL bIsActive = (BOOL)SendMessage(hMF, WM_FRAME_ACTIVE, 0, 0);
        if (bIsActive)
            m_pRenderTarget->Clear(D2D1::ColorF(0x8DBFE7));
        else
            m_pRenderTarget->Clear(D2D1::ColorF(RGB(191, 191, 191)));
        D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

        // Begin painting
        //========================================================================
        D2D1_RECT_F frect = {5, 0, rtSize.width, rtSize.height};
        m_pRenderTarget->DrawText(L"Folders", 7, m_pTFL, &frect, m_pBrBlack);

        if (bMouseOverBtn == 0)
        {
            //m_pRenderTarget->DrawRoundedRectangle(m_buttons[0].rr_outer, m_pBr2, 2);
            //m_pRenderTarget->DrawRoundedRectangle(m_buttons[0].rr_innner, m_pBr2, 2);
            m_pRenderTarget->DrawRoundedRectangle(m_buttons[0].rr_middle, m_pBr1);
            //if (m_buttons[0].state & BST_CHECKED)
            //    m_pRenderTarget->DrawText(m_buttons[0].title, 2, m_pTFC, &m_buttons[0].rr_middle.rect, m_pBr1);
            //else
            m_pRenderTarget->DrawText(m_buttons[0].title, 2, m_pTFC, &m_buttons[0].rr_middle.rect, m_pBr1);
        }
        else
        {
            //if (m_buttons[0].state & BST_CHECKED)
            //{
            //    m_pRenderTarget->DrawRoundedRectangle(m_buttons[0].rr_middle, m_pBr2);
            //    //m_pRenderTarget->DrawText(m_buttons[0].title, 2, m_pTFC, &m_buttons[0].rr_middle.rect, m_pBr1);
            //}
            //else
            //{
            //    //m_pRenderTarget->DrawRoundedRectangle(m_buttons[0].rr_middle, m_pBrGray196);
            //}
            m_pRenderTarget->DrawText(m_buttons[0].title, 2, m_pTFC, &m_buttons[0].rr_middle.rect, m_pBrGray033);
        }

        //if (bMouseOverBtn == 1)
        //{
        //    //m_pRenderTarget->DrawRoundedRectangle(m_buttons[1].rr_outer, m_pBr2, 2);
        //    //m_pRenderTarget->DrawRoundedRectangle(m_buttons[1].rr_innner, m_pBr2, 2);
        //    m_pRenderTarget->DrawRoundedRectangle(m_buttons[1].rr_middle, m_pBr1);
        //    //if (m_buttons[1].state & BST_CHECKED)
        //    //    m_pRenderTarget->DrawText(m_buttons[1].title, 2, m_pTFC, &m_buttons[1].rr_middle.rect, m_pBr1);
        //    //else
        //    m_pRenderTarget->DrawText(m_buttons[1].title, 2, m_pTFC, &m_buttons[1].rr_middle.rect, m_pBr1);
        //}
        //else
        //{
        //    //if (m_buttons[1].state & BST_CHECKED)
        //    //{
        //    //    m_pRenderTarget->DrawRoundedRectangle(m_buttons[1].rr_middle, m_pBr2);
        //    //    //m_pRenderTarget->DrawText(m_buttons[1].title, 2, m_pTFC, &m_buttons[1].rr_middle.rect, m_pBr1);
        //    //}
        //    //else
        //    //{
        //    //    //m_pRenderTarget->DrawRoundedRectangle(m_buttons[1].rr_middle, m_pBrGray196);
        //    //}
        //    m_pRenderTarget->DrawText(m_buttons[1].title, 2, m_pTFC, &m_buttons[1].rr_middle.rect, m_pBrGray033);
        //}


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

void CPaneHeader::OnMouseMove(UINT flag, POINT pt)
{
    POINT point = pt;
    point.x = (int)(point.x / m_fScale);
    point.y = (int)(point.y / m_fScale);

    if (PtInRect(&m_buttons[0].rect, point))
    {
        bMouseOverBtn = 0;
        InvalidateRect(m_hWnd, NULL, NULL);
        return;
    }

    //if (PtInRect(&m_buttons[1].rect, point))
    //{
    //    bMouseOverBtn = 1;
    //    InvalidateRect(m_hWnd, NULL, NULL);
    //    return;
    //}

    bMouseOverBtn = -1;
}

void CPaneHeader::OnSize(UINT nType, UINT nWidth, UINT nHeight)
{
    if (m_pRenderTarget)
    {
        m_pRenderTarget->Resize(D2D1::SizeU(nWidth, nHeight));
    }

    RECT rc = {0, 0, (int)nWidth, (int)nHeight };
    CalculateLayout(rc);
}

void CPaneHeader::CalculateLayout(RECT rc)
{
    RECT rect = rc;

    rect.top = 2;
    rect.bottom = rect.top + 16;
    rect.left = (int)(rc.right / m_fScale - 18);
    rect.right = rect.left + 16;
    m_buttons[0].SetRect(rect);
    m_buttons[0].state = BST_CHECKED; //BST_UNCHECKED; // BST_CHECKED;

    rect.left -= 20;
    rect.right -= 20;
    m_buttons[1].SetRect(rect);
    m_buttons[1].state = BST_UNCHECKED;
}

void CPaneHeader::OnLButtonDown(UINT nType, UINT nX, UINT nY)
{
    //if (bMouseOverBtn >= 0 && bMouseOverBtn <= 2)
    //{
    //    if (bMouseOverBtn == 1)
    //    {
    //        if (m_nPanePos == 0)
    //            SetPosition(1);
    //        else
    //            SetPosition(0);

    //        //Inform parent that need to change entire pane position
    //        PostMessage(GetParent(m_hWnd), WM_PANE_FOLDER, 0, m_nPanePos);
    //    }
    //    else
    //    {
    //        if (m_buttons[bMouseOverBtn].state & BST_CHECKED)
    //            m_buttons[bMouseOverBtn].state = BST_UNCHECKED;
    //        else
    //            m_buttons[bMouseOverBtn].state = BST_CHECKED;
    //    }

        if (bMouseOverBtn == 0)
        {
            POINT pt;
            HMENU hMenuPopup = CreatePopupMenu();
            AppendMenu(hMenuPopup, MF_STRING, IDM_CONTEXT_LINE, L"Line");
            AppendMenu(hMenuPopup, MF_STRING, IDM_CONTEXT_RECTAN, L"Rectangle");
            AppendMenu(hMenuPopup, MF_STRING, IDM_CONTEXT_CIRCLE, L"Circle");
            AppendMenu(hMenuPopup, MF_SEPARATOR, 0, NULL);
            if (m_nPanePos == 0)
                AppendMenu(hMenuPopup, MF_STRING, IDM_MOVE_PANE, L"To the right side ->");
            else
                AppendMenu(hMenuPopup, MF_STRING, IDM_MOVE_PANE, L"To the left side <-");

            pt.x = nX;
            pt.y = nY;
            ClientToScreen(m_hWnd, &pt);
            TrackPopupMenu(hMenuPopup,
                TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                pt.x, pt.y, 0, m_hWnd, NULL);
            DestroyMenu(hMenuPopup);
        }

        //InvalidateRect(m_hWnd, NULL, NULL);
    //}
}

void CPaneHeader::SetPosition(int nPosition)
{
    m_nPanePos = nPosition;
    if (m_nPanePos == 0)
        m_buttons[1].title = L">";
    else
        m_buttons[1].title = L"<";
}