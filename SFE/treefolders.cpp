#include <windows.h>
#include <windowsx.h>
#include "treefolders.h"
#include "globals.h"
#include "resource.h"

TCHAR m_szFolderClassName[] = TEXT("TREEFOLDER");


#define CX_BITMAP 16	// Each icon width  (tileset)
#define CY_BITMAP 16	// Each icon height (tileset)

LRESULT CALLBACK TreeProcEx(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    UNALIGNED CTreeFolders* pThis = (CTreeFolders*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (Message == WM_NCCREATE)
    {
        LPCREATESTRUCT lpCS = (LPCREATESTRUCT)lParam;
        CTreeFolders* pThis = (CTreeFolders*)lpCS->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->m_hWnd = hwnd;
    }

    if (!pThis)
        return ::DefWindowProc(hwnd, Message, wParam, lParam);

    return pThis->WindowProcedure(hwnd, Message, wParam, lParam);
}

LRESULT  CTreeFolders::WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
            //m_bNCLButtonDown = TRUE;

            //LRESULT hitTest = ::SendMessage(m_hWnd, WM_NCHITTEST, wParam, lParam);
            //if (hitTest == HTLEFT)
            //{
            //    SetCapture(m_hWnd);
            //}
            break;
        }

        case WM_LBUTTONDOWN:
        {
            HWND hParent = ::GetParent(m_hWnd);
            ::PostMessage(hParent, WM_LBUTTONDOWN, (WPARAM)0, (LPARAM)0);

            m_bLButtonDown = TRUE;
            break;
        }

        //case WM_LBUTTONUP:
        //{
        //    m_bLButtonDown = FALSE;
        //    m_bNCLButtonDown = FALSE;
        //    ReleaseCapture();
        //    break;
        //}

        //case WM_NCLBUTTONUP:
        //{
        //    m_bLButtonDown = FALSE;
        //    m_bNCLButtonDown = FALSE;
        //    ReleaseCapture();
        //    break;
        //}

        //case WM_NCHITTEST:
        //{
        //    POINT pt{ GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) };
        //    ScreenToClient(hWnd, &pt);
        //    if (pt.x < 0)
        //        return HTLEFT;
        //    if (pt.y < 0)
        //        return HTCAPTION;
        //    RECT rect;
        //    GetClientRect(hWnd, &rect);
        //    if (PtInRect(&rect, pt))
        //        return HTCLIENT;
        //    if (pt.x > (rect.right - rect.left))
        //        return HTRIGHT;
        //    if (pt.y > (rect.bottom - rect.top))
        //        return HTBOTTOM;


        //    return HTCLIENT;
        //}

        //case WM_NCCALCSIZE:
        //{
        //    if (wParam == FALSE)
        //    {
        //        LPRECT r = (LPRECT)lParam;
        //        return 0;
        //    }
        //    else
        //    {
        //        UINT dpi = GetDpiForWindow(hWnd);

        //        int frame_x = GetSystemMetricsForDpi(SM_CXFRAME, dpi);
        //        int frame_y = GetSystemMetricsForDpi(SM_CYFRAME, dpi);
        //        int padding = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);

        //        LPNCCALCSIZE_PARAMS p = (LPNCCALCSIZE_PARAMS)lParam;
        //        p->rgrc[0].top += padding; // (int)(20 * m_fScale);
        //        p->rgrc[0].left += 3; // frame_x + padding; //3;
        //        p->rgrc[0].right -= 3; // frame_x + padding; //3;
        //        p->rgrc[0].bottom -= 3; // frame_y + padding; // 3;

        //        return WVR_VALIDRECTS;
        //    }
        //}

        //case WM_MOUSEMOVE:
        //{
        //    POINT pt{ GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) };
        //    if (m_bNCLButtonDown)
        //    {
        //        //RECT rect;
        //        //GetWindowRect(m_hWnd, &rect);
        //        //m_nWidth += abs(pt.x);
        //        ClientToScreen(m_hWnd, &pt);
        //        //rect.left += pt.x;
        //        //MoveWindow(m_hWnd, rect.left, rect.top, m_nWidth, rect.bottom - rect.top, TRUE);
        //        PostMessage(::GetParent(m_hWnd), WM_FOLDER_RESIZE, (WPARAM)0, MAKELPARAM(pt.x, pt.y));
        //    }
        //}

        //case WM_NCMOUSEMOVE:
        //{
        //    POINT pt{ GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) };
        //    //ScreenToClient(hWnd, &pt);
        //    if (m_bNCLButtonDown)
        //    {
        //        RECT rect;
        //        GetWindowRect(m_hWnd, &rect);
        //        //m_nWidth += abs(pt.x);
        //        ClientToScreen(m_hWnd, &pt);
        //        PostMessage(::GetParent(m_hWnd), WM_FOLDER_RESIZE, (WPARAM)0, MAKELPARAM(pt.x, pt.y));
        //    }
        //    break;
        //}

        //case WM_NCPAINT:
        //{
        //    OnNcPaint(wParam);
        //    break;
        //}

        case WM_PAINT:
        {
            RECT rect;
            BOOL bRes = GetUpdateRect(hWnd, &rect, FALSE);
            if (!bRes)
                return 0;

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(m_hWnd, &ps);
            OnPaint2D();

            //HBRUSH title_bar_brush = CreateSolidBrush(RGB(150, 200, 180));
            //RECT wr = rect;
            //wr.top = 0;
            //wr.bottom = 20;
            //FillRect(hdc, &wr, title_bar_brush);
            EndPaint(m_hWnd, &ps);

            return 0;
        }

        default:
            return ::CallWindowProc((WNDPROC)oldTreeWndProc, hWnd, message, wParam, lParam);
            //return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

CTreeFolders::CTreeFolders()
    : m_hWnd(NULL)
    , m_nWidth(0)
    , m_bLButtonDown(FALSE)
    , m_bNCLButtonDown(FALSE)
    , m_pDirect2dFactory(NULL)
    , m_pRenderTarget(NULL)
    , m_pBr1(NULL)
    , m_pBr2(NULL)
    , oldTreeWndProc(NULL)
    , m_fScale(1.0f)
    , m_nPosition(1)
{

}

CTreeFolders::~CTreeFolders()
{
    SafeRelease(&m_pDirect2dFactory);
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pBr1);
    SafeRelease(&m_pBr2);
}

HWND CTreeFolders::Create(HWND hWndParent, HINSTANCE hInstance, LPVOID lpParam)
{
    HRESULT hr = CreateDeviceIndependentResources();
    if (!SUCCEEDED(hr))
        return NULL;

    //WNDCLASSEX wincl;

    ///* The Window structure */
    //wincl.cbSize = sizeof(WNDCLASSEX);
    //wincl.hInstance = hInstance;
    //wincl.lpszClassName = m_szFolderClassName;
    //wincl.lpfnWndProc = TreeProcEx;
    //wincl.style = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;

    ///* Use default icon and mouse-pointer */
    //wincl.hIcon = NULL;
    //wincl.hIconSm = NULL;

    //wincl.hCursor = (HCURSOR)LoadCursor(NULL, IDC_ARROW);
    //wincl.lpszMenuName = NULL;
    //wincl.cbClsExtra = sizeof(CTreeFolders*); //sizeof(LONG_PTR)
    //wincl.cbWndExtra = 0;

    ///* Use Windows's default color as the background of the window */
    ////wincl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    //wincl.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);

    ///* Register the window class, and if it fails quit the program */
    //if (!::RegisterClassEx(&wincl))
    //{
    //    DWORD dwError = GetLastError();
    //    if (dwError != ERROR_CLASS_ALREADY_EXISTS)
    //        return FALSE;
    //}

    m_hWnd = CreateWindowEx(
        //WS_EX_CLIENTEDGE |
        WS_EX_TRANSPARENT |
        WS_EX_COMPOSITED,
        WC_TREEVIEW, //m_szFolderClassName,
        NULL,
        WS_VISIBLE | WS_CHILD | WS_VSCROLL,
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
        SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);
        oldTreeWndProc = (LONG_PTR)SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)TreeProcEx);
    }

    return m_hWnd;
}

HTREEITEM CTreeFolders::AddFolder(HWND hwndTV, LPCTSTR lpszItem, int nLevel)
{
    TVITEM tvi = { 0 };
    TVINSERTSTRUCT tvins;
    static HTREEITEM hPrev = (HTREEITEM)TVI_FIRST;
    static HTREEITEM hPrevRootItem = NULL;
    static HTREEITEM hPrevLev2Item = NULL;
    static HTREEITEM hPrevLev3Item = NULL;
    HTREEITEM hti;

    tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;

    tvi.pszText = (LPTSTR)lpszItem;
    tvi.cchTextMax = lstrlen(lpszItem); // (long)sizeof(tvi.pszText) / (long)sizeof(tvi.pszText[0]);

    tvi.iImage = 0;
    tvi.iSelectedImage = 0;

    tvi.lParam = (LPARAM)nLevel;
    tvins.item = tvi;
    tvins.hInsertAfter = hPrev;

    if (nLevel == 1)
        tvins.hParent = TVI_ROOT;
    else if (nLevel == 2)
        tvins.hParent = hPrevRootItem;
    else if (nLevel == 3)
        tvins.hParent = hPrevLev2Item;

    hPrev = (HTREEITEM)SendMessage(hwndTV, TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tvins);

    if (hPrev == NULL)
        return NULL;

    if (nLevel == 1)
        hPrevRootItem = hPrev;
    else if (nLevel == 2)
        hPrevLev2Item = hPrev;
    else if (nLevel == 3)
        hPrevLev3Item = hPrev;

    if (nLevel > 1) {
        hti = TreeView_GetParent(hwndTV, hPrev);
        tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        tvi.hItem = hti;
        tvi.iImage = 0;
        tvi.iSelectedImage = 0;
        TreeView_SetItem(hwndTV, &tvi);
    }

    return hPrev;
}

BOOL CTreeFolders::InitTreeViewImageLists(HWND hwnd)
{
    HIMAGELIST himl;
    HBITMAP hbmp;

    if ((himl = ImageList_Create(CX_BITMAP, CY_BITMAP, ILC_COLOR16, 2, 0)) == NULL)
        return FALSE;
    hbmp = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_TILESET));
    ImageList_Add(himl, hbmp, NULL);
    DeleteObject(hbmp);

    TreeView_SetImageList(hwnd, himl, TVSIL_NORMAL);
    return TRUE;
}

void CTreeFolders::OnSize(UINT nType, UINT nWidth, UINT nHeight)
{
    if (nType == SIZE_MINIMIZED)
        return;

    if (m_pRenderTarget != NULL)
    {
        RECT rc;
        GetClientRect(m_hWnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        m_pRenderTarget->Resize(size);
        //CalculateLayout();
        InvalidateRect(m_hWnd, NULL, FALSE);
    }
}

HRESULT CTreeFolders::CreateDeviceIndependentResources()
{
    // Create a Direct2D factory.
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);

    return hr;
}

HRESULT CTreeFolders::CreateDeviceDependentResources()
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
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::SkyBlue), &m_pBr1);
        if (SUCCEEDED(hr))
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGray), &m_pBr2);

        float dpiX{ 96.0f };
        float dpiY{ 96.0f };
        m_pRenderTarget->GetDpi(&dpiX, &dpiY);
        m_fScale = dpiX / 96.0f;

        // Force WM_NCCALCSIZE
        SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED  | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOCOPYBITS);
    }

    return hr;
}

void CTreeFolders::DiscardDeviceDependentResources()
{
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pBr1);
    SafeRelease(&m_pBr2);
}

HRESULT CTreeFolders::OnPaint2D()
{
    if (m_hWnd == NULL)
        return S_OK;

    HRESULT hr = CreateDeviceDependentResources();

    if (SUCCEEDED(hr))
    {
        m_pRenderTarget->BeginDraw();

        m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::WhiteSmoke)); // D2D1::ColorF::WhiteSmoke));
        D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

        // Begin painting
        //========================================================================

        D2D1_RECT_F rect{ 0.0f, 0.0f, rtSize.width, rtSize.height };


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

void CTreeFolders::OnNcPaint(WPARAM wParam)
{
    RECT rect;
    GetWindowRect(m_hWnd, &rect);

    HRGN region = NULL;
    if (wParam == NULLREGION)
    {
        region = CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom);
    }
    else
    {
        HRGN copy = CreateRectRgn(0, 0, 0, 0);
        if (CombineRgn(copy, (HRGN)wParam, NULL, RGN_COPY))
        {
            region = copy;
        }
        else
        {
            DeleteObject(copy);
        }
    }

    HDC dc{ GetDCEx(m_hWnd, region, DCX_WINDOW | DCX_CACHE | DCX_INTERSECTRGN | DCX_LOCKWINDOWUPDATE) };
    if (!dc && region)
    {
        DeleteObject(region);
    }
    else
    {
        HPEN pen1{ CreatePen(PS_INSIDEFRAME, 1, RGB(18, 184, 221)) };
        HPEN pen2{ CreatePen(PS_INSIDEFRAME, 1, RGB(13, 250, 255)) };

        //HGDIOBJ old{ SelectObject(dc, pen1) };
        //if (old != 0)
        //{
        //    if (dc != 0)
        //    {
        //        Rectangle(dc, 0, 0, rect.right - rect.left, rect.bottom - rect.top);

        //        SelectObject(dc, pen2);
        //        Rectangle(dc, 1, 1, rect.right - rect.left - 1, rect.bottom - rect.top - 1);
        //    }
        //}

        if (dc)
        {
            POINT pt{ rect.left, rect.top };
            ScreenToClient(m_hWnd, &pt);
            rect.left = 0;// pt.x;
            rect.top = 0; // pt.y;
            pt.x = rect.right - pt.x; pt.y = rect.bottom - pt.y;
            ScreenToClient(m_hWnd, &pt);
            rect.right = pt.x;
            rect.bottom = pt.y;
            DrawFrameControl(dc, &rect, DFC_BUTTON, DFCS_BUTTONPUSH);
        }

        //if (old != 0)
        //    SelectObject(dc, old);
        if (dc != 0)
            ReleaseDC(m_hWnd, dc);
        DeleteObject(pen1);
        DeleteObject(pen2);
    }
}