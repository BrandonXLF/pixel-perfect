#include "windows.h"
#include "consts.h"
#include "action.h"

constexpr UINT SIZE_INPUT = 10;
constexpr UINT ZOOM_INPUT = 20;
constexpr UINT ESC_CHECKBOX = 30;
constexpr UINT RESET_BUTTON = 40;
constexpr WCHAR ACTION_CLASS_NAME[] = L"PixelPerfectActionWindow";

bool noChange = false;

void ResetAllValues(HWND hWnd) {
	noChange = true;
    SetDlgItemInt(hWnd, 10, PIXEL_COUNT, FALSE);
    SetDlgItemInt(hWnd, 20, PIXEL_SIZE, FALSE);
    CheckDlgButton(hWnd, 30, ESC_TO_EXIT);
	noChange = false;
}

void NumberInputProc(LPCWSTR name, int action, WPARAM wParam, DWORD *value, HWND hWnd) {
    switch (LOWORD(wParam) - action) {
        case 0: {
            if (noChange || HIWORD(wParam) != EN_CHANGE) return;

            BOOL lpTranslated;
            UINT val = GetDlgItemInt(hWnd, action, &lpTranslated, FALSE);
                
            if (lpTranslated) {
                *value = max(1, val);
            }

            break;
        }
        case 1:
            *value = max(1, *value - 1);
            SetDlgItemInt(hWnd, action, *value, FALSE);
            break;
        case 2:
            *value = *value + 1;
            SetDlgItemInt(hWnd, action, *value, FALSE);
            break;
    }

	RegSetKeyValue(HKEY_CURRENT_USER, REG_KEY, name, REG_DWORD, value, sizeof(DWORD));
}

LRESULT CALLBACK ActionWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    int a = LOWORD(wParam);
    int b = HIWORD(wParam);

    switch (msg) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case SIZE_INPUT:
                case SIZE_INPUT + 1:
                case SIZE_INPUT + 2:
                    NumberInputProc(L"PixelCount", SIZE_INPUT, wParam, &PIXEL_COUNT, hWnd);
                    break;
                case ZOOM_INPUT:
                case ZOOM_INPUT + 1:
                case ZOOM_INPUT + 2:
                    NumberInputProc(L"PixelSize", ZOOM_INPUT, wParam, &PIXEL_SIZE, hWnd);
                    break;
                case ESC_CHECKBOX:
                    ESC_TO_EXIT = IsDlgButtonChecked(hWnd, ESC_CHECKBOX);
                    RegSetKeyValue(HKEY_CURRENT_USER, REG_KEY, L"EscToExit", REG_DWORD, &ESC_TO_EXIT, sizeof(DWORD));
                    break;
                case RESET_BUTTON:
                    PIXEL_COUNT = PIXEL_COUNT_DEFAULT;
                    PIXEL_SIZE = PIXEL_SIZE_DEFAULT;
                    ESC_TO_EXIT = ESC_TO_EXIT_DEFAULT;
			        
                    ResetAllValues(hWnd);
					RegDeleteTree(HKEY_CURRENT_USER, REG_KEY);
                    break;
            }

            break;
        case WM_DESTROY:
			PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);

    }

    return 0;
}

void RegisterActionWndClass(HINSTANCE hInstance) {
    WNDCLASS wc;

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = ActionWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = ACTION_CLASS_NAME;

    RegisterClass(&wc);
}

void CreateNumberInput(LPCWSTR label, int x, int labelWidth, int action, HWND hWnd, HINSTANCE hInstance) {
    CreateWindow(
        L"STATIC", label,
        WS_CHILD | WS_VISIBLE,
        x, 10, labelWidth, 20, hWnd, (HMENU)0, hInstance, NULL
    );

    CreateWindow(
        L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
        x + labelWidth + 5, 8, 30, 20, hWnd, (HMENU)action, hInstance, NULL
    );

    CreateWindow(
        L"BUTTON", L"-",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x + labelWidth + 40, 5, 25, 25, hWnd, (HMENU)(action + 1), hInstance, NULL
    );

    CreateWindow(
        L"BUTTON", L"+",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x + labelWidth + 70, 5, 25, 25, hWnd, (HMENU)(action + 2), hInstance, NULL
    );
}

void ShowActionWindow(HINSTANCE hInstance) {
    POINT cursorPos;
    if (!GetCursorPos(&cursorPos)) {
        MessageBox(NULL, L"Failed to get cursor position", L"Error", MB_OK);
        exit(1);
    }

    RECT findRect = { cursorPos.x, cursorPos.y, cursorPos.x, cursorPos.y };
    HMONITOR hMonitor = MonitorFromRect(&findRect, MONITOR_DEFAULTTONEAREST);

    MONITORINFO monitorInfo;
    monitorInfo.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(hMonitor, &monitorInfo);
    RECT monitorRect = monitorInfo.rcMonitor;

    HWND hWnd = CreateWindowEx(
        WS_EX_TOPMOST,
        ACTION_CLASS_NAME,
        WINDOW_TITLE,
        WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        monitorRect.left + (monitorRect.right - monitorRect.left - 440) / 2, monitorRect.top,
        440, 55 + GetSystemMetrics(SM_CYSMCAPTION),
        NULL, NULL,
        hInstance, NULL
    );

    CreateNumberInput(L"Size:", 10, 35, SIZE_INPUT, hWnd, hInstance);
    CreateNumberInput(L"Zoom:", 152, 40, ZOOM_INPUT, hWnd, hInstance);

    CreateWindow(
        L"BUTTON", L"Esc",
        WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX,
        302, 5, 45, 25, hWnd, (HMENU)ESC_CHECKBOX, hInstance, NULL
    );

    CreateWindow(
        L"BUTTON", L"Reset",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        355, 5, 60, 25, hWnd, (HMENU)RESET_BUTTON, hInstance, NULL
    );

	ResetAllValues(hWnd);
}

DWORD WINAPI StartActionThread(LPVOID lpParam) {
    ActionThreadData* params = (ActionThreadData*)lpParam;

    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED);

    RegisterActionWndClass(params->hInstance);
    ShowActionWindow(params->hInstance);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    PostThreadMessage(params->pThreadId, WM_QUIT, 0, 0);
    return 0;
}
