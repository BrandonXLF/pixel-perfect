#pragma once
#include "windows.h"

struct ActionThreadData {
    HINSTANCE hInstance;
    int pThreadId;
};

DWORD WINAPI StartActionThread(LPVOID hInstance);
