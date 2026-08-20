#pragma once
#include <windows.h>

class InputHook {
public:
    static InputHook& GetInstance();

    bool InstallHooks(HINSTANCE hInstance);
    void UninstallHooks();
    bool IsInstalled() const { return m_keyboardHook != NULL && m_mouseHook != NULL; }

private:
    InputHook();
    ~InputHook();

    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);

    HHOOK m_keyboardHook = NULL;
    HHOOK m_mouseHook = NULL;
};
