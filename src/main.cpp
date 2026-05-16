#include "windows.h"
#include "shellscalingapi.h"
#include "math.h"
#include "consts.h"
#include "config.h"
#include "action.h"

constexpr int UPDATE_INTERVAL = 40;
constexpr WCHAR CLASS_NAME[] = L"PixelPerfectWindow";

bool hasError = false;

void ShowError(HWND hWnd, LPCWSTR msg) {
    hasError = true;

    if (MessageBox(hWnd, msg, L"Error", MB_RETRYCANCEL | MB_ICONERROR) == IDCANCEL) {
        PostQuitMessage(1);
	}

    hasError = false;
}

void ShowPixels(HWND hWnd) {
	if (hasError) return;

    RECT rcClient;
    if (!GetClientRect(hWnd, &rcClient)) {
        ShowError(hWnd, L"Failed to get window dimensions");
    }

    POINT cursorPos;
    if (!GetCursorPos(&cursorPos)) {
        ShowError(hWnd, L"Failed to get cursor position");
    }

    HDC hdcScreen = GetDC(NULL);
    HDC hdcWindow = GetDC(hWnd);

    SetStretchBltMode(hdcWindow, COLORONCOLOR);

    if (!StretchBlt(
        hdcWindow,
        0, 0,
        rcClient.right, rcClient.bottom,
        hdcScreen,
        cursorPos.x - (PIXEL_COUNT / 2), cursorPos.y - (PIXEL_COUNT / 2),
        PIXEL_COUNT, PIXEL_COUNT,
        SRCCOPY
    )) {
        ShowError(hWnd, L"Failed to copy screen");
    }

    if (SHOW_GRID) {
        HBRUSH hBr = CreateSolidBrush(RGB(0, 0, 0));
        int width = rcClient.right - rcClient.left;
        int height = rcClient.bottom - rcClient.top;

        // Vertical lines
        for (int i = 1; i < PIXEL_COUNT; i++) {
            int frac = (width * i) / PIXEL_COUNT;
            RECT lineRect = { frac, 0, frac + 1, height };
            FillRect(hdcWindow, &lineRect, hBr);
        }

        // Horizontal lines
        for (int i = 1; i < PIXEL_COUNT; i++) {
            int frac = (height * i) / PIXEL_COUNT;
            RECT lineRect = { 0, frac, width, frac + 1 };
            FillRect(hdcWindow, &lineRect, hBr);
        }

        DeleteObject(hBr);
    }

    ReleaseDC(NULL, hdcScreen);
    ReleaseDC(hWnd, hdcWindow);
}

void MoveToCursor(HWND hWnd) {
    if (hasError) return;

    int windowSize = PIXEL_COUNT * PIXEL_SIZE;
    int cursorGap = ceil((float) PIXEL_COUNT / 2) + 4;

    POINT cursorPos;
    if (!GetCursorPos(&cursorPos)) {
        ShowError(hWnd, L"Failed to get cursor position");
    }

    RECT findRect = { cursorPos.x, cursorPos.y, cursorPos.x, cursorPos.y };
    HMONITOR hMonitor = MonitorFromRect(&findRect, MONITOR_DEFAULTTONEAREST);

    UINT dpiX, dpiY;
    GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);

    float scaleX = dpiX / 96.0f;
    float scaleY = dpiY / 96.0f;

    RECT viewingRect = {
        cursorPos.x - (windowSize + cursorGap) * scaleX,
        cursorPos.y - (windowSize + cursorGap) * scaleY,
        cursorPos.x - cursorGap * scaleX,
        cursorPos.y - cursorGap * scaleY,
    };

    MONITORINFO monitorInfo{};
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
        windowOffset = (-windowSize - cursorGap) * scaleX;
    }
    else {
        windowOffset = cursorGap * scaleX;
    }

    MoveWindow(
        hWnd,
        cursorPos.x + windowOffset, cursorPos.y + windowOffset,
        windowSize * scaleX, windowSize * scaleY,
        FALSE
    );

    RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_UPDATE:
        case WM_TIMER:
            MoveToCursor(hWnd);
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            ShowPixels(hWnd);
            EndPaint(hWnd, &ps);
            break;
        }
        case WM_HOTKEY:
			if (ESC_TO_EXIT) PostQuitMessage(0);
            break;
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
    _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine, _In_ int nCmdShow
) {
    LoadConfig();

    RegisterWindowClass(hInstance);
    HWND hWnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        CLASS_NAME,
        WINDOW_TITLE,
        WS_POPUPWINDOW,
        CW_USEDEFAULT, NULL,
        PIXEL_COUNT * PIXEL_SIZE, PIXEL_COUNT * PIXEL_SIZE,
        NULL, NULL,
        hInstance, NULL
    );

    if (!hWnd) return 1;

    ActionThreadData* actData = new ActionThreadData();
    actData->hInstance = hInstance;
    actData->pHWnd = hWnd;
    actData->pThreadId = GetCurrentThreadId();
    CreateThread(NULL, 0, StartActionThread, actData, 0, NULL);

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

    return 0;
}
