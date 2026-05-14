#include "windows.h"
#include "consts.h"
#include "config.h"

constexpr WCHAR REG_KEY[] = L"Software\\PixelPerfect";
DWORD size = sizeof(DWORD);

DWORD PIXEL_COUNT = PIXEL_COUNT_DEFAULT;
DWORD PIXEL_SIZE = PIXEL_SIZE_DEFAULT;
DWORD ESC_TO_EXIT = ESC_TO_EXIT_DEFAULT;
DWORD SHOW_GRID = SHOW_GRID_DEFAULT;

void LoadValue(LPCWSTR name, PDWORD var) {
    RegGetValue(HKEY_CURRENT_USER, REG_KEY, name, RRF_RT_DWORD, NULL, var, &size);
}

void LoadConfig() {
	LoadValue(L"PixelCount", &PIXEL_COUNT);
    LoadValue(L"PixelSize", &PIXEL_SIZE);
    LoadValue(L"EscToExit", &ESC_TO_EXIT);
    LoadValue(L"EscToExit", &SHOW_GRID);
}

void TriggerUpdate(HWND mainHWnd) {
    SendMessage(mainHWnd, WM_UPDATE, 0, 0);
}

void SaveValue(LPCWSTR name, PDWORD var, DWORD value, HWND mainHWnd) {
    *var = value;
    RegSetKeyValue(HKEY_CURRENT_USER, REG_KEY, name, REG_DWORD, var, sizeof(DWORD));
    TriggerUpdate(mainHWnd);
}

void ResetConfig(HWND mainHWnd) {
    PIXEL_COUNT = PIXEL_COUNT_DEFAULT;
    PIXEL_SIZE = PIXEL_SIZE_DEFAULT;
    ESC_TO_EXIT = ESC_TO_EXIT_DEFAULT;
    SHOW_GRID = SHOW_GRID_DEFAULT;

    RegDeleteTree(HKEY_CURRENT_USER, REG_KEY);
    TriggerUpdate(mainHWnd);
}
