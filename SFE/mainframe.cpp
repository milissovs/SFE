#include <windows.h>
#include <commctrl.h>
#include "globals.h"
#include "resource.h"
#include "mainframe.h"
#include <atltypes.h>
//#include <atlbase.h>
#include <atlstr.h>

/*  Make the class name into a global variable  */
TCHAR szClassName[] = TEXT("SFE_App");

int nWnd_Width = 0;
int nWnd_Height = 0;
HIMAGELIST g_hImageList;
HWND hWndToolbar;

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    UNALIGNED CMainFrame* pThis = (CMainFrame*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (Message == WM_NCCREATE)
    {
        LPCREATESTRUCT lpCS = (LPCREATESTRUCT)lParam;
        CMainFrame* pThis = (CMainFrame*)lpCS->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->m_hWnd = hwnd;
    }

    if (!pThis)
        return ::DefWindowProc(hwnd, Message, wParam, lParam);

    return pThis->WindowProcedure(hwnd, Message, wParam, lParam);
}

/*  This function is called by the Windows function DispatchMessage()  */
LRESULT  CMainFrame::WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CRect rect;
    GetClientRect(hWnd, &rect);
    //UNALIGNED CMainFrame* pThis = (CMainFrame*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (message)                  /* handle the messages */
    {
    case WM_ERASEBKGND:
        return 1;

    case WM_CREATE:
    {
        LPCREATESTRUCT lpCS = (LPCREATESTRUCT)lParam;
        return OnCreate(hWnd, (LPCREATESTRUCT)lParam);
    }

    case WM_DESTROY:
    {
        SendMessage(hWnd, WM_UPDATE_SETTINGS, 0, 0);
        OnDestroy();
        PostQuitMessage(0);      /* send a wm_quit to the message queue */
        break;
    }

    case WM_SIZE:
    {
        UINT width = LOWORD(lParam);
        UINT height = HIWORD(lParam);
        OnSize((UINT)wParam, width, height);
        if (wParam == SIZE_RESTORED ||
            wParam == SIZE_MAXIMIZED)
            SendMessage(hWnd, WM_UPDATE_SETTINGS, 0 ,0);
        break;
    }

    case WM_GETMINMAXINFO:
    {
        float fScale = GetScale(hWnd);
        MINMAXINFO* mmi = (MINMAXINFO*)lParam;
        mmi->ptMinTrackSize.x = (int)(480.0f * fScale);
        mmi->ptMinTrackSize.y = (int)(360.0f * fScale);
        break;
    }

    case WM_COMMAND:
    {
        WORD id = LOWORD(wParam);

        switch (id)
        {
            case ID_VIEW_SPLIT_NONE:
            case ID_VIEW_SPLIT_VERT:
            case ID_VIEW_SPLIT_HORZ:
            {
                SendMessage(m_wndMainView.m_hWnd, message, wParam, lParam);
                break;
            }
            default:
                return OnCommand(hWnd, wParam, lParam);
        }
        break;
    }

    case WM_DISPLAYCHANGE:
    {
        InvalidateRect(hWnd, NULL, FALSE);
        break;
    }

    case WM_UPDATE_SETTINGS:
    {
        UpdateSettings();
        break;
    }

    case WM_CLOSE:
    {
        DestroyWindow(hWnd);
        break;
    }

    default: /* for messages that we don't deal with */
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

CMainFrame::CMainFrame()
    : m_hWnd(NULL)
    , m_hInstance( NULL )
    , m_wndPossition({})
    , m_bMaximized(FALSE)
    , m_wSnap(SNAP_NONE)
{

}

CMainFrame::~CMainFrame()
{
}

BOOL CMainFrame::RegisterClassEx(HINSTANCE hInstance)
{
    m_hInstance = hInstance;
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    wincl.cbSize = sizeof(WNDCLASSEX);
    wincl.hInstance = hInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WndProc;
    wincl.style = CS_DBLCLKS;

    /* Use default icon and mouse-pointer */
    //wincl.hIcon         = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 32, 32, 0);
    wincl.hIconSm = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 16, 16, 0);

    wincl.hCursor = (HCURSOR)LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_SHARED);
    wincl.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
    wincl.cbClsExtra = sizeof(CMainFrame*);
    wincl.cbWndExtra = 0;

    /* Use Windows's default color as the background of the window */
    wincl.hbrBackground = (HBRUSH)COLOR_BACKGROUND + 1;
    //wincl.hbrBackground = (HBRUSH)COLOR_BTNTEXT + 1;

    /* Register the window class, and if it fails quit the program */
    return ::RegisterClassEx(&wincl);
}

HWND CMainFrame::Create(
    DWORD dwStyle,
    int x,
    int y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam)
{
    //HRESULT hr = CreateDeviceIndependentResources();
    //if (!SUCCEEDED(hr))
    //    return NULL;

    if (!RegisterClassEx(hInstance))
        return NULL;

    m_hWnd = CreateWindowEx(
        WS_EX_APPWINDOW |
        WS_EX_COMPOSITED |
        WS_EX_OVERLAPPEDWINDOW, /* Extended possibilites for variation */
        szClassName,            /* Classname */
        TEXT("SFE"),            /* Title Text */
        dwStyle |
        WS_OVERLAPPEDWINDOW,    /* default window */
        x,                      /* Windows decides the position */
        y,                      /* where the window ends up on the screen */
        nWidth,                 /* The programs width */
        nHeight,                /* and height in pixels */
        HWND_DESKTOP,           /* The window is a child-window to desktop */
        NULL,                   /* No menu */
        hInstance,              /* Program Instance handler */
        lpParam                 /* Window Creation data */
    );

    if (!m_hWnd)
        return NULL;


    return m_hWnd;
}

int  CMainFrame::MessageLoop()
{
    MSG  msg;

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);/* Translate virtual-key messages into character messages */
        DispatchMessage(&msg); /* Send message to WindowProcedure */
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return (int)msg.wParam;
}

BOOL CMainFrame::ShowWindow(int nCmdShow)
{
    return ::ShowWindow(m_hWnd, nCmdShow);
}

BOOL CMainFrame::UpdateWindow()
{
    return ::UpdateWindow(m_hWnd);
}

float CMainFrame::GetScale(HWND hWnd)
{
    HDC hDC = ::GetDC(hWnd);
    INT xdpi = ::GetDeviceCaps(hDC, LOGPIXELSX);
    ::ReleaseDC(hWnd, hDC);
    float f = xdpi / 96.0f;
    //TCHAR str[80];
    //swprintf_s(str, L"SFE - scale: %.3f", f);
    //::SetWindowText(hWnd, str);
    return f;
}

int CMainFrame::OnCreate(HWND hWnd, LPCREATESTRUCT lpCS)
{
    float fScale = GetScale(hWnd);
    nWnd_Width = (int)(lpCS->cx * fScale);
    nWnd_Height = (int)(lpCS->cy * fScale);
    CMainFrame* pThis = (CMainFrame*)lpCS->lpCreateParams;

    lpCS->cx = nWnd_Width;
    lpCS->cy = nWnd_Height;

    //const BYTE buttonStyles = BTNS_AUTOSIZE;
    //int buttonSize = 24;

    //hWndToolbar = CreateWindowEx(0,
    //    TOOLBARCLASSNAME, NULL,
    //    //WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | WS_CHILD | CCS_NODIVIDER,
    //    WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_DLGFRAME |
    //    TBSTYLE_FLAT | TBSTYLE_TOOLTIPS,
    //    0, 0, 0, 0,
    //    hWnd, NULL, lpCS->hInstance, NULL);

    //if (hWndToolbar == NULL)
    //    return 0;

    //g_hImageList = ImageList_Create(buttonSize, buttonSize,   // Dimensions of individual bitmaps.
    //    ILC_COLOR32 | ILC_MASK,   // Ensures transparent background.
    //    3, 0);

    //::SendMessage(hWndToolbar, TB_SETIMAGELIST,
    //    (WPARAM)0,
    //    (LPARAM)g_hImageList);

    //::SendMessage(hWndToolbar, TB_SETMAXTEXTROWS, 0, 0);

    //::SendMessage(hWndToolbar, TB_LOADIMAGES,
    //    (WPARAM)IDB_STD_LARGE_COLOR,
    //    (LPARAM)HINST_COMMCTRL);

    //// Initialize button info.
    //// IDM_NEW, IDM_OPEN, and IDM_SAVE are application-defined command constants.

    //TBBUTTON tbButtons[3] =
    //{
    //    { MAKELONG(STD_FILENEW,  0), ID_FILE_NEW,  TBSTATE_ENABLED, buttonStyles, {0}, 0, (INT_PTR)L"New" },
    //    { MAKELONG(STD_FILEOPEN, 0), ID_FILE_OPEN, TBSTATE_ENABLED, buttonStyles, {0}, 0, (INT_PTR)L"Open"},
    //    { MAKELONG(STD_FILESAVE, 0), ID_FILE_SAVE, 0,               buttonStyles, {0}, 0, (INT_PTR)L"Save"}
    //};

    //// Add buttons.
    //::SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    //::SendMessage(hWndToolbar, TB_ADDBUTTONS, (WPARAM)3, (LPARAM)&tbButtons);

    //// Resize the toolbar, and then show it.
    //::SendMessage(hWndToolbar, TB_AUTOSIZE, 0, 0);
    //::ShowWindow(hWndToolbar, TRUE);

    pThis->m_wndMainView.m_hWnd = pThis->m_wndMainView.Create(hWnd, lpCS->hInstance, NULL);
    ::ShowWindow(pThis->m_wndMainView.m_hWnd, SW_SHOW);

    REGISTRY regs;
    if (ReadRegistry(regs))
    {
        pThis->m_wndMainView.m_wndSplitter.m_split_type = (CSplitterWnd::SPLIT_TYPE)regs.nSplitType;
        pThis->m_wndMainView.m_wndSplitter.m_fRatio = regs.fSplitRatio;
        pThis->m_wndMainView.SetFolderPositions(regs.nFolderPosition);
        pThis->m_wndMainView.SetActiveFrame(regs.nActiveFrame);
        pThis->m_wndMainView.SetFolderPaneWidth(regs.nFolderPaneWidth, TRUE);
        pThis->m_wndPossition = regs.rect;
        pThis->m_bMaximized = regs.bMaximaized;
        pThis->m_wSnap = regs.wSnap;
    }

    return 1;
}

BOOL CMainFrame::CenterWindow()
{
    int nWidth = m_wndPossition.right - m_wndPossition.left;
    int nHeight = m_wndPossition.bottom - m_wndPossition.top;

    if (nWidth > 0 &&
        nHeight > 0)
    {
        MoveWindow(
            m_hWnd,
            m_wndPossition.left,
            m_wndPossition.top,
            nWidth,
            nHeight,
            FALSE);
    }
    else
    {
        RECT rcWnd;

        HMONITOR hMonitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);

        MONITORINFO mi;
        mi.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(hMonitor, &mi);

        GetWindowRect(m_hWnd, &rcWnd);

        int nWndW = rcWnd.right - rcWnd.left;
        int nWndH = rcWnd.bottom - rcWnd.top;

        int nScrW = mi.rcWork.right - mi.rcWork.left;
        int nScrH = mi.rcWork.bottom - mi.rcWork.top;

        int nWndX = (nScrW - nWndW) / 2;
        int nWndY = (nScrH - nWndH) / 2;

        // make sure that the dialog box never moves outside of the screen
        if (nWndX < 0) nWndX = 0;
        if (nWndY < 0) nWndY = 0;
        if (nWndX + nWndW > nScrW) nWndW = nScrW - nWndX;
        if (nWndY + nWndH > nScrH) nWndH = nScrH - nWndY;

        MoveWindow(m_hWnd, nWndX, nWndY, nWndW, nWndH, FALSE);
    }

    return TRUE;
}

int CMainFrame::OnCommand(HWND hWnd, WPARAM wp, LPARAM lp)
{
    WORD id = LOWORD(wp);
    //WORD form = HIWORD(wp);

    switch (id)
    {
        case ID_FILE_NEW:
        {
            return 0;
        }
        case ID_FILE_CLOSE:
        {
            PostQuitMessage(0);
            return 0;
        }
    }
    return 0;
}

void CMainFrame::OnSize(UINT nType, UINT nWidth, UINT nHeight)
{
    if (nType == SIZE_MINIMIZED)
        return;

    CRect rect;
    GetClientRect(m_hWnd, &rect);

   // RECT rToolBar;
    //GetWindowRect(hWndToolbar, &rToolBar);
    int nH = 0;// rToolBar.bottom - rToolBar.top;
    MoveWindow(hWndToolbar, 0, 0, rect.right - rect.left, nHeight, TRUE);

    MoveWindow(m_wndMainView.m_hWnd, rect.left, rect.top + nH, rect.right - rect.left, rect.bottom - rect.top, TRUE);

    wchar_t text_buffer[1024] = { 0 };
    swprintf(text_buffer, _countof(text_buffer), L"Toolbar H: %d\n", nH);
    OutputDebugString(text_buffer);


    InvalidateRect(m_hWnd, NULL, TRUE);
}

BOOL CMainFrame::ReadRegistry(REGISTRY& regs)
{
    LPCTSTR sk = TEXT("SOFTWARE\\SFE");
    ULONG sz = 0;

    regs.nSplitType = 1;
    ReadRegValue(L"Splitter Type", (void*)&regs.nSplitType, sizeof(regs.nSplitType));

    regs.fSplitRatio = 0.5f;
    ReadRegValue(L"Splitter Ratio", (void*)&regs.fSplitRatio, sizeof(regs.fSplitRatio));

    regs.rect = { 0, 0, 0, 0 };
    ReadRegValue(L"Window possition", (void*)&regs.rect, sizeof(regs.rect));

    regs.bMaximaized = FALSE;
    ReadRegValue(L"Window maximized", (void*)&regs.bMaximaized, sizeof(regs.bMaximaized));

    regs.wSnap = SNAP_NONE;
    ReadRegValue(L"Window snap", (void*)&regs.wSnap, sizeof(regs.wSnap));

    regs.nFolderPosition[0] = 1;
    regs.nFolderPosition[1] = 1;
    ReadRegValue(L"Folder position", (void*)&regs.nFolderPosition, sizeof(regs.nFolderPosition));

    regs.nFolderPaneWidth[0] = 300;
    regs.nFolderPaneWidth[1] = 300;
    ReadRegValue(L"Folder pane width", (void*)&regs.nFolderPaneWidth, sizeof(regs.nFolderPaneWidth));

    regs.nActiveFrame = 0;
    ReadRegValue(L"Active frame", (void*)&regs.nActiveFrame, sizeof(regs.nActiveFrame));

    //reg.Close();
    return TRUE;
}

BOOL CMainFrame::WriteRegistry(REGISTRY& regs)
{
    WriteRegValue(L"Splitter Type", (void*)&regs.nSplitType, sizeof(regs.nSplitType));
    WriteRegValue(L"Splitter Ratio", (void*)&regs.fSplitRatio, sizeof(regs.fSplitRatio));
    WriteRegValue(L"Window possition", (void*)&regs.rect, sizeof(regs.rect));
    WriteRegValue(L"Window maximized", (void*)&regs.bMaximaized, sizeof(regs.bMaximaized));
    WriteRegValue(L"Window snap", (void*)&regs.wSnap, sizeof(regs.wSnap));
    WriteRegValue(L"Folder position", (void*)&regs.nFolderPosition, sizeof(regs.nFolderPosition));
    WriteRegValue(L"Folder pane width", (void*)&regs.nFolderPaneWidth, sizeof(regs.nFolderPaneWidth));
    WriteRegValue(L"Active frame", (void*)&regs.nActiveFrame, sizeof(regs.nActiveFrame));

    return TRUE;
}

void CMainFrame::OnDestroy()
{
    OleUninitialize();
}

void CMainFrame::SnapWindow()
{
    if (m_wSnap == SNAP_NONE)
        return;

    if (m_wSnap & SNAP_LEFT)
    {
        INPUT inputs[6] = {};
        ZeroMemory(inputs, sizeof(inputs));

        inputs[0].type = INPUT_KEYBOARD;
        inputs[0].ki.wVk = VK_LWIN;

        inputs[1].type = INPUT_KEYBOARD;
        inputs[1].ki.wVk = VK_LEFT;

        inputs[2].type = INPUT_KEYBOARD;
        inputs[2].ki.wVk = VK_LEFT;
        inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

        inputs[3].type = INPUT_KEYBOARD;
        inputs[3].ki.wVk = VK_LWIN;
        inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

        inputs[4].type = INPUT_KEYBOARD;
        inputs[4].ki.wVk = VK_ESCAPE;

        inputs[5].type = INPUT_KEYBOARD;
        inputs[5].ki.wVk = VK_ESCAPE;
        inputs[5].ki.dwFlags = KEYEVENTF_KEYUP;

        UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
        return;
    }
    if (m_wSnap & SNAP_RIGHT)
    {
        INPUT inputs[6] = {};
        ZeroMemory(inputs, sizeof(inputs));

        inputs[0].type = INPUT_KEYBOARD;
        inputs[0].ki.wVk = VK_LWIN;

        inputs[1].type = INPUT_KEYBOARD;
        inputs[1].ki.wVk = VK_RIGHT;

        inputs[2].type = INPUT_KEYBOARD;
        inputs[2].ki.wVk = VK_RIGHT;
        inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

        inputs[3].type = INPUT_KEYBOARD;
        inputs[3].ki.wVk = VK_LWIN;
        inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

        inputs[4].type = INPUT_KEYBOARD;
        inputs[4].ki.wVk = VK_ESCAPE;

        inputs[5].type = INPUT_KEYBOARD;
        inputs[5].ki.wVk = VK_ESCAPE;
        inputs[5].ki.dwFlags = KEYEVENTF_KEYUP;

        SetCapture(m_hWnd);
        UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
        ReleaseCapture();
        return;
    }

}

BOOL CMainFrame::WriteRegValue(LPCTSTR pszValueName, const void* pData, ULONG nBytes, HKEY hKey)
{
    LPCTSTR sk = TEXT("SOFTWARE\\SFE");
    CRegKey reg;

    if (!hKey)
    {
        if (reg.Open(HKEY_CURRENT_USER, sk) != ERROR_SUCCESS)
            return FALSE;
    }
    else
    {
        reg.m_hKey = hKey;
    }

    reg.SetBinaryValue(pszValueName, pData, nBytes);

    if (!hKey)
        reg.Close();

    return TRUE;
}

BOOL CMainFrame::ReadRegValue(LPCTSTR pszValueName, void* pData, ULONG nBytes, HKEY hKey)
{
    LPCTSTR sk = TEXT("SOFTWARE\\SFE");
    CRegKey reg;

    if (!hKey)
    {
        if (reg.Open(HKEY_CURRENT_USER, TEXT("SOFTWARE")) != ERROR_SUCCESS)
            return FALSE;
        if (reg.Open(HKEY_CURRENT_USER, TEXT("SFE")) != ERROR_SUCCESS)
        {
            reg.Create(reg.m_hKey, TEXT("SFE"));
        }
    }
    else
    {
        reg.m_hKey = hKey;
    }

    if (reg.QueryBinaryValue(pszValueName, (void*)pData, &nBytes) != ERROR_SUCCESS)
    {
        WriteRegValue(pszValueName, (void*)pData, nBytes);
        if (!hKey)
            reg.Close();
        return FALSE;
    }

    if (!hKey)
        reg.Close();

    return TRUE;
}

void CMainFrame::UpdateSettings()
{
    REGISTRY regs;
    regs.nSplitType = m_wndMainView.m_wndSplitter.m_split_type;
    regs.fSplitRatio = m_wndMainView.m_wndSplitter.m_fRatio;
    regs.nActiveFrame = m_wndMainView.GetActiveFrame();

    m_wndMainView.GetFolderPositions(
        regs.nFolderPosition[0],
        regs.nFolderPosition[1]);
    m_wndMainView.GetFolderPaneWidth(
        regs.nFolderPaneWidth[0],
        regs.nFolderPaneWidth[1]);

    RECT rect;
    ::GetWindowRect(m_hWnd, &rect);

    WINDOWPLACEMENT wp{};
    wp.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(m_hWnd, &wp);
    regs.rect = wp.rcNormalPosition;

    if (wp.flags & WPF_RESTORETOMAXIMIZED)
        regs.bMaximaized = TRUE;
    else
        regs.bMaximaized = FALSE;

    WORD wSnap = SNAP_NONE;
    if (rect.left != wp.rcNormalPosition.left ||
        rect.top != wp.rcNormalPosition.top ||
        rect.right != wp.rcNormalPosition.right ||
        rect.bottom != wp.rcNormalPosition.bottom)
    {
        // Probably snapped
        HMONITOR hMonitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);

        MONITORINFO mi{};
        mi.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(hMonitor, &mi);

        if (rect.left <= mi.rcWork.left)
        {
            wSnap = SNAP_LEFT;
            if (rect.top <= mi.rcWork.top && rect.bottom < mi.rcWork.bottom)
            {
                // On the right top
                wSnap |= SNAP_TOP;
            }
            else if (rect.top > mi.rcWork.top && rect.bottom >= mi.rcWork.bottom)
            {
                // On the right bottom
                wSnap |= SNAP_BOTTOM;
            }
            if (rect.right >= mi.rcWork.right)
            {
                wSnap &= ~SNAP_LEFT;
            }
        }
        else if (rect.top <= mi.rcWork.top)
        {
            wSnap = SNAP_TOP;
            if (rect.left <= mi.rcWork.left && rect.right < mi.rcWork.right)
            {
                // On the right top
                wSnap |= SNAP_LEFT;
            }
            else if (rect.left > mi.rcWork.left && rect.right >= mi.rcWork.right)
            {
                // On the right bottom
                wSnap |= SNAP_RIGHT;
            }
            if (rect.bottom >= mi.rcWork.bottom)
            {
                wSnap &= ~SNAP_TOP;
            }
        }
        else if (rect.right >= mi.rcWork.right)
        {
            wSnap = SNAP_RIGHT;
            if (rect.top <= mi.rcWork.top && rect.bottom < mi.rcWork.bottom)
            {
                // On the right top
                wSnap |= SNAP_TOP;
            }
            else if (rect.top > mi.rcWork.top && rect.bottom >= mi.rcWork.bottom)
            {
                // On the right bottom
                wSnap |= SNAP_BOTTOM;
            }
            if (rect.left <= mi.rcWork.left)
            {
                wSnap &= ~SNAP_LEFT;
            }
        }
        else if (rect.bottom >= mi.rcWork.bottom)
        {
            wSnap = SNAP_BOTTOM;
            if (rect.left <= mi.rcWork.left && rect.right < mi.rcWork.right)
            {
                // On the right top
                wSnap |= SNAP_LEFT;
            }
            else if (rect.left > mi.rcWork.left && rect.right >= mi.rcWork.right)
            {
                // On the right bottom
                wSnap |= SNAP_RIGHT;
            }
            if (rect.top <= mi.rcWork.top)
            {
                wSnap &= ~SNAP_BOTTOM;
            }
        }
    }
    regs.wSnap = wSnap;

    WriteRegistry(regs);
}
