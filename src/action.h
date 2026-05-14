#pragma once

struct ActionThreadData {
    HINSTANCE hInstance;
	HWND pHWnd;
    int pThreadId;
};

DWORD WINAPI StartActionThread(LPVOID hInstance);
