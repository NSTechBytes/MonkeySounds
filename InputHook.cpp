#include "framework.h"
#include "InputHook.h"
#include "AudioEngine.h"

InputHook& InputHook::GetInstance() {
    static InputHook instance;
    return instance;
}

InputHook::InputHook() {}

InputHook::~InputHook() {
    UninstallHooks();
}

bool InputHook::InstallHooks(HINSTANCE hInstance) {
    if (m_keyboardHook == NULL) {
        m_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);
    }
    if (m_mouseHook == NULL) {
        m_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, hInstance, 0);
    }
    return (m_keyboardHook != NULL && m_mouseHook != NULL);
}

void InputHook::UninstallHooks() {
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
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            AudioEngine::GetInstance().PlayKey(pKbd->vkCode, true);
        } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
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
            break;
        case WM_LBUTTONUP:
            AudioEngine::GetInstance().PlayMouse("left", false);
            break;
        case WM_RBUTTONDOWN:
            AudioEngine::GetInstance().PlayMouse("right", true);
            break;
        case WM_RBUTTONUP:
            AudioEngine::GetInstance().PlayMouse("right", false);
            break;
        case WM_MBUTTONDOWN:
            AudioEngine::GetInstance().PlayMouse("middle", true);
            break;
        case WM_MBUTTONUP:
            AudioEngine::GetInstance().PlayMouse("middle", false);
            break;
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
