#include "windows.h"

constexpr UINT UPDATE_TIMER = 1001;
constexpr UINT MOVE_TIMER = 1002;

constexpr int PIXEL_COUNT = 16;
constexpr int WINDOW_SIZE = PIXEL_COUNT * 12;
constexpr int CURSOR_GAP = 12;
constexpr int UPDATE_INTERVAL = 100;

constexpr WCHAR WINDOW_TITLE[] = L"Pixel Perfect";
constexpr WCHAR CLASS_NAME[] = L"PixelPerfectWindow";

void ShowPixels(HWND hWnd) {
    HDC hdcScreen = GetDC(NULL);
    HDC hdcWindow = GetDC(hWnd);

    RECT rcClient;
    if (!GetClientRect(hWnd, &rcClient)) {
        MessageBox(hWnd, L"Failed to get window dimensions", L"Error", MB_OK);
        exit(1);
    }

    POINT cursorPos;
    if (!GetCursorPos(&cursorPos)) {
        MessageBox(hWnd, L"Failed to get cursor position", L"Error", MB_OK);
        exit(1);
    }

    SetStretchBltMode(hdcWindow, HALFTONE);

    if (!StretchBlt(
        hdcWindow,
        0, 0,
        rcClient.right, rcClient.bottom,
        hdcScreen,
        cursorPos.x - (PIXEL_COUNT / 2),
        cursorPos.y - (PIXEL_COUNT / 2),
        PIXEL_COUNT, PIXEL_COUNT,
        SRCCOPY
    )) {
        MessageBox(hWnd, L"Failed to copy screen", L"Error", MB_OK);
        exit(1);
    }

    HBRUSH hBr = CreateSolidBrush(RGB(0, 0, 0));
    int width = rcClient.right - rcClient.left;
    int height = rcClient.bottom - rcClient.top;

    for (int i = 1; i < PIXEL_COUNT; i++) {
        int frac = (width * i) / PIXEL_COUNT;
        RECT myRect = { frac, 0, frac + 1, height };
        FillRect(hdcWindow, &myRect, hBr);
    }

    for (int i = 1; i < PIXEL_COUNT; i++) {
        int frac = (height * i) / PIXEL_COUNT;
        RECT myRect = { 0, frac, width, frac + 1 };
        FillRect(hdcWindow, &myRect, hBr);
    }

    DeleteObject(hBr);
    ReleaseDC(NULL, hdcScreen);
    ReleaseDC(hWnd, hdcWindow);
}

void MoveToCursor(HWND hWnd) {
    POINT cursorPos;
    if (!GetCursorPos(&cursorPos)) {
        MessageBox(hWnd, L"Failed to get cursor position", L"Error", MB_OK);
        exit(1);
    }

    RECT viewingRect = {
        cursorPos.x - WINDOW_SIZE - CURSOR_GAP,
        cursorPos.y - WINDOW_SIZE - CURSOR_GAP,
        cursorPos.x - CURSOR_GAP,
        cursorPos.y - CURSOR_GAP,
    };

    HMONITOR hMonitor = MonitorFromRect(&viewingRect, MONITOR_DEFAULTTONEAREST);

    MONITORINFO monitorInfo;
    monitorInfo.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(hMonitor, &monitorInfo);
    RECT monitorRect = monitorInfo.rcMonitor;

    int windowOffset;
    if (
        monitorRect.left <= viewingRect.left &&
        monitorRect.right >= viewingRect.right &&
        monitorRect.top <= viewingRect.top &&
        monitorRect.bottom >= viewingRect.bottom
    ) {
        windowOffset = -WINDOW_SIZE - CURSOR_GAP;
    } else {
        windowOffset = CURSOR_GAP;
    }

    MoveWindow(
        hWnd,
        cursorPos.x + windowOffset, cursorPos.y + windowOffset,
        WINDOW_SIZE, WINDOW_SIZE,
        FALSE
    );
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_TIMER: {
            ShowPixels(hWnd);
            MoveToCursor(hWnd);
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;

            BeginPaint(hWnd, &ps);
            ShowPixels(hWnd);
            EndPaint(hWnd, &ps);

            break;
        }
        case WM_HOTKEY:
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    return 0;
}

void RegisterWindowClass(HINSTANCE hInstance) {
    WNDCLASS wc;

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);
}

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow
) {
    RegisterWindowClass(hInstance);

    HWND hWnd = CreateWindowEx(
        WS_EX_TOPMOST,
        CLASS_NAME,
        WINDOW_TITLE,
        WS_POPUPWINDOW,
        CW_USEDEFAULT, NULL,
        WINDOW_SIZE, WINDOW_SIZE,
        NULL, NULL,
        hInstance, NULL
    );

    if (!hWnd) {
        return 1;
    }

    ShowPixels(hWnd);
    MoveToCursor(hWnd);
    ShowWindow(hWnd, SW_NORMAL);

    SetTimer(hWnd, NULL, UPDATE_INTERVAL, NULL);
    RegisterHotKey(hWnd, NULL, 0, VK_ESCAPE);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    KillTimer(hWnd, NULL);

    return (int)msg.wParam;
}
