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
    , m_pBrBlack(NULL)
    , m_pBrBorder(NULL)
    , m_pBrFill(NULL)
    , m_pDirect2dFactory(NULL)
    , m_pRenderTarget(NULL)
    , m_pDWriteFactory(NULL)
    , m_pTFC(NULL)
    , m_spInlineObjec_TFC(NULL)
    , m_bMouseTracking(FALSE)
    , m_nLastHoverdCrumb(-2)
{

}

CBreadCrumb::~CBreadCrumb()
{
    SafeRelease(&m_pDirect2dFactory);
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pDWriteFactory);
    SafeRelease(&m_pBr0);
    SafeRelease(&m_pBr1);
    SafeRelease(&m_pBrBlack);
    SafeRelease(&m_pBrBorder);
    SafeRelease(&m_pBrFill);
    ClearVisbleCrumbs();
}

LRESULT  CBreadCrumb::WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_ERASEBKGND:
            return 1;

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
            m_nLastHoverdCrumb = -2;
            ClearHoverCrumbs();
            InvalidateRect(m_hWnd, NULL, TRUE);
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
    SafeRelease(&m_pBrBlack);
    SafeRelease(&m_pBrBorder);
    SafeRelease(&m_pBrFill);
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
        if (SUCCEEDED(hr))
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x000000), &m_pBrBlack);
        if (SUCCEEDED(hr))
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xCCE8FF), &m_pBrBorder);
        if (SUCCEEDED(hr))
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xE5F3FF), &m_pBrFill);

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

        InitVisibleCrumbs();

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

    //RECT rc;
    //GetWindowRect(m_hWnd, &rc);

    HRESULT hr = CreateDeviceDependentResources();

    if (SUCCEEDED(hr))
    {
        m_pRenderTarget->BeginDraw();

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

    BOOL bMark = FALSE;
    //float fStart = 0.0f;
    //for (size_t t = 0; t < m_visible_crumbs.size(); t++)
    //{
    //    BREAD_CRUMB_ITEM& bci = m_visible_crumbs.at(t);
    //    if (bci.bInvalidate)
    //    {
    //        DrawCrumb(pRT, bci, fStart, size.height);
    //        bMark = TRUE;
    //        bci.bInvalidate = FALSE;
    //    }
    //    fStart += bci.fWidth;
    //}

    if (!bMark)
    {
        pRT->Clear(D2D1::ColorF(0xFFFFFF)); // GetSysColor(COLOR_WINDOW)));

        float fStart = 0.0f;
        for (size_t t = 0; t < m_visible_crumbs.size(); t++)
        {
            BREAD_CRUMB_ITEM& bci = m_visible_crumbs.at(t);
            DrawCrumb(pRT, bci, fStart, size.height);
            fStart += bci.fWidth;
        }
    }

    return S_OK;
}

void CBreadCrumb::DrawCrumb(ID2D1HwndRenderTarget* pRT, BREAD_CRUMB_ITEM& bci, float fStart, float fHeight)
{
    D2D1_RECT_F frc;
    frc.left = fStart;
    frc.top = 0;
    frc.right = fStart + bci.fWidth;
    frc.bottom = fHeight;

    //pRT->DrawTextLayout()
    if (bci.nHoveredType > 0)
    {
        D2D1_RECT_F chevron{ frc };
        chevron.left = chevron.right - CHEVRON_WIDTH;

        //if (bci.nHoveredType == 1)
        //    pRT->FillRectangle(&frc, m_pBrFill);
        //else
        //    pRT->FillRectangle(&chevron, m_pBrFill);

        pRT->FillRectangle(&frc, m_pBrFill);
        pRT->DrawRectangle(&frc, m_pBrBorder, 1);
        if (bci.bHasChevron)
        {
            D2D1_POINT_2F p0{ chevron.left, 5};
            D2D1_POINT_2F p1{ chevron.left, chevron.bottom - 5 };
            pRT->DrawLine(p0, p1, m_pBr1);
        }
    }

    if (bci.bHasChevron)
        frc.right -= CHEVRON_WIDTH;

    //pRT->DrawRectangle(&frc, m_pBrBlack, 1);
    int txt_size = lstrlen(bci.text.c_str());
    pRT->DrawText(bci.text.c_str(), txt_size, m_pTFC, &frc, m_pBrBlack);

    if (bci.bHasChevron)
    {
        frc.left = frc.right;
        frc.right = frc.left + CHEVRON_WIDTH;
        pRT->DrawText(TEXT("›"), lstrlen(TEXT("›")), m_pTFC, &frc, m_pBrBlack);
    }
}

void CBreadCrumb::OnMouseMove(UINT nFlag, POINT pt)
{
    D2D1_POINT_2F point{ (float)pt.x, (float)pt.y };
    point.x = pt.x / m_fScale;
    point.y = pt.y / m_fScale;

    int res = HiTest(point);
    if (m_nLastHoverdCrumb != res)
    {
        //SetInvalidVisibleCrumb(m_nLastHoverdCrumb);
        //SetInvalidVisibleCrumb(res);

        m_nLastHoverdCrumb = res;

        InvalidateRect(m_hWnd, NULL, NULL);
    }

    //InvalidateRect(m_hWnd, NULL, NULL);
}

int CBreadCrumb::HiTest(D2D1_POINT_2F pt)
{
    LONG res = -1;
    float fWidth = 0;
    for (size_t t = 0; t < m_visible_crumbs.size(); t++)
    {
        BREAD_CRUMB_ITEM& bci = m_visible_crumbs.at(t);
        if (bci.bHasChevron)
        {
            if (pt.x > fWidth && pt.x < fWidth + bci.fWidth - CHEVRON_WIDTH)
            {
                bci.nHoveredType = 1; // Color ertyre button
                res = bci.nID;
            }
            else if (pt.x > fWidth + bci.fWidth - CHEVRON_WIDTH && pt.x < fWidth + bci.fWidth)
            {
                bci.nHoveredType = 2; // Color chevron part
                res = bci.nID;
            }
            else
                bci.nHoveredType = 0;
        }
        else
        {
            if (pt.x >= fWidth && pt.x < fWidth + bci.fWidth)
            {
                bci.nHoveredType = 1; // Color ertyre button
                res = bci.nID;
            }
            else
                bci.nHoveredType = 0;
        }
        fWidth += bci.fWidth;
    }
    return res;
}

void CBreadCrumb::OnSize(UINT nType, UINT nWidth, UINT nHeight)
{
    if (nType == SIZE_MINIMIZED)
        return;

    if (m_pRenderTarget)
        m_pRenderTarget->Resize(D2D1::SizeU(nWidth, nHeight));

    InitVisibleCrumbs();
}

int CBreadCrumb::FindNextCrumBarPos(const std::wstring& path, int iStart)
{
    int nFind = (int)path.find(DELEMITER, iStart);

    // Check if we have a double delimiter. we should then not break the string at this position.
    int delimiter_size = lstrlen(DELEMITER);
    if (path.size() >= (nFind + 2 * delimiter_size))
    {
        if (path.substr(nFind + delimiter_size, delimiter_size).compare(DELEMITER) == 0)
            return FindNextCrumBarPos(path, nFind + 2 * delimiter_size);
    }

    return nFind;
}

void CBreadCrumb::SetPath(LPCTSTR lpszPath)
{
    std::wstring path{lpszPath};

    std::wstring pathPart;
    m_crumbs.clear();
    int nFind = -1;
    while (!path.empty())
    {
        nFind = FindNextCrumBarPos(path, 0);

        if (nFind < 0)
        {
            m_crumbs.push_back(path);
            break;
        }
        else
        {
            pathPart = path.substr(0, nFind);
            path = path.substr(nFind + lstrlen(DELEMITER));

            if (!pathPart.empty())
                m_crumbs.push_back(pathPart);
        }
    }

    InitVisibleCrumbs();
}

void CBreadCrumb::ClearVisbleCrumbs()
{
    for (BREAD_CRUMB_ITEM& crumb : m_visible_crumbs)
    {
        SafeRelease(&crumb.m_pDWriteLayout);
    }
    m_visible_crumbs.clear();
}

BREAD_CRUMB_ITEM CBreadCrumb::InitCrumb(std::wstring text, int nID, BOOL bHasChevron)
{
    BREAD_CRUMB_ITEM bsi;
    bsi.nID = nID;
    bsi.text = text;
    bsi.bHasChevron = bHasChevron;
    m_pDWriteFactory->CreateTextLayout(bsi.text.c_str(), lstrlen(bsi.text.c_str()), m_pTFC, 1000, 1000, &bsi.m_pDWriteLayout);

    bsi.m_pDWriteLayout->GetMetrics(&bsi.tm);

    if (bsi.tm.width + 5 < MIN_CRUMB_WIDTH)
        bsi.fWidth = MIN_CRUMB_WIDTH;
    else
        bsi.fWidth = bsi.tm.width + 5;

    if (bHasChevron)
        bsi.fWidth += CHEVRON_WIDTH;

    return bsi;
}

void CBreadCrumb::InitVisibleCrumbs()
{
    if (!m_pDWriteFactory)
        return;

    if (m_crumbs.size() < 1)
        return;

    ClearVisbleCrumbs();
    RECT rc{};
    GetClientRect(m_hWnd, &rc);
    D2D1_RECT_F frc{};
    frc.left = rc.left / m_fScale;
    frc.right = rc.right / m_fScale;
    frc.top = rc.top / m_fScale;
    frc.bottom = rc.bottom / m_fScale;

    float real_width = frc.right - frc.left;
    float total_width = 0.0f;

    // Най-малкото трябва да има един елемент - последният
    std::wstring& text_last = m_crumbs.at(m_crumbs.size() - 1);
    BREAD_CRUMB_ITEM bsi = InitCrumb(text_last, (int)(m_crumbs.size() - 1), FALSE);
    m_visible_crumbs.push_back(bsi);

    if (m_crumbs.size() < 2)
        return;

    // Има ли достатъчно място
    total_width += bsi.fWidth;
    if (total_width > real_width)
        return;

    // Опитай да вмъкнеш първият crumb най-отпред
    std::wstring& text_first = m_crumbs.at(0);
    bsi = InitCrumb(text_first, 0);

    total_width += bsi.fWidth;

    // Ако мястото не стига
    if (total_width > real_width)
    {
        // Опитай да добавиш специален crumb най-отпред
        bsi.fWidth = MIN_CRUMB_WIDTH;
        bsi.text = TEXT("...");
        bsi.nID = -1;
        bsi.bHasChevron = FALSE;
        for (size_t k = m_crumbs.size() - 1; k > 0; k--)
            bsi.txt_special.push_back(m_crumbs.at(k));
        m_visible_crumbs.insert(m_visible_crumbs.begin(), bsi);
        return;
    }

    m_visible_crumbs.insert(m_visible_crumbs.begin(), bsi);

    // Добавяй елементи отзад-напред след първия
    for (size_t t = m_crumbs.size() - 2; t > 0; t--)
    {
        std::wstring& text = m_crumbs.at(t);

        BREAD_CRUMB_ITEM bsi = InitCrumb(text, (int)t);
        total_width += bsi.fWidth;
        if (total_width > real_width)
        {
            bsi.fWidth = MIN_CRUMB_WIDTH;
            bsi.text = TEXT("...");
            bsi.nID = -1;
            bsi.bHasChevron = FALSE;
            for (size_t k = t; k > 0; k--)
                bsi.txt_special.push_back(m_crumbs.at(k));

            m_visible_crumbs.insert(m_visible_crumbs.begin() + 1, bsi);
            break;
        }
        else
        {
            m_visible_crumbs.insert(m_visible_crumbs.begin() + 1, bsi);
        }
    }
}

void CBreadCrumb::ClearHoverCrumbs()
{
    for (BREAD_CRUMB_ITEM& bci : m_visible_crumbs)
        bci.nHoveredType = 0;
}

void CBreadCrumb::SetInvalidVisibleCrumb(int nID)
{
    for (BREAD_CRUMB_ITEM & bci : m_visible_crumbs)
    {
        if (bci.nID == nID)
            bci.bInvalidate = TRUE;
    }
}