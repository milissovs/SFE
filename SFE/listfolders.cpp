#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <string>
#include "listfolders.h"
#include "globals.h"
#include "resource.h"
#include "shlwapi.h"
//#include "dddd.h"

#define _AFXWIN_INLINE

HGLOBAL CListFolders::CopyItem(LONG nItem)
{
    if (nItem < 0)
        return NULL;

    FOLDER_ITEM fi = m_items.at(nItem);
    fi.move_index = nItem;
    SIZE_T size = sizeof(fi);
    HGLOBAL hMem = GlobalAlloc(GHND, size);

    if (hMem != NULL)
    {
        FOLDER_ITEM* pfi = (FOLDER_ITEM*)GlobalLock(hMem);
        *pfi = fi;
        GlobalUnlock(hMem);
    }

    return hMem;
};

TCHAR m_szListFolderClassName[] = TEXT("LISTFOLDER");

LRESULT CALLBACK ListProcExt(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    UNALIGNED CListFolders* pThis = (CListFolders*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (Message == WM_NCCREATE)
    {
        LPCREATESTRUCT lpCS = (LPCREATESTRUCT)lParam;
        CListFolders* pThis = (CListFolders*)lpCS->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->m_hWnd = hwnd;
    }

    if (!pThis)
        return ::DefWindowProc(hwnd, Message, wParam, lParam);

    return pThis->WindowProcedure(hwnd, Message, wParam, lParam);
}

LRESULT  CListFolders::WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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

        case WM_VSCROLL:
        {
            UINT nSBCode = LOWORD(wParam);
            UINT nPos = HIWORD(wParam);
            OnVScroll(nSBCode, nPos, (HWND)lParam);
            break;
        }

        case WM_MOUSEWHEEL:
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            UINT nFlag = GET_KEYSTATE_WPARAM(wParam);
            OnMouseWheel(nFlag, zDelta, pt);
            break;
        }

        case WM_LBUTTONDOWN:
        {
            //SetCapture(hWnd);
            m_bLButtonDown = TRUE;
            HWND hParent = ::GetParent(m_hWnd);
            ::PostMessage(hParent, WM_LBUTTONDOWN, (WPARAM)0, (LPARAM)0);
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            OnLButtonDown((UINT)wParam, pt);
            break;
        }
        
        case WM_LBUTTONUP:
        {
            //ReleaseCapture();
            m_bLButtonDown = FALSE;
            m_bDraging = FALSE;
            break;
        }

        case WM_LBUTTONDBLCLK:
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            OnLButtonDblClk((UINT)wParam, pt);
            break;
        }

        case WM_MOUSEMOVE:
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            OnMouseMove((UINT)wParam, pt);
            break;
        }

        case WM_MOUSELEAVE:
        {
            //SendMessage(m_hWndTT, TTM_TRACKACTIVATE, FALSE, (LPARAM)&toolTipInfo);
            m_bMouseTracking = FALSE;
            m_hovered.item = -1;
            m_hovered.part = -1;
            m_hovered.level = 0;
            m_bLButtonDown = FALSE;
            m_bDraging = FALSE;

            InvalidateRect(m_hWnd, NULL, NULL);
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
            OnPaint2D();

            EndPaint(m_hWnd, &ps);

            return 0;
        }

        case WM_DDD_FOLDERS:
        {
            if (wParam == 0)
            {
                m_bDraging = TRUE;
                POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                SELECTED_ITEM nHover = HitTest(pt);
                LONG nItemHover = nHover.item;
                WORD nPartHover = nHover.part;
                WORD nLevelHover = nHover.level;

                int scrollPos = GetScrollPos32(SB_VERT);
                RECT rc{};
                GetClientRect(m_hWnd, &rc);
                if (pt.y < 10 * m_fScale)
                {
                    SetScrollPos32(SB_VERT, __max(0, scrollPos - DEFAULT_ITEM_HEIGHT));
                }
                else if (pt.y > rc.bottom - 10 * m_fScale)
                {
                    SetScrollPos32(SB_VERT, scrollPos + DEFAULT_ITEM_HEIGHT);
                }

                if (nHover != m_hovered)
                {
                    m_hovered = nHover;
                    InvalidateRect(m_hWnd, NULL, TRUE);
                }
            }
            else
            {
                m_bDraging = FALSE;
                m_hovered.item = -1;
                m_hovered.part = HOVER_PART::HP_INVALID;
                InvalidateRect(m_hWnd, NULL, TRUE);
            }

            break;
        }

        case LVM_INSERTITEM:
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ScreenToClient(m_hWnd, &pt);
            SELECTED_ITEM nHover = HitTest(pt);
            LONG nItemHover = nHover.item;
            WORD nPartHover = nHover.part;
            WORD nLevelHover = nHover.level;

            // Not valid item - take last
            if (nItemHover < 0)
            {
                nItemHover = (LONG)m_items.size();
                nPartHover = 2;
            }

            FOLDER_ITEM* pfi = (FOLDER_ITEM * )wParam;

            // Insert item between others
            if (nPartHover != 1)
            {
                // Is same window - moved item has been removed already
                if (pfi->hWndParent == m_hWnd && pfi->move_index < nItemHover)
                    nItemHover--;

                if (nPartHover == 0)
                    InsertFolder(pfi, nItemHover);
                if (nPartHover == 2)
                    InsertFolder(pfi, nItemHover + 1);
            }

            // Convert item to subitem
            else
            {

            }

            InvalidateRect(m_hWnd, NULL, FALSE);

            break;
        }

        case LVM_DELETEITEM:
        {
            RemoveFolder(wParam);
            break;
        }

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

CListFolders::CListFolders()
    : m_hWnd(NULL)
    , m_nWidth(0)
    , m_bLButtonDown(FALSE)
    , m_bDraging(FALSE)
    , m_bNCLButtonDown(FALSE)
    , m_pDirect2dFactory(NULL)
    , m_pRenderTarget(NULL)
    , m_pDWriteFactory(NULL)
    , m_pBr1(NULL)
    , m_pBr2(NULL)
    , m_pBrRed(NULL)
    , m_pTFL(NULL)
    , m_pTFLB(NULL)
    , m_plgbBorder(NULL)
    , m_plgbHover(NULL)
    , m_spInlineObjec_TFL(NULL)
    , m_spInlineObjec_TFLB(NULL)
    , m_fScale(1.0f)
    , m_nPosition(1)
    , m_items()
    , m_nVScrollMax(0)
    , m_bMouseTracking(FALSE)
    , m_pointLbuttonDown()
    , pDropTarget(NULL)
    , m_hovered()
{

}

CListFolders::~CListFolders()
{
    SafeRelease(&m_pDirect2dFactory);
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pDWriteFactory);
    SafeRelease(&m_pBr1);
    SafeRelease(&m_pBr2);
    SafeRelease(&m_pBrRed);
    SafeRelease(&m_pTFL);
    SafeRelease(&m_pTFLB);
    SafeRelease(&m_plgbBorder);
    SafeRelease(&m_plgbHover);
    SafeRelease(&m_spInlineObjec_TFL);
    SafeRelease(&m_spInlineObjec_TFLB);

    //UnregisterDropWindow(m_hWnd, pDropTarget);
    RevokeDragDrop(m_hWnd);
}

HWND CListFolders::Create(HWND hWndParent, HINSTANCE hInstance, LPVOID lpParam)
{
    HRESULT hr = CreateDeviceIndependentResources();
    if (!SUCCEEDED(hr))
        return NULL;

    WNDCLASSEX wincl;

    /* The Window structure */
    wincl.cbSize = sizeof(WNDCLASSEX);
    wincl.hInstance = hInstance;
    wincl.lpszClassName = m_szListFolderClassName;
    wincl.lpfnWndProc = ListProcExt;
    wincl.style = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;

    /* Use default icon and mouse-pointer */
    wincl.hIcon = NULL;
    wincl.hIconSm = NULL;

    wincl.hCursor = (HCURSOR)LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = sizeof(CListFolders*); //sizeof(LONG_PTR)
    wincl.cbWndExtra = 0;

    /* Use Windows's default color as the background of the window */
    //wincl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wincl.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);

    /* Register the window class, and if it fails quit the program */
    if (!::RegisterClassEx(&wincl))
    {
        DWORD dwError = GetLastError();
        if (dwError != ERROR_CLASS_ALREADY_EXISTS)
            return FALSE;
    }

    m_hWnd = CreateWindowEx(
        //WS_EX_CLIENTEDGE |
        WS_EX_TRANSPARENT |
        WS_EX_COMPOSITED |
        TVS_EX_DOUBLEBUFFER,
        m_szListFolderClassName,//WC_LISTVIEW, 
        NULL,
        WS_VISIBLE |
        WS_CHILD |
        WS_VSCROLL |
        TVS_SHOWSELALWAYS |
        TVS_NOHSCROLL |
        TVS_FULLROWSELECT |
        TVS_HASBUTTONS,
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
        m_nWidth = 300;
        //SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);
        //oldListWndProc = (LONG_PTR)SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)ListProcExt);

        HRESULT hr = RegisterDropWindow(m_hWnd, &pDropTarget);
    }

    return m_hWnd;
}

void CListFolders::DiscardDeviceDependentResources()
{
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pDWriteFactory);
    SafeRelease(&m_pBr1);
    SafeRelease(&m_pBr2);
    SafeRelease(&m_pBrRed);
    SafeRelease(&m_pTFL);
    SafeRelease(&m_pTFLB);
    SafeRelease(&m_spInlineObjec_TFL);
    SafeRelease(&m_spInlineObjec_TFLB);
    SafeRelease(&m_plgbBorder);
    SafeRelease(&m_plgbHover);

    for (ID2D1LinearGradientBrush* pBrush : m_colors_normal)
        SafeRelease(&pBrush);
    for (ID2D1LinearGradientBrush* pBrush : m_colors_pressed)
        SafeRelease(&pBrush);
}

HRESULT CListFolders::CreateDeviceIndependentResources()
{
    // Create a Direct2D factory.
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);

    return hr;
}

HRESULT CListFolders::CreateDeviceDependentResources()
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

        // Direct write factory
        m_pRenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE::D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(m_pDWriteFactory), reinterpret_cast<IUnknown**>(&m_pDWriteFactory));

        // Text formats
        hr = m_pDWriteFactory->CreateTextFormat(
            L"Cascadia Code",
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            12,
            L"",
            &m_pTFL);
        hr = m_pDWriteFactory->CreateTextFormat(
            L"Cascadia Code",
            NULL,
            DWRITE_FONT_WEIGHT_BOLD,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            12,
            L"",
            &m_pTFLB);

        hr = m_pTFL->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        hr = m_pTFL->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

        hr = m_pTFLB->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        hr = m_pTFLB->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

        if (m_pTFL != 0)
            hr = m_pDWriteFactory->CreateEllipsisTrimmingSign(m_pTFL, &m_spInlineObjec_TFL);
        if (m_pTFLB != 0)
            hr = m_pDWriteFactory->CreateEllipsisTrimmingSign(m_pTFLB, &m_spInlineObjec_TFLB);

        DWRITE_TRIMMING trimming = { DWRITE_TRIMMING_GRANULARITY_WORD, '\\', 2 };

        hr = m_pTFL->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        hr = m_pTFL->SetTrimming(&trimming, m_spInlineObjec_TFL);

        hr = m_pTFLB->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        hr = m_pTFLB->SetTrimming(&trimming, m_spInlineObjec_TFLB);

        //----------------------------------------------------------------
        // Mono color brushes
        if (SUCCEEDED(hr))
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pBr1);
        if (SUCCEEDED(hr))
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGray), &m_pBr2);
        if (SUCCEEDED(hr))
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &m_pBrRed);

        //----------------------------------------------------------------
        // Border brushes
        D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES lgbp{ D2D1::Point2F(0, 0) , D2D1::Point2F(0, 0) };
        D2D1_GRADIENT_STOP gpt2[2];
        gpt2[0].position = 0.0f;
        gpt2[1].position = 1.0f;
        gpt2[0].color = D2D1::ColorF(RGB(224, 224, 224));
        gpt2[1].color = D2D1::ColorF(RGB(96, 96, 96));

        ID2D1GradientStopCollection* gsc2 = NULL;
        m_pRenderTarget->CreateGradientStopCollection(&gpt2[0], ARRAYSIZE(gpt2), &gsc2);
        if (gsc2 != NULL)
            m_pRenderTarget->CreateLinearGradientBrush(lgbp, gsc2, &m_plgbBorder);

        //gpt2[0].color = D2D1::ColorF(RGB(255, 255, 0));
        //gpt2[1].color = D2D1::ColorF(RGB(127, 96, 0));
        gpt2[0].color = D2D1::ColorF(RGB(21, 21, 255));
        gpt2[1].color = D2D1::ColorF(RGB(0, 0, 96));
        m_pRenderTarget->CreateGradientStopCollection(&gpt2[0], ARRAYSIZE(gpt2), &gsc2);
        if (gsc2 != NULL)
            m_pRenderTarget->CreateLinearGradientBrush(lgbp, gsc2, &m_plgbHover);

        //----------------------------------------------------------------
        // Scale
        float dpiX{ 96.0f };
        float dpiY{ 96.0f };
        if (m_pRenderTarget != NULL)
            m_pRenderTarget->GetDpi(&dpiX, &dpiY);
        m_fScale = dpiX / 96.0f;

        //----------------------------------------------------------------
        // Add default color
        AddColor(m_pRenderTarget, COLOR_DEFAULT);

        // Force WM_NCCALCSIZE
        //SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED  | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOCOPYBITS);

        //TreeView_SetItemHeight(m_hWnd, (int)(20 * m_fScale));

    }

    return hr;
}

void CListFolders::OnSize(UINT nType, UINT nWidth, UINT nHeight)
{
    if (nType == SIZE_MINIMIZED)
        return;

    if (m_pRenderTarget != NULL)
    {
        RECT rc;
        GetClientRect(m_hWnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        m_pRenderTarget->Resize(size);
        InvalidateRect(m_hWnd, NULL, FALSE);

        ListView_SetColumnWidth(m_hWnd, 0, LVSCW_AUTOSIZE_USEHEADER);
    }
    if (m_items.size() > 0)
        ResetScrollBar();
}

HRESULT CListFolders::OnPaint2D()
{
    if (m_hWnd == NULL)
        return S_OK;

    HRESULT hr = CreateDeviceDependentResources();

    if (SUCCEEDED(hr))
    {
        m_pRenderTarget->BeginDraw();

        m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::AliceBlue)); // D2D1::ColorF::WhiteSmoke));
        D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

        // Begin painting
        //========================================================================

        D2D1_RECT_F rect{ 0.0f, 0.0f, rtSize.width, rtSize.height };

        DrawItems(m_pRenderTarget, rect);

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

void CListFolders::DrawItems(ID2D1HwndRenderTarget* pRT, D2D1_RECT_F& rect)
{
    D2D1_RECT_F rDropMark{};
    D2D1_RECT_F rc{ rect };
    rc.bottom = rc.top + DEFAULT_ITEM_HEIGHT;

    if (m_bDraging)
        OutputDebugString(L"Paint while drag\n");

    long lTop = GetTopItem();

    static D2D1_POINT_2F ptMark0{};
    static D2D1_POINT_2F ptMark1{};

    for (size_t t = lTop; t < m_items.size(); t++)
    {
        if (rc.top > rect.bottom)
            break;

        DrawItem(pRT, rect, rc, m_items, t, ptMark0, ptMark1);
    }

    if (m_bDraging && m_hovered.part != HOVER_PART::HP_MIDDLE)
    {
        pRT->DrawLine(ptMark0, ptMark1, m_pBrRed, 2);
        m_bDraging = FALSE;
    }
}

void CListFolders::DrawItem(ID2D1HwndRenderTarget* pRT, D2D1_RECT_F& rect, D2D1_RECT_F& rc, std::vector <FOLDER_ITEM> m_items, SIZE_T nItem, D2D1_POINT_2F& ptMark0, D2D1_POINT_2F& ptMark1, UINT nLevel)
{
    D2D1_POINT_2F pt0{};
    D2D1_POINT_2F pt1{};

    FOLDER_ITEM& fi = m_items.at(nItem);

    // Fill
    ID2D1LinearGradientBrush* pBrush{ NULL };
    if (fi.bSelected)
        pBrush = m_colors_pressed.at(fi.nColorIndex);
    else
        pBrush = m_colors_normal.at(fi.nColorIndex);

    pt0 = { 1000.0f, rc.top };
    pt1 = { 1000.0f, rc.bottom };
    pBrush->SetStartPoint(pt0);
    pBrush->SetEndPoint(pt1);

    // Background
    D2D1_RECT_F rcBorder{ rc };
    rcBorder.left += nLevel * 16;
    pRT->FillRectangle(&rcBorder, pBrush);

    // Border ----------------------------------------------------------------------
    m_plgbBorder->SetStartPoint(pt0);
    m_plgbBorder->SetEndPoint(pt1);
    pRT->DrawRectangle(&rcBorder, m_plgbBorder);

    // Drag mark -------------------------------------------------------------------
    if (m_bDraging && nItem == m_hovered.item && m_hovered.part != 1)
    {
        if (m_hovered.part == 0)
        {
            ptMark0 = { rc.left, rc.top };
            ptMark1 = { rc.right, rc.top };
        }
        else
        {
            ptMark0 = { rc.left, rc.bottom };
            ptMark1 = { rc.right, rc.bottom };
        }

        if (ptMark0.y < 1.5f)
            ptMark0.y = 1.5f;
        if (ptMark1.y < 1.5f)
            ptMark1.y = 1.5f;
        if (ptMark0.y > rect.bottom - 1.5f)
            ptMark0.y = rect.bottom - 1.5f;
        if (ptMark1.y > rect.bottom - 1.5f)
            ptMark1.y = rect.bottom - 1.5f;
    }

    // Left mark ----------------------------------------------------------------------
    D2D1_RECT_F rfl{ rcBorder };
    rfl.left += 0.5f;
    rfl.right = rfl.left + 3.5f;
    rfl.bottom += 1.0f;

    pt0 = { rfl.left, rfl.top };
    pt1 = { rfl.left + (rfl.right - rfl.left) / 2, rfl.bottom };
    m_plgbBorder->SetStartPoint(pt0);
    m_plgbBorder->SetEndPoint(pt1);
    m_plgbHover->SetStartPoint(pt0);
    m_plgbHover->SetEndPoint(pt1);

    if (m_bDraging)
    {
        if (m_hovered.part == 1 && nItem == m_hovered.item)
            pRT->FillRectangle(&rfl, m_plgbHover);
        else
            pRT->FillRectangle(&rfl, m_plgbBorder);
    }
    else
    {
        if (nItem == m_hovered.item && m_hovered.level == nLevel)
            pRT->FillRectangle(&rfl, m_plgbHover);
        else
            pRT->FillRectangle(&rfl, m_plgbBorder);
    }

    // Right mark ----------------------------------------------------------------------
    D2D1_RECT_F rfr{ rcBorder };
    rfr.left = rcBorder.right - 4.0f;
    rfr.right = rfr.left + 4.0f;

    pt0 = { rfr.left, rfr.top };
    pt1 = { rfr.left + (rfr.right - rfr.left) / 2, rfr.bottom };

    m_plgbBorder->SetStartPoint(pt0);
    m_plgbBorder->SetEndPoint(pt1);
    m_plgbHover->SetStartPoint(pt0);
    m_plgbHover->SetEndPoint(pt1);

    if (m_bDraging)
    {
        if (m_hovered.part == 1 && nItem == m_hovered.item)
            pRT->FillRectangle(&rfr, m_plgbHover);
        else
            pRT->FillRectangle(&rfr, m_plgbBorder);
    }
    else
    {
        if (nItem == m_hovered.item && m_hovered.level == nLevel)
            pRT->FillRectangle(&rfr, m_plgbHover);
        else
            pRT->FillRectangle(&rfr, m_plgbBorder);
    }

    // Text ----------------------------------------------------------------------
    D2D1_RECT_F rflock{ rcBorder };
    rflock.top += 4;
    rflock.bottom -= 4;
    rflock.left += 5;
    rflock.right = rflock.left + 10;

    std::wstring wch{};

    //Collapsed mark
    if (fi.HasChildren())
    {
        if (fi.bCollapsed)
            wch = L"+";
        else
            wch = L"━";
    }
    else
        wch += L" ";
    pRT->DrawText(wch.c_str(), (UINT32)wch.length(), m_pTFL, rflock, m_pBr1);

    //Cocked mark
    rflock.left += 8;
    rflock.right = rflock.left + 10;
    if (fi.bLocked)
        wch = L"∗";
    else
        wch = L" ";
    pRT->DrawText(wch.c_str(), (UINT32)wch.length(), m_pTFL, rflock, m_pBr1);

    D2D1_RECT_F rft{ rc };
    rft.top += 1;
    rft.bottom -= 1;
    rft.left += 21;
    rft.right -= 2;
    IDWriteTextFormat* pDWTF = m_pTFL;
    if (fi.bSelected)
    {
        rft.left += 1;
        pDWTF = m_pTFLB;
    }
    pRT->DrawText(fi.lpszPath, lstrlen(fi.lpszPath), pDWTF, rft, m_pBr1);

    // -----------------------------------------------------------------------
    rc.top = rc.bottom;
    rc.bottom = rc.top + DEFAULT_ITEM_HEIGHT;

    // Children ----------------------------------------------------------------------
    if (fi.HasChildren() && !fi.bCollapsed)
    {
        for (size_t t = 0; t < fi.children.size(); t++)
        {
            if (rc.top > rect.bottom)
                break;

            DrawItem(pRT, rect, rc, fi.children, t, ptMark0, ptMark1, nLevel + 1);
        }
    }

}

HANDLE CListFolders::AddFolder(LPCTSTR lpszText, HANDLE hParent)
{
    FOLDER_ITEM fi{};
    fi.hParent = hParent;

    if (hParent != NULL)
    {
        FOLDER_ITEM *pfi = (FOLDER_ITEM*)hParent;
        fi.hWndParent = m_hWnd;
        fi.lpszPath = lpszText;
        fi.move_index = -1;
        fi.nColorIndex = 0;
        fi.nLevel = pfi->nLevel + 1;
        pfi->children.push_back(fi);

        FOLDER_ITEM& nfi = pfi->children.at(pfi->children.size() - 1);
        nfi.hHandle = &nfi;
        ResetScrollBar();
        return fi.hHandle;
    }
    else
    {
        fi.nLevel = 0;
        fi.hWndParent = m_hWnd;
        fi.lpszPath = lpszText;
        fi.move_index = -1;
        fi.nColorIndex = 0;
        m_items.push_back(fi);


        FOLDER_ITEM& nfi = m_items.at(m_items.size() - 1);
        nfi.hHandle = &nfi;
        ResetScrollBar();
        return nfi.hHandle;
    }
}

void CListFolders::InsertFolder(FOLDER_ITEM* pfi, size_t at)
{
    pfi->hWndParent = m_hWnd;
    m_items.insert(m_items.begin() + at, *pfi);
    //pfi->lParent = (LONG_PTR)&m_items.at(at);
    ResetScrollBar();
    m_bDraging = FALSE;
}

void CListFolders::RemoveFolder(size_t at)
{
    if (at < 0)
        return;

    m_items.erase(m_items.begin() + at);
    ResetScrollBar();
    m_bDraging = FALSE;
}

long CListFolders::GetVirtualHeight()
{
    if (m_items.size() < 1)
        return 0;

    long lVirtualHeight = 0;
    size_t count = m_items.size();

    for (FOLDER_ITEM & fi : m_items)
    {
         lVirtualHeight += DEFAULT_ITEM_HEIGHT;

        if (!fi.bCollapsed)
            lVirtualHeight += fi.GetChildrenHeight();
    }

    return lVirtualHeight;
}

long CListFolders::GetTopItem()
{
    if (m_items.size() < 1)
        return LONG_MIN;

    int nVertScroll = GetScrollPos32(SB_VERT); // GetScrollPos(m_hWnd, SB_VERT);

    int nTop = 0;
    int nItem = 0;
    while (nTop < nVertScroll && nItem < m_items.size())
    {
        nTop += DEFAULT_ITEM_HEIGHT;
        nItem++;
        // Check expanded items
        // ....
    }

    return nItem;
}

int  CListFolders::GetScrollPos32(int nBar, BOOL bGetTrackPos /* = FALSE */)
{
    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);

    if (bGetTrackPos)
    {
        si.fMask = SIF_TRACKPOS;
        if (GetScrollInfo(m_hWnd, nBar, &si))
            return si.nTrackPos;
    }
    else
    {
        si.fMask = SIF_POS | SIF_ALL;
        if (GetScrollInfo(m_hWnd, nBar, &si))
            return si.nPos;
    }

    return 0;
}

BOOL CListFolders::SetScrollPos32(int nBar, int nPos, BOOL bRedraw /* = TRUE */)
{
    //m_idTopLeftCell.row = -1;

    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_POS;
    si.nPos = nPos;
    return SetScrollInfo(m_hWnd, nBar, &si, bRedraw);
}

void CListFolders::ResetScrollBar()
{
    if (!::IsWindow(m_hWnd))
        return;

    if (m_items.size() < 1)
        return;

    HDC screen = ::GetDC(0);
    int dpiX = GetDeviceCaps(screen, LOGPIXELSX);
    int dpiY = GetDeviceCaps(screen, LOGPIXELSY);
    ::ReleaseDC(0, screen);

    RECT rect{};
    GetClientRect(m_hWnd, &rect);
    if (dpiX > 0)
    {
        rect.left = ::MulDiv(rect.left, 96, dpiX);
        rect.top = ::MulDiv(rect.top, 96, dpiX);
        rect.right = ::MulDiv(rect.right, 96, dpiX);
        rect.bottom = ::MulDiv(rect.bottom, 96, dpiX);
    }

    int virtual_height = GetVirtualHeight();
    int visual_height = rect.bottom - rect.top;

    //int m_nVScrollMax = 0;
    if (virtual_height > visual_height)
    {
        ::EnableScrollBar(m_hWnd, SB_VERT, TRUE);
        m_nVScrollMax = virtual_height;
    }
    else
    {
        ::EnableScrollBar(m_hWnd, SB_VERT, FALSE);
        m_nVScrollMax = 0;
    }

    SCROLLINFO si;
    si.fMask = SIF_PAGE | SIF_RANGE | SIF_DISABLENOSCROLL;
    si.nPage = (m_nVScrollMax > 0) ? visual_height - 1 : 0;
    si.nMin = 0;
    si.nMax = m_nVScrollMax;
    SetScrollInfo(m_hWnd, SB_VERT, &si, TRUE);
}

void CListFolders::OnVScroll(UINT nSBCode, UINT nPos, HWND hScroll)
{
    int scrollPos = GetScrollPos32(SB_VERT);

    long idTopLeft = GetTopItem();

    RECT rect;
    GetClientRect(m_hWnd, &rect);

    switch (nSBCode)
    {
        case SB_LINEDOWN:
        {
            SetScrollPos32(SB_VERT, scrollPos + DEFAULT_ITEM_HEIGHT);
            break;
        }
        case SB_LINEUP:
        {
            SetScrollPos32(SB_VERT, __max(0, scrollPos - DEFAULT_ITEM_HEIGHT));
            break;
        }
        case SB_PAGEDOWN:
        {
            if (scrollPos < m_nVScrollMax)
            {
                int offset = ::MulDiv(rect.bottom - rect.top, 96, (int)(96 * m_fScale));
                int items = (int)(offset / DEFAULT_ITEM_HEIGHT);
                scrollPos = min(m_nVScrollMax, scrollPos + items * DEFAULT_ITEM_HEIGHT);
                SetScrollPos32(SB_VERT, scrollPos);
            }
            break;
        }
        case SB_PAGEUP:
        {
            if (scrollPos > 0)
            {
                int offset = -::MulDiv(rect.bottom - rect.top, 96, (int)(96 * m_fScale));
                int items = (int)(offset / DEFAULT_ITEM_HEIGHT);
                int pos = __max(0, scrollPos + items * DEFAULT_ITEM_HEIGHT);
                SetScrollPos32(SB_VERT, pos);
            }
            break;
        }
        case SB_TOP:
        {
            if (scrollPos > 0)
            {
                SetScrollPos32(SB_VERT, 0);
            }
            break;
        }
        case SB_BOTTOM:
        {
            if (scrollPos < m_nVScrollMax)
            {
                SetScrollPos32(SB_VERT, m_nVScrollMax);
            }
            break;
        }
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK: {
            SetScrollPos32(SB_VERT, GetScrollPos32(SB_VERT, TRUE));
            break;
        }

        default:
            break;
    }

    //POINT point;
    //GetCursorPos(&point);
    //ScreenToClient(m_hWnd, &point);
    //m_cellMouseOver = GetCellFromPt(point);
    //InvalidateRect(m_hWnd, NULL, FALSE);
}

void CListFolders::OnMouseWheel(UINT nFlags, short zDelta, POINT pt)
{
    static int m_zDelta = 0;

    UINT lines_per_notc = 0;
    SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &lines_per_notc, 0);

    m_zDelta += zDelta;
    if (abs(m_zDelta) >= WHEEL_DELTA)
    {
        if (m_zDelta > 0)
        {
            while (m_zDelta > 0)
            {
                for (UINT i = 0;  i < lines_per_notc; i++)
                    PostMessage(m_hWnd, WM_VSCROLL, SB_LINEUP, 0);
                m_zDelta -= WHEEL_DELTA;
            }
        }
        else
            while (m_zDelta < 0)
            {

                for (UINT i = 0; i < lines_per_notc; i ++)
                    PostMessage(m_hWnd, WM_VSCROLL, SB_LINEDOWN, 0);
                m_zDelta += WHEEL_DELTA;
            }

        m_zDelta = 0;
    }
}

void CListFolders::OnLButtonDown(UINT nFlags, POINT point)
{
    m_pointLbuttonDown = point;

    SELECTED_ITEM nSelected = HitTest(point);

    SelectItem(nSelected, m_items);

    InvalidateRect(m_hWnd, NULL, TRUE);
}

void CListFolders::SelectItem(SELECTED_ITEM selected, std::vector <FOLDER_ITEM>& items)
{
    for (size_t t = 0; t < items.size(); t++)
    {
        FOLDER_ITEM& fi = items.at(t);

        if (t == selected.item && selected.level == fi.nLevel && selected.handle_item == fi.hHandle)
        {
            fi.bSelected = TRUE;
            if (fi.HasChildren())
            {
                if (!fi.IsThereSlectedChild())
                {
                    if (fi.nLastChildSelected >= 0)
                        fi.children.at(fi.nLastChildSelected).bSelected = TRUE;
                    else
                    {
                        fi.children.at(0).bSelected = TRUE;
                        fi.nLastChildSelected = 0;
                    }
                }
            }

            if (fi.hParent != NULL)
            {
                FOLDER_ITEM* pfi = (FOLDER_ITEM*)fi.hParent;
                pfi->nLastChildSelected = (LONG)t;
            }
        }
        else
        {
            fi.bSelected = FALSE;
            if (fi.HasChildren())
            {
                SelectItem(selected, fi.children);
            }
            if (fi.IsThereSlectedChild())
            {
                fi.bSelected = TRUE;
            }
        }
    }

}

void CListFolders::OnLButtonDblClk(UINT nFlags, POINT point)
{
    SELECTED_ITEM nSelected = HitTest(point);
    LONG nItem = nSelected.item;
    if (nItem < 0)
        return;

    FOLDER_ITEM& fi = m_items.at(nItem);
    if (fi.HasChildren())
    {
        fi.bCollapsed = !fi.bCollapsed;

        for (FOLDER_ITEM& cfi : fi.children)
        {
            cfi.bHiden = fi.bCollapsed;
        }

        ResetScrollBar();
        InvalidateRect(m_hWnd, NULL, TRUE);
    }
}

void CListFolders::OnMouseMove(UINT nFlags, POINT point)
{
    if (!m_bMouseTracking)
    {
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = m_hWnd;
        TrackMouseEvent(&tme);
        m_bMouseTracking = TRUE;
    }

    SELECTED_ITEM siHovered = HitTest(point);

    if (m_bLButtonDown//)// && !m_bDraging)// &&
        && hypot(m_pointLbuttonDown.x - point.x, m_pointLbuttonDown.y - point.y) > 5)
    {
        m_bDraging = TRUE;
        DWORD dw = 0;

        std::vector<size_t> selected{};
        GetSelectedItems(selected);
        if (selected.size() > 0)
        {
            size_t nMoveItem = selected.at(0);

            IDataObject* pDataObject{};
            IDropSource* pDropSource{};
            DWORD		 dwEffect{};
            DWORD		 dwResult{};

            FORMATETC fmtetc = { FOLDER_FRMT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
            STGMEDIUM stgmed = { TYMED_HGLOBAL, { 0 }, 0 };

            // transfer the current selection into the IDataObject
            // If more then one items are selected - TODO
            // ...
            stgmed.hGlobal = CopyItem((LONG)nMoveItem);

            if (stgmed.hGlobal != NULL)
            {
                // Create IDataObject and IDropSource COM objects
                CreateDropSource(&pDropSource);
                CreateDataObject(&fmtetc, &stgmed, 1, &pDataObject);

                //SetCapture(m_hWnd);

                //============================================================================================
                //	** ** ** The drag-drop operation starts here! ** ** **
                dwResult = DoDragDrop(pDataObject, pDropSource, /*DROPEFFECT_COPY | */ DROPEFFECT_MOVE, &dwEffect);

                // success!
                if (dwResult == DRAGDROP_S_DROP)
                {
                    if (dwEffect & DROPEFFECT_MOVE)
                    {
                        m_hovered.item = -1;
                        m_hovered.part = HOVER_PART::HP_INVALID;
                    }
                }
                // cancelled
                else if (dwResult == DRAGDROP_S_CANCEL)
                {
                }

                //ReleaseCapture();

                pDataObject->Release();
                pDropSource->Release();
                GlobalFree(stgmed.hGlobal);

                InvalidateRect(m_hWnd, NULL, TRUE);
            }
            m_bDraging = FALSE;
        }
    }

    //if (nItemHover != m_nLastItemHover ||
    //    nPartHover != m_nLastPartHover && m_bDraging)
    if (siHovered != m_hovered)
    {
        m_hovered = siHovered;
        InvalidateRect(m_hWnd, NULL, TRUE);
    }
}

UINT CListFolders::AddColor(ID2D1HwndRenderTarget* pRT, COLORREF color)
{
    if (!pRT)
        return NULL;

    if (color == COLOR_DEFAULT && m_colors_normal.size() > 0) // Ако цвета е по подразбиране
        return 0;

    WORD wH = 0;
    WORD wL = 0;
    WORD wS = 0;
    ColorRGBToHLS(color, &wH, &wL, &wS);

    WORD wL1 = wL + 40 >= 240 ? 240 : wL + 40;
    WORD wL2 = wL - 40 <= 0 ? 0 : wL - 40;

    COLORREF clr1 = ColorHLSToRGB(wH, wL1, wS);
    COLORREF clr2 = ColorHLSToRGB(wH, wL2, wS);

    ID2D1LinearGradientBrush * pBrush = NULL;
    D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES lgbp{ D2D1::Point2F(0, 0) , D2D1::Point2F(0, 0) };

    D2D1_GRADIENT_STOP gpt4[4];
    gpt4[0].position = 0.0f;
    gpt4[1].position = 0.25f;
    gpt4[2].position = 0.75f;
    gpt4[3].position = 1.0f;

    // ------------------------------------------------------------------------------------------------------------
    gpt4[0].color = D2D1::ColorF(GetRValue(clr1) / 255.0f, GetGValue(clr1) / 255.0f, GetBValue(clr1) / 255.0f);
    gpt4[1].color = D2D1::ColorF(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);
    gpt4[2].color = D2D1::ColorF(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);
    gpt4[3].color = D2D1::ColorF(GetRValue(clr2) / 255.0f, GetGValue(clr2) / 255.0f, GetBValue(clr2) / 255.0f);

    ID2D1GradientStopCollection* gsc0 = NULL;
    pRT->CreateGradientStopCollection(&gpt4[0], ARRAYSIZE(gpt4), &gsc0);
    if (gsc0 != NULL)
        pRT->CreateLinearGradientBrush(lgbp, gsc0, &pBrush);

    D2D1_POINT_2F pt0{1000.0f, 0.0f};
    D2D1_POINT_2F pt1{ 1000.0f, DEFAULT_ITEM_HEIGHT};
    pBrush->SetStartPoint(pt0);
    pBrush->SetEndPoint(pt1);

    m_colors_normal.push_back(pBrush);

    // ------------------------------------------------------------------------------------------------------------
    gpt4[0].color = D2D1::ColorF(GetRValue(clr2) / 255.0f, GetGValue(clr2) / 255.0f, GetBValue(clr2) / 255.0f);
    gpt4[1].color = D2D1::ColorF(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);
    gpt4[2].color = D2D1::ColorF(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);
    gpt4[3].color = D2D1::ColorF(GetRValue(clr1) / 255.0f, GetGValue(clr1) / 255.0f, GetBValue(clr1) / 255.0f);

    ID2D1GradientStopCollection* gsc1 = NULL;
    pRT->CreateGradientStopCollection(&gpt4[0], ARRAYSIZE(gpt4), &gsc1);
    if (gsc1 != NULL)
        pRT->CreateLinearGradientBrush(lgbp, gsc1, &pBrush);
    pBrush->SetStartPoint(pt0);
    pBrush->SetEndPoint(pt1);

    m_colors_pressed.push_back(pBrush);

    // ------------------------------------------------------------------------------------------------------------
    return (UINT)m_colors_normal.size() - 1;
}

SELECTED_ITEM CListFolders::HitTest(POINT point)
{
    SELECTED_ITEM si{};

    D2D1_POINT_2F fpt{ (float)point.x / m_fScale, (float)point.y / m_fScale };
    float fOffset = 0.0f;

    long top = GetTopItem();
    for (size_t t = 0; t < m_items.size(); t++)
    {
        FOLDER_ITEM& fi = m_items.at(t);
        if (t >= top)
        {
            HiTestItem((LONG)t, m_items, fOffset, fpt, si);
            if (si.item >= 0)
                return si;
        }
    }
    return si;
}

void CListFolders::HiTestItem(long nItem, std::vector <FOLDER_ITEM> m_items, float& fOffset, D2D1_POINT_2F& fpt, SELECTED_ITEM& si, WORD nLevel)
{
    FOLDER_ITEM& fi = m_items.at(nItem);

    if (fpt.y >= fOffset && fpt.y <= fOffset + DEFAULT_ITEM_HEIGHT)
    {
        if (fpt.y >= fOffset && fpt.y <= fOffset + DEFAULT_ITEM_HEIGHT / 4.0f)
        {
            si.handle_item = fi.hHandle;
            si.parent_item = fi.hParent;
            si.level = nLevel;
            si.part = 0;
            si.item = (LONG)nItem;
            return;
        }

        if (fpt.y >= fOffset + DEFAULT_ITEM_HEIGHT / 4.0f && fpt.y <= fOffset + DEFAULT_ITEM_HEIGHT * 3 / 4.0f)
        {
            si.handle_item = fi.hHandle;
            si.parent_item = fi.hParent;
            si.level = nLevel;
            si.item = (LONG)nItem;
            si.part = 1;
            return;
        }

        if (fpt.y >= fOffset + DEFAULT_ITEM_HEIGHT * 3 / 4.0f && fpt.y <= fOffset + DEFAULT_ITEM_HEIGHT)
        {
            si.handle_item = fi.hHandle;
            si.parent_item = fi.hParent;
            si.level = nLevel;
            si.item = (LONG)nItem;
            si.part = 2;
            return;
        }
    }
    fOffset += DEFAULT_ITEM_HEIGHT;


    if (fi.HasChildren() && !fi.bCollapsed)
    {
        for (size_t t = 0; t < fi.children.size(); t++)
        {
            HiTestItem((LONG)t, fi.children, fOffset, fpt, si, nLevel + 1);
            if (si.item >= 0)
                return;
        }
    }
}

void CListFolders::GetSelectedItems(std::vector<size_t>& selected)
{
    selected.clear();

    size_t i = 0;
    for (FOLDER_ITEM& fi : m_items)
    {
        if (fi.bSelected)
            selected.push_back(i);
        i++;
    }
}
