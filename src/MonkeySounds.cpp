#include "framework.h"
#include "MonkeySounds.h"
#include "Resource.h"
#include "AudioEngine.h"
#include "InputHook.h"
#include "AppSettings.h"
#include <commctrl.h>
#include <gdiplus.h>
#include <windowsx.h>
#include <shlobj.h>
#include <filesystem>
#include <vector>
#include <string>

namespace fs = std::filesystem;

static std::wstring Utf8ToWide(const std::string& str) {
    if (str.empty()) return L"";
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

#define WINDOW_WIDTH  480
#define WINDOW_HEIGHT 380
#define HEADER_HEIGHT 30
#define TIMER_CPU_ID  1001

// Global variables
HINSTANCE g_hInstance = NULL;
HWND g_hMainWnd = NULL;
HWND g_hTabCtrl = NULL;
HWND g_hStatusBar = NULL;

// Fonts
HFONT g_hFontNormal = NULL;
HFONT g_hFontBold = NULL;
HFONT g_hFontTitle = NULL;
HFONT g_hFontMono = NULL;

// Control HWNDs - Sounds Tab
HWND g_hKbGroup = NULL;
HWND g_hKbEnable = NULL;
HWND g_hKbPresetLbl = NULL;
HWND g_hKbPresetCombo = NULL;
HWND g_hKbCustomBtn = NULL;
HWND g_hKbVolLbl = NULL;
HWND g_hKbVolSlider = NULL;

HWND g_hMouseGroup = NULL;
HWND g_hMouseEnable = NULL;
HWND g_hMousePresetLbl = NULL;
HWND g_hMousePresetCombo = NULL;
HWND g_hMouseCustomBtn = NULL;
HWND g_hMouseVolLbl = NULL;
HWND g_hMouseVolSlider = NULL;

// Control HWNDs - Settings Tab
HWND g_hAppSettingsGroup = NULL;
HWND g_hVersionLbl = NULL;
HWND g_hCheckUpdatesBtn = NULL;
HWND g_hSeparator = NULL;
HWND g_hAutoStartChk = NULL;

// Action Buttons
HWND g_hApplyBtn = NULL;
HWND g_hOkBtn = NULL;
HWND g_hCancelBtn = NULL;

// Profiles lists
std::vector<SoundProfileInfo> g_kbProfiles;
std::vector<SoundProfileInfo> g_mouseProfiles;

// GDI+ Image for About page
Gdiplus::Image* g_pMonkeySoundsImage = NULL;
HICON g_hAppIcon = NULL;

// CPU Calculation globals
static ULONGLONG g_lastIdleTime = 0;
static ULONGLONG g_lastKernelTime = 0;
static ULONGLONG g_lastUserTime = 0;

// Header buttons state
static int g_hoverHeaderBtn = 0; // 0=none, 1=min, 2=max, 3=close
static int g_activeTab = 0;

// Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void CreateControls(HWND hWnd);
void UpdateTabVisibility(int tabIndex);
void PopulatePresets();
void ApplySettingsFromUI();
void LoadSettingsToUI();
void UpdateCpuUsage();
void ShowTrayMenu(HWND hWnd);
void ChooseCustomProfile(bool isKeyboard);

static ULONGLONG FileTimeToULL(const FILETIME& ft) {
    return ((ULONGLONG)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int nCmdShow) {
    g_hInstance = hInstance;

    // Common Controls v6 initialization
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TAB_CLASSES | ICC_BAR_CLASSES | ICC_STANDARD_CLASSES | ICC_USEREX_CLASSES;
    InitCommonControlsEx(&icex);

    // GDI+ initialization
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Load App Icon
    g_hAppIcon = (HICON)LoadImageW(hInstance, MAKEINTRESOURCEW(IDI_APP_ICON), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
    if (!g_hAppIcon) {
        if (fs::exists(L"assets\\Icon.ico")) {
            g_hAppIcon = (HICON)LoadImageW(NULL, L"assets\\Icon.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
        } else if (fs::exists(L"Icon.ico")) {
            g_hAppIcon = (HICON)LoadImageW(NULL, L"Icon.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
        }
    }

    // Load PNG logo
    if (fs::exists(L"assets\\MonkeySounds.png")) {
        g_pMonkeySoundsImage = Gdiplus::Image::FromFile(L"assets\\MonkeySounds.png");
    } else if (fs::exists(L"MonkeySounds.png")) {
        g_pMonkeySoundsImage = Gdiplus::Image::FromFile(L"MonkeySounds.png");
    } else if (fs::exists(L"D:\\Novadesk-Project\\MonkeySounds\\assets\\MonkeySounds.png")) {
        g_pMonkeySoundsImage = Gdiplus::Image::FromFile(L"D:\\Novadesk-Project\\MonkeySounds\\assets\\MonkeySounds.png");
    }

    // Initialize Audio Engine
    AudioEngine::GetInstance().Initialize();

    // Load App Settings
    AppSettings::GetInstance().Load();
    const auto& config = AppSettings::GetInstance().GetConfig();

    AudioEngine::GetInstance().SetKeyboardEnabled(config.keyboardEnabled);
    AudioEngine::GetInstance().SetMouseEnabled(config.mouseEnabled);
    AudioEngine::GetInstance().SetKeyboardVolume(config.keyboardVolume);
    AudioEngine::GetInstance().SetMouseVolume(config.mouseVolume);

    // Scan profiles
    g_kbProfiles = AudioEngine::GetInstance().ScanKeyboardProfiles();
    g_mouseProfiles = AudioEngine::GetInstance().ScanMouseProfiles();

    // Load default or configured keyboard profile
    bool kbLoaded = false;
    if (!config.keyboardProfilePath.empty() && fs::exists(config.keyboardProfilePath)) {
        kbLoaded = AudioEngine::GetInstance().LoadKeyboardProfile(config.keyboardProfilePath);
    }
    if (!kbLoaded && !g_kbProfiles.empty()) {
        AudioEngine::GetInstance().LoadKeyboardProfile(g_kbProfiles[0].profileJsonPath);
    }

    // Load default or configured mouse profile
    bool mouseLoaded = false;
    if (!config.mouseProfilePath.empty() && fs::exists(config.mouseProfilePath)) {
        mouseLoaded = AudioEngine::GetInstance().LoadMouseProfile(config.mouseProfilePath);
    }
    if (!mouseLoaded && !g_mouseProfiles.empty()) {
        AudioEngine::GetInstance().LoadMouseProfile(g_mouseProfiles[0].profileJsonPath);
    }

    // Install Input Hooks
    InputHook::GetInstance().InstallHooks(hInstance);

    // Create Fonts (Segoe UI)
    g_hFontNormal = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    g_hFontBold = CreateFontW(-12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    g_hFontTitle = CreateFontW(-14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    g_hFontMono = CreateFontW(-11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");

    // Register Window Class
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = g_hAppIcon;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wcex.lpszClassName = L"MonkeySoundsMainWindow";
    wcex.hIconSm = g_hAppIcon;
    RegisterClassExW(&wcex);

    // Calculate window rectangle centered on screen
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int posX = (screenW - WINDOW_WIDTH) / 2;
    int posY = (screenH - WINDOW_HEIGHT) / 2;

    HWND hWnd = CreateWindowExW(
        WS_EX_APPWINDOW,
        L"MonkeySoundsMainWindow",
        L"MokeySounds",
        WS_POPUP | WS_CLIPCHILDREN | WS_BORDER | WS_MINIMIZEBOX,
        posX, posY, WINDOW_WIDTH, WINDOW_HEIGHT,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hWnd) return FALSE;
    g_hMainWnd = hWnd;

    // Add System Tray Icon
    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = g_hAppIcon;
    wcscpy_s(nid.szTip, L"MonkeySounds");
    Shell_NotifyIconW(NIM_ADD, &nid);

    ShowWindow(hWnd, (nCmdShow == 0 || nCmdShow == SW_HIDE) ? SW_SHOW : nCmdShow);
    UpdateWindow(hWnd);

    // Main message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup Tray Icon
    Shell_NotifyIconW(NIM_DELETE, &nid);

    // Uninstall hooks and audio
    InputHook::GetInstance().UninstallHooks();
    AudioEngine::GetInstance().Shutdown();

    if (g_pMonkeySoundsImage) delete g_pMonkeySoundsImage;
    if (g_hFontNormal) DeleteObject(g_hFontNormal);
    if (g_hFontBold) DeleteObject(g_hFontBold);
    if (g_hFontTitle) DeleteObject(g_hFontTitle);
    if (g_hFontMono) DeleteObject(g_hFontMono);

    Gdiplus::GdiplusShutdown(gdiplusToken);
    return (int)msg.wParam;
}

void SetControlFont(HWND hWndCtrl, HFONT hFont) {
    if (hWndCtrl && hFont) {
        SendMessageW(hWndCtrl, WM_SETFONT, (WPARAM)hFont, TRUE);
    }
}

void CreateControls(HWND hWnd) {
    // 1. Tab Control
    g_hTabCtrl = CreateWindowExW(
        0, WC_TABCONTROL, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        8, HEADER_HEIGHT + 6, WINDOW_WIDTH - 16, 290,
        hWnd, (HMENU)IDC_TAB_CONTROL, g_hInstance, NULL
    );
    SetControlFont(g_hTabCtrl, g_hFontNormal);

    TCITEMW tie = {};
    tie.mask = TCIF_TEXT;
    tie.pszText = (LPWSTR)L"Sounds";
    TabCtrl_InsertItem(g_hTabCtrl, 0, &tie);
    tie.pszText = (LPWSTR)L"Settings";
    TabCtrl_InsertItem(g_hTabCtrl, 1, &tie);
    tie.pszText = (LPWSTR)L"About";
    TabCtrl_InsertItem(g_hTabCtrl, 2, &tie);

    // --- TAB 1: SOUNDS ---
    // Keyboard Group
    g_hKbGroup = CreateWindowExW(
        0, L"BUTTON", L"Keyboard Sounds",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        16, HEADER_HEIGHT + 36, WINDOW_WIDTH - 32, 115,
        hWnd, (HMENU)IDC_GB_KEYBOARD, g_hInstance, NULL
    );
    SetControlFont(g_hKbGroup, g_hFontBold);

    g_hKbEnable = CreateWindowExW(
        0, L"BUTTON", L"Enable Keyboard Sounds",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        28, HEADER_HEIGHT + 56, 180, 20,
        hWnd, (HMENU)IDC_CHK_KB_ENABLE, g_hInstance, NULL
    );
    SetControlFont(g_hKbEnable, g_hFontNormal);

    g_hKbPresetLbl = CreateWindowExW(
        0, L"STATIC", L"Preset:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        28, HEADER_HEIGHT + 83, 50, 20,
        hWnd, (HMENU)IDC_LBL_KB_PRESET, g_hInstance, NULL
    );
    SetControlFont(g_hKbPresetLbl, g_hFontNormal);

    g_hKbPresetCombo = CreateWindowExW(
        0, L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
        80, HEADER_HEIGHT + 80, 210, 160,
        hWnd, (HMENU)IDC_COMBO_KB_PRESET, g_hInstance, NULL
    );
    SetControlFont(g_hKbPresetCombo, g_hFontNormal);

    g_hKbCustomBtn = CreateWindowExW(
        0, L"BUTTON", L"Choose Custom Sound...",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        298, HEADER_HEIGHT + 79, 150, 24,
        hWnd, (HMENU)IDC_BTN_KB_CUSTOM, g_hInstance, NULL
    );
    SetControlFont(g_hKbCustomBtn, g_hFontNormal);

    g_hKbVolLbl = CreateWindowExW(
        0, L"STATIC", L"Volume:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        28, HEADER_HEIGHT + 116, 50, 20,
        hWnd, (HMENU)IDC_LBL_KB_VOLUME, g_hInstance, NULL
    );
    SetControlFont(g_hKbVolLbl, g_hFontNormal);

    g_hKbVolSlider = CreateWindowExW(
        0, TRACKBAR_CLASS, L"",
        WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_NOTICKS | WS_TABSTOP,
        80, HEADER_HEIGHT + 114, 368, 24,
        hWnd, (HMENU)IDC_SLIDER_KB_VOLUME, g_hInstance, NULL
    );
    SendMessageW(g_hKbVolSlider, TBM_SETRANGE, TRUE, MAKELONG(0, 100));

    // Mouse Group
    g_hMouseGroup = CreateWindowExW(
        0, L"BUTTON", L"Mouse Sounds",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        16, HEADER_HEIGHT + 158, WINDOW_WIDTH - 32, 115,
        hWnd, (HMENU)IDC_GB_MOUSE, g_hInstance, NULL
    );
    SetControlFont(g_hMouseGroup, g_hFontBold);

    g_hMouseEnable = CreateWindowExW(
        0, L"BUTTON", L"Enable Mouse Sounds",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        28, HEADER_HEIGHT + 178, 180, 20,
        hWnd, (HMENU)IDC_CHK_MOUSE_ENABLE, g_hInstance, NULL
    );
    SetControlFont(g_hMouseEnable, g_hFontNormal);

    g_hMousePresetLbl = CreateWindowExW(
        0, L"STATIC", L"Preset:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        28, HEADER_HEIGHT + 205, 50, 20,
        hWnd, (HMENU)IDC_LBL_MOUSE_PRESET, g_hInstance, NULL
    );
    SetControlFont(g_hMousePresetLbl, g_hFontNormal);

    g_hMousePresetCombo = CreateWindowExW(
        0, L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
        80, HEADER_HEIGHT + 202, 210, 160,
        hWnd, (HMENU)IDC_COMBO_MOUSE_PRESET, g_hInstance, NULL
    );
    SetControlFont(g_hMousePresetCombo, g_hFontNormal);

    g_hMouseCustomBtn = CreateWindowExW(
        0, L"BUTTON", L"Choose Custom Sound...",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        298, HEADER_HEIGHT + 201, 150, 24,
        hWnd, (HMENU)IDC_BTN_MOUSE_CUSTOM, g_hInstance, NULL
    );
    SetControlFont(g_hMouseCustomBtn, g_hFontNormal);

    g_hMouseVolLbl = CreateWindowExW(
        0, L"STATIC", L"Volume:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        28, HEADER_HEIGHT + 238, 50, 20,
        hWnd, (HMENU)IDC_LBL_MOUSE_VOLUME, g_hInstance, NULL
    );
    SetControlFont(g_hMouseVolLbl, g_hFontNormal);

    g_hMouseVolSlider = CreateWindowExW(
        0, TRACKBAR_CLASS, L"",
        WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_NOTICKS | WS_TABSTOP,
        80, HEADER_HEIGHT + 236, 368, 24,
        hWnd, (HMENU)IDC_SLIDER_MOUSE_VOLUME, g_hInstance, NULL
    );
    SendMessageW(g_hMouseVolSlider, TBM_SETRANGE, TRUE, MAKELONG(0, 100));

    // --- TAB 2: SETTINGS ---
    g_hAppSettingsGroup = CreateWindowExW(
        0, L"BUTTON", L"Application",
        WS_CHILD | BS_GROUPBOX,
        16, HEADER_HEIGHT + 36, WINDOW_WIDTH - 32, 237,
        hWnd, (HMENU)IDC_GB_APPLICATION, g_hInstance, NULL
    );
    SetControlFont(g_hAppSettingsGroup, g_hFontBold);

    g_hVersionLbl = CreateWindowExW(
        0, L"STATIC", L"Current Version: 1.0.4",
        WS_CHILD | SS_LEFT,
        28, HEADER_HEIGHT + 60, 200, 20,
        hWnd, (HMENU)IDC_LBL_VERSION, g_hInstance, NULL
    );
    SetControlFont(g_hVersionLbl, g_hFontNormal);

    g_hCheckUpdatesBtn = CreateWindowExW(
        0, L"BUTTON", L"Check for Updates",
        WS_CHILD | BS_PUSHBUTTON,
        28, HEADER_HEIGHT + 86, 130, 26,
        hWnd, (HMENU)IDC_BTN_CHECK_UPDATES, g_hInstance, NULL
    );
    SetControlFont(g_hCheckUpdatesBtn, g_hFontNormal);

    g_hSeparator = CreateWindowExW(
        0, L"STATIC", L"",
        WS_CHILD | SS_ETCHEDHORZ,
        28, HEADER_HEIGHT + 130, WINDOW_WIDTH - 56, 2,
        hWnd, (HMENU)-1, g_hInstance, NULL
    );

    g_hAutoStartChk = CreateWindowExW(
        0, L"BUTTON", L"Auto-start on Windows startup",
        WS_CHILD | BS_AUTOCHECKBOX,
        28, HEADER_HEIGHT + 142, 240, 20,
        hWnd, (HMENU)IDC_CHK_AUTOSTART, g_hInstance, NULL
    );
    SetControlFont(g_hAutoStartChk, g_hFontNormal);

    // --- Action Buttons (Bottom) ---
    g_hApplyBtn = CreateWindowExW(
        0, L"BUTTON", L"Apply",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        WINDOW_WIDTH - 216, HEADER_HEIGHT + 280, 62, 26,
        hWnd, (HMENU)IDC_BTN_APPLY, g_hInstance, NULL
    );
    SetControlFont(g_hApplyBtn, g_hFontNormal);

    g_hOkBtn = CreateWindowExW(
        0, L"BUTTON", L"OK",
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        WINDOW_WIDTH - 146, HEADER_HEIGHT + 280, 62, 26,
        hWnd, (HMENU)IDC_BTN_OK, g_hInstance, NULL
    );
    SetControlFont(g_hOkBtn, g_hFontNormal);

    g_hCancelBtn = CreateWindowExW(
        0, L"BUTTON", L"Cancel",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        WINDOW_WIDTH - 76, HEADER_HEIGHT + 280, 62, 26,
        hWnd, (HMENU)IDC_BTN_CANCEL, g_hInstance, NULL
    );
    SetControlFont(g_hCancelBtn, g_hFontNormal);

    // --- Status Bar ---
    g_hStatusBar = CreateWindowExW(
        0, STATUSCLASSNAME, L"",
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0,
        hWnd, (HMENU)IDC_STATUSBAR, g_hInstance, NULL
    );
    SetControlFont(g_hStatusBar, g_hFontMono);

    int statwidths[] = { WINDOW_WIDTH - 90, -1 };
    SendMessageW(g_hStatusBar, SB_SETPARTS, 2, (LPARAM)statwidths);
    SendMessageW(g_hStatusBar, SB_SETTEXTW, 0, (LPARAM)L"Ready");
    SendMessageW(g_hStatusBar, SB_SETTEXTW, 1, (LPARAM)L"CPU: 0%");
}

void UpdateTabVisibility(int tabIndex) {
    g_activeTab = tabIndex;

    // Tab 0: Sounds
    int showSounds = (tabIndex == 0) ? SW_SHOW : SW_HIDE;
    ShowWindow(g_hKbGroup, showSounds);
    ShowWindow(g_hKbEnable, showSounds);
    ShowWindow(g_hKbPresetLbl, showSounds);
    ShowWindow(g_hKbPresetCombo, showSounds);
    ShowWindow(g_hKbCustomBtn, showSounds);
    ShowWindow(g_hKbVolLbl, showSounds);
    ShowWindow(g_hKbVolSlider, showSounds);

    ShowWindow(g_hMouseGroup, showSounds);
    ShowWindow(g_hMouseEnable, showSounds);
    ShowWindow(g_hMousePresetLbl, showSounds);
    ShowWindow(g_hMousePresetCombo, showSounds);
    ShowWindow(g_hMouseCustomBtn, showSounds);
    ShowWindow(g_hMouseVolLbl, showSounds);
    ShowWindow(g_hMouseVolSlider, showSounds);

    // Tab 1: Settings
    int showSettings = (tabIndex == 1) ? SW_SHOW : SW_HIDE;
    ShowWindow(g_hAppSettingsGroup, showSettings);
    ShowWindow(g_hVersionLbl, showSettings);
    ShowWindow(g_hCheckUpdatesBtn, showSettings);
    ShowWindow(g_hSeparator, showSettings);
    ShowWindow(g_hAutoStartChk, showSettings);

    // Action buttons visible on tab 0 & 1
    int showButtons = (tabIndex == 0 || tabIndex == 1) ? SW_SHOW : SW_HIDE;
    ShowWindow(g_hApplyBtn, showButtons);
    ShowWindow(g_hOkBtn, showButtons);
    ShowWindow(g_hCancelBtn, showButtons);

    // Trigger repaint (especially for About tab custom drawing)
    InvalidateRect(g_hMainWnd, NULL, TRUE);
}

void PopulatePresets() {
    SendMessageW(g_hKbPresetCombo, CB_RESETCONTENT, 0, 0);
    SendMessageW(g_hMousePresetCombo, CB_RESETCONTENT, 0, 0);

    // Keyboard presets
    int selKbIndex = 0;
    std::wstring currentKbPath = AudioEngine::GetInstance().GetCurrentKeyboardProfilePath();

    for (size_t i = 0; i < g_kbProfiles.size(); ++i) {
        std::wstring nameW = Utf8ToWide(g_kbProfiles[i].name);
        SendMessageW(g_hKbPresetCombo, CB_ADDSTRING, 0, (LPARAM)nameW.c_str());
        if (!currentKbPath.empty() && _wcsicmp(currentKbPath.c_str(), g_kbProfiles[i].profileJsonPath.c_str()) == 0) {
            selKbIndex = (int)i;
        }
    }
    SendMessageW(g_hKbPresetCombo, CB_SETCURSEL, selKbIndex, 0);

    // Mouse presets
    int selMouseIndex = 0;
    std::wstring currentMousePath = AudioEngine::GetInstance().GetCurrentMouseProfilePath();

    for (size_t i = 0; i < g_mouseProfiles.size(); ++i) {
        std::wstring nameW = Utf8ToWide(g_mouseProfiles[i].name);
        SendMessageW(g_hMousePresetCombo, CB_ADDSTRING, 0, (LPARAM)nameW.c_str());
        if (!currentMousePath.empty() && _wcsicmp(currentMousePath.c_str(), g_mouseProfiles[i].profileJsonPath.c_str()) == 0) {
            selMouseIndex = (int)i;
        }
    }
    SendMessageW(g_hMousePresetCombo, CB_SETCURSEL, selMouseIndex, 0);
}

void LoadSettingsToUI() {
    const auto& config = AppSettings::GetInstance().GetConfig();

    Button_SetCheck(g_hKbEnable, config.keyboardEnabled ? BST_CHECKED : BST_UNCHECKED);
    Button_SetCheck(g_hMouseEnable, config.mouseEnabled ? BST_CHECKED : BST_UNCHECKED);

    int kbVolInt = (int)(config.keyboardVolume * 100.0f + 0.5f);
    int mouseVolInt = (int)(config.mouseVolume * 100.0f + 0.5f);
    SendMessageW(g_hKbVolSlider, TBM_SETPOS, TRUE, kbVolInt);
    SendMessageW(g_hMouseVolSlider, TBM_SETPOS, TRUE, mouseVolInt);

    Button_SetCheck(g_hAutoStartChk, config.autoStart ? BST_CHECKED : BST_UNCHECKED);

    PopulatePresets();
}

void ApplySettingsFromUI() {
    bool kbEnabled = (Button_GetCheck(g_hKbEnable) == BST_CHECKED);
    bool mouseEnabled = (Button_GetCheck(g_hMouseEnable) == BST_CHECKED);
    int kbVol = (int)SendMessageW(g_hKbVolSlider, TBM_GETPOS, 0, 0);
    int mouseVol = (int)SendMessageW(g_hMouseVolSlider, TBM_GETPOS, 0, 0);
    bool autoStart = (Button_GetCheck(g_hAutoStartChk) == BST_CHECKED);

    AudioEngine::GetInstance().SetKeyboardEnabled(kbEnabled);
    AudioEngine::GetInstance().SetMouseEnabled(mouseEnabled);
    AudioEngine::GetInstance().SetKeyboardVolume((float)kbVol / 100.0f);
    AudioEngine::GetInstance().SetMouseVolume((float)mouseVol / 100.0f);

    int selKb = (int)SendMessageW(g_hKbPresetCombo, CB_GETCURSEL, 0, 0);
    if (selKb >= 0 && selKb < (int)g_kbProfiles.size()) {
        AudioEngine::GetInstance().LoadKeyboardProfile(g_kbProfiles[selKb].profileJsonPath);
    }

    int selMouse = (int)SendMessageW(g_hMousePresetCombo, CB_GETCURSEL, 0, 0);
    if (selMouse >= 0 && selMouse < (int)g_mouseProfiles.size()) {
        AudioEngine::GetInstance().LoadMouseProfile(g_mouseProfiles[selMouse].profileJsonPath);
    }

    auto& cfg = AppSettings::GetInstance().GetConfig();
    cfg.keyboardEnabled = kbEnabled;
    cfg.mouseEnabled = mouseEnabled;
    cfg.keyboardVolume = (float)kbVol / 100.0f;
    cfg.mouseVolume = (float)mouseVol / 100.0f;
    cfg.keyboardProfilePath = AudioEngine::GetInstance().GetCurrentKeyboardProfilePath();
    cfg.mouseProfilePath = AudioEngine::GetInstance().GetCurrentMouseProfilePath();
    cfg.autoStart = autoStart;

    AppSettings::GetInstance().Save();
}

void ChooseCustomProfile(bool isKeyboard) {
    WCHAR szFile[MAX_PATH] = {};
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = g_hMainWnd;
    ofn.lpstrFilter = L"Profile JSON (profile.json)\0profile.json\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrTitle = isKeyboard ? L"Select Keyboard profile.json" : L"Select Mouse profile.json";

    if (GetOpenFileNameW(&ofn)) {
        if (isKeyboard) {
            if (AudioEngine::GetInstance().LoadKeyboardProfile(szFile)) {
                g_kbProfiles = AudioEngine::GetInstance().ScanKeyboardProfiles();
                PopulatePresets();
                AudioEngine::GetInstance().PlayKey(VK_SPACE, true);
            }
        } else {
            if (AudioEngine::GetInstance().LoadMouseProfile(szFile)) {
                g_mouseProfiles = AudioEngine::GetInstance().ScanMouseProfiles();
                PopulatePresets();
                AudioEngine::GetInstance().PlayMouse("left", true);
            }
        }
    }
}

void UpdateCpuUsage() {
    FILETIME idleTime, kernelTime, userTime;
    if (GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        ULONGLONG idle = FileTimeToULL(idleTime);
        ULONGLONG kernel = FileTimeToULL(kernelTime);
        ULONGLONG user = FileTimeToULL(userTime);

        if (g_lastKernelTime != 0 || g_lastUserTime != 0) {
            ULONGLONG idleDelta = idle - g_lastIdleTime;
            ULONGLONG kernelDelta = kernel - g_lastKernelTime;
            ULONGLONG userDelta = user - g_lastUserTime;
            ULONGLONG totalSystem = kernelDelta + userDelta;

            if (totalSystem > 0) {
                ULONGLONG usedSystem = totalSystem - idleDelta;
                int cpuPercent = (int)((usedSystem * 100) / totalSystem);
                if (cpuPercent < 0) cpuPercent = 0;
                if (cpuPercent > 100) cpuPercent = 100;

                WCHAR buf[32];
                swprintf_s(buf, L"CPU: %d%%", cpuPercent);
                SendMessageW(g_hStatusBar, SB_SETTEXTW, 1, (LPARAM)buf);
            }
        }

        g_lastIdleTime = idle;
        g_lastKernelTime = kernel;
        g_lastUserTime = user;
    }
}

void ShowTrayMenu(HWND hWnd) {
    POINT pt;
    GetCursorPos(&pt);
    HMENU hMenu = LoadMenuW(g_hInstance, MAKEINTRESOURCEW(IDR_TRAY_MENU));
    if (hMenu) {
        HMENU hSubMenu = GetSubMenu(hMenu, 0);
        SetForegroundWindow(hWnd);

        // Update mute checkmarks
        CheckMenuItem(hSubMenu, IDM_TRAY_MUTE_KB, AudioEngine::GetInstance().IsKeyboardEnabled() ? MF_UNCHECKED : MF_CHECKED);
        CheckMenuItem(hSubMenu, IDM_TRAY_MUTE_MOUSE, AudioEngine::GetInstance().IsMouseEnabled() ? MF_UNCHECKED : MF_CHECKED);

        TrackPopupMenu(hSubMenu, TPM_RIGHTBUTTON | TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
        DestroyMenu(hMenu);
    }
}

void DrawCustomHeader(HWND hWnd, HDC hdc) {
    RECT rcHeader = { 0, 0, WINDOW_WIDTH, HEADER_HEIGHT };
    HBRUSH hBlueBrush = CreateSolidBrush(RGB(0, 90, 158));
    FillRect(hdc, &rcHeader, hBlueBrush);
    DeleteObject(hBlueBrush);

    // Draw speaker icon
    HICON hSmallIcon = (HICON)LoadImageW(g_hInstance, MAKEINTRESOURCEW(IDI_SMALL), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    if (!hSmallIcon) {
        hSmallIcon = (HICON)LoadImageW(NULL, L"Icon.ico", IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
    }
    if (hSmallIcon) {
        DrawIconEx(hdc, 8, 7, hSmallIcon, 16, 16, 0, NULL, DI_NORMAL);
        DestroyIcon(hSmallIcon);
    }

    // Title Text
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 255, 255));
    HFONT hOldFont = (HFONT)SelectObject(hdc, g_hFontTitle);
    TextOutW(hdc, 28, 6, L"MokeySounds", 11);

    // Min / Max / Close Buttons on right
    // Close button (x: 452, y: 5, w: 22, h: 20)
    // Max button (x: 428, y: 5, w: 22, h: 20)
    // Min button (x: 404, y: 5, w: 22, h: 20)
    int btnY = 5;
    int btnH = 20;
    int btnW = 20;

    // Min Button
    RECT rcMin = { WINDOW_WIDTH - 66, btnY, WINDOW_WIDTH - 46, btnY + btnH };
    HBRUSH hMinBrush = CreateSolidBrush((g_hoverHeaderBtn == 1) ? RGB(30, 115, 185) : RGB(220, 220, 220));
    FillRect(hdc, &rcMin, hMinBrush);
    DeleteObject(hMinBrush);
    FrameRect(hdc, &rcMin, (HBRUSH)GetStockObject(GRAY_BRUSH));
    HPEN hBlackPen = CreatePen(PS_SOLID, 2, RGB(40, 40, 40));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hBlackPen);
    MoveToEx(hdc, rcMin.left + 5, rcMin.bottom - 6, NULL);
    LineTo(hdc, rcMin.right - 5, rcMin.bottom - 6);

    // Max / Restore Button (disabled style)
    RECT rcMax = { WINDOW_WIDTH - 44, btnY, WINDOW_WIDTH - 24, btnY + btnH };
    HBRUSH hMaxBrush = CreateSolidBrush(RGB(220, 220, 220));
    FillRect(hdc, &rcMax, hMaxBrush);
    DeleteObject(hMaxBrush);
    FrameRect(hdc, &rcMax, (HBRUSH)GetStockObject(GRAY_BRUSH));
    Rectangle(hdc, rcMax.left + 4, rcMax.top + 4, rcMax.right - 4, rcMax.bottom - 4);

    // Close Button (Red on hover)
    RECT rcClose = { WINDOW_WIDTH - 22, btnY, WINDOW_WIDTH - 2, btnY + btnH };
    HBRUSH hCloseBrush = CreateSolidBrush((g_hoverHeaderBtn == 3) ? RGB(232, 17, 35) : RGB(220, 220, 220));
    FillRect(hdc, &rcClose, hCloseBrush);
    DeleteObject(hCloseBrush);
    FrameRect(hdc, &rcClose, (HBRUSH)GetStockObject(GRAY_BRUSH));

    COLORREF closeColor = (g_hoverHeaderBtn == 3) ? RGB(255, 255, 255) : RGB(180, 0, 0);
    HPEN hClosePen = CreatePen(PS_SOLID, 2, closeColor);
    SelectObject(hdc, hClosePen);
    MoveToEx(hdc, rcClose.left + 5, rcClose.top + 5, NULL);
    LineTo(hdc, rcClose.right - 5, rcClose.bottom - 5);
    MoveToEx(hdc, rcClose.right - 5, rcClose.top + 5, NULL);
    LineTo(hdc, rcClose.left + 5, rcClose.bottom - 5);

    SelectObject(hdc, hOldPen);
    DeleteObject(hBlackPen);
    DeleteObject(hClosePen);

    SelectObject(hdc, hOldFont);
}

void DrawAboutTab(HWND hWnd, HDC hdc) {
    if (g_activeTab != 2) return;

    // Centered Badge Container for Image
    int badgeX = (WINDOW_WIDTH - 76) / 2;
    int badgeY = HEADER_HEIGHT + 45;
    int badgeW = 76;
    int badgeH = 76;

    // Rounded rectangle / Card border
    RECT rcCard = { badgeX - 4, badgeY - 4, badgeX + badgeW + 4, badgeY + badgeH + 4 };
    HBRUSH hCardBg = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(hdc, &rcCard, hCardBg);
    DeleteObject(hCardBg);
    FrameRect(hdc, &rcCard, (HBRUSH)GetStockObject(GRAY_BRUSH));

    if (g_pMonkeySoundsImage) {
        Gdiplus::Graphics graphics(hdc);
        graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
        graphics.DrawImage(g_pMonkeySoundsImage, badgeX, badgeY, badgeW, badgeH);
    } else if (g_hAppIcon) {
        DrawIconEx(hdc, badgeX + (badgeW - 48) / 2, badgeY + (badgeH - 48) / 2, g_hAppIcon, 48, 48, 0, NULL, DI_NORMAL);
    }

    SetBkMode(hdc, TRANSPARENT);

    // Title: MonkeySounds
    HFONT hOldFont = (HFONT)SelectObject(hdc, g_hFontTitle);
    SetTextColor(hdc, RGB(20, 20, 20));
    RECT rcTitle = { 0, badgeY + badgeH + 12, WINDOW_WIDTH, badgeY + badgeH + 30 };
    DrawTextW(hdc, L"MonkeySounds", -1, &rcTitle, DT_CENTER | DT_SINGLELINE);

    // Description
    SelectObject(hdc, g_hFontNormal);
    SetTextColor(hdc, RGB(60, 60, 60));
    RECT rcDesc1 = { 0, badgeY + badgeH + 32, WINDOW_WIDTH, badgeY + badgeH + 48 };
    DrawTextW(hdc, L"Professional system sound", -1, &rcDesc1, DT_CENTER | DT_SINGLELINE);
    RECT rcDesc2 = { 0, badgeY + badgeH + 48, WINDOW_WIDTH, badgeY + badgeH + 64 };
    DrawTextW(hdc, L"customization utility.", -1, &rcDesc2, DT_CENTER | DT_SINGLELINE);

    // Copyright
    SelectObject(hdc, g_hFontMono);
    SetTextColor(hdc, RGB(90, 90, 90));
    RECT rcCopy = { 0, badgeY + badgeH + 80, WINDOW_WIDTH, badgeY + badgeH + 100 };
    DrawTextW(hdc, L"© 2024 MonkeySounds. All rights reserved.", -1, &rcCopy, DT_CENTER | DT_SINGLELINE);

    SelectObject(hdc, hOldFont);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        CreateControls(hWnd);
        LoadSettingsToUI();
        UpdateTabVisibility(0);
        SetTimer(hWnd, TIMER_CPU_ID, 1000, NULL);
        UpdateCpuUsage();
        break;

    case WM_TIMER:
        if (wParam == TIMER_CPU_ID) {
            UpdateCpuUsage();
        }
        break;

    case WM_TRAYICON:
        if (lParam == WM_LBUTTONDBLCLK || lParam == WM_LBUTTONUP) {
            ShowWindow(hWnd, SW_SHOW);
            SetForegroundWindow(hWnd);
        } else if (lParam == WM_RBUTTONUP) {
            ShowTrayMenu(hWnd);
        }
        break;

    case WM_NCHITTEST: {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ScreenToClient(hWnd, &pt);
        if (pt.y >= 0 && pt.y < HEADER_HEIGHT) {
            // Check header buttons area
            if (pt.x >= WINDOW_WIDTH - 66) {
                return HTCLIENT;
            }
            return HTCAPTION; // Allows moving the window by dragging the blue header
        }
        return HTCLIENT;
    }

    case WM_MOUSEMOVE: {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        int prevHover = g_hoverHeaderBtn;
        g_hoverHeaderBtn = 0;
        if (y >= 5 && y <= 25) {
            if (x >= WINDOW_WIDTH - 66 && x < WINDOW_WIDTH - 46) {
                g_hoverHeaderBtn = 1; // Min
            } else if (x >= WINDOW_WIDTH - 44 && x < WINDOW_WIDTH - 24) {
                g_hoverHeaderBtn = 2; // Max
            } else if (x >= WINDOW_WIDTH - 22 && x < WINDOW_WIDTH - 2) {
                g_hoverHeaderBtn = 3; // Close
            }
        }
        if (prevHover != g_hoverHeaderBtn) {
            RECT rcBtnArea = { WINDOW_WIDTH - 70, 0, WINDOW_WIDTH, HEADER_HEIGHT };
            InvalidateRect(hWnd, &rcBtnArea, FALSE);
        }
        break;
    }

    case WM_LBUTTONDOWN: {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        if (y >= 5 && y <= 25) {
            if (x >= WINDOW_WIDTH - 66 && x < WINDOW_WIDTH - 46) {
                // Minimize to tray
                ShowWindow(hWnd, SW_HIDE);
            } else if (x >= WINDOW_WIDTH - 22 && x < WINDOW_WIDTH - 2) {
                // Close / Minimize to tray
                ShowWindow(hWnd, SW_HIDE);
            }
        }
        break;
    }

    case WM_NOTIFY: {
        LPNMHDR pnmh = (LPNMHDR)lParam;
        if (pnmh->idFrom == IDC_TAB_CONTROL && pnmh->code == TCN_SELCHANGE) {
            int curSel = TabCtrl_GetCurSel(g_hTabCtrl);
            UpdateTabVisibility(curSel);
        }
        break;
    }

    case WM_HSCROLL: {
        HWND hScrollBar = (HWND)lParam;
        if (hScrollBar == g_hKbVolSlider) {
            int pos = (int)SendMessageW(g_hKbVolSlider, TBM_GETPOS, 0, 0);
            AudioEngine::GetInstance().SetKeyboardVolume((float)pos / 100.0f);
        } else if (hScrollBar == g_hMouseVolSlider) {
            int pos = (int)SendMessageW(g_hMouseVolSlider, TBM_GETPOS, 0, 0);
            AudioEngine::GetInstance().SetMouseVolume((float)pos / 100.0f);
        }
        break;
    }

    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        int wmEvent = HIWORD(wParam);

        switch (wmId) {
        case IDC_CHK_KB_ENABLE:
            AudioEngine::GetInstance().SetKeyboardEnabled(Button_GetCheck(g_hKbEnable) == BST_CHECKED);
            break;

        case IDC_CHK_MOUSE_ENABLE:
            AudioEngine::GetInstance().SetMouseEnabled(Button_GetCheck(g_hMouseEnable) == BST_CHECKED);
            break;

        case IDC_COMBO_KB_PRESET:
            if (wmEvent == CBN_SELCHANGE) {
                int sel = (int)SendMessageW(g_hKbPresetCombo, CB_GETCURSEL, 0, 0);
                if (sel >= 0 && sel < (int)g_kbProfiles.size()) {
                    AudioEngine::GetInstance().LoadKeyboardProfile(g_kbProfiles[sel].profileJsonPath);
                    AudioEngine::GetInstance().PlayKey(VK_SPACE, true);
                }
            }
            break;

        case IDC_COMBO_MOUSE_PRESET:
            if (wmEvent == CBN_SELCHANGE) {
                int sel = (int)SendMessageW(g_hMousePresetCombo, CB_GETCURSEL, 0, 0);
                if (sel >= 0 && sel < (int)g_mouseProfiles.size()) {
                    AudioEngine::GetInstance().LoadMouseProfile(g_mouseProfiles[sel].profileJsonPath);
                    AudioEngine::GetInstance().PlayMouse("left", true);
                }
            }
            break;

        case IDC_BTN_KB_CUSTOM:
            ChooseCustomProfile(true);
            break;

        case IDC_BTN_MOUSE_CUSTOM:
            ChooseCustomProfile(false);
            break;

        case IDC_BTN_CHECK_UPDATES:
            MessageBoxW(hWnd, L"MonkeySounds is up to date!\n\nCurrent Version: 1.0.4", L"Check for Updates", MB_OK | MB_ICONINFORMATION);
            break;

        case IDC_BTN_APPLY:
            ApplySettingsFromUI();
            break;

        case IDC_BTN_OK:
            ApplySettingsFromUI();
            ShowWindow(hWnd, SW_HIDE);
            break;

        case IDC_BTN_CANCEL:
            LoadSettingsToUI();
            ShowWindow(hWnd, SW_HIDE);
            break;

        // Tray menu commands
        case IDM_TRAY_SHOW:
            ShowWindow(hWnd, SW_SHOW);
            SetForegroundWindow(hWnd);
            break;

        case IDM_TRAY_MUTE_KB: {
            bool nextState = !AudioEngine::GetInstance().IsKeyboardEnabled();
            AudioEngine::GetInstance().SetKeyboardEnabled(nextState);
            Button_SetCheck(g_hKbEnable, nextState ? BST_CHECKED : BST_UNCHECKED);
            break;
        }

        case IDM_TRAY_MUTE_MOUSE: {
            bool nextState = !AudioEngine::GetInstance().IsMouseEnabled();
            AudioEngine::GetInstance().SetMouseEnabled(nextState);
            Button_SetCheck(g_hMouseEnable, nextState ? BST_CHECKED : BST_UNCHECKED);
            break;
        }

        case IDM_TRAY_EXIT:
            DestroyWindow(hWnd);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        DrawCustomHeader(hWnd, hdc);
        DrawAboutTab(hWnd, hdc);
        EndPaint(hWnd, &ps);
        break;
    }

    case WM_DESTROY:
        KillTimer(hWnd, TIMER_CPU_ID);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
