#pragma once
#include <windows.h>

extern HHOOK hHook;
extern HWND g_GameHwnd;
extern HANDLE g_hProcess;
extern uintptr_t g_ClientBase;
extern bool g_SpaceHeld;
extern bool g_Running;
extern const ULONG_PTR MAGIC_MARKER;

uintptr_t GetModuleBaseAddress(DWORD procId, const char* modName);
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
void BhopThread();