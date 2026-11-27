#include "framework.h"
#include "InputHook.h"
#include "AudioEngine.h"
#include "Resource.h"
#include <algorithm>

// Forward reference to the main window handle (defined in MonkeySounds.cpp)
extern HWND g_hMainWnd;

InputHook& InputHook::GetInstance() {
    static InputHook instance;
    return instance;
}

InputHook::InputHook() {}

InputHook::~InputHook() {
    UninstallHooks();
}

void InputHook::SetIgnoreAutorepeat(bool ignore) {
    m_ignoreAutorepeat = ignore;
    ResetKeyState();
}

void InputHook::ResetKeyState() {
    std::fill(std::begin(m_keyState), std::end(m_keyState), false);
    std::fill(std::begin(m_lastKeyTime), std::end(m_lastKeyTime), 0);
}

bool InputHook::InstallHooks(HINSTANCE hInstance) {
    ResetKeyState();
    if (m_keyboardHook == NULL) {
        m_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);
    }
    if (m_mouseHook == NULL) {
        m_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, hInstance, 0);
    }
    return (m_keyboardHook != NULL && m_mouseHook != NULL);
}

void InputHook::UninstallHooks() {
    ResetKeyState();
    if (m_keyboardHook != NULL) {
        UnhookWindowsHookEx(m_keyboardHook);
        m_keyboardHook = NULL;
    }
    if (m_mouseHook != NULL) {
        UnhookWindowsHookEx(m_mouseHook);
        m_mouseHook = NULL;
    }
}

LRESULT CALLBACK InputHook::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* pKbd = (KBDLLHOOKSTRUCT*)lParam;
        InputHook& hook = InputHook::GetInstance();

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            bool isRepeat = false;
            if (hook.m_ignoreAutorepeat && pKbd->vkCode < 256) {
                DWORD now = pKbd->time != 0 ? pKbd->time : GetTickCount();
                if (hook.m_keyState[pKbd->vkCode]) {
                    if ((DWORD)(now - hook.m_lastKeyTime[pKbd->vkCode]) < 2000) {
                        isRepeat = true;
                    }
                }
                hook.m_keyState[pKbd->vkCode] = true;
                hook.m_lastKeyTime[pKbd->vkCode] = now;
            }

            if (!isRepeat) {
                AudioEngine::GetInstance().PlayKey(pKbd->vkCode, true);
                if (g_hMainWnd) PostMessageW(g_hMainWnd, WM_VU_PULSE, 0, 0);
            }
        } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            if (pKbd->vkCode < 256) {
                hook.m_keyState[pKbd->vkCode] = false;
            }
            AudioEngine::GetInstance().PlayKey(pKbd->vkCode, false);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK InputHook::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        switch (wParam) {
        case WM_LBUTTONDOWN:
            AudioEngine::GetInstance().PlayMouse("left", true);
            if (g_hMainWnd) PostMessageW(g_hMainWnd, WM_VU_PULSE, 0, 0);
            break;
        case WM_LBUTTONUP:
            AudioEngine::GetInstance().PlayMouse("left", false);
            break;
        case WM_RBUTTONDOWN:
            AudioEngine::GetInstance().PlayMouse("right", true);
            if (g_hMainWnd) PostMessageW(g_hMainWnd, WM_VU_PULSE, 0, 0);
            break;
        case WM_RBUTTONUP:
            AudioEngine::GetInstance().PlayMouse("right", false);
            break;
        case WM_MBUTTONDOWN:
            AudioEngine::GetInstance().PlayMouse("middle", true);
            if (g_hMainWnd) PostMessageW(g_hMainWnd, WM_VU_PULSE, 0, 0);
            break;
        case WM_MBUTTONUP:
            AudioEngine::GetInstance().PlayMouse("middle", false);
            break;
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
