#include <windows.h>
#include <windowsx.h>
#include <atltypes.h>
#include "resource.h"
#include "globals.h"
#include "mdirame.h"
#include <tchar.h>

TCHAR m_szMDIFrameClassName[] = TEXT("MDIFRAME");

LRESULT CALLBACK MDIFrameProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    UNALIGNED CMDIFrame* pThis = (CMDIFrame*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (Message == WM_NCCREATE)
    {
        LPCREATESTRUCT lpCS = (LPCREATESTRUCT)lParam;
        CMDIFrame* pThis = (CMDIFrame*)lpCS->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->m_hWnd = hwnd;
    }

    if (!pThis)
        return ::DefWindowProc(hwnd, Message, wParam, lParam);

    return pThis->WindowProcedure(hwnd, Message, wParam, lParam);
}

LRESULT  CMDIFrame::WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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

        case WM_NCLBUTTONDOWN:
        case WM_LBUTTONDOWN:
        {
            //m_bActive = TRUE;
            HWND hParent = GetParent(hWnd);
            PostMessage(hParent, WM_FRAME_ACTIVE, (WPARAM)0, (LPARAM)hWnd);
            break;
        }

        case WM_FRAME_ACTIVE:
        {
            return (LRESULT)m_bActive;
            break;
        }

        case WM_SIZE:
        {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
            OnSize((UINT)wParam, width, height);
            break;
        }

        case WM_UPDATE_SETTINGS:
        {
            SendMessage(GetParent(hWnd), WM_UPDATE_SETTINGS, 0, 0);
            break;
        }

        case WM_FOLDER_RESIZE:
        {
            switch (wParam)
            {
                case 0:
                {
                    static WINDOWPOS wp;
                    POINT pt{ GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) };
                    ScreenToClient(m_hWnd, &pt);
                    RECT rcClient;
                    GetClientRect(hWnd, &rcClient);

                    HDWP defStruct = BeginDeferWindowPos(1);

                    if (m_wndFolderPane.GetPanePosition() == 1)
                    {
                        //DeferWindowPos(defStruct, m_wndFolderPane.m_hWnd, NULL,
                        //    pt.x,
                        //    rcClient.top,
                        //    rcClient.right - pt.x,
                        //    rcClient.bottom - rcClient.top,
                        //    SWP_NOZORDER | SWP_NOCOPYBITS);
                        m_wndFolderPane.SetPaneWidth(rcClient.right - pt.x);
                    }
                    else
                    {
                        //DeferWindowPos(defStruct, m_wndFolderPane.m_hWnd, NULL,
                        //    0,
                        //    rcClient.top,
                        //    pt.x,
                        //    rcClient.bottom - rcClient.top,
                        //    SWP_NOZORDER | SWP_NOCOPYBITS);
                        m_wndFolderPane.SetPaneWidth(rcClient.left + pt.x);
                    }

                    EndDeferWindowPos(defStruct);

                    SendMessage(m_hWnd, WM_SIZE, 0, MAKELPARAM(rcClient.right - rcClient.left, rcClient.bottom - rcClient.top));

                    //PostMessage(m_wndFolderPane.m_hWnd, WM_WINDOWPOSCHANGED, 0, (LPARAM)&wp);

                    //InvalidateRect(m_hWnd, NULL, TRUE);
                    PostMessage(GetParent(m_hWnd), WM_UPDATE_SETTINGS, 0, 0);
                    break;
                }
                case 1:
                {
                    //SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOCOPYBITS);
                    RECT rcClient;
                    GetClientRect(hWnd, &rcClient);
                    RECT fr = MoveFolder(rcClient);
                    //SendMessage(m_wndFolderPane.m_hWnd, WM_SIZE, 0, MAKELPARAM(fr.right - fr.left, fr.bottom - fr.top));
                    //InvalidateRect(m_hWnd, NULL, TRUE);
                    break;
                }
                case 2: // Pressed PIN button
                {
                    //if (m_wndFolderPane.GetCollapsible())
                    //{
                    //    BOOL bCollapsed = m_wndFolderPane.GetCollapsed();
                    //    if (!bCollapsed)
                    //    {
                            RECT rc;
                            GetClientRect(m_hWnd, &rc);
                            PostMessage(m_hWnd, WM_SIZE, 0, MAKELPARAM(rc.right - rc.left, rc.bottom - rc.top));
                    //    }
                    //}
                    break;
                }
            }
            break;
        }

        case WM_MOUSEMOVE:
        {
            RECT rc;
            GetClientRect(m_hWnd, &rc);
            POINT pt{ GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) };

            if (rc.right - pt.x < 10)
            {
                if (m_wndFolderPane.GetCollapsed())
                {
                    m_wndFolderPane.SetCollapsed(FALSE);
                    //ShowWindow(m_wndFolderPane.m_hWnd, SW_RESTORE);
                    //SetWindowPos(m_wmdListView.m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOREPOSITION);
                    //SetWindowPos(m_wndFolderPane.m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOREPOSITION);
                    RECT rc;
                    GetClientRect(m_hWnd, &rc);
                    OnSize((UINT)0, rc.right - rc.left, rc.bottom - rc.top);
                }
            }
            break;
        }

        case WM_CLOSE:
        {
            DestroyWindow(hWnd);
            break;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

HWND CMDIFrame::Create(HWND hWndParent, HINSTANCE hInstance, LPVOID lpParam)
{

    WNDCLASSEX wincl;

    /* The Window structure */
    wincl.cbSize = sizeof(WNDCLASSEX);
    wincl.hInstance = hInstance;
    wincl.lpszClassName = m_szMDIFrameClassName;
    wincl.lpfnWndProc = MDIFrameProc;
    wincl.style = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;

    /* Use default icon and mouse-pointer */
    wincl.hIcon = NULL;
    wincl.hIconSm = NULL;

    wincl.hCursor = (HCURSOR)LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = sizeof(CMDIFrame*); //sizeof(LONG_PTR)
    wincl.cbWndExtra = 0;

    /* Use Windows's default color as the background of the window */
    wincl.hbrBackground = (HBRUSH)COLOR_WINDOW + 1;

    /* Register the window class, and if it fails quit the program */
    if (!::RegisterClassEx(&wincl))
    {
        DWORD dwError = GetLastError();
        if (dwError != ERROR_CLASS_ALREADY_EXISTS)
            return FALSE;
    }

    m_hWnd = CreateWindowEx(
        WS_EX_COMPOSITED |
        WS_EX_TRANSPARENT |
        WS_EX_CONTROLPARENT,                      /* Extended possibilites for variation */
        m_szMDIFrameClassName,  /* Classname */
        NULL,                   /* Title Text */
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, // | WS_BORDER,  /* child window */
        0,                      /* Windows decides the position */
        0,                      /* where the window ends up on the screen */
        100,                    /* The programs width */
        100,                    /* and height in pixels */
        hWndParent,             /* The window is a child-window to desktop */
        NULL,                   /* No menu */
        hInstance,              /* Program Instance handler */
        (LPVOID)this            /* Window Creation data */
    );

    return m_hWnd;
}

CMDIFrame::CMDIFrame()
    : m_hWnd(NULL)
    , m_bActive(FALSE)
    , nID(0)
    , m_fScale(1.0f)
{
}

CMDIFrame::~CMDIFrame()
{
}

int CMDIFrame::OnCreate(HWND hWnd, LPCREATESTRUCT lpCS)
{
    m_wndTPane.Create(hWnd, lpCS->hInstance, &m_wndTPane);

    HWND m_hWnd = m_wndFolderPane.Create(hWnd, lpCS->hInstance, &m_wndFolderPane);
    ShowWindow(m_hWnd, SW_SHOW);

    m_wmdListView.Create(hWnd, lpCS->hInstance, &m_wmdListView);

    //HWND m_hWnd = m_wndFolders.Create(hWnd, lpCS->hInstance, &m_wndFolders);
    //ShowWindow(m_hWnd, SW_SHOW);

    ////m_wndFolders.AddFolder(m_wndFolders.m_hWnd, L"Emo", 1);
    ////m_wndFolders.AddFolder(m_wndFolders.m_hWnd, L"Kalina", 1);

    return 1;
}

void CMDIFrame::OnSize(UINT nType, UINT nWidth, UINT nHeight)
{
    if (nType == SIZE_MINIMIZED)
        return;

    RECT rect;
    GetClientRect(m_hWnd, &rect);

    HDC hDC = ::GetDC(m_hWnd);
    INT xdpi = ::GetDeviceCaps(hDC, LOGPIXELSX);
    ::ReleaseDC(m_hWnd, hDC);
    m_fScale = xdpi / 96.0f;

    int toolbar_height = (int)(m_wndTPane.GetHeight() * m_fScale);
    MoveWindow(m_wndTPane.m_hWnd, rect.left, rect.top, nWidth, toolbar_height, FALSE);

    MoveFolder(rect);

    InvalidateRect(m_hWnd, NULL, TRUE);
}

RECT CMDIFrame::MoveFolder(RECT rect)
{
    RECT fr{ rect };
    RECT lr{ rect };

    if (m_wndFolderPane.GetPanePosition() == 0)
    {
        fr.right = rect.left + m_wndFolderPane.GetPaneWidth();
        lr.left = fr.right;
    }
    else
    {
        fr.left = rect.right - m_wndFolderPane.GetPaneWidth();
        lr.right = fr.left;
    }

    int toolbar_height = (int)(m_wndTPane.GetHeight() * m_fScale);
    fr.top += toolbar_height;
    lr.top += toolbar_height;

    if (!m_wndFolderPane.GetCollapsible())
    {
        MoveWindow(m_wndFolderPane.m_hWnd, fr.left, fr.top, fr.right - fr.left, fr.bottom - fr.top, FALSE);

        MoveWindow(m_wmdListView.m_hWnd, lr.left, lr.top, lr.right - lr.left, lr.bottom - lr.top, FALSE);
    }
    else
    {
        rect.top += (int)(m_wndTPane.GetHeight() * m_fScale);
        if (!m_wndFolderPane.GetCollapsed())
        {
            //ShowWindow(m_wndFolderPane.m_hWnd, SW_HIDE);
            m_wndFolderPane.SetCollapsed(TRUE);
            MoveWindow(m_wndFolderPane.m_hWnd, rect.right - 30, rect.top, rect.right - rect.left, rect.bottom - rect.top, FALSE);
            SetWindowPos(m_wndFolderPane.m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }
        else
        {
            m_wndFolderPane.SetCollapsed(FALSE);
            MoveWindow(m_wndFolderPane.m_hWnd, rect.right - m_wndFolderPane.GetPaneWidth(), rect.top, rect.right, rect.bottom - rect.top, FALSE);
        }
        MoveWindow(m_wmdListView.m_hWnd, rect.left, rect.top, rect.right - rect.left - 30, rect.bottom - rect.top, FALSE);
    }

    return fr;
}