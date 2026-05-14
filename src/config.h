#pragma once

constexpr DWORD PIXEL_COUNT_DEFAULT = 16;
constexpr DWORD PIXEL_SIZE_DEFAULT = 12;
constexpr DWORD ESC_TO_EXIT_DEFAULT = TRUE;
constexpr DWORD SHOW_GRID_DEFAULT = TRUE;

extern DWORD PIXEL_COUNT;
extern DWORD PIXEL_SIZE;
extern DWORD ESC_TO_EXIT;
extern DWORD SHOW_GRID;

void LoadConfig();
void SaveValue(LPCWSTR name, PDWORD var, DWORD value, HWND mainHWnd);
void ResetConfig(HWND mainHWnd);
