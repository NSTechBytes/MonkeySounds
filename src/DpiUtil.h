#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#ifndef USER_DEFAULT_SCREEN_DPI
#define USER_DEFAULT_SCREEN_DPI 96
#endif

namespace DpiUtil {

// Scales a 96-DPI design value to the given DPI.
inline int Scale(int value, UINT dpi) {
    return MulDiv(value, (int)dpi, USER_DEFAULT_SCREEN_DPI);
}

inline UINT GetWindowDpi(HWND hWnd) {
    if (hWnd && IsWindow(hWnd)) {
        UINT dpi = GetDpiForWindow(hWnd);
        if (dpi > 0) return dpi;
    }
    UINT sysDpi = GetDpiForSystem();
    return sysDpi > 0 ? sysDpi : USER_DEFAULT_SCREEN_DPI;
}

inline void EnablePerMonitorDpiAwareness() {
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32) {
        using fnSetProcessDpiAwarenessContext = BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT);
        auto pfn = (fnSetProcessDpiAwarenessContext)GetProcAddress(hUser32, "SetProcessDpiAwarenessContext");
        if (pfn && pfn(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
            return;
        }
    }
    SetProcessDPIAware();
}

} // namespace DpiUtil
