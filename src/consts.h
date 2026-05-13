#pragma once
#include "windows.h"

constexpr DWORD PIXEL_COUNT_DEFAULT = 16;
constexpr DWORD PIXEL_SIZE_DEFAULT = 12;
constexpr BOOL ESC_TO_EXIT_DEFAULT = TRUE;

extern DWORD PIXEL_COUNT;
extern DWORD PIXEL_SIZE;
extern BOOL ESC_TO_EXIT;

constexpr WCHAR REG_KEY[] = L"Software\\PixelPerfect";
constexpr WCHAR WINDOW_TITLE[] = L"Pixel Perfect";
