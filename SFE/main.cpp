
//#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <iostream> 

#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include <winuser.h>
#include "mainframe.h"
#include "globals.h"

int WINAPI wWinMain(
    _In_     HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_     LPWSTR    lpCmdLine,
    _In_     int       nCmdShow)
{

    // Initialise common controls.
    INITCOMMONCONTROLSEX icc{};
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_WIN95_CLASSES | ICC_COOL_CLASSES | ICC_TREEVIEW_CLASSES;
    InitCommonControlsEx(&icc);

    // This program requires COM
    HRESULT hr = OleInitialize(0);

    // Set DPI awareness
    //SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    /* Register the window class, and if it fails quit the program */
    CMainFrame mf;
    mf.Create(
            0,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            800,
            600,
            HWND_DESKTOP,
            NULL,
            hInstance,
            (LPVOID)&mf);

    mf.CenterWindow();
    if (mf.m_bMaximized)
        mf.ShowWindow(SW_SHOWMAXIMIZED);
    else
        mf.ShowWindow(nCmdShow);
    mf.SnapWindow(); // Dock program to screen edges
    //mf.UpdateWindow();

    int iRes =  mf.MessageLoop();

    // Shutdown COM
    OleUninitialize();

    //_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    _CrtDumpMemoryLeaks();

    return iRes;
}
