#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <string>
#include "listfolders.h"
#include "globals.h"
#include "resource.h"
//#include "shlwapi.h"
#include "colorpick.h"

#define _AFXWIN_INLINE

//#define IDM_CONTEXT_RECTAN 1005
//#define IDM_CONTEXT_CIRCLE 1006

struct TaskDialogData
{
    int X;
    int Y;
};

static HRESULT CALLBACK TaskDialogCallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LONG_PTR lpRefData)
{
    if (msg == TDN_DIALOG_CONSTRUCTED) // or TDN_CREATED, either one works
    {
        TaskDialogData* data = (TaskDialogData*)lpRefData;
        //SetWindowPos(hwnd, NULL, data->X, data->Y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
    return S_OK;
}

int MessageBoxPos(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType, int X, int Y)
{
    TaskDialogData data = {X, Y};

    TASKDIALOGCONFIG config = {};
    config.cbSize = sizeof(config);
    config.hwndParent = hWnd;
    config.pszWindowTitle = lpCaption;
    config.pszContent = lpText;
    // configure other settings as desired, based on uType...
    config.dwFlags = TDF_POSITION_RELATIVE_TO_WINDOW | TDF_ENABLE_HYPERLINKS;
    config.pszMainIcon = (PCWSTR)IDI_QUESTION;
    config.dwCommonButtons = TDCBF_YES_BUTTON | TDCBF_NO_BUTTON;
    config.pfCallback = &TaskDialogCallback;
    config.lpCallbackData = (LONG_PTR)&data;
    config.nDefaultButton = IDYES;

    int button = 0;
    TaskDialogIndirect(&config, &button, NULL, NULL);
    return button;
}

HGLOBAL CListFolders::CopyItem(LONG nItem)
{
    if (nItem < 0 || nItem >= m_root.children.size())
        return NULL;

    CFolderItem* fi = m_root.children.at(nItem);
    fi->move_index = nItem;
    SIZE_T size = sizeof(fi);
    HGLOBAL hMem = GlobalAlloc(GHND, size);

    if (hMem != NULL)
    {
        CFolderItem* pfi = (CFolderItem*)GlobalLock(hMem);
        *pfi = *fi;
        GlobalUnlock(hMem);
    }

    return hMem;
};

TCHAR m_szListFolderClassName[] = TEXT("LISTFOLDER");

COLORREF g_back;
COLORREF g_text;

LRESULT CALLBACK BtnBackgroundProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
        case WM_ERASEBKGND:
        {
            HDC hDC = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            HBRUSH hBrush = CreateSolidBrush(g_back);
            FillRect(hDC, &rc, hBrush);

            return TRUE;
        }

        case WM_PAINT:
        {
            RECT rc;
            GetClientRect(hwnd, &rc);

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            LOGFONT lf{};
            memset(&lf, 0, sizeof(LOGFONT));
            lf.lfHeight = -MulDiv(20, GetDeviceCaps(hdc, LOGPIXELSY), 72);

            HFONT font = CreateFontIndirect(&lf);
            HFONT hFont = (HFONT)SelectObject(hdc, font);

            SetTextColor(hdc, g_text);
            SetBkMode(hdc, TRANSPARENT);

            RECT rdebug(rc);
            int height = DrawText(hdc, L"Sample Text", (int)wcslen(L"Sample Text"), &rdebug, DT_CENTER | DT_VCENTER | DT_CALCRECT);
            int th = rc.bottom - rc.top;
            rc.top = (int)( (th - height) / 2);
            rc.bottom = rc.top + height;

            DrawText(hdc, L"Sample Text", (int)wcslen(L"Sample Text"), &rc, DT_CENTER | DT_VCENTER);

            SelectObject(hdc, hFont);
            EndPaint(hwnd, &ps);
        }

        default:
            return DefWindowProc(hwnd, Message, wParam, lParam);
    }
    return 0;
}

LRESULT  CALLBACK PropertiesDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    static DLG_LRESULT_INIT * pdlg_res {NULL};

    switch (Message)
    {
        case WM_INITDIALOG:
        {
            pdlg_res = (DLG_LRESULT_INIT*)lParam;

            // Title
            HWND hEditTitle = GetDlgItem(hwnd, IDR_EDIT_TITLE);
            INT_PTR lStyle = GetWindowLongPtr(hEditTitle, GWL_STYLE);
            lStyle |= WS_BORDER;
            SetWindowLongPtr(hEditTitle, GWL_STYLE, lStyle);

            if (pdlg_res != NULL)
            {
                if (pdlg_res->sTitle.length() > 0)
                {
                    EnableWindow(hEditTitle, TRUE);
                    SetDlgItemText(hwnd, IDR_EDIT_TITLE, pdlg_res->sTitle.c_str());
                    CheckDlgButton(hwnd, IDR_CHECK_TITLE, BST_CHECKED);
                }
                else
                {
                    EnableWindow(hEditTitle, FALSE);
                    CheckDlgButton(hwnd, IDR_CHECK_TITLE, BST_UNCHECKED);
                }
            }

            if (pdlg_res != NULL)
            {
                g_back = pdlg_res->clrBack;
                g_text = pdlg_res->clrText;
            }

            // Background button
            HWND hBtnBckg = GetDlgItem(hwnd, IDR_SAMPLE);
            SetWindowLongPtr(hBtnBckg, GWLP_WNDPROC, (LONG_PTR)BtnBackgroundProc);

            if (pdlg_res != NULL)
            {
                RECT rc{};
                GetWindowRect(hwnd, &rc);
                int w = rc.right - rc.left;
                int h = rc.bottom - rc.top;
                int x = (int)((pdlg_res->rect.right - pdlg_res->rect.left - w) / 2.0f);
                int y = (int)((pdlg_res->rect.bottom - pdlg_res->rect.top - h) / 2.0f);

                //POINT tl{x, y};
                //ClientToScreen(hwnd, &tl);

                MoveWindow(hwnd, pdlg_res->rect.left + x, pdlg_res->rect.top + y, w, h, FALSE);
            }

            return (LRESULT)TRUE;
        }

        //case WM

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    TCHAR lpString[101] = {};
                    UINT iChars = GetDlgItemText(hwnd, IDR_EDIT_TITLE, lpString, 101);
                    pdlg_res->sTitle = lpString;

                    EndDialog(hwnd, IDOK);
                    break;
                }
                case IDCANCEL:
                {
                    EndDialog(hwnd, IDCANCEL);
                    break;
                }
                case IDR_CHECK_TITLE:
                {
                    HWND hEditTitle = GetDlgItem(hwnd, IDR_EDIT_TITLE);
                    if (IsDlgButtonChecked(hwnd, IDR_CHECK_TITLE) == BST_UNCHECKED)
                    {
                        EnableWindow(hEditTitle, FALSE);
                    }
                    else
                    {
                        EnableWindow(hEditTitle, TRUE);
                        SetFocus(hEditTitle);
                    }
                    break;
                }
                case IDR_COLOR_PICK_BG:
                {
                    COLORPICK p;
                    D2D1_COLOR_F c1 = {
                        GetRValue(g_back) / 255.0f,
                        GetGValue(g_back) / 255.0f,
                        GetBValue(g_back) / 255.0f,
                        1.0f};
                    HRESULT hr = (HRESULT)p.Show(hwnd, c1);
                    if (hr == S_OK)
                    {
                        pdlg_res->clrBack = RGB(
                            (int)(c1.r * 255),
                            (int)(c1.g * 255),
                            (int)(c1.b * 255));
                        g_back = pdlg_res->clrBack;
                        InvalidateRect(hwnd, NULL, NULL);
                    }
                    break;
                }
                case IDR_COLOR_PICK_TX:
                {
                    COLORPICK p;
                    D2D1_COLOR_F c1 = {
                        GetRValue(g_text) / 255.0f,
                        GetGValue(g_text) / 255.0f,
                        GetBValue(g_text) / 255.0f, 1.0f };
                    HRESULT hr = (HRESULT)p.Show(hwnd, c1);
                    if (hr == S_OK)
                    {
                        pdlg_res->clrText = RGB(
                            (int)(c1.r * 255),
                            (int)(c1.g * 255),
                            (int)(c1.b * 255));
                        g_text = pdlg_res->clrText;
                        InvalidateRect(hwnd, NULL, NULL);
                    }
                    break;
                }
            }
            break;
        }

        default:
            return FALSE;
    }
    return FALSE;
}

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
            m_hovered.part = -1;
            m_hovered.level = 0;
            m_hovered.handle_item = 0;
            m_hovered.parent_item = 0;
            m_bLButtonDown = FALSE;
            m_bDraging = FALSE;

            SendMessage(m_hWndTT, TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_TI);
            InvalidateRect(m_hWnd, NULL, NULL);
            break;
        }

        case WM_MOUSEHOVER:
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            OnMouseHover((UINT)wParam, pt);

            break;
        }

        case WM_RBUTTONDOWN:
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            OnRButtonDown((UINT)wParam, pt);
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
            WORD nPartHover = nHover.part;
            WORD nLevelHover = nHover.level;

            //// Not valid item - take last
            //if (nItemHover < 0)
            //{
            //    nItemHover = (LONG)m_root.children.size();
            //    nPartHover = 2;
            //}

            CFolderItem* pfi = (CFolderItem* )wParam;

            //// Insert item between others
            //if (nPartHover != 1)
            //{
            //    // Is same window - moved item has been removed already
            //    if (pfi->hWndParent == m_hWnd && pfi->move_index < nItemHover)
            //        nItemHover--;

            //    if (nPartHover == 0)
            //        InsertFolder(pfi, nItemHover);
            //    if (nPartHover == 2)
            //        InsertFolder(pfi, nItemHover + 1);
            //}

            //// Convert item to subitem
            //else
            //{

            //}

            InvalidateRect(m_hWnd, NULL, FALSE);

            break;
        }

        case LVM_DELETEITEM:
        {
            RemoveFolder(wParam);
            break;
        }

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDM_FOLDER_PROPERTIES: // Show properties for folder item
                {
                    OnFolderProperties(hWnd);
                    m_hWndProperties = NULL;
                    break;
                }
                case IDM_FOLDER_ADD:
                {
                    break;
                }
                case IDM_FOLDER_REMOVE:
                {
                    OnFolderRemove(hWnd);
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

CListFolders::CListFolders()
    : m_hWnd(NULL)
    , m_hWndTT(NULL)
    , m_hWndProperties(NULL)
    , m_hInstance(NULL)
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
    , m_plgbSelected(NULL)
    , m_spInlineObjec_TFL(NULL)
    , m_spInlineObjec_TFLB(NULL)
    , m_fScale(1.0f)
    , m_nPosition(1)
    //, m_items()
    , m_root()
    , m_nVScrollMax(0)
    , m_bMouseTracking(FALSE)
    , m_pointLbuttonDown()
    , pDropTarget(NULL)
    , m_hovered()
    , m_TI()
    , m_dlg_init()
    , m_selected()
    , m_colors_back()
    , m_colors_text()
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
    SafeRelease(&m_plgbSelected);
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
        m_hWndTT = CreateTrackingToolTip(1, m_hWnd, hInstance);
    }

    m_hInstance = hInstance;

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
    SafeRelease(&m_plgbSelected);

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
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pBr2);
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

        gpt2[0].color = D2D1::ColorF(RGB(21, 21, 255));
        gpt2[1].color = D2D1::ColorF(RGB(0, 0, 96));
        m_pRenderTarget->CreateGradientStopCollection(&gpt2[0], ARRAYSIZE(gpt2), &gsc2);
        if (gsc2 != NULL)
            m_pRenderTarget->CreateLinearGradientBrush(lgbp, gsc2, &m_plgbHover);

        gpt2[0].color = D2D1::ColorF(RGB(255, 255, 21));
        gpt2[1].color = D2D1::ColorF(RGB(127, 96, 0));
        m_pRenderTarget->CreateGradientStopCollection(&gpt2[0], ARRAYSIZE(gpt2), &gsc2);
        if (gsc2 != NULL)
            m_pRenderTarget->CreateLinearGradientBrush(lgbp, gsc2, &m_plgbSelected);

        //----------------------------------------------------------------
        // Scale
        float dpiX{ 96.0f };
        float dpiY{ 96.0f };
        if (m_pRenderTarget != NULL)
            m_pRenderTarget->GetDpi(&dpiX, &dpiY);
        m_fScale = dpiX / 96.0f;

        //----------------------------------------------------------------
        // Add default color
        //AddColor(m_pRenderTarget, COLOR_DEFAULT);
        m_root.CreateDPColors(m_pRenderTarget);

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

        //ListView_SetColumnWidth(m_hWnd, 0, LVSCW_AUTOSIZE_USEHEADER);
    }
    if (m_root.children.size() > 0)
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

    HANDLE hTop = GetTopItem();

    static D2D1_POINT_2F ptMark0{};
    static D2D1_POINT_2F ptMark1{};

    while (hTop)
    {
        DrawItem(pRT, rect, rc, (CFolderItem*)hTop, ptMark0, ptMark1);
        hTop = ((CFolderItem*)hTop)->GetNextItem();
    }

    if (m_bDraging && m_hovered.part != HOVER_PART::HP_MIDDLE)
    {
        pRT->DrawLine(ptMark0, ptMark1, m_pBrRed, 2);
        m_bDraging = FALSE;
    }
}

void CListFolders::DrawItem(ID2D1HwndRenderTarget* pRT, D2D1_RECT_F& rect, D2D1_RECT_F& rc, CFolderItem* pfi, D2D1_POINT_2F& ptMark0, D2D1_POINT_2F& ptMark1)
{
    D2D1_POINT_2F pt0{};
    D2D1_POINT_2F pt1{};

    // Fill
    ID2D1LinearGradientBrush* pBrush{ NULL };
    if (pfi->bSelected)
        pBrush = pfi->draw_res.lgb_btn_pressed; //m_colors_pressed.at(fi->nColorIndex);
    else
        pBrush = pfi->draw_res.lgb_btn_normal; //m_colors_normal.at(fi->nColorIndex);

    pt0 = { 1000.0f, rc.top };
    pt1 = { 1000.0f, rc.bottom };
    pBrush->SetStartPoint(pt0);
    pBrush->SetEndPoint(pt1);

    // Background
    int nLevel = pfi->GetLevel();
    D2D1_RECT_F rcBorder{ rc };
    rcBorder.left += nLevel * 12;
    pRT->FillRectangle(&rcBorder, pBrush);

    // Border ----------------------------------------------------------------------
    m_plgbBorder->SetStartPoint(pt0);
    m_plgbBorder->SetEndPoint(pt1);
    pRT->DrawRectangle(&rcBorder, m_plgbBorder);

    //// Drag mark -------------------------------------------------------------------
    //if (m_bDraging && nItem == m_hovered.item && m_hovered.part != 1)
    //{
    //    if (m_hovered.part == 0)
    //    {
    //        ptMark0 = { rc.left, rc.top };
    //        ptMark1 = { rc.right, rc.top };
    //    }
    //    else
    //    {
    //        ptMark0 = { rc.left, rc.bottom };
    //        ptMark1 = { rc.right, rc.bottom };
    //    }

    //    if (ptMark0.y < 1.5f)
    //        ptMark0.y = 1.5f;
    //    if (ptMark1.y < 1.5f)
    //        ptMark1.y = 1.5f;
    //    if (ptMark0.y > rect.bottom - 1.5f)
    //        ptMark0.y = rect.bottom - 1.5f;
    //    if (ptMark1.y > rect.bottom - 1.5f)
    //        ptMark1.y = rect.bottom - 1.5f;
    //}

    // Left mark ----------------------------------------------------------------------
    DrawItem_LeftMark(pRT, pfi, rcBorder);

    // Right mark ----------------------------------------------------------------------
    DrawItem_RightMark(pRT, pfi, rcBorder);

    // Text ----------------------------------------------------------------------
    DrawItem_Text(pRT, pfi, rcBorder, rc);

    // -----------------------------------------------------------------------
    // Prepare for next item
    rc.top = rc.bottom;
    rc.bottom = rc.top + DEFAULT_ITEM_HEIGHT;

    // Children ----------------------------------------------------------------------
    if (pfi->HasChildren() && !pfi->bCollapsed)
    {
        for (size_t t = 0; t < pfi->children.size(); t++)
        {
            if (rc.top > rect.bottom)
                break;

            DrawItem(pRT, rect, rc, pfi->children.at(t), ptMark0, ptMark1);
        }
    }

}

void CListFolders::DrawItem_Text(ID2D1HwndRenderTarget* pRT, CFolderItem* fi, D2D1_RECT_F& rcBorder, D2D1_RECT_F& rc)
{
    D2D1_RECT_F rflock{ rcBorder };
    rflock.top += 4;
    rflock.bottom -= 4;
    rflock.left += 5;
    rflock.right = rflock.left + 10;

    D2D1_RECT_F rflockO{ rflock };
    rflockO.left += 0.5f;
    rflockO.right = rflockO.left + 8.0f;
    rflockO.top += 0.5f;
    rflockO.bottom += 0.5f;


    std::wstring wch{};

    //Collapsed mark
    if (fi->HasChildren())
    {
        if (fi->bCollapsed)
            wch = L"+";
        else
            wch = L"–"; //━
    }
    else
        wch += L" ";

    if (m_hovered.handle_item == fi && m_hovered.plus && fi->HasChildren())
    {
        pRT->DrawText(wch.c_str(), (UINT32)wch.length(), m_pTFL, rflockO, m_pBr1);
        pRT->DrawRectangle(rflockO, m_pBr1);
    }

    pRT->DrawText(wch.c_str(), (UINT32)wch.length(), m_pTFL, rflock, m_pBr1);

    //Locked mark
    rflock.left += 8;
    rflock.right = rflock.left + 10;
    if (fi->bLocked)
        wch = L"∗";
    else
        wch = L" ";

    pRT->DrawText(wch.c_str(), (UINT32)wch.length(), m_pTFL, rflock, m_pBr1);

    D2D1_RECT_F rft{ rc };
    rft.top += 1;
    rft.bottom -= 1;
    rft.left += 21;
    rft.right -= 2;

    if (fi->hParent != &m_root)
    {
        D2D1_POINT_2F pt0{ rc.left + 6, rc.top };
        D2D1_POINT_2F pt1{ rc.left + 6, rc.bottom };

        CFolderItem* pfi = (CFolderItem*)fi->hParent;
        if (pfi->IsLastChild(fi))
        {
            pt1.y -= (rc.bottom - rc.top) / 2;
            pRT->DrawLine(pt0, pt1, m_pBr1);

            pt0 = pt1;
            pt1.x = rc.left + 12;

            pRT->DrawLine(pt0, pt1, m_pBr1);
        }
        else
        {
            pRT->DrawLine(pt0, pt1, m_pBr1);

            pt0.y += (rc.bottom - rc.top) / 2;
            pt1 = pt0;
            pt1.x = rc.left + 12;
            pRT->DrawLine(pt0, pt1, m_pBr1);
        }
    }

    int nLevel = fi->GetLevel();
    rft.left += nLevel * 12;
    IDWriteTextFormat* pDWTF = m_pTFL;
    if (fi->bSelected)
    {
        rft.left += 1;
        pDWTF = m_pTFLB;
    }

    //ID2D1SolidColorBrush* pBrText = m_colors_brtext.at(fi->nColorIndex);

    if (fi->sTitle.length() > 0)
        pRT->DrawText(fi->sTitle.c_str(), (UINT32)fi->sTitle.length(), pDWTF, rft, fi->draw_res.scb_txt);
    else
        pRT->DrawText(fi->sPath.c_str(), (UINT32)fi->sPath.length(), pDWTF, rft, fi->draw_res.scb_txt);
}

void CListFolders::DrawItem_LeftMark(ID2D1HwndRenderTarget* pRT, CFolderItem* fi, D2D1_RECT_F& rcBorder)
{
    D2D1_RECT_F rfl{ rcBorder };
    rfl.left += 0.5f;
    rfl.right = rfl.left + 3.5f;
    rfl.bottom -= 1.0f;

    D2D1_POINT_2F pt0{ rfl.left, rfl.top };
    D2D1_POINT_2F pt1{ rfl.left + (rfl.right - rfl.left) / 2, rfl.bottom };

    m_plgbBorder->SetStartPoint(pt0);
    m_plgbBorder->SetEndPoint(pt1);
    m_plgbHover->SetStartPoint(pt0);
    m_plgbHover->SetEndPoint(pt1);
    m_plgbSelected->SetStartPoint(pt0);
    m_plgbSelected->SetEndPoint(pt1);

    if (m_bDraging)
    {
        //if (m_hovered.part == 1 && nItem == m_hovered.item)
        //    pRT->FillRectangle(&rfl, m_plgbHover);
        //else
        //    pRT->FillRectangle(&rfl, m_plgbBorder);
    }
    else
    {
        if (m_hovered.handle_item == fi->hHandle)
        {
            pRT->FillRectangle(&rfl, m_plgbHover);
            //wchar_t text_buffer[200] = { 0 }; //temporary buffer
            //swprintf(text_buffer, _countof(text_buffer), L"%I64x: Level: %d\n", (unsigned long long)m_hovered.handle_item, m_hovered.level); // convert
            //OutputDebugString(text_buffer);
        }
        else
        {
            if (fi->bSelected)
            {
                pRT->FillRectangle(&rfl, m_plgbSelected);
            }
            else
                pRT->FillRectangle(&rfl, m_plgbBorder);
        }
    }

}

void CListFolders::DrawItem_RightMark(ID2D1HwndRenderTarget* pRT, CFolderItem* fi, D2D1_RECT_F& rcBorder)
{
    D2D1_RECT_F rfr{ rcBorder };
    rfr.left = rcBorder.right - 4.0f;
    rfr.right = rfr.left + 4.0f;
    rfr.bottom -= 1.0f;

    D2D1_POINT_2F pt0{ rfr.left, rfr.top };
    D2D1_POINT_2F pt1{ rfr.left + (rfr.right - rfr.left) / 2, rfr.bottom };

    pt0 = { rfr.left, rfr.top };
    pt1 = { rfr.left + (rfr.right - rfr.left) / 2, rfr.bottom };

    m_plgbBorder->SetStartPoint(pt0);
    m_plgbBorder->SetEndPoint(pt1);
    m_plgbHover->SetStartPoint(pt0);
    m_plgbHover->SetEndPoint(pt1);
    m_plgbSelected->SetStartPoint(pt0);
    m_plgbSelected->SetEndPoint(pt1);

    if (m_bDraging)
    {
        //if (m_hovered.handle_item == fi->hHandle)
        //    pRT->FillRectangle(&rfr, m_plgbHover);
        //else
        //    pRT->FillRectangle(&rfr, m_plgbBorder);
    }
    else
    {
        //if (nItem == m_hovered.item && m_hovered.level == nLevel && m_hovered.handle_item == fi.hHandle)
        if (m_hovered.handle_item == fi->hHandle)
            pRT->FillRectangle(&rfr, m_plgbHover);
        else
        {
            if (fi->bSelected)
            {
                pRT->FillRectangle(&rfr, m_plgbSelected);
            }
            else
                pRT->FillRectangle(&rfr, m_plgbBorder);
        }
    }
}

HANDLE CListFolders::AddFolder(LPCTSTR lpszText, HANDLE hParent)
{
    CFolderItem* fi = new CFolderItem();

    if (hParent != NULL)
    {
        CFolderItem* pfi = (CFolderItem*)hParent;
        fi->hWndParent = m_hWnd;
        fi->sPath = lpszText;
        fi->move_index = -1;
        fi->nColorIndex = 0;
        fi->hHandle = fi;
        fi->hParent = hParent;
        fi->nLevel = pfi->nLevel + 1;
        if (pfi->children.end() != pfi->children.begin())
        {
            fi->hPrev = pfi->children.back();
            ((CFolderItem*)pfi->children.back())->hNext = (HANDLE)fi;
        }
        else
        {
            fi->hPrev = NULL;
            fi->hNext = NULL;
        }
        pfi->children.push_back(fi);
    }
    else
    {
        fi->hWndParent = m_hWnd;
        fi->sPath = lpszText;
        fi->move_index = -1;
        fi->nColorIndex = 0;
        fi->hHandle = fi;
        fi->hParent = &m_root;
        fi->nLevel = m_root.nLevel + 1;
        if (m_root.children.end() != m_root.children.begin())
        {
            fi->hPrev = m_root.children.back();
            ((CFolderItem*)m_root.children.back())->hNext = (HANDLE)fi;
        }
        else
        {
            fi->hPrev = NULL;
            fi->hNext = NULL;
        }
        m_root.children.push_back(fi);
    }

    ResetScrollBar();
    return fi->hHandle;
}

void CListFolders::InsertFolder(CFolderItem* pfi, size_t at)
{
    pfi->hWndParent = m_hWnd;
    m_root.children.insert(m_root.children.begin() + at, pfi);
    //pfi->lParent = (LONG_PTR)&m_items.at(at);
    ResetScrollBar();
    m_bDraging = FALSE;
}

void CListFolders::RemoveFolder(size_t at)
{
    if (at < 0)
        return;

    m_root.children.erase(m_root.children.begin() + at);
    ResetScrollBar();
    m_bDraging = FALSE;
}

long CListFolders::GetVirtualHeight()
{
    if (m_root.children.size() < 1)
        return 0;

    long lVirtualHeight = 0;
    size_t count = m_root.children.size();

    for (CFolderItem * fi : m_root.children)
    {
        if (!fi->bHiden)
        {
            lVirtualHeight += fi->GetItemHeight();
        }
    }

    //wchar_t text_buffer[200] = { 0 }; //temporary buffer
    //swprintf(text_buffer, _countof(text_buffer), L"VirtualHeight: %d\n", (unsigned long)lVirtualHeight); // convert
    //OutputDebugString(text_buffer);

    return lVirtualHeight;
}

HANDLE CListFolders::GetTopItem()
{
    if (m_root.children.size() < 1)
        return NULL;

    int nVertScroll = GetScrollPos32(SB_VERT);

    int nTop = 0;
    int nItem = 0;

    for (CFolderItem* pfi : m_root.children)
    {
        HANDLE hTop = pfi->GetTopItem(nTop, nVertScroll);
        if (hTop)
            return hTop;
    }

    return NULL;
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

    if (m_root.children.size() < 1)
        return;

    HDC screen = ::GetDC(0);
    int dpiX = GetDeviceCaps(screen, LOGPIXELSX);
    int dpiY = GetDeviceCaps(screen, LOGPIXELSY);
    ::ReleaseDC(0, screen);

    RECT rect{};
    GetClientRect(m_hWnd, &rect);
    float f = dpiX / 96.0f;

    int virtual_height = (int)(GetVirtualHeight());
    int visual_height = (int)((rect.bottom - rect.top) / f);

    int i = (int)floor((float)visual_height / (float)DEFAULT_ITEM_HEIGHT);

    int m_nVScrollMax = 0;
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

    SCROLLINFO si{};
    si.fMask = SIF_PAGE | SIF_RANGE | SIF_DISABLENOSCROLL;
    si.nPage = (m_nVScrollMax > 0) ? i * DEFAULT_ITEM_HEIGHT + 1: 0;
    si.nMin = 0;
    si.nMax = m_nVScrollMax;
    SetScrollInfo(m_hWnd, SB_VERT, &si, TRUE);
}

void CListFolders::OnVScroll(UINT nSBCode, UINT nPos, HWND hScroll)
{
    int scrollPos = GetScrollPos32(SB_VERT);

    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_POS | SIF_ALL;
    GetScrollInfo(m_hWnd, SB_VERT, &si);

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
            if (scrollPos < si.nMax)
            {
                int offset = ::MulDiv(rect.bottom - rect.top, 96, (int)(96 * m_fScale));
                int items = (int)(offset / DEFAULT_ITEM_HEIGHT);
                scrollPos = __min(si.nMax, scrollPos + items * DEFAULT_ITEM_HEIGHT);
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
    BOOL b = FALSE;
    m_pointLbuttonDown = point;

    m_selected = HitTest(point);

    CFolderItem* fi = (CFolderItem * )m_selected.handle_item;
    if (fi == NULL)
        return;
    
    if (m_selected.plus)
    {
        fi->bCollapsed = !fi->bCollapsed;
        b = TRUE;
    }

    fi->Select();

    if (b)
        ResetScrollBar();

    InvalidateRect(m_hWnd, NULL, TRUE);
}

void CListFolders::OnLButtonDblClk(UINT nFlags, POINT point)
{
    BOOL b = FALSE;
    SELECTED_ITEM nSelected = HitTest(point);

    CFolderItem* fi = (CFolderItem*)nSelected.handle_item;
    if (fi == NULL)
        return;

    if (fi->HasChildren())
    {
        fi->bCollapsed = !fi->bCollapsed;
        b = TRUE;
    }

    fi->Select();

    if (b)
        ResetScrollBar();

    InvalidateRect(m_hWnd, NULL, TRUE);
}

void CListFolders::OnRButtonDown(UINT nFlags, POINT point)
{
    m_selected = HitTest(point);

    CFolderItem* fi = (CFolderItem*)m_selected.handle_item;
    if (fi != NULL)
    {
        OnLButtonDown(nFlags, point);
    }

    POINT pt = point;
    HMENU hMenuPopup = CreatePopupMenu();
    DWORD dw = MF_STRING;
    if (fi == NULL)
    {
        dw |= MF_DISABLED | MF_GRAYED;
    }
    AppendMenu(hMenuPopup, dw, IDM_FOLDER_PROPERTIES, L"Properties ...");
    AppendMenu(hMenuPopup, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenuPopup, MF_STRING, IDM_FOLDER_ADD, L"Add");
    AppendMenu(hMenuPopup, dw, IDM_FOLDER_REMOVE, L"Remove!");
    AppendMenu(hMenuPopup, MF_SEPARATOR, 0, NULL);

    AppendMenu(hMenuPopup, MF_STRING, IDM_FOLDER_CREATE_GROUP, L"Create group ...");

    HBITMAP hIcon =  (HBITMAP)LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_SETTINGS), IMAGE_ICON, 16, 16, LR_LOADTRANSPARENT);

    //MENUITEMINFO mii = {};
    //mii.cbSize = sizeof(MENUITEMINFO);
    //mii.fMask = MIIM_BITMAP;
    ////mii.fType = MFT_BITMAP;
    //mii.hbmpChecked = hIcon;
    //mii.hbmpUnchecked = hIcon;
    //mii.hbmpItem = hIcon;

    //SetMenuItemInfo(hMenuPopup, 0, TRUE, &mii);
    BOOL b = SetMenuItemBitmaps(hMenuPopup, IDM_FOLDER_PROPERTIES, MF_BITMAP | MF_BYPOSITION, hIcon, hIcon);

    ClientToScreen(m_hWnd, &pt);
    TrackPopupMenu(hMenuPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_hWnd, NULL);
    DestroyMenu(hMenuPopup);
}

void CListFolders::OnMouseMove(UINT nFlags, POINT point)
{
    if (!m_bMouseTracking)
    {
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_LEAVE | TME_HOVER;
        tme.hwndTrack = m_hWnd;
        tme.dwHoverTime = 250; //HOVER_DEFAULT; 
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

    if (siHovered != m_hovered)
    {
        m_hovered = siHovered;
        InvalidateRect(m_hWnd, NULL, TRUE);
        SendMessage(m_hWndTT, TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_TI);

        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_LEAVE | TME_HOVER;
        tme.hwndTrack = m_hWnd;
        tme.dwHoverTime = 250; //HOVER_DEFAULT; 
        TrackMouseEvent(&tme);
    }
}

void CListFolders::OnMouseHover(UINT nFlags, POINT pt)
{
    if (m_hovered.handle_item == NULL)
        return;

    SendMessage(m_hWndTT, TTM_GETTOOLINFO, 0, (LPARAM)&m_TI);
    TCHAR txt[4096];
    m_TI.lpszText = txt;

    // Tooltip title
    std::wstring title{};
    ((CFolderItem*)m_hovered.handle_item)->GetTitle(title);
    SendMessage(m_hWndTT, TTM_SETTITLE, TTI_NONE, (LPARAM)title.c_str());

    // Tooltip text
    std::wstring tttext{};
    if (((CFolderItem*)m_hovered.handle_item)->HasChildren())
    {
        HANDLE hChild = ((CFolderItem*)m_hovered.handle_item)->IsThereSlectedChild();
        if (hChild == NULL)
        {
            hChild = ((CFolderItem*)m_hovered.handle_item)->children.at(0);
        }
        swprintf_s(txt, ARRAYSIZE(txt), L"%ls", ((CFolderItem*)hChild)->sPath.c_str());
    }
    else
    {
        swprintf_s(txt, ARRAYSIZE(txt), L"%ls", ((CFolderItem*)m_hovered.handle_item)->sPath.c_str());
    }
    m_TI.lpszText = txt;
    SendMessage(m_hWndTT, TTM_SETTOOLINFO, 0, (LPARAM)&m_TI);
    SendMessage(m_hWndTT, TTM_TRACKACTIVATE, TRUE, (LPARAM)&m_TI);

    //TCHAR theme[20];
    //swprintf_s(theme, ARRAYSIZE(theme), L"%ls", L"Explorer");
    //SendMessage(m_hWndTT, TTM_SETWINDOWTHEME, 0, (LPARAM)theme);

    LRESULT l = SendMessage(m_hWndTT, TTM_GETBUBBLESIZE, 0, (LPARAM)&m_TI);
    int h = HIWORD(l);
    int w = LOWORD(l);

    pt.x += (LONG)m_fScale * 2;
    pt.y += (DEFAULT_ITEM_HEIGHT)+2;

    ClientToScreen(m_hWnd, &pt);

    RECT parent_rect{};
    GetWindowRect(GetParent(m_hWnd), &parent_rect);
    if (SendMessage(GetParent(m_hWnd), WM_PANE_FOLDER, 1, 0) == 1)
    {
        if (pt.x + w > parent_rect.right)
            pt.x = parent_rect.right - w;
    }
    else
    {
        if (pt.x + w > parent_rect.right)
            pt.x = parent_rect.left;
    }
    SendMessage(m_hWndTT, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(pt.x, pt.y));
}

UINT CListFolders::AddColor(ID2D1HwndRenderTarget* pRT, COLORREF color, COLORREF tcolor, size_t index)
{
    if (!pRT)
        return NULL;

    if (color == COLOR_DEFAULT && tcolor == 0 && m_colors_normal.size() > 0) // Ако цвета е по подразбиране
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

    if (index == 0)
    {
        m_colors_normal.push_back(pBrush);
        m_colors_back.push_back(color);
    }
    else
    {
        m_colors_normal.at(index) = pBrush;
        m_colors_back.at(index) = color;
    }

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

    ID2D1SolidColorBrush* pBrText = NULL;

    m_pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(GetRValue(tcolor) / 255.0f,
                     GetGValue(tcolor) / 255.0f,
                     GetBValue(tcolor) / 255.0f), &pBrText);

    if (index == 0)
    {
        m_colors_pressed.push_back(pBrush);
        m_colors_text.push_back(tcolor);
        m_colors_brtext.push_back(pBrText);
    }
    else
    {
        m_colors_pressed.at(index) = pBrush;
        m_colors_text.at(index) = tcolor;
        m_colors_brtext.at(index) = pBrText;
    }

    // ------------------------------------------------------------------------------------------------------------
    return (UINT)m_colors_normal.size() - 1;
}

SELECTED_ITEM CListFolders::HitTest(POINT point)
{
    SELECTED_ITEM si{};

    D2D1_POINT_2F fpt =
    {
        (float)point.x / m_fScale,
        (float)point.y / m_fScale
    };
    float fOffset = 0.0f;

    HANDLE hTop = GetTopItem();
    while (hTop)
    {
        HANDLE hRes = HiTestItem((CFolderItem*)hTop, fOffset, fpt, si, 0);
        if (hRes)
            return si;

        hTop = ((CFolderItem*)hTop)->GetNextItem();
    }

    return si;
}

HANDLE CListFolders::HiTestItem(CFolderItem* fi, float& fOffset, D2D1_POINT_2F& fpt, SELECTED_ITEM& si, WORD nLevel)
{
    if (fpt.y >= fOffset && fpt.y <= fOffset + DEFAULT_ITEM_HEIGHT)
    {
        if (fpt.y >= fOffset && fpt.y <= fOffset + DEFAULT_ITEM_HEIGHT / 4.0f)
        {
            si.handle_item = fi->hHandle;
            si.parent_item = fi->hParent;
            si.level = nLevel;
            si.part = 0;
            return fi;
        }

        if (fpt.y >= fOffset + DEFAULT_ITEM_HEIGHT / 4.0f && fpt.y <= fOffset + DEFAULT_ITEM_HEIGHT * 3 / 4.0f)
        {
            si.handle_item = fi->hHandle;
            si.parent_item = fi->hParent;
            si.level = nLevel;
            si.part = 1;

            if (fpt.x > 4 && fpt.x < 12)
                si.plus = TRUE;
            else
                si.plus = FALSE;
            return fi;
        }

        if (fpt.y >= fOffset + DEFAULT_ITEM_HEIGHT * 3 / 4.0f && fpt.y <= fOffset + DEFAULT_ITEM_HEIGHT)
        {
            si.handle_item = fi->hHandle;
            si.parent_item = fi->hParent;
            si.level = nLevel;
            si.part = 2;
            return fi;
        }
    }

    fOffset += DEFAULT_ITEM_HEIGHT;

    if (fi->HasChildren() && !fi->bCollapsed)
    {
        for (size_t t = 0; t < fi->children.size(); t++)
        {
            HANDLE hItem = HiTestItem(fi->children.at(t), fOffset, fpt, si, nLevel + 1);
            if (hItem)
                return hItem;
        }
    }
    return NULL;
}

void CListFolders::SetSelectedItem(CFolderItem* fi)
{
    SELECTED_ITEM si = {};
    si.handle_item = fi->hHandle;
    si.parent_item = fi->hParent;
    si.level = fi->GetLevel();
    si.part = 1;
    m_selected = si;
    fi->Select();
    InvalidateRect(m_hWnd, NULL, TRUE);
}

void CListFolders::GetSelectedItems(std::vector<size_t>& selected)
{
    selected.clear();

    size_t i = 0;
    for (CFolderItem* fi : m_root.children)
    {
        if (fi->bSelected)
            selected.push_back(i);
        i++;
    }
}

HWND CListFolders::CreateTrackingToolTip(int toolID, HWND hWndParent, HINSTANCE hInst)
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

void CListFolders::OnFolderProperties(HWND hWnd)
{
    CFolderItem* fi = (CFolderItem*)m_selected.handle_item;

    DLG_LRESULT_INIT dlg_res{};
    dlg_res.hParent = hWnd;
    dlg_res.sTitle = fi->sTitle;
    UINT nColorIndex = fi->nColorIndex;
    dlg_res.clrBack = fi->clrs.btn; //m_colors_back.at(nColorIndex);
    dlg_res.clrText = fi->clrs.txt; // m_colors_text.at(nColorIndex);

    GetWindowRect(hWnd, &dlg_res.rect);

    INT_PTR ret = DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_DLG_FOLDER_PROPERTIES), hWnd, PropertiesDlgProc, (LPARAM)&dlg_res);
    if (ret == IDOK)
    {
        fi->sTitle = dlg_res.sTitle;
        if (dlg_res.clrBack != COLOR_DEFAULT || dlg_res.clrText != 0)
        {
            //fi->nColorIndex = AddColor(m_pRenderTarget, dlg_res.clrBack, dlg_res.clrText, fi->nColorIndex);
            fi->SetColors(dlg_res.clrText, dlg_res.clrBack);
            fi->CreateDPColors(m_pRenderTarget);
        }
        InvalidateRect(hWnd, NULL, NULL);
    }
}

void CListFolders::OnFolderRemove(HWND hWnd)
{
    CFolderItem* fi = (CFolderItem*)m_selected.handle_item;
    if (!fi)
        return;

    wchar_t text_buffer[2048] = { 0 }; //temporary buffer
    std::wstring sTitle = {};
    fi->GetTitle(sTitle);
    swprintf(text_buffer, _countof(text_buffer), L"Remove <A>%s</A>\" tab?", sTitle.c_str());

    if (MessageBoxPos(m_hWnd, text_buffer, L"Are You Sure?", MB_YESNO | MB_ICONQUESTION, 0, 0) != IDYES)
        return;

    //if (MessageBox(m_hWnd, text_buffer, L"Are You Sure?", MB_YESNO | MB_ICONQUESTION) != IDYES)
    //    return;

    fi->Remove();

    InvalidateRect(m_hWnd, NULL, TRUE);
}

HANDLE CListFolders::GetSelectedItem()
{
    HANDLE hSelectedItem = NULL;
    for (CFolderItem* pfi : m_root.children)
    {
        hSelectedItem = pfi->GetSelectedItem();
        if (hSelectedItem)
            return hSelectedItem;
    }
    return hSelectedItem;
}