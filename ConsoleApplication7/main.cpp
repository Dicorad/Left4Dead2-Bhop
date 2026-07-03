#include <windows.h>
#include <tlhelp32.h>
#include <thread>

#include "bhop_L4D2.h"


HHOOK hHook = NULL;
HWND g_GameHwnd = NULL;
HANDLE g_hProcess = NULL;
uintptr_t g_ClientBase = 0;
bool g_SpaceHeld = false;
bool g_Running = true;
const ULONG_PTR MAGIC_MARKER = 0xDEADC0DE;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(40, 40, 40));

    HFONT hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    RECT rect;
    GetClientRect(hwnd, &rect);
    rect.top += 20;
    rect.left += 20;
    rect.right -= 20;

    const char* text = "Автор программы: Dicorad\n\n"
      "Инструкция:\n"
      "1. Запусти Left 4 Dead 2\n"
      "2. Зайди в игру на карту\n"
      "3. Просто ЗАЖМИ ПРОБЕЛ для бхопа\n\n"
      "Чтобы закрыть чит — нажмите F1 или крестик.";

    DrawTextA(hdc, text, -1, &rect, DT_LEFT);

    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
    EndPaint(hwnd, &ps);
    return 0;
  }
  case WM_DESTROY:
    g_Running = false;
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  const char CLASS_NAME[] = "DicoradBhopClass";

  WNDCLASSEX wc = { };
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

  if (!RegisterClassEx(&wc)) return 0;

  HWND hwnd = CreateWindowEx(
    0,
    CLASS_NAME,
    "By_Dicorad",
    WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
    CW_USEDEFAULT, CW_USEDEFAULT, 380, 220,
    NULL, NULL, hInstance, NULL
  );

  if (hwnd == NULL) return 0;

  ShowWindow(hwnd, nCmdShow);

  hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInstance, 0);

  std::thread logicThread(BhopThread);

  MSG msg = { };
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  g_Running = false;
  if (logicThread.joinable()) {
    logicThread.join();
  }

  if (hHook) UnhookWindowsHookEx(hHook);
  if (g_hProcess) CloseHandle(g_hProcess);

  return 0;
}