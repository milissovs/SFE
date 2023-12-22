#ifndef GLOBALS_H_INCLUDED
#define GLOBALS_H_INCLUDED

#include <windows.h>
#include <vector>
#include <string>

typedef std::vector<std::wstring> str_vector;

template<class Interface>
inline void SafeRelease(Interface** ppInterfaceToRelease)
{
    if (*ppInterfaceToRelease != NULL)
    {
        (*ppInterfaceToRelease)->Release();
        (*ppInterfaceToRelease) = NULL;
    }
}

static void PASCAL SendMessageToDescendants(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, BOOL bDeep)
{
	for (HWND hWndChild = ::GetTopWindow(hWnd); hWndChild != NULL; hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
	{
		// send message with Windows SendMessage API
		::SendMessage(hWndChild, message, wParam, lParam);

		if (bDeep && ::GetTopWindow(hWndChild) != NULL)
		{
			// send to child windows after parent
			SendMessageToDescendants(hWndChild, message, wParam, lParam, bDeep);
		}
	}
}

static void PASCAL PostMessageToDescendants(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, BOOL bDeep)
{
	for (HWND hWndChild = ::GetTopWindow(hWnd); hWndChild != NULL; hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
	{
		// send message with Windows SendMessage API
		::PostMessage(hWndChild, message, wParam, lParam);

		if (bDeep && ::GetTopWindow(hWndChild) != NULL)
		{
			// send to child windows after parent
			PostMessageToDescendants(hWndChild, message, wParam, lParam, bDeep);
		}
	}
}

#define TEST_PATH L"\\\\server3\\Public\\PROJECTS DOCUMENTS\\Table reports\\!!! Unicredit 2023\\Individuals W1 2023\\!Reports\\01_IT\\AdNow"

#define TO_SYSTEM_TRAY_ON_CLOSE 0

#define FOLDER_POSITION_LEFT 0
#define FOLDER_POSITION_RIGHT 1

#define WM_SPLITTER          WM_USER + 1
#define WM_FRAME_ACTIVE      WM_USER + 2
#define WM_FOLDER_RESIZE     WM_USER + 3
#define WM_PANE_FOLDER       WM_USER + 4
#define WM_UPDATE_SETTINGS   WM_USER + 5
#define WM_DDD_FOLDERS       WM_USER + 6
#define WM_PROP_FOLDERS      WM_USER + 7
#define WM_TRAYMESSAGE       WM_USER + 8

#define WM_PANE_FOLDER_SET_POSITION 0
#define WM_PANE_FOLDER_GET_POSITION 1
#define WM_PANE_FOLDER_LIST_FOLDERS 2

#define WM_PANE_FOLDER_LIST_FOLDERS_GET_SELECTED 0
#define WM_PANE_FOLDER_LIST_FOLDERS_GET_HWND     1
#define WM_PANE_FOLDER_LIST_FOLDERS_GET_CRUMB    2

#define WM_MDIFRAME         WM_USER + 10
#define WP_MDIFRAME_GET_CRUMB_HWND     0
#define WP_MDIFRAME_SET_FOLDER_PATH    1

#define WM_CRUMBBAR          WM_USER + 9
#define WP_CRUMBBAR_SET_MODE           0
#define WP_CRUMBBAR_SET_PATH           1

#define LP_CRUMBBAR_SET_MODE_CRUMB     0
#define LP_CRUMBBAR_SET_MODE_EDIT      1

#ifndef _DPI_AWARENESS_CONTEXTS_
#define _DPI_AWARENESS_CONTEXTS_

DECLARE_HANDLE(DPI_AWARENESS_CONTEXT);

typedef enum DPI_AWARENESS {
    DPI_AWARENESS_INVALID = -1,
    DPI_AWARENESS_UNAWARE = 0,
    DPI_AWARENESS_SYSTEM_AWARE = 1,
    DPI_AWARENESS_PER_MONITOR_AWARE = 2
} DPI_AWARENESS;

#define DPI_AWARENESS_CONTEXT_UNAWARE               ((DPI_AWARENESS_CONTEXT)-1)
#define DPI_AWARENESS_CONTEXT_SYSTEM_AWARE          ((DPI_AWARENESS_CONTEXT)-2)
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE     ((DPI_AWARENESS_CONTEXT)-3)
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2  ((DPI_AWARENESS_CONTEXT)-4)
#define DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED     ((DPI_AWARENESS_CONTEXT)-5)

typedef enum DPI_HOSTING_BEHAVIOR {
    DPI_HOSTING_BEHAVIOR_INVALID = -1,
    DPI_HOSTING_BEHAVIOR_DEFAULT = 0,
    DPI_HOSTING_BEHAVIOR_MIXED = 1
} DPI_HOSTING_BEHAVIOR;

extern "C" WINUSERAPI BOOL WINAPI SetProcessDpiAwarenessContext(_In_ DPI_AWARENESS_CONTEXT value);

typedef enum MONITOR_DPI_TYPE {
    MDT_EFFECTIVE_DPI = 0,
    MDT_ANGULAR_DPI = 1,
    MDT_RAW_DPI = 2,
    MDT_DEFAULT
} MONITOR_DPI_TYPE;
#endif

#endif // GLOBALS_H_INCLUDED
