#include "bhop_L4D2.h"
#include <tlhelp32.h>

uintptr_t GetModuleBaseAddress(DWORD procId, const char* modName) {
  uintptr_t modBaseAddr = 0;
  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
  if (hSnap != INVALID_HANDLE_VALUE) {
    MODULEENTRY32 modEntry;
    modEntry.dwSize = sizeof(modEntry);
    if (Module32First(hSnap, &modEntry)) {
      do {
        if (_stricmp(modEntry.szModule, modName) == 0) {
          modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
          break;
        }
      } while (Module32Next(hSnap, &modEntry));
    }
  }
  CloseHandle(hSnap);
  return modBaseAddr;
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode >= 0) {
    KBDLLHOOKSTRUCT* kbd = (KBDLLHOOKSTRUCT*)lParam;

    if (kbd->vkCode == VK_F1) {
      if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
        g_Running = false;
        PostQuitMessage(0);
        return 1;
      }
    }

    if (kbd->vkCode == VK_SPACE) {
      if (kbd->dwExtraInfo == MAGIC_MARKER) {
        return CallNextHookEx(hHook, nCode, wParam, lParam);
      }

      if (g_GameHwnd && GetForegroundWindow() == g_GameHwnd) {
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
          g_SpaceHeld = true;
        }
        else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
          g_SpaceHeld = false;
        }
        return 1;
      }
    }
  }
  return CallNextHookEx(hHook, nCode, wParam, lParam);
}

void BhopThread() {
  while (g_Running) {
    if (!g_hProcess || !g_ClientBase) {
      g_GameHwnd = FindWindowA(NULL, "Left 4 Dead 2 - Direct3D 9");
      if (!g_GameHwnd) g_GameHwnd = FindWindowA(NULL, "Left 4 Dead 2");

      if (g_GameHwnd) {
        DWORD pid = 0;
        GetWindowThreadProcessId(g_GameHwnd, &pid);
        g_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        if (g_hProcess) {
          g_ClientBase = GetModuleBaseAddress(pid, "client.dll");
        }
      }
      Sleep(500);
      continue;
    }

    if (g_SpaceHeld) {
      DWORD flagsAddr = 0;
      DWORD pointerLevel1 = 0;
      SIZE_T bytesRead = 0;

      if (ReadProcessMemory(g_hProcess, (LPCVOID)(g_ClientBase + 0x00781D24), &pointerLevel1, 4, &bytesRead) && pointerLevel1 != 0) {
        DWORD pointerLevel2 = 0;
        if (ReadProcessMemory(g_hProcess, (LPCVOID)(uintptr_t)(pointerLevel1 + 0x4), &pointerLevel2, 4, &bytesRead) && pointerLevel2 != 0) {
          flagsAddr = pointerLevel2 + 0xC;
        }
      }

      if (flagsAddr != 0) {
        int flags = 0;
        if (ReadProcessMemory(g_hProcess, (LPCVOID)(uintptr_t)flagsAddr, &flags, sizeof(flags), &bytesRead)) {
          if (flags & 1) {
            INPUT ip;
            ip.type = INPUT_KEYBOARD;
            ip.ki.wVk = VK_SPACE;
            ip.ki.wScan = 0x39;
            ip.ki.dwFlags = KEYEVENTF_SCANCODE;
            ip.ki.dwExtraInfo = MAGIC_MARKER;

            SendInput(1, &ip, sizeof(INPUT));
            Sleep(10);

            ip.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
            SendInput(1, &ip, sizeof(INPUT));
            Sleep(10);
          }
        }
      }
    }
    Sleep(1);
  }
}