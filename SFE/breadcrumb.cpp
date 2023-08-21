#include <windows.h>
#include <windowsx.h>
#include "breadcrumb.h"
#include "globals.h"
#include "resource.h"

TCHAR m_szTBCClassName[] = TEXT("BREADCRUMB");
#define ID_TIMER 101
static BOOL blinck = TRUE;
std::wstring str_teminator = L" <>:\"/\\|?*\0";

int FindTerminator(std::wstring& str, int start, BOOL bReverse = FALSE)
{
    int str_size = lstrlen(str.c_str());
    if (!bReverse)
    {
        int nEnd = -1;
        for (int t = start; t < str_size; t++)
        {
            TCHAR ch = str[t];
            if (str_teminator.find(ch, 0) != std::wstring::npos ||
                ch < 31)
                return t - 1;
        }
        return nEnd;
    }
    else
    {
        int nBeg = -1;
        for (int t = start; t >= 0; t--)
        {
            TCHAR ch = str[t];
            if (str_teminator.find(ch, 0) != std::wstring::npos ||
                ch < 31)
                return t + 1;
        }
        return nBeg;
    }
}

BOOL CopyStrToClipboard(std::wstring str)
{
    HGLOBAL hdst = NULL;
    size_t len = lstrlen(str.c_str());

    hdst = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (len + 1) * sizeof(WCHAR));
    if (hdst == NULL)
        return FALSE;

    LPTSTR dst = (LPWSTR)GlobalLock(hdst);
    if (dst == NULL)
        return FALSE;

    memcpy(dst, str.c_str(), len * sizeof(WCHAR));
    dst[len] = 0;
    GlobalUnlock(hdst);

    if (!OpenClipboard(NULL))
        return FALSE;

    EmptyClipboard();
    if (!SetClipboardData(CF_UNICODETEXT, hdst))
        return FALSE;

    CloseClipboard();
    return TRUE;
}

BOOL GetClipboardText(std::wstring& text)
{
    // Try opening the clipboard
    if (!OpenClipboard(nullptr))
        return FALSE; // error

    // Get handle of clipboard object for ANSI text
    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (hData == nullptr)
        return FALSE; // error

    // Lock the handle to get the actual text pointer
    TCHAR* pszText = static_cast<TCHAR*>(GlobalLock(hData));
    if (pszText == nullptr)
        return FALSE; // error

    // Save text in a string class instance
    text = pszText;

    // Release the lock
    GlobalUnlock(hData);

    // Release the clipboard
    CloseClipboard();

    return TRUE;
}

std::wstring ReplaceString(std::wstring subject, const std::wstring& search, const std::wstring& replace)
{
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::wstring::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    return subject;
}

void  Timerproc(HWND hWnd, UINT unnamedParam2, UINT_PTR unnamedParam3, DWORD unnamedParam4)
{
    blinck = !blinck;
    SendMessage(hWnd, WM_PAINT, 0, 0);
}

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
    , m_pBrFill2(NULL)
    , m_pBrSelection(NULL)
    , m_pDirect2dFactory(NULL)
    , m_pRenderTarget(NULL)
    , m_pDWriteFactory(NULL)
    , m_pTFC(NULL)
    , m_pTFL(NULL)
    , m_spInlineObjec_TFC(NULL)
    , m_txtLayout(NULL)
    , m_bMouseTracking(FALSE)
    , m_nLastHoverdCrumb(-2)
    , m_bMode(MODE_CRUMB)
    , m_bLButtonDown(FALSE)
    , m_bDragging(FALSE)
    , m_path()
    , m_path_backup()
    , m_path_undo()
    , m_hOldFocus(NULL)
    , m_txt_range()
    , m_nCaretPos(0)
    , hCursorIBeam(NULL)
    , hCursorArrow(NULL)
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
    SafeRelease(&m_pBrFill2);
    SafeRelease(&m_pTFC);
    SafeRelease(&m_pTFL);
    SafeRelease(&m_pBrSelection);
    SafeRelease(&m_txtLayout);
    ClearVisbleCrumbs();
    KillTimer(m_hWnd, ID_TIMER);
}

LRESULT  CBreadCrumb::WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_ERASEBKGND:
            return 1;

        case WM_CREATE:
        {
            hCursorIBeam = LoadCursor(NULL, IDC_IBEAM);
            hCursorArrow = LoadCursor(NULL, IDC_ARROW);
            break;
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
            m_bLButtonDown = TRUE;
            UINT nFlag = (UINT)wParam;
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            HWND hParent = ::GetParent(m_hWnd);
            OnLButtonDown(nFlag, pt);
            ::PostMessage(hParent, WM_LBUTTONDOWN, (WPARAM)0, (LPARAM)0);
            break;
        }

        case WM_LBUTTONUP:
        {
            m_bLButtonDown = FALSE;
            m_bDragging = FALSE;
            break;
        }

        case WM_LBUTTONDBLCLK:
        {
            if (GetMode() == MODE_EDIT)
            {
                POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                SetTxtRange(MK_CONTROL | MK_SHIFT, (POINT)pt);
            }
            break;
        }

        case WM_MOUSEMOVE:
        {
            if (!m_bMouseTracking && GetMode() == MODE_CRUMB)
            {
                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof(TRACKMOUSEEVENT);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hWnd;
                TrackMouseEvent(&tme);
                m_bMouseTracking = TRUE;
            }

            if (m_bLButtonDown && GetMode() == MODE_EDIT)
            {
                m_bDragging = TRUE;
            }

            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            OnMouseMove((UINT)wParam, pt);

            RECT rc = {};
            GetClientRect(hWnd, &rc);

            if (!PtInRect(&rc, pt) || GetMode() != MODE_EDIT)
                SetCursor(hCursorArrow);
            else
                SetCursor(hCursorIBeam);
            break;
        }

        case WM_MOUSELEAVE:
        {
            if (GetMode() == MODE_EDIT)
                break;
            m_bMouseTracking = FALSE;
            m_nLastHoverdCrumb = -2;
            ClearHoverCrumbs();
            InvalidateRect(m_hWnd, NULL, TRUE);
            break;
        }

        case WM_RBUTTONDOWN:
        {
            m_bLButtonDown = FALSE;
            UINT nFlags = (UINT)wParam;
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            OnRButtonDown(nFlags, pt);
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

        case WM_SETFOCUS:
        {
            CreateCaret(hWnd, NULL, 2, 20);
            SetCaretPos(50, 0);
            ShowCaret(hWnd);
            //DWRITE_TEXT_RANGE caretRange = GetSelectionRange();
            break;
        }

        case WM_KILLFOCUS:
        {
            if (m_bLButtonDown)
                break;
            DestroyCaret();
            KillTimer(m_hWnd, ID_TIMER);
            if (GetMode() != MODE_CRUMB)
                SetMode(MODE_CRUMB);
            break;
        }

        case WM_KEYDOWN:
        {
            OnKeyDown(wParam, lParam);
            break;
        }

        case WM_CHAR:
        {
            SELECT_TEXT_RANGE selection = NormalizeSelection();
            TCHAR ch = (TCHAR)wParam;
            if (ch <= 31)
                break;

            std::wstring sch;
            sch.push_back(ch);

            if (m_txt_range.length == 0)
            {
                m_path = m_path.insert(selection.startPosition, sch);
                m_txt_range.startPosition = selection.startPosition + (INT32)sch.length();
            }
            else
            {
                std::wstring str = m_path.substr(selection.startPosition, selection.length);
                m_path = ReplaceString(m_path, str, sch);
                m_txt_range.startPosition = selection.startPosition + (INT32)sch.length();
                m_txt_range.length = 0;
            }
            GreateTextLayout();
            InvalidateRect(m_hWnd, NULL, NULL);

            break;
        }

        case WM_CONTEXTMENU:
        {
            POINT point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            OnContextMenu(point);
            break;
        }

        case WM_CRUMBBAR:
        {
            switch (wParam)
            {
                case WP_CRUMBBAR_SET_MODE:
                {
                    if (lParam == LP_CRUMBBAR_SET_MODE_EDIT)
                        SetMode(MODE_EDIT);
                    else
                        SetMode(MODE_CRUMB);
                    break;
                }
                case WP_CRUMBBAR_SET_PATH:
                {
                    SetPath((TCHAR*)lParam);
                    break;
                }
            }
            break;
        }

        case WM_COMMAND:
        {
            OnCommand(wParam, wParam);
            break;
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
        WS_EX_TRANSPARENT, // | WS_EX_CLIENTEDGE | WS_EX_COMPOSITED
        m_szTBCClassName,
        NULL,
        WS_VISIBLE | WS_CHILD, // | WS_BORDER,
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
    SafeRelease(&m_pBrFill2);
    SafeRelease(&m_pTFC);
    SafeRelease(&m_pTFL);
    SafeRelease(&m_pBrSelection);
    SafeRelease(&m_txtLayout);
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
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x8DBFE7), &m_pBrFill);
        if (SUCCEEDED(hr))
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xE5F3FF), &m_pBrFill2);
        if (SUCCEEDED(hr))
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x99C9EF), &m_pBrSelection);

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


        hr = m_pDWriteFactory->CreateTextFormat(
            L"Colibi",
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            12,
            L"",
            &m_pTFL);
        hr = m_pTFL->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        hr = m_pTFL->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

        GreateTextLayout();

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

    pRT->Clear(D2D1::ColorF(0xFFFFFF)); // GetSysColor(COLOR_WINDOW)));

    if (!m_bMode)
    {
        float fStart = 0.0f;
        for (size_t t = 0; t < m_visible_crumbs.size(); t++)
        {
            BREAD_CRUMB_ITEM& bci = m_visible_crumbs.at(t);
            DrawCrumb(pRT, bci, fStart, size.height);
            fStart += bci.fWidth;
        }
    }
    else
    {
        //pRT->Clear(D2D1::ColorF(0xFFFFFF));
        POINT pt = {};
        GetCursorPos(&pt);
        ScreenToClient(m_hWnd, &pt);
        DrawEdit(pRT, pt);
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

        if (bci.nHoveredType == 1)
        {
            pRT->FillRectangle(&frc, m_pBrFill);
            pRT->DrawRectangle(&frc, m_pBrBorder, 1);
        }
        else
        {
            pRT->FillRectangle(&frc, m_pBrFill2);

            D2D1_RECT_F crc{ frc };

            crc.left = crc.right - CHEVRON_WIDTH;
            pRT->FillRectangle(&crc, m_pBrFill);

            pRT->DrawRectangle(&frc, m_pBrBorder, 1);
        }

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

void CBreadCrumb::DrawEdit(ID2D1HwndRenderTarget* pRT, POINT pt)
{
    D2D1_SIZE_F size = pRT->GetSize();
    //size.height -= 1.0f;
    D2D1_RECT_F frc;
    frc.left = 0;
    frc.top = 0;
    frc.right = size.width;
    frc.bottom = size.height - 1.0f;

    // Create text layout

    // Draw selection
    DrawSelection(pRT, m_txtLayout);

    // Draw caret
    DrawCaret(pRT, m_txtLayout);

    // Draw path
    //pRT->DrawText(m_path.c_str(), lstrlen(m_path.c_str()), m_pTFL, frc, m_pBrBlack);
    D2D1_POINT_2F fpt = { 1.0f, 0.0f };
    pRT->DrawTextLayout(fpt, m_txtLayout, m_pBrBlack);

}

void CBreadCrumb::DrawSelection(ID2D1HwndRenderTarget* pRT, IDWriteTextLayout* pTL)
{
    SELECT_TEXT_RANGE selection = NormalizeSelection();

    if (selection.length > 0)
    {
        UINT32 actualHitTestCount = 0;
        pTL->HitTestTextRange
        (
            selection.startPosition,
            selection.length,
            0, // x
            0, // y
            NULL,
            0, // metrics count
            &actualHitTestCount
        );

        DWRITE_HIT_TEST_METRICS hitTestMetrics;
        pTL->HitTestTextRange
        (
            selection.startPosition,
            selection.length,
            0, // x
            0, // y
            &hitTestMetrics, 1, &actualHitTestCount
        );

        D2D1_RECT_F highlightRect =
        {
            hitTestMetrics.left + 1,
            hitTestMetrics.top,
            (hitTestMetrics.left + 1 + hitTestMetrics.width),
            (hitTestMetrics.top + hitTestMetrics.height)
        };
        pRT->FillRectangle(highlightRect, m_pBrSelection);
    }
}

void CBreadCrumb::DrawCaret(ID2D1HwndRenderTarget* pRT, IDWriteTextLayout* pTL)
{
    if (!blinck)
    {
        m_nCaretPos = m_txt_range.startPosition;

        DWRITE_HIT_TEST_METRICS hitTestMetrics;
        float caretX = {0.0f};
        float caretY = { 0.0f };

        pTL->HitTestTextPosition(m_nCaretPos, false, &caretX, &caretY, &hitTestMetrics);
        //pTL->HitTestPoint(caretX, caretY, &isTrailingHit, &isInsidem, &hitTestMetrics);

        DWRITE_HIT_TEST_METRICS caretMetrics;
        UINT32 actualHitTestCount = 1;
        pTL->HitTestTextRange(m_nCaretPos, 0, 0, 0, &caretMetrics, 1, &actualHitTestCount);

        caretY = caretMetrics.top;

        D2D_POINT_2F p0 = { caretX + 1, caretY };
        D2D_POINT_2F p1 = { caretX + 1, caretY + caretMetrics.height };
        pRT->SetAntialiasMode(D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        pRT->DrawLine(p0, p1, m_pBrBlack, 1.0f);
        pRT->SetAntialiasMode(D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);
    }
}

void CBreadCrumb::OnMouseMove(UINT nFlag, POINT pt)
{
    D2D1_POINT_2F point{ (float)pt.x, (float)pt.y };
    point.x = pt.x / m_fScale;
    point.y = pt.y / m_fScale;

    if (GetMode() == MODE_CRUMB)
    {
        int res = HiTest(point);
        if (m_nLastHoverdCrumb != res)
        {
            //SetInvalidVisibleCrumb(m_nLastHoverdCrumb);
            //SetInvalidVisibleCrumb(res);

            m_nLastHoverdCrumb = res;

            InvalidateRect(m_hWnd, NULL, NULL);
        }
        return;
    }

    if (GetMode() == MODE_EDIT)
    {
        if (m_bDragging)
        {
            SetTxtRange(nFlag, pt);
        }
    }
}

int  CBreadCrumb::HiTest(D2D1_POINT_2F pt)
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

int  CBreadCrumb::FindNextCrumBarPos(const std::wstring& path, int iStart)
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
    m_path = lpszPath;
    size_t z = m_path.rfind('\\');
    if (z == m_path.size() - 1)
        m_path = m_path.substr(0, z);

    std::wstring path{m_path};

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

void CBreadCrumb::SetMode(BOOL bMode)
{
    m_bMode = bMode;
    if (bMode == MODE_CRUMB)
    {
        KillTimer(m_hWnd, ID_TIMER);
        if (m_path_backup.length() > 0)
        {
            // m_path_backup трябва да се изпрати в HISTORY
            // ...

            HWND hWnd = GetParent(m_hWnd); // CTPane
            hWnd = GetParent(hWnd);        // CMDIFrame
            SendMessage(hWnd, WM_MDIFRAME, WP_MDIFRAME_SET_FOLDER_PATH, (LPARAM)m_path.c_str());
        }
    }

    if (bMode == MODE_EDIT)
    {
        // Keep original path
        m_path_backup = m_path;
        GreateTextLayout();
    }

    InvalidateRect(m_hWnd, NULL, NULL);
}

void CBreadCrumb::OnRButtonDown(UINT nFlag, POINT pt)
{
    POINT point = pt;

    RECT rect;
    GetClientRect(m_hWnd, &rect);

    if (PtInRect(&rect, pt))
    {
        //m_hOldFocus = SetFocus(m_hWnd);
        //SetCapture(m_hWnd);

        PostMessage(m_hWnd, WM_CONTEXTMENU, (WPARAM)m_hWnd, MAKELPARAM(pt.x, pt.y));
    }
    else
    {
        ReleaseCapture();
        SetFocus(m_hOldFocus);
        SetMode(MODE_CRUMB);

        ClientToScreen(m_hWnd, &pt);
        HWND hWnd = WindowFromPoint(pt);
        ScreenToClient(hWnd, &pt);
        PostMessage(hWnd, WM_RBUTTONDOWN, nFlag, MAKELPARAM(pt.x, pt.y));
    }
}

void CBreadCrumb::OnLButtonDown(UINT nFlag, POINT pt)
{
    float lenght = 0;
    for (BREAD_CRUMB_ITEM & bcr : m_visible_crumbs)
    {
        lenght += bcr.fWidth * m_fScale;
    }

    RECT rect;
    GetClientRect(m_hWnd, &rect);

    if (PtInRect(&rect, pt))
    {
        if (pt.x > lenght && GetMode() != MODE_EDIT)
        {
            m_txt_range.startPosition = 0;
            m_txt_range.length = lstrlen(m_path.c_str());
            SetMode(MODE_EDIT);

            m_hOldFocus = SetFocus(m_hWnd);
            SetCapture(m_hWnd);
            SetCursor(hCursorIBeam);
        }

        if (pt.x <= lenght && GetMode() == MODE_EDIT)
        {
            SetTxtRange(nFlag, pt);
        }

        if (GetMode() == MODE_EDIT)
        {
            m_bMouseTracking = FALSE;
            KillTimer(m_hWnd, ID_TIMER);
            SetTimer(m_hWnd, ID_TIMER, 400, Timerproc);
            blinck = FALSE;
        }
    }
    else
    {
        if (!m_bDragging)
        {
            ReleaseCapture();
            SetFocus(m_hOldFocus);
            SetMode(MODE_CRUMB);

            ClientToScreen(m_hWnd, &pt);
            HWND hWnd = WindowFromPoint(pt);
            ScreenToClient(hWnd, &pt);
            PostMessage(hWnd, WM_LBUTTONDOWN, nFlag, MAKELPARAM(pt.x, pt.y));
        }
    }
}

void CBreadCrumb::OnContextMenu(POINT point)
{
    POINT pt = point;
    if (GetMode() == MODE_CRUMB)
    {
        HMENU hMenuPopup = CreatePopupMenu();

        AppendMenu(hMenuPopup, MF_STRING, IDM_CRUMB_COPY_AS_TEXT, L"Copy address");
        AppendMenu(hMenuPopup, MF_STRING, IDM_CRUMB_COPY_DOUBLE_SLASH, L"Copy with double \"\\\"");
        AppendMenu(hMenuPopup, MF_STRING, IDM_CRUMB_EDIT, L"Edit address");

        ClientToScreen(m_hWnd, &pt);
        TrackPopupMenu(hMenuPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_hWnd, NULL);
        DestroyMenu(hMenuPopup);
    }
    else
    {
        HMENU hMenuPopup = CreatePopupMenu();

        AppendMenu(hMenuPopup, MF_STRING, ID_EDIT_CUT, L"Cut");
        AppendMenu(hMenuPopup, MF_STRING, ID_EDIT_COPY, L"Copy");
        AppendMenu(hMenuPopup, MF_STRING, ID_EDIT_PASTE, L"Paste");
        AppendMenu(hMenuPopup, MF_STRING, ID_EDIT_CLEAR, L"Delete");
        AppendMenu(hMenuPopup, MF_SEPARATOR, 0, NULL);
        AppendMenu(hMenuPopup, MF_STRING, ID_EDIT_SELECT_ALL, L"Select all");

        ClientToScreen(m_hWnd, &pt);
        TrackPopupMenu(hMenuPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_hWnd, NULL);
        DestroyMenu(hMenuPopup);

        SetCapture(m_hWnd);

        //GetCursorPos(&pt);
        //RECT rect;
        //GetClientRect(m_hWnd, &rect);

        //if (PtInRect(&rect, pt))
        //{
        //}
        //else
        //    SetMode(MODE_CRUMB);
    }
}

void CBreadCrumb::OnCommand(WPARAM wParam, LPARAM lParam)
{
    WORD control = HIWORD(wParam);
    WORD nID = LOWORD(wParam);

    switch (nID)
    {
        case IDM_CRUMB_EDIT:
        {
            m_txt_range.startPosition = 0;
            m_txt_range.length = lstrlen(m_path.c_str());
            SetMode(MODE_EDIT);
            SetCursor(hCursorIBeam);

            m_hOldFocus = SetFocus(m_hWnd);
            SetCapture(m_hWnd);

            KillTimer(m_hWnd, ID_TIMER);
            SetTimer(m_hWnd, ID_TIMER, 400, Timerproc);
            blinck = FALSE;
            break;
        }

        case IDM_CRUMB_COPY_AS_TEXT:
        {
            CopyStrToClipboard(m_path);
            break;
        }

        case IDM_CRUMB_COPY_DOUBLE_SLASH:
        {
            std::wstring str = m_path;
            std::wstring x = L"\\";
            std::wstring y = L"\\\\";

            size_t pos = str.find(L"\\", 0);
            while (pos != std::string::npos)
            {
                str.replace(pos, 1, L"\\\\");
                pos += 2;
                pos = str.find(L"\\", pos);
            }

            CopyStrToClipboard(str);
            break;
        }
    }
}

void CBreadCrumb::GreateTextLayout()
{
    if (!m_pRenderTarget ||
        !m_pDWriteFactory)
        return;

    D2D1_SIZE_F size = m_pRenderTarget->GetSize();
    int txt_size = lstrlen(m_path.c_str());

    if (m_txtLayout != NULL)
        SafeRelease(&m_txtLayout);

    HRESULT hr = m_pDWriteFactory->CreateTextLayout(m_path.c_str(), txt_size, m_pTFL, size.width, size.height, &m_txtLayout);
}

void CBreadCrumb::SetTxtRange(UINT nFlag, POINT pt)
{
    if (!m_txtLayout)
        return;

    FLOAT ptX = pt.x / m_fScale;
    FLOAT ptY = pt.y / m_fScale;

    BOOL isTrailing = FALSE;
    BOOL isInside = FALSE;
    DWRITE_HIT_TEST_METRICS hitTestMetrics = {};
    m_txtLayout->HitTestPoint(ptX -1, ptY, &isTrailing, &isInside, &hitTestMetrics);

    UINT32 start = hitTestMetrics.textPosition;
    if (isTrailing)
        start++;

    if (nFlag & MK_CONTROL && nFlag & MK_SHIFT)
    {
        int nBeg = FindTerminator(m_path, start, TRUE);
        int nEnd = FindTerminator(m_path, start, FALSE);

        if (nBeg == std::wstring::npos)
            nBeg = 0;
        if (nEnd == std::wstring::npos)
            nEnd = lstrlen(m_path.c_str());
        m_txt_range.startPosition = nBeg;
        m_txt_range.length = nEnd - nBeg + 1;

        return;
    }

    // Select directory (between "\" and "\")
    if (nFlag & MK_CONTROL)
    {
        size_t nEnd = m_path.find(L"\\", start);
        if (nEnd == std::wstring::npos)
            nEnd = lstrlen(m_path.c_str());
        else
            nEnd--;

        size_t nBeg = m_path.rfind(L"\\", start);
        if (nBeg == std::wstring::npos)
            nBeg = -1;

        m_txt_range.startPosition = (UINT32)nBeg + 1;
        m_txt_range.length = (UINT32)(nEnd - nBeg);

        return;
    }

    // Select or Extend selection OR Select with mouse
    if (nFlag & MK_SHIFT || m_bDragging)
    {
        m_txt_range.length = start - m_txt_range.startPosition;
        return;
    }

    // No selection
    m_txt_range.startPosition = start;
    m_txt_range.length = 0;
}

SELECT_TEXT_RANGE CBreadCrumb::NormalizeSelection()
{
    SELECT_TEXT_RANGE selection = m_txt_range;
    if (selection.length < 0)
    {
        selection.startPosition = selection.startPosition + selection.length;
        selection.length = abs(selection.length);
    }
    return selection;
}

void CBreadCrumb::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    BOOL bShft = GetKeyState(VK_SHIFT) & 0x8000;
    BOOL bCtrl = GetKeyState(VK_CONTROL) & 0x8000;

    if (GetMode() != MODE_EDIT)
        return;

    // Don't care about selection
    switch (wParam)
    {
        case VK_ESCAPE:
        {
            m_path = m_path_backup;
            m_path_backup.clear(); // Не променяй нищо
            ReleaseCapture();
            SetFocus(m_hOldFocus);
            SetMode(MODE_CRUMB);

            return;
        }
        case VK_RETURN:
        {
            SetMode(MODE_CRUMB);
            SetPath(m_path.c_str());

            ReleaseCapture();
            SetFocus(m_hOldFocus);
            return;
        }
        case VK_DELETE:
        {
            SELECT_TEXT_RANGE selection = NormalizeSelection();
            if (selection.length <= 0)
                selection.length = 1;

            m_path_undo.push_back(m_path);
            m_path = m_path.erase(selection.startPosition, selection.length);
            m_txt_range.startPosition = selection.startPosition;
            m_txt_range.length = 0;
            GreateTextLayout();
            InvalidateRect(m_hWnd, NULL, NULL);

            return;
        }
        case VK_BACK:
        {
            SELECT_TEXT_RANGE selection = NormalizeSelection();
            if (selection.length == 0)
            {
                selection.startPosition--;
                selection.length = 1;
            }

            m_path_undo.push_back(m_path);
            m_path = m_path.erase(selection.startPosition, selection.length);
            m_txt_range.startPosition = selection.startPosition;
            m_txt_range.length = 0;
            GreateTextLayout();
            InvalidateRect(m_hWnd, NULL, NULL);

            return;
        }
        case 'C': // Copy
        {
            if (bCtrl)
            {
                SELECT_TEXT_RANGE selection = NormalizeSelection();
                std::wstring str = m_path.substr(selection.startPosition, selection.length);
                CopyStrToClipboard(str);
            }
            return;
        }
        case 'V': // Pate
        {
            SELECT_TEXT_RANGE selection = NormalizeSelection();
            std::wstring str_clipboard = {};

            if (GetClipboardText(str_clipboard))
            {
                if (selection.length != 0)
                {
                    std::wstring str = m_path.substr(selection.startPosition, selection.length);

                    m_path_undo.push_back(m_path);
                    m_path = ReplaceString(m_path, str, str_clipboard);
                    m_txt_range.startPosition = selection.startPosition;
                    m_txt_range.length = (INT32)str_clipboard.length();
                }
                else
                {
                    m_path_undo.push_back(m_path);
                    m_path = m_path.insert(selection.startPosition, str_clipboard);
                    m_txt_range.length = (INT32)str_clipboard.length();
                }
                GreateTextLayout();
                InvalidateRect(m_hWnd, NULL, NULL);
            }
            return;
        }
        case 'X': // Cut
        {
            if (bCtrl)
            {
                SELECT_TEXT_RANGE selection = NormalizeSelection();
                std::wstring str = m_path.substr(selection.startPosition, selection.length);
                CopyStrToClipboard(str);

                m_path_undo.push_back(m_path);
                m_path = m_path.erase(selection.startPosition, selection.length);
                m_txt_range.startPosition = selection.startPosition;
                m_txt_range.length = 0;
                GreateTextLayout();
                InvalidateRect(m_hWnd, NULL, NULL);
            }
            return;
        }
        case 'Z': // Undo
        {
            size_t size_undo = m_path_undo.size();
            if (size_undo > 0)
            {
                m_path = m_path_undo.back();
                m_path_undo.pop_back();
                GreateTextLayout();
                InvalidateRect(m_hWnd, NULL, NULL);
            }
            break;
        }
    }

    // There is NO selction
    if (m_txt_range.length == 0)
    {
        switch (wParam)
        {
            case VK_LEFT:
            {
                if (bShft && bCtrl)
                {
                    size_t nPos = m_path.rfind(L"\\", m_txt_range.startPosition - 1);
                    if (nPos == std::wstring::npos)
                        nPos = 0;
                    m_txt_range.length = (INT32)nPos - m_txt_range.startPosition + 1;
                    return;
                }

                if (bShft)
                {
                    m_txt_range.length--;
                    return;
                }

                m_txt_range.startPosition--;
                m_txt_range.length = 0;
                return;
            }
            case VK_RIGHT:
            {
                if (bShft && bCtrl)
                {
                    size_t nPos = m_path.find(L"\\", m_txt_range.startPosition + 1);
                    m_txt_range.length = (INT32)nPos - m_txt_range.startPosition;
                    return;
                }

                if (bShft)
                {
                    m_txt_range.length++;
                    return;
                }

                m_txt_range.startPosition++;
                m_txt_range.length = 0;
                return;
            }
            case VK_END:
            {
                if (bShft)
                {
                    m_txt_range.length = lstrlen(m_path.c_str()) - m_txt_range.startPosition;
                    return;
                }

                m_txt_range.startPosition = lstrlen(m_path.c_str());
                m_txt_range.length = 0;
                return;
            }
            case VK_HOME:
            {
                if (bShft)
                {
                    m_txt_range.length = 0 - m_txt_range.startPosition;
                    return;
                }

                m_txt_range.startPosition = 0;
                m_txt_range.length = 0;
                return;
            }
        }
        return;
    }

    // There IS selction
    if (m_txt_range.length != 0)
    {
        switch (wParam)
        {
            case VK_LEFT:
            {
                if (bShft && bCtrl)
                {
                    size_t nPos = m_path.rfind(L"\\", m_txt_range.startPosition + m_txt_range.length - 1);
                    if (nPos == m_txt_range.startPosition + m_txt_range.length - 1)
                    {
                        nPos = m_path.rfind(L"\\", nPos - 1);
                    }
                    if (nPos == std::wstring::npos)
                        nPos = 0;

                    m_txt_range.length = (INT32)nPos - m_txt_range.startPosition + 1;

                    return;
                }

                if (bShft)
                {
                    m_txt_range.length--;
                    return;
                }

                if (m_txt_range.length <= 0)
                    m_txt_range.startPosition += m_txt_range.length;
                m_txt_range.length = 0;
                return;
            }
            case VK_RIGHT:
            {
                if (bShft && bCtrl)
                {
                    size_t nPos = m_path.find(L"\\", m_txt_range.startPosition + m_txt_range.length + 1);
                    if (nPos == std::wstring::npos)
                        nPos = lstrlen(m_path.c_str());

                    m_txt_range.length = (INT32)nPos - m_txt_range.startPosition;

                    return;
                }

                if (bShft)
                {
                    m_txt_range.length++;
                    return;
                }

                if (m_txt_range.length >= 0)
                    m_txt_range.startPosition += m_txt_range.length;
                m_txt_range.length = 0;
                return;
            }
            case VK_END:
            {
                if (bShft)
                {
                    return;
                }
                m_txt_range.startPosition = lstrlen(m_path.c_str());
                m_txt_range.length = 0;
                return;
            }
            case VK_HOME:
            {
                if (bShft)
                {
                    return;
                }

                m_txt_range.startPosition = 0;
                m_txt_range.length = 0;
                return;
            }
        }
    }
}
