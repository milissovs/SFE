#include <windows.h>
#include <windowsx.h>
#include "folderpane.h"
#include "globals.h"
#include "resource.h"

TCHAR m_szPaneClassName[] = TEXT("FOLDERPANE");


#define CX_BITMAP 16	// Each icon width  (tileset)
#define CY_BITMAP 16	// Each icon height (tileset)

LRESULT CALLBACK PaneProcEx(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    UNALIGNED CFolderPane* pThis = (CFolderPane*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (Message == WM_NCCREATE)
    {
        LPCREATESTRUCT lpCS = (LPCREATESTRUCT)lParam;
        CFolderPane* pThis = (CFolderPane*)lpCS->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->m_hWnd = hwnd;
    }

    if (!pThis)
        return ::DefWindowProc(hwnd, Message, wParam, lParam);

    return pThis->WindowProcedure(hwnd, Message, wParam, lParam);
}

LRESULT  CFolderPane::WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_ERASEBKGND:
        return 1;

    case WM_NCLBUTTONDOWN:
    {
        HWND hParent = ::GetParent(m_hWnd);
        m_bNCLButtonDown = TRUE;
        ::PostMessage(hParent, WM_NCLBUTTONDOWN, (WPARAM)0, (LPARAM)0);

        LRESULT hitTest = ::SendMessage(m_hWnd, WM_NCHITTEST, wParam, lParam);
        if (hitTest == HTLEFT || hitTest == HTRIGHT)
        {
            SetCapture(m_hWnd);
        }
        break;
    }

    case WM_LBUTTONDOWN:
    {
        HWND hParent = ::GetParent(m_hWnd);
        ::PostMessage(hParent, WM_LBUTTONDOWN, (WPARAM)0, (LPARAM)0);

        if (m_bCollapsed)
        {
            ::PostMessage(hParent, WM_FOLDER_RESIZE, 2, 0);
        }

        m_bLButtonDown = TRUE;
        break;
    }

    case WM_LBUTTONUP:
    {
        m_bLButtonDown = FALSE;
        m_bNCLButtonDown = FALSE;
        ReleaseCapture();
        break;
    }

    case WM_NCLBUTTONUP:
    {
        m_bLButtonDown = FALSE;
        m_bNCLButtonDown = FALSE;
        ReleaseCapture();
        break;
    }

    case WM_NCHITTEST:
    {
        POINT pt{ GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) };
        ScreenToClient(hWnd, &pt);
        RECT rect;
        GetClientRect(hWnd, &rect);
        if (pt.x <= 0)
            return HTLEFT;
        if (pt.x >= (rect.right - rect.left))
            return HTRIGHT;


        return HTCLIENT;
    }

    case WM_MOUSEMOVE:
    {
        POINT pt{ GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) };
        if (m_bNCLButtonDown)
        {
            ClientToScreen(m_hWnd, &pt);
            PostMessage(::GetParent(m_hWnd), WM_FOLDER_RESIZE, (WPARAM)0, MAKELPARAM(pt.x, pt.y));
            return 0;
        }
        if (!m_bMouseTracking)
        {
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(TRACKMOUSEEVENT);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hWnd;
            TrackMouseEvent(&tme);
            m_bMouseTracking = TRUE;
        }
        break;
    }

    case WM_NCMOUSEMOVE:
    {
        POINT pt{ GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) };
        if (m_bNCLButtonDown)
        {
            ClientToScreen(m_hWnd, &pt);
            PostMessage(::GetParent(m_hWnd), WM_FOLDER_RESIZE, (WPARAM)0, MAKELPARAM(pt.x, pt.y));
            return 0;
        }
        OnNCMouseMove(pt);
        break;
    }

    case WM_MOUSELEAVE:
    {
        m_bMouseTracking = FALSE;
        if (m_bCollapsible && !m_bCollapsed)
        {
            m_bCollapsed = TRUE;
            //ShowWindow(m_hWnd, SW_HIDE);
            HWND hParent = ::GetParent(m_hWnd);
            ::PostMessage(hParent, WM_FOLDER_RESIZE, 2, 0);
        }
        break;
    }

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

    case WM_PANE_FOLDER:
    {
        switch (wParam)
        {
            case 0:
            {
                SetPanePosition((int)lParam);
                PostMessage(GetParent(m_hWnd), WM_FOLDER_RESIZE, 1, lParam);
                break;
            }
            case 1:
            {
                return GetPanePosition();
            }
            case WM_PANE_FOLDER_LIST_FOLDERS:
            {
                switch (lParam)
                {
                    case WM_PANE_FOLDER_LIST_FOLDERS_GET_SELECTED:
                        return (LRESULT)(m_wndListFolders.m_selected.handle_item);

                    case WM_PANE_FOLDER_LIST_FOLDERS_GET_HWND:
                        return (LRESULT)m_wndListFolders.m_hWnd;

                    case WM_PANE_FOLDER_LIST_FOLDERS_GET_CRUMB:
                        return (LRESULT)SendMessage(GetParent(hWnd), WM_MDIFRAME, WP_MDIFRAME_GET_CRUMB_HWND, 0);
                }
            }
        }
        break;
    }

    case WM_NOTIFY:
    {
        LRESULT lResult;
        if (OnNotify(wParam, lParam, &lResult))
            return lResult;
        else
            return DefWindowProc(hWnd, message, wParam, lParam);

        break;
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

CFolderPane::CFolderPane()
    : m_hWnd(NULL)
    , m_bNCLButtonDown(FALSE)
    , m_bLButtonDown(FALSE)
    , m_nWidth(300)
    , m_nPanePos(0)
    , m_bCollapsible(FALSE)
    , m_bCollapsed(FALSE)
    , m_bMouseTracking(FALSE)
{

}

CFolderPane::~CFolderPane()
{
}

HWND CFolderPane::Create(HWND hWndParent, HINSTANCE hInstance, LPVOID lpParam)
{

    WNDCLASSEX wincl;

    /* The Window structure */
    wincl.cbSize = sizeof(WNDCLASSEX);
    wincl.hInstance = hInstance;
    wincl.lpszClassName = m_szPaneClassName;
    wincl.lpfnWndProc = PaneProcEx;
    wincl.style = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;

    /* Use default icon and mouse-pointer */
    wincl.hIcon = NULL;
    wincl.hIconSm = NULL;

    wincl.hCursor = (HCURSOR)LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = sizeof(CFolderPane*); //sizeof(LONG_PTR)
    wincl.cbWndExtra = 0;

    /* Use Windows's default color as the background of the window */
    wincl.hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);

    /* Register the window class, and if it fails quit the program */
    if (!::RegisterClassEx(&wincl))
    {
        DWORD dwError = GetLastError();
        if (dwError != ERROR_CLASS_ALREADY_EXISTS)
            return FALSE;
    }

    m_hWnd = CreateWindowEx(
        WS_EX_COMPOSITED | WS_EX_TRANSPARENT,
        m_szPaneClassName,
        NULL,
        WS_VISIBLE | WS_CHILD | WS_BORDER | WS_CLIPSIBLINGS,
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

void CFolderPane::SetPanePosition(int nPosition)
{
    m_nPanePos = nPosition;
    m_wndPaneHeader.SetPosition(nPosition);

    LONG_PTR style = GetWindowLongPtr(m_wndListFolders.m_hWnd, GWL_EXSTYLE);
    if (nPosition == 0)
        style |= WS_EX_LEFTSCROLLBAR;
    else
        style &= ~WS_EX_LEFTSCROLLBAR;

    SetWindowLongPtr(m_wndListFolders.m_hWnd, GWL_EXSTYLE, style);
}

void CFolderPane::SetPaneWidth(int nWidth, BOOL bInit)
{
    if (bInit)
    {
        m_nWidth = nWidth;
        return;
    }

    if (nWidth < 200)
        return;

    RECT rect;
    GetClientRect(GetParent(m_hWnd), &rect);
    if (nWidth > rect.right - 200)
        return;

    m_nWidth = nWidth;
}

int CFolderPane::OnCreate(HWND hWnd, LPCREATESTRUCT lpCS)
{
    HWND hW = m_wndPaneHeader.Create(hWnd, lpCS->hInstance, &m_wndPaneHeader);

    hW = m_wndListFolders.Create(hWnd, lpCS->hInstance, &m_wndListFolders);

    m_wndListFolders.AddFolder(TEST_PATH);
    HANDLE addr = m_wndListFolders.AddFolder(L"D:\\Milissov\\Work\\2023\\Radisson\\");
    //m_wndListFolders.AddFolder(L"12", addr);
    //m_wndListFolders.AddFolder(L"13", addr);
    //m_wndListFolders.AddFolder(L"14", addr);
    //m_wndListFolders.AddFolder(L"15", addr);
    m_wndListFolders.AddFolder(L"5");
    m_wndListFolders.AddFolder(L"6");
    m_wndListFolders.AddFolder(L"7");
    m_wndListFolders.AddFolder(L"8");
    m_wndListFolders.AddFolder(L"9");
    m_wndListFolders.AddFolder(L"10");

    addr  = m_wndListFolders.AddFolder(L"50");

    m_wndListFolders.AddFolder(L"51", addr);
    m_wndListFolders.AddFolder(L"52", addr);
    m_wndListFolders.AddFolder(L"53", addr);
    m_wndListFolders.AddFolder(L"54", addr);
    m_wndListFolders.AddFolder(L"55", addr);
    m_wndListFolders.AddFolder(L"56", addr);

    m_wndListFolders.AddFolder(L"60");
    m_wndListFolders.AddFolder(L"61");
    m_wndListFolders.AddFolder(L"62");

    addr = m_wndListFolders.AddFolder(L"70");
    m_wndListFolders.AddFolder(L"71", addr);
    m_wndListFolders.AddFolder(L"72", addr);
    m_wndListFolders.AddFolder(L"73", addr);
    m_wndListFolders.AddFolder(L"74", addr);
    m_wndListFolders.AddFolder(L"75", addr);
    m_wndListFolders.AddFolder(L"74", addr);
    m_wndListFolders.AddFolder(L"77", addr);
    m_wndListFolders.AddFolder(L"78", addr);
    m_wndListFolders.AddFolder(L"79", addr);
    m_wndListFolders.SetSelectedItem((CFolderItem*)m_wndListFolders.GetFirstItem());

    return 1;
}

void CFolderPane::OnSize(UINT nType, UINT nWidth, UINT nHeight)
{
    if (nType == SIZE_MINIMIZED)
        return;

    RECT rect;
    GetClientRect(m_hWnd, &rect);

    MoveWindow(m_wndPaneHeader.m_hWnd, rect.left, rect.top, rect.right - rect.left, 30, TRUE);
    MoveWindow(m_wndListFolders.m_hWnd, rect.left, 30, rect.right - rect.left, rect.bottom - 30, TRUE);
    

    InvalidateRect(m_hWnd, NULL, TRUE);
}

void CFolderPane::OnNCMouseMove(POINT pt)
{

}

BOOL CFolderPane::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
    // TODO: Add your specialized code here and/or call the base class

    //LPNMHDR nmh = (LPNMHDR)lParam;
    //if (nmh->hwndFrom == m_wndPaneHeader.m_hWnd)
    //{
    //    if (nmh->idFrom == 0)
    //    {
    //        switch (nmh->code)
    //        {
    //            case BN_CLICKED:
    //            {
    //                m_bCollapsible = !m_bCollapsible;
    //                PostMessage(GetParent(m_hWnd), WM_FOLDER_RESIZE, 2, lParam);
    //                break;
    //            }
    //        }
    //    }
    //}

    //if (nmh->hwndFrom == m_wndTreeForlders.m_hWnd)
    //{
    //    switch (nmh->code)
    //    {
    //        case NM_CLICK:
    //        {
    //            PostMessage(m_hWnd, WM_LBUTTONDOWN, 0, 0);
    //            break;
    //        }

    //        case NM_CUSTOMDRAW:
    //        {
    //            LPNMTVCUSTOMDRAW pCustomDraw = (LPNMTVCUSTOMDRAW)lParam;

    //            switch (pCustomDraw->nmcd.dwDrawStage)
    //            {

    //            case CDDS_PREPAINT:
    //            {
    //                // Need to process this case and set pResult to CDRF_NOTIFYITEMDRAW,
    //                // otherwise parent will never receive CDDS_ITEMPREPAINT notification. (GGH)
    //                *pResult = CDRF_NOTIFYITEMDRAW;

    //                return true;
    //            }

    //            case CDDS_ITEMPREPAINT:
    //            {
    //                switch (pCustomDraw->iLevel)
    //                {
    //                    // painting all 0-level items blue,
    //                    // and all 1-level items red (GGH)

    //                case 0:
    //                {
    //                    if (pCustomDraw->nmcd.uItemState == (CDIS_FOCUS | CDIS_SELECTED)) // selected
    //                    {
    //                        pCustomDraw->clrText = RGB(255, 255, 255);
    //                        pCustomDraw->clrTextBk = RGB(127, 127, 127);
    //                    }
    //                    else
    //                        pCustomDraw->clrText = RGB(255, 0, 0);
    //                    break;
    //                }
    //                case 1:
    //                {
    //                    if (pCustomDraw->nmcd.uItemState == (CDIS_FOCUS | CDIS_SELECTED)) // selected
    //                        pCustomDraw->clrText = RGB(255, 255, 255);
    //                    else
    //                        pCustomDraw->clrText = RGB(0, 255, 127);
    //                    break;
    //                }
    //                }

    //                *pResult = CDRF_SKIPDEFAULT;
    //                return false;
    //            }
    //            }

    //            break;
    //        }
    //    }
    //}


    return FALSE;
}