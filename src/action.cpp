#include "windows.h"
#include "consts.h"
#include "config.h"
#include "action.h"

constexpr int WINDOW_WIDTH = 470;
constexpr int WINDOW_HEIGHT = 54;
constexpr WCHAR ACTION_CLASS_NAME[] = L"PixelPerfectActionWindow";

constexpr UINT SIZE_INPUT = 10;
constexpr UINT ZOOM_INPUT = 20;
constexpr UINT ESC_CHECKBOX = 30;
constexpr UINT GRID_CHECKBOX = 50;
constexpr UINT RESET_BUTTON = 40;

bool noChange = false;
HFONT hFont;
HFONT hFontLarge;
HWND pHWnd;

void ResetControls(HWND hWnd) {
	noChange = true;
    SetDlgItemInt(hWnd, SIZE_INPUT, PIXEL_COUNT, FALSE);
    SetDlgItemInt(hWnd, ZOOM_INPUT, PIXEL_SIZE, FALSE);
    CheckDlgButton(hWnd, ESC_CHECKBOX, ESC_TO_EXIT);
    CheckDlgButton(hWnd, GRID_CHECKBOX, SHOW_GRID);
	noChange = false;
}

void NumberInputProc(LPCWSTR name, int action, WPARAM wParam, DWORD *value, HWND hWnd) {
    switch (LOWORD(wParam) - action) {
        case 0: {
            if (noChange || HIWORD(wParam) != EN_CHANGE) break;

            BOOL lpTranslated;
            UINT val = GetDlgItemInt(hWnd, action, &lpTranslated, FALSE);
            if (lpTranslated) SaveValue(name, value, val, pHWnd);

            break;
        }
        case 1:
            SaveValue(name, value, max(1, *value - 1), pHWnd);
            SetDlgItemInt(hWnd, action, *value, FALSE);
            break;
        case 2:
            SaveValue(name, value, *value + 1, pHWnd);
            SetDlgItemInt(hWnd, action, *value, FALSE);
            break;
    }
}

LRESULT CALLBACK ActionWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
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
					SaveValue(L"EscToExit", &ESC_TO_EXIT, IsDlgButtonChecked(hWnd, ESC_CHECKBOX), pHWnd);
                    break;
                case GRID_CHECKBOX:
                    SaveValue(L"ShowGrid", &SHOW_GRID, IsDlgButtonChecked(hWnd, GRID_CHECKBOX), pHWnd);
                    break;
                case RESET_BUTTON:
                    ResetConfig(pHWnd);
                    ResetControls(hWnd);
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

void CreateWidget(
    LPCWSTR className, int style, LPCWSTR text,
    int x, int y, int width, int height, int action,
    HWND hWnd, HINSTANCE hInstance, bool large = false
) {
    HWND hCtrl = CreateWindow(
        className, text,
        WS_CHILD | WS_VISIBLE | style,
        x, y, width, height,
        hWnd, (HMENU)action, hInstance, NULL
    );

    SendMessage(hCtrl, WM_SETFONT, (WPARAM)(large ? hFontLarge : hFont), TRUE);
}

void CreateNumberInput(LPCWSTR label, int x, int labelWidth, int action, HWND hWnd, HINSTANCE hInstance) {
    CreateWidget(L"STATIC", 0, label, x, 10, labelWidth, 20, 0, hWnd, hInstance);
    CreateWidget(L"EDIT", WS_BORDER | ES_NUMBER, L"", x + labelWidth + 5, 8, 30, 20, action, hWnd, hInstance);
    CreateWidget(L"BUTTON", BS_PUSHBUTTON, L"\u2212", x + labelWidth + 40, 5, 26, 25, action + 1, hWnd, hInstance, true);
    CreateWidget(L"BUTTON", BS_PUSHBUTTON, L"+", x + labelWidth + 70, 5, 26, 25, action + 2, hWnd, hInstance, true);
}

void ShowActionWindow(HINSTANCE hInstance) {
    POINT cursorPos;
    if (!GetCursorPos(&cursorPos)) {
        MessageBox(NULL, L"Failed to get cursor position", L"Error", MB_OK);
        exit(1);
    }

    RECT findRect = { cursorPos.x, cursorPos.y, cursorPos.x, cursorPos.y };
    HMONITOR hMonitor = MonitorFromRect(&findRect, MONITOR_DEFAULTTONEAREST);

    MONITORINFO monitorInfo{};
    monitorInfo.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(hMonitor, &monitorInfo);
    RECT monitorRect = monitorInfo.rcMonitor;

    HWND hWnd = CreateWindowEx(
        WS_EX_TOPMOST,
        ACTION_CLASS_NAME,
        WINDOW_TITLE,
        WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        monitorRect.left + (monitorRect.right - monitorRect.left - WINDOW_WIDTH) / 2, monitorRect.top,
        WINDOW_WIDTH, WINDOW_HEIGHT + GetSystemMetrics(SM_CYSMCAPTION),
        NULL, NULL,
        hInstance, NULL
    );

    CreateNumberInput(L"Size:", 10, 30, SIZE_INPUT, hWnd, hInstance);
    CreateNumberInput(L"Zoom:", 146, 38, ZOOM_INPUT, hWnd, hInstance);
    CreateWidget(L"BUTTON", BS_CHECKBOX | BS_AUTOCHECKBOX, L"Grid", 292, 5, 45, 25, GRID_CHECKBOX, hWnd, hInstance);
    CreateWidget(L"BUTTON", BS_CHECKBOX | BS_AUTOCHECKBOX, L"Esc", 343, 5, 45, 25, ESC_CHECKBOX, hWnd, hInstance);
    CreateWidget(L"BUTTON", BS_PUSHBUTTON, L"Reset", 390, 5, 56, 25, RESET_BUTTON, hWnd, hInstance);

    ResetControls(hWnd);
}

DWORD WINAPI StartActionThread(LPVOID lpParam) {
    ActionThreadData* params = (ActionThreadData*)lpParam;
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED);

    hFont = CreateFont(
        18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH,
        L"MS Shell Dlg"
    );

    hFontLarge = CreateFont(
        21, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH,
        L"MS Shell Dlg"
    );

	pHWnd = params->pHWnd;
    RegisterActionWndClass(params->hInstance);
    ShowActionWindow(params->hInstance);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DeleteObject(hFont);
    DeleteObject(hFontLarge);
    PostThreadMessage(params->pThreadId, WM_QUIT, 0, 0);

    return 0;
}
