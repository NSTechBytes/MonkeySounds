#include "framework.h"
#include "MonkeySounds.h"
#include "Resource.h"
#include "AudioEngine.h"
#include "InputHook.h"
#include "AppSettings.h"
#include "Utils.h"
#include "ZipUtils.h"
#include "ProfileWizard.h"
#include <commctrl.h>
#include <gdiplus.h>
#include <windowsx.h>
#include <shlobj.h>
#include <shellapi.h>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace fs = std::filesystem;

static std::wstring Utf8ToWide(const std::string& str) {
    if (str.empty()) return L"";
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

#define WINDOW_WIDTH  510
#define WINDOW_HEIGHT 410
#define TIMER_CPU_ID  1001
#define TIMER_VU_ID   1002

// VU visualizer state
#define NUM_VU_BARS   14
static float  g_vuLevels[NUM_VU_BARS]  = {};   // 0.0 – 1.0 current bar height
static float  g_vuPeak[NUM_VU_BARS]    = {};   // peak hold per bar
static float  g_vuDecay[NUM_VU_BARS]   = {};   // per-bar decay speed
static HWND   g_hVuWnd   = NULL;               // owner-draw static for the VU
static HBRUSH g_hVuBgBrush = NULL;

// Global Variables
HINSTANCE g_hInstance = NULL;
HWND g_hMainWnd = NULL;
HWND g_hTabCtrl = NULL;
HWND g_hStatusBar = NULL;
int g_activeTab = 0;

// CPU Monitor State
static ULONGLONG g_lastIdleTime = 0;
static ULONGLONG g_lastKernelTime = 0;
static ULONGLONG g_lastUserTime = 0;

// Fonts
HFONT g_hFontNormal = NULL;
HFONT g_hFontBold = NULL;
HFONT g_hFontTitle = NULL;
HFONT g_hFontMono = NULL;
HFONT g_hFontIcon = NULL;
HBRUSH g_hTabBgBrush = NULL;

// Control HWNDs - Sounds Tab
HWND g_hKbGroup = NULL;
HWND g_hKbEnable = NULL;
HWND g_hKbNewBtn = NULL;
HWND g_hKbPresetLbl = NULL;
HWND g_hKbPresetCombo = NULL;
HWND g_hKbCustomBtn = NULL;
HWND g_hKbTestBtn = NULL;
HWND g_hKbExportBtn = NULL;
HWND g_hKbFavBtn = NULL;
HWND g_hKbInfoBtn = NULL;
HWND g_hKbVolLbl = NULL;
HWND g_hKbVolSlider = NULL;

HWND g_hMouseGroup = NULL;
HWND g_hMouseEnable = NULL;
HWND g_hMouseNewBtn = NULL;
HWND g_hMousePresetLbl = NULL;
HWND g_hMousePresetCombo = NULL;
HWND g_hMouseCustomBtn = NULL;
HWND g_hMouseTestBtn = NULL;
HWND g_hMouseExportBtn = NULL;
HWND g_hMouseFavBtn = NULL;
HWND g_hMouseInfoBtn = NULL;
HWND g_hMouseVolLbl = NULL;
HWND g_hMouseVolSlider = NULL;

// Control HWNDs - Settings Tab
HWND g_hAppSettingsGroup = NULL;
HWND g_hVersionLbl = NULL;
HWND g_hCheckUpdatesBtn = NULL;
HWND g_hSeparator = NULL;
HWND g_hAutoStartChk = NULL;
HWND g_hStartupNotifChk = NULL;

// Control HWNDs - About Tab
HWND g_hAboutLogo = NULL;
HWND g_hAboutTitle = NULL;
HWND g_hAboutDesc1 = NULL;
HWND g_hAboutDesc2 = NULL;
HWND g_hAboutCopy = NULL;

// Profiles lists
std::vector<SoundProfileInfo> g_kbProfiles;
std::vector<SoundProfileInfo> g_mouseProfiles;

// GDI+ Image for About page
Gdiplus::Image* g_pMonkeySoundsImage = NULL;
HICON g_hAppIcon = NULL;



// Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void CreateControls(HWND hWnd);
void UpdateTabVisibility(int tabIndex);
void PopulatePresets();
void SaveCurrentSettings();
void LoadSettingsToUI();
void UpdateCpuUsage();
void ShowTrayMenu(HWND hWnd);
void ChooseCustomProfile(bool isKeyboard);
void ExportCurrentProfile(bool isKeyboard);
void TestCurrentProfile(bool isKeyboard);
void ToggleFavorite(bool isKeyboard);
void UpdateFavoriteButton(bool isKeyboard);
void ShowProfileInfo(bool isKeyboard);
void VuPulse();      // spike the VU on a keypress/click
void VuDecay();      // called each timer tick to decay bar levels
void VuDraw(HDC hdc, const RECT& rc); // paint the VU bars

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int nCmdShow) {
    // Ensure only a single instance of the application runs
    HANDLE hSingleInstanceMutex = CreateMutexW(NULL, TRUE, L"Local\\MonkeySounds_SingleInstance_Mutex_9A8B");
    if (GetLastError() == ERROR_ALREADY_EXISTS || hSingleInstanceMutex == NULL) {
        HWND hExistingWnd = FindWindowW(L"MonkeySoundsMainWindow", NULL);
        if (hExistingWnd) {
            ShowWindow(hExistingWnd, SW_SHOW);
            SetForegroundWindow(hExistingWnd);
        }
        if (hSingleInstanceMutex) {
            CloseHandle(hSingleInstanceMutex);
        }
        return 0;
    }

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
    std::wstring iconPath = Utils::GetAssetPath(L"Icon.ico");
    g_hAppIcon = (HICON)LoadImageW(hInstance, MAKEINTRESOURCEW(IDI_APP_ICON), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
    if (!g_hAppIcon && fs::exists(iconPath)) {
            g_hAppIcon = (HICON)LoadImageW(NULL, iconPath.c_str(), IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
    }

    // Load PNG logo
    std::wstring logoPath = Utils::GetAssetPath(L"MonkeySounds.png");
    if (fs::exists(logoPath)) {
        g_pMonkeySoundsImage = Gdiplus::Image::FromFile(logoPath.c_str());
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
    // Segoe MDL2 Assets — used for icon-only buttons (play ▶ U+E102, info ⓘ U+E946)
    g_hFontIcon = CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe MDL2 Assets");

    g_hTabBgBrush = CreateSolidBrush(RGB(249, 249, 249));

    // Register Window Class
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = g_hAppIcon;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = g_hTabBgBrush;
    wcex.lpszClassName = L"MonkeySoundsMainWindow";
    wcex.hIconSm = g_hAppIcon;
    RegisterClassExW(&wcex);

    // Calculate window rectangle at bottom-right of the usable screen work area
    RECT rcWorkArea;
    if (!SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, 0)) {
        rcWorkArea.left = 0;
        rcWorkArea.top = 0;
        rcWorkArea.right = GetSystemMetrics(SM_CXSCREEN);
        rcWorkArea.bottom = GetSystemMetrics(SM_CYSCREEN);
    }
    int posX = rcWorkArea.right - WINDOW_WIDTH - 12;
    int posY = rcWorkArea.bottom - WINDOW_HEIGHT - 12;

    HWND hWnd = CreateWindowExW(
        WS_EX_APPWINDOW,
        L"MonkeySoundsMainWindow",
        L"MonkeySounds",
        WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN,
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

    // Start automatically hidden to tray
    ShowWindow(hWnd, SW_HIDE);
    UpdateWindow(hWnd);

    // Show startup balloon notification if enabled
    if (AppSettings::GetInstance().GetConfig().showStartupNotification) {
        NOTIFYICONDATA nidBalloon = {};
        nidBalloon.cbSize = sizeof(NOTIFYICONDATA);
        nidBalloon.hWnd = hWnd;
        nidBalloon.uID = 1;
        nidBalloon.uFlags = NIF_INFO;
        nidBalloon.dwInfoFlags = NIIF_INFO | NIIF_NOSOUND;
        nidBalloon.uTimeout = 4000;
        wcscpy_s(nidBalloon.szInfoTitle, L"MonkeySounds is running");
        wcscpy_s(nidBalloon.szInfo,
            L"Keyboard & mouse sounds are active.\n"
            L"Right-click the tray icon to mute or open settings.");
        Shell_NotifyIconW(NIM_MODIFY, &nidBalloon);
    }

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
    if (g_hFontIcon) DeleteObject(g_hFontIcon);
    if (g_hTabBgBrush) DeleteObject(g_hTabBgBrush);

    if (hSingleInstanceMutex) {
        ReleaseMutex(hSingleInstanceMutex);
        CloseHandle(hSingleInstanceMutex);
    }

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
        8, 6, WINDOW_WIDTH - 30, 310,
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
        16, 36, WINDOW_WIDTH - 50, 130,
        hWnd, (HMENU)IDC_GB_KEYBOARD, g_hInstance, NULL
    );
    SetControlFont(g_hKbGroup, g_hFontBold);

    g_hKbEnable = CreateWindowExW(
        0, L"BUTTON", L"Enable Keyboard Sounds",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        28, 56, 180, 20,
        hWnd, (HMENU)IDC_CHK_KB_ENABLE, g_hInstance, NULL
    );
    SetControlFont(g_hKbEnable, g_hFontNormal);

    g_hKbNewBtn = CreateWindowExW(
        0, L"BUTTON", L"+ New Profile...",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        WINDOW_WIDTH - 140, 53, 104, 23,
        hWnd, (HMENU)IDC_BTN_KB_NEW, g_hInstance, NULL
    );
    SetControlFont(g_hKbNewBtn, g_hFontNormal);

    g_hKbPresetLbl = CreateWindowExW(
        0, L"STATIC", L"Preset:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        28, 83, 44, 20,
        hWnd, (HMENU)IDC_LBL_KB_PRESET, g_hInstance, NULL
    );
    SetControlFont(g_hKbPresetLbl, g_hFontNormal);

    g_hKbPresetCombo = CreateWindowExW(
        0, L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
        76, 80, WINDOW_WIDTH - 346, 160,
        hWnd, (HMENU)IDC_COMBO_KB_PRESET, g_hInstance, NULL
    );
    SetControlFont(g_hKbPresetCombo, g_hFontNormal);

    // Favourite toggle button (★ / ☆)
    g_hKbFavBtn = CreateWindowExW(
        0, L"BUTTON", L"\u2606",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        WINDOW_WIDTH - 266, 79, 24, 24,
        hWnd, (HMENU)IDC_BTN_KB_FAVORITE, g_hInstance, NULL
    );
    SetControlFont(g_hKbFavBtn, g_hFontNormal);

    // Info button
    g_hKbInfoBtn = CreateWindowExW(
        0, L"BUTTON", L"\uE946",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        WINDOW_WIDTH - 238, 79, 24, 24,
        hWnd, (HMENU)IDC_BTN_KB_INFO, g_hInstance, NULL
    );
    SetControlFont(g_hKbInfoBtn, g_hFontIcon);

    // Play button
    g_hKbTestBtn = CreateWindowExW(
        0, L"BUTTON", L"\u25B6",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        WINDOW_WIDTH - 210, 79, 28, 24,
        hWnd, (HMENU)IDC_BTN_KB_TEST, g_hInstance, NULL
    );
    SetControlFont(g_hKbTestBtn, g_hFontNormal);

    // Export button
    g_hKbExportBtn = CreateWindowExW(
        0, L"BUTTON", L"Export",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        WINDOW_WIDTH - 178, 79, 54, 24,
        hWnd, (HMENU)IDC_BTN_KB_EXPORT, g_hInstance, NULL
    );
    SetControlFont(g_hKbExportBtn, g_hFontNormal);

    // Import ZIP button
    g_hKbCustomBtn = CreateWindowExW(
        0, L"BUTTON", L"Import ZIP...",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        WINDOW_WIDTH - 120, 79, 84, 24,
        hWnd, (HMENU)IDC_BTN_KB_CUSTOM, g_hInstance, NULL
    );
    SetControlFont(g_hKbCustomBtn, g_hFontNormal);

    g_hKbVolLbl = CreateWindowExW(
        0, L"STATIC", L"Volume:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        28, 116, 50, 20,
        hWnd, (HMENU)IDC_LBL_KB_VOLUME, g_hInstance, NULL
    );
    SetControlFont(g_hKbVolLbl, g_hFontNormal);

    g_hKbVolSlider = CreateWindowExW(
        0, TRACKBAR_CLASS, L"",
        WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_NOTICKS | WS_TABSTOP,
        80, 114, WINDOW_WIDTH - 126, 24,
        hWnd, (HMENU)IDC_SLIDER_KB_VOLUME, g_hInstance, NULL
    );
    SendMessageW(g_hKbVolSlider, TBM_SETRANGE, TRUE, MAKELONG(0, 100));

    // Mouse Group
    g_hMouseGroup = CreateWindowExW(
        0, L"BUTTON", L"Mouse Sounds",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        16, 173, WINDOW_WIDTH - 50, 130,
        hWnd, (HMENU)IDC_GB_MOUSE, g_hInstance, NULL
    );
    SetControlFont(g_hMouseGroup, g_hFontBold);

    g_hMouseEnable = CreateWindowExW(
        0, L"BUTTON", L"Enable Mouse Sounds",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        28, 193, 180, 20,
        hWnd, (HMENU)IDC_CHK_MOUSE_ENABLE, g_hInstance, NULL
    );
    SetControlFont(g_hMouseEnable, g_hFontNormal);

    g_hMouseNewBtn = CreateWindowExW(
        0, L"BUTTON", L"+ New Profile...",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        WINDOW_WIDTH - 140, 190, 104, 23,
        hWnd, (HMENU)IDC_BTN_MOUSE_NEW, g_hInstance, NULL
    );
    SetControlFont(g_hMouseNewBtn, g_hFontNormal);

    g_hMousePresetLbl = CreateWindowExW(
        0, L"STATIC", L"Preset:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        28, 220, 44, 20,
        hWnd, (HMENU)IDC_LBL_MOUSE_PRESET, g_hInstance, NULL
    );
    SetControlFont(g_hMousePresetLbl, g_hFontNormal);

    g_hMousePresetCombo = CreateWindowExW(
        0, L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
        76, 217, WINDOW_WIDTH - 346, 160,
        hWnd, (HMENU)IDC_COMBO_MOUSE_PRESET, g_hInstance, NULL
    );
    SetControlFont(g_hMousePresetCombo, g_hFontNormal);

    // Favourite toggle button (★ / ☆)
    g_hMouseFavBtn = CreateWindowExW(
        0, L"BUTTON", L"\u2606",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        WINDOW_WIDTH - 266, 216, 24, 24,
        hWnd, (HMENU)IDC_BTN_MOUSE_FAVORITE, g_hInstance, NULL
    );
    SetControlFont(g_hMouseFavBtn, g_hFontNormal);

    // Info button
    g_hMouseInfoBtn = CreateWindowExW(
        0, L"BUTTON", L"\uE946",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        WINDOW_WIDTH - 238, 216, 24, 24,
        hWnd, (HMENU)IDC_BTN_MOUSE_INFO, g_hInstance, NULL
    );
    SetControlFont(g_hMouseInfoBtn, g_hFontIcon);

    // Play button
    g_hMouseTestBtn = CreateWindowExW(
        0, L"BUTTON", L"\u25B6",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        WINDOW_WIDTH - 210, 216, 28, 24,
        hWnd, (HMENU)IDC_BTN_MOUSE_TEST, g_hInstance, NULL
    );
    SetControlFont(g_hMouseTestBtn, g_hFontNormal);

    // Export button
    g_hMouseExportBtn = CreateWindowExW(
        0, L"BUTTON", L"Export",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        WINDOW_WIDTH - 178, 216, 54, 24,
        hWnd, (HMENU)IDC_BTN_MOUSE_EXPORT, g_hInstance, NULL
    );
    SetControlFont(g_hMouseExportBtn, g_hFontNormal);

    // Import ZIP button
    g_hMouseCustomBtn = CreateWindowExW(
        0, L"BUTTON", L"Import ZIP...",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        WINDOW_WIDTH - 120, 216, 84, 24,
        hWnd, (HMENU)IDC_BTN_MOUSE_CUSTOM, g_hInstance, NULL
    );
    SetControlFont(g_hMouseCustomBtn, g_hFontNormal);

    g_hMouseVolLbl = CreateWindowExW(
        0, L"STATIC", L"Volume:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        28, 253, 50, 20,
        hWnd, (HMENU)IDC_LBL_MOUSE_VOLUME, g_hInstance, NULL
    );
    SetControlFont(g_hMouseVolLbl, g_hFontNormal);

    g_hMouseVolSlider = CreateWindowExW(
        0, TRACKBAR_CLASS, L"",
        WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_NOTICKS | WS_TABSTOP,
        80, 251, WINDOW_WIDTH - 126, 24,
        hWnd, (HMENU)IDC_SLIDER_MOUSE_VOLUME, g_hInstance, NULL
    );
    SendMessageW(g_hMouseVolSlider, TBM_SETRANGE, TRUE, MAKELONG(0, 100));

    // --- TAB 2: SETTINGS ---
    g_hAppSettingsGroup = CreateWindowExW(
        0, L"BUTTON", L"Application",
        WS_CHILD | BS_GROUPBOX,
        16, 36, WINDOW_WIDTH - 50, 237,
        hWnd, (HMENU)IDC_GB_APPLICATION, g_hInstance, NULL
    );
    SetControlFont(g_hAppSettingsGroup, g_hFontBold);

    g_hVersionLbl = CreateWindowExW(
        0, L"STATIC", (L"Current Version: " + Utils::GetAppVersion()).c_str(),
        WS_CHILD | SS_LEFT,
        28, 60, 200, 20,
        hWnd, (HMENU)IDC_LBL_VERSION, g_hInstance, NULL
    );
    SetControlFont(g_hVersionLbl, g_hFontNormal);

    g_hCheckUpdatesBtn = CreateWindowExW(
        0, L"BUTTON", L"Check for Updates",
        WS_CHILD | BS_PUSHBUTTON,
        28, 86, 130, 26,
        hWnd, (HMENU)IDC_BTN_CHECK_UPDATES, g_hInstance, NULL
    );
    SetControlFont(g_hCheckUpdatesBtn, g_hFontNormal);

    g_hSeparator = CreateWindowExW(
        0, L"STATIC", L"",
        WS_CHILD | SS_ETCHEDHORZ,
        28, 130, WINDOW_WIDTH - 74, 2,
        hWnd, (HMENU)-1, g_hInstance, NULL
    );

    g_hAutoStartChk = CreateWindowExW(
        0, L"BUTTON", L"Auto-start on Windows startup",
        WS_CHILD | BS_AUTOCHECKBOX,
        28, 142, 240, 20,
        hWnd, (HMENU)IDC_CHK_AUTOSTART, g_hInstance, NULL
    );
    SetControlFont(g_hAutoStartChk, g_hFontNormal);

    g_hStartupNotifChk = CreateWindowExW(
        0, L"BUTTON", L"Show notification on startup",
        WS_CHILD | BS_AUTOCHECKBOX,
        28, 167, 240, 20,
        hWnd, (HMENU)IDC_CHK_STARTUP_NOTIF, g_hInstance, NULL
    );
    SetControlFont(g_hStartupNotifChk, g_hFontNormal);

    // --- TAB 3: ABOUT ---
    g_hAboutLogo = CreateWindowExW(
        0, L"STATIC", L"",
        WS_CHILD | SS_OWNERDRAW,
        (WINDOW_WIDTH - 88) / 2, 45, 88, 88,
        hWnd, (HMENU)IDC_STATIC_ABOUT_LOGO, g_hInstance, NULL
    );

    g_hAboutTitle = CreateWindowExW(
        0, L"STATIC", L"MonkeySounds",
        WS_CHILD | SS_CENTER,
        20, 140, WINDOW_WIDTH - 40, 22,
        hWnd, (HMENU)IDC_STATIC_ABOUT_TITLE, g_hInstance, NULL
    );
    SetControlFont(g_hAboutTitle, g_hFontTitle);

    g_hAboutDesc1 = CreateWindowExW(
        0, L"STATIC", L"Professional system sound",
        WS_CHILD | SS_CENTER,
        20, 165, WINDOW_WIDTH - 40, 18,
        hWnd, (HMENU)IDC_STATIC_ABOUT_DESC1, g_hInstance, NULL
    );
    SetControlFont(g_hAboutDesc1, g_hFontNormal);

    g_hAboutDesc2 = CreateWindowExW(
        0, L"STATIC", L"customization utility.",
        WS_CHILD | SS_CENTER,
        20, 183, WINDOW_WIDTH - 40, 18,
        hWnd, (HMENU)IDC_STATIC_ABOUT_DESC2, g_hInstance, NULL
    );
    SetControlFont(g_hAboutDesc2, g_hFontNormal);

    g_hAboutCopy = CreateWindowExW(
        0, L"STATIC", L"\u00A9 2026 MonkeySounds. All rights reserved.",
        WS_CHILD | SS_CENTER,
        20, 215, WINDOW_WIDTH - 40, 18,
        hWnd, (HMENU)IDC_STATIC_ABOUT_COPY, g_hInstance, NULL
    );
    SetControlFont(g_hAboutCopy, g_hFontMono);

    // --- Status Bar ---
    g_hStatusBar = CreateWindowExW(
        0, STATUSCLASSNAME, L"",
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0,
        hWnd, (HMENU)IDC_STATUSBAR, g_hInstance, NULL
    );
    SetControlFont(g_hStatusBar, g_hFontMono);

    // 3 parts: [left text (160px) | VU meter | CPU % (fixed 90px right pane)]
    // -1 means the last pane stretches to the window edge.
    // We anchor the second pane so the CPU pane is always >= 90px wide.
    int statwidths[] = { 160, WINDOW_WIDTH - 90, -1 };
    SendMessageW(g_hStatusBar, SB_SETPARTS, 3, (LPARAM)statwidths);
    SendMessageW(g_hStatusBar, SB_SETTEXTW, 0, (LPARAM)L"  Ready");
    SendMessageW(g_hStatusBar, SB_SETTEXTW, 2, (LPARAM)L" CPU: 0%");

    // VU meter — owner-draw static that lives over the middle status bar pane
    g_hVuBgBrush = CreateSolidBrush(RGB(30,30,30));
    g_hVuWnd = CreateWindowExW(
        0, L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
        163, 0, WINDOW_WIDTH - 88 - 163, 20,   // positioned over middle pane; y/h adjusted after SB sizes
        hWnd, (HMENU)IDC_VU_METER, g_hInstance, NULL
    );
}

void UpdateTabVisibility(int tabIndex) {
    g_activeTab = tabIndex;

    // Tab 0: Sounds
    int showSounds = (tabIndex == 0) ? SW_SHOW : SW_HIDE;
    ShowWindow(g_hKbGroup, showSounds);
    ShowWindow(g_hKbEnable, showSounds);
    ShowWindow(g_hKbNewBtn, showSounds);
    ShowWindow(g_hKbPresetLbl, showSounds);
    ShowWindow(g_hKbPresetCombo, showSounds);
    ShowWindow(g_hKbTestBtn, showSounds);
    ShowWindow(g_hKbInfoBtn, showSounds);
    ShowWindow(g_hKbExportBtn, showSounds);
    ShowWindow(g_hKbFavBtn, showSounds);
    ShowWindow(g_hKbCustomBtn, showSounds);
    ShowWindow(g_hKbVolLbl, showSounds);
    ShowWindow(g_hKbVolSlider, showSounds);

    ShowWindow(g_hMouseGroup, showSounds);
    ShowWindow(g_hMouseEnable, showSounds);
    ShowWindow(g_hMouseNewBtn, showSounds);
    ShowWindow(g_hMousePresetLbl, showSounds);
    ShowWindow(g_hMousePresetCombo, showSounds);
    ShowWindow(g_hMouseTestBtn, showSounds);
    ShowWindow(g_hMouseInfoBtn, showSounds);
    ShowWindow(g_hMouseExportBtn, showSounds);
    ShowWindow(g_hMouseFavBtn, showSounds);
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
    ShowWindow(g_hStartupNotifChk, showSettings);

    // Tab 2: About
    int showAbout = (tabIndex == 2) ? SW_SHOW : SW_HIDE;
    ShowWindow(g_hAboutLogo, showAbout);
    ShowWindow(g_hAboutTitle, showAbout);
    ShowWindow(g_hAboutDesc1, showAbout);
    ShowWindow(g_hAboutDesc2, showAbout);
    ShowWindow(g_hAboutCopy, showAbout);

    if (showAbout == SW_SHOW) {
        SetWindowPos(g_hAboutLogo, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        SetWindowPos(g_hAboutTitle, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        SetWindowPos(g_hAboutDesc1, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        SetWindowPos(g_hAboutDesc2, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        SetWindowPos(g_hAboutCopy, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        InvalidateRect(g_hAboutLogo, NULL, TRUE);
        InvalidateRect(g_hAboutTitle, NULL, TRUE);
        InvalidateRect(g_hAboutDesc1, NULL, TRUE);
        InvalidateRect(g_hAboutDesc2, NULL, TRUE);
        InvalidateRect(g_hAboutCopy, NULL, TRUE);
    }

    // Trigger repaint
    InvalidateRect(g_hMainWnd, NULL, TRUE);
}

void PopulatePresets() {
    const auto& cfg = AppSettings::GetInstance().GetConfig();

    // ---- Keyboard ----
    SendMessageW(g_hKbPresetCombo, CB_RESETCONTENT, 0, 0);

    std::wstring currentKbPath = AudioEngine::GetInstance().GetCurrentKeyboardProfilePath();
    int selKbIndex = 0;
    int comboIdx = 0;

    // Favorites first (★ prefix)
    for (size_t i = 0; i < g_kbProfiles.size(); ++i) {
        const auto& p = g_kbProfiles[i];
        bool isFav = false;
        for (auto& f : cfg.kbFavorites)
            if (_wcsicmp(f.c_str(), p.profileJsonPath.c_str()) == 0) { isFav = true; break; }
        if (!isFav) continue;

        std::wstring label = L"\u2605 " + Utils::Utf8ToWide(p.name);
        SendMessageW(g_hKbPresetCombo, CB_ADDSTRING, 0, (LPARAM)label.c_str());
        // store original index so we can retrieve the profile later
        SendMessageW(g_hKbPresetCombo, CB_SETITEMDATA, comboIdx, (LPARAM)i);
        if (!currentKbPath.empty() && _wcsicmp(currentKbPath.c_str(), p.profileJsonPath.c_str()) == 0)
            selKbIndex = comboIdx;
        ++comboIdx;
    }

    // Rest (non-favorites)
    for (size_t i = 0; i < g_kbProfiles.size(); ++i) {
        const auto& p = g_kbProfiles[i];
        bool isFav = false;
        for (auto& f : cfg.kbFavorites)
            if (_wcsicmp(f.c_str(), p.profileJsonPath.c_str()) == 0) { isFav = true; break; }
        if (isFav) continue;

        std::wstring label = Utils::Utf8ToWide(p.name);
        SendMessageW(g_hKbPresetCombo, CB_ADDSTRING, 0, (LPARAM)label.c_str());
        SendMessageW(g_hKbPresetCombo, CB_SETITEMDATA, comboIdx, (LPARAM)i);
        if (!currentKbPath.empty() && _wcsicmp(currentKbPath.c_str(), p.profileJsonPath.c_str()) == 0)
            selKbIndex = comboIdx;
        ++comboIdx;
    }
    SendMessageW(g_hKbPresetCombo, CB_SETCURSEL, selKbIndex, 0);

    // ---- Mouse ----
    SendMessageW(g_hMousePresetCombo, CB_RESETCONTENT, 0, 0);

    std::wstring currentMousePath = AudioEngine::GetInstance().GetCurrentMouseProfilePath();
    int selMouseIndex = 0;
    comboIdx = 0;

    // Favorites first
    for (size_t i = 0; i < g_mouseProfiles.size(); ++i) {
        const auto& p = g_mouseProfiles[i];
        bool isFav = false;
        for (auto& f : cfg.mouseFavorites)
            if (_wcsicmp(f.c_str(), p.profileJsonPath.c_str()) == 0) { isFav = true; break; }
        if (!isFav) continue;

        std::wstring label = L"\u2605 " + Utils::Utf8ToWide(p.name);
        SendMessageW(g_hMousePresetCombo, CB_ADDSTRING, 0, (LPARAM)label.c_str());
        SendMessageW(g_hMousePresetCombo, CB_SETITEMDATA, comboIdx, (LPARAM)i);
        if (!currentMousePath.empty() && _wcsicmp(currentMousePath.c_str(), p.profileJsonPath.c_str()) == 0)
            selMouseIndex = comboIdx;
        ++comboIdx;
    }

    // Rest
    for (size_t i = 0; i < g_mouseProfiles.size(); ++i) {
        const auto& p = g_mouseProfiles[i];
        bool isFav = false;
        for (auto& f : cfg.mouseFavorites)
            if (_wcsicmp(f.c_str(), p.profileJsonPath.c_str()) == 0) { isFav = true; break; }
        if (isFav) continue;

        std::wstring label = Utils::Utf8ToWide(p.name);
        SendMessageW(g_hMousePresetCombo, CB_ADDSTRING, 0, (LPARAM)label.c_str());
        SendMessageW(g_hMousePresetCombo, CB_SETITEMDATA, comboIdx, (LPARAM)i);
        if (!currentMousePath.empty() && _wcsicmp(currentMousePath.c_str(), p.profileJsonPath.c_str()) == 0)
            selMouseIndex = comboIdx;
        ++comboIdx;
    }
    SendMessageW(g_hMousePresetCombo, CB_SETCURSEL, selMouseIndex, 0);

    // Sync the star buttons to reflect current selection
    UpdateFavoriteButton(true);
    UpdateFavoriteButton(false);
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
    Button_SetCheck(g_hStartupNotifChk, config.showStartupNotification ? BST_CHECKED : BST_UNCHECKED);

    PopulatePresets(); // also calls UpdateFavoriteButton for both
}

void SaveCurrentSettings() {
    auto& cfg = AppSettings::GetInstance().GetConfig();
    cfg.keyboardEnabled = (Button_GetCheck(g_hKbEnable) == BST_CHECKED);
    cfg.mouseEnabled = (Button_GetCheck(g_hMouseEnable) == BST_CHECKED);
    cfg.keyboardVolume = (float)SendMessageW(g_hKbVolSlider, TBM_GETPOS, 0, 0) / 100.0f;
    cfg.mouseVolume = (float)SendMessageW(g_hMouseVolSlider, TBM_GETPOS, 0, 0) / 100.0f;
    cfg.keyboardProfilePath = AudioEngine::GetInstance().GetCurrentKeyboardProfilePath();
    cfg.mouseProfilePath = AudioEngine::GetInstance().GetCurrentMouseProfilePath();
    cfg.autoStart = (Button_GetCheck(g_hAutoStartChk) == BST_CHECKED);
    cfg.showStartupNotification = (Button_GetCheck(g_hStartupNotifChk) == BST_CHECKED);
    AppSettings::GetInstance().Save();
}

void ChooseCustomProfile(bool isKeyboard) {
    WCHAR szFile[MAX_PATH] = {};
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = g_hMainWnd;
    ofn.lpstrFilter = L"Sound Pack ZIP (*.zip)\0*.zip\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrTitle = isKeyboard ? L"Import Keyboard Sound Pack (.zip)"
                                : L"Import Mouse Sound Pack (.zip)";

    if (!GetOpenFileNameW(&ofn)) return;

    // Determine destination: Keyboard or Mouse sounds directory
    std::wstring soundsRoot = Utils::GetSoundsDirectory();
    std::wstring subDir     = isKeyboard ? L"Keyboard\\Custom" : L"Mouse\\Custom";
    std::wstring destBase   = (fs::path(soundsRoot) / subDir).wstring();

    // Use the zip filename (without extension) as the profile folder name
    std::wstring zipName = fs::path(szFile).stem().wstring();
    std::wstring destDir = (fs::path(destBase) / zipName).wstring();

    // Extract
    SendMessageW(g_hStatusBar, SB_SETTEXTW, 0, (LPARAM)L"  Importing sound pack...");
    if (!ZipUtils::ExtractZip(szFile, destDir)) {
        MessageBoxW(g_hMainWnd,
            L"Failed to extract the ZIP file.\nMake sure it contains a valid profile.json.",
            L"Import Error", MB_OK | MB_ICONERROR);
        SendMessageW(g_hStatusBar, SB_SETTEXTW, 0, (LPARAM)L"  Import failed.");
        return;
    }

    // Find profile.json inside extracted folder
    std::wstring profileJson = (fs::path(destDir) / L"profile.json").wstring();
    if (!fs::exists(profileJson)) {
        // Try one level deeper (zip might have a subfolder)
        for (const auto& entry : fs::directory_iterator(destDir)) {
            if (entry.is_directory()) {
                std::wstring candidate = (entry.path() / L"profile.json").wstring();
                if (fs::exists(candidate)) { profileJson = candidate; break; }
            }
        }
    }

    if (!fs::exists(profileJson)) {
        MessageBoxW(g_hMainWnd,
            L"No profile.json found in the extracted ZIP.\n"
            L"The sound pack must contain a profile.json at its root.",
            L"Import Error", MB_OK | MB_ICONERROR);
        SendMessageW(g_hStatusBar, SB_SETTEXTW, 0, (LPARAM)L"  Import failed — no profile.json.");
        return;
    }

    // Load the profile
    bool loaded = false;
    if (isKeyboard) {
        loaded = AudioEngine::GetInstance().LoadKeyboardProfile(profileJson);
        if (loaded) {
            g_kbProfiles = AudioEngine::GetInstance().ScanKeyboardProfiles();
            PopulatePresets();
            AudioEngine::GetInstance().PlayKey(VK_SPACE, true);
            SaveCurrentSettings();
        }
    } else {
        loaded = AudioEngine::GetInstance().LoadMouseProfile(profileJson);
        if (loaded) {
            g_mouseProfiles = AudioEngine::GetInstance().ScanMouseProfiles();
            PopulatePresets();
            AudioEngine::GetInstance().PlayMouse("left", true);
            SaveCurrentSettings();
        }
    }

    if (loaded) {
        SendMessageW(g_hStatusBar, SB_SETTEXTW, 0,
            (LPARAM)(std::wstring(L"  Imported: ") + zipName).c_str());
    } else {
        MessageBoxW(g_hMainWnd,
            L"The profile.json was found but could not be loaded.\n"
            L"Please check that the file is valid.",
            L"Import Error", MB_OK | MB_ICONERROR);
        SendMessageW(g_hStatusBar, SB_SETTEXTW, 0, (LPARAM)L"  Import failed — invalid profile.");
    }
}

void ExportCurrentProfile(bool isKeyboard) {
    std::wstring profilePath = isKeyboard
        ? AudioEngine::GetInstance().GetCurrentKeyboardProfilePath()
        : AudioEngine::GetInstance().GetCurrentMouseProfilePath();

    if (profilePath.empty() || !fs::exists(profilePath)) {
        MessageBoxW(g_hMainWnd, L"No profile is currently loaded.",
                    L"Export", MB_OK | MB_ICONWARNING);
        return;
    }

    std::wstring profileDir = fs::path(profilePath).parent_path().wstring();
    std::wstring profileName = fs::path(profileDir).filename().wstring();
    std::wstring defaultFile = profileName + L".zip";

    WCHAR szFile[MAX_PATH] = {};
    wcscpy_s(szFile, defaultFile.c_str());

    OPENFILENAMEW ofn = {};
    ofn.lStructSize  = sizeof(OPENFILENAMEW);
    ofn.hwndOwner    = g_hMainWnd;
    ofn.lpstrFilter  = L"ZIP Archive (*.zip)\0*.zip\0";
    ofn.lpstrFile    = szFile;
    ofn.nMaxFile     = MAX_PATH;
    ofn.lpstrDefExt  = L"zip";
    ofn.Flags        = OFN_EXPLORER | OFN_OVERWRITEPROMPT;
    ofn.lpstrTitle   = isKeyboard ? L"Export Keyboard Sound Pack"
                                  : L"Export Mouse Sound Pack";

    if (!GetSaveFileNameW(&ofn)) return;

    SendMessageW(g_hStatusBar, SB_SETTEXTW, 0, (LPARAM)L"  Exporting sound pack...");

    if (ZipUtils::CreateZipFromDir(profileDir, szFile)) {
        SendMessageW(g_hStatusBar, SB_SETTEXTW, 0,
            (LPARAM)(std::wstring(L"  Exported: ") + fs::path(szFile).filename().wstring()).c_str());
    } else {
        MessageBoxW(g_hMainWnd, L"Failed to create ZIP archive.",
                    L"Export Error", MB_OK | MB_ICONERROR);
        SendMessageW(g_hStatusBar, SB_SETTEXTW, 0, (LPARAM)L"  Export failed.");
    }
}

void UpdateFavoriteButton(bool isKeyboard) {
    const auto& cfg = AppSettings::GetInstance().GetConfig();
    const auto& favList = isKeyboard ? cfg.kbFavorites : cfg.mouseFavorites;
    HWND hBtn = isKeyboard ? g_hKbFavBtn : g_hMouseFavBtn;
    if (!hBtn) return;

    std::wstring currentPath = isKeyboard
        ? AudioEngine::GetInstance().GetCurrentKeyboardProfilePath()
        : AudioEngine::GetInstance().GetCurrentMouseProfilePath();

    bool isFav = false;
    for (auto& f : favList)
        if (_wcsicmp(f.c_str(), currentPath.c_str()) == 0) { isFav = true; break; }

    // ★ filled = favorited, ☆ hollow = not favorited
    SetWindowTextW(hBtn, isFav ? L"\u2605" : L"\u2606");
}

void ToggleFavorite(bool isKeyboard) {
    auto& cfg = AppSettings::GetInstance().GetConfig();
    auto& favList = isKeyboard ? cfg.kbFavorites : cfg.mouseFavorites;

    std::wstring currentPath = isKeyboard
        ? AudioEngine::GetInstance().GetCurrentKeyboardProfilePath()
        : AudioEngine::GetInstance().GetCurrentMouseProfilePath();

    if (currentPath.empty()) return;

    // Check if already a favorite
    auto it = std::find_if(favList.begin(), favList.end(),
        [&](const std::wstring& f) {
            return _wcsicmp(f.c_str(), currentPath.c_str()) == 0;
        });

    if (it != favList.end()) {
        // Remove from favorites
        favList.erase(it);
        SendMessageW(g_hStatusBar, SB_SETTEXTW, 0, (LPARAM)L"  Removed from favorites.");
    } else {
        // Add to favorites
        favList.push_back(currentPath);
        SendMessageW(g_hStatusBar, SB_SETTEXTW, 0, (LPARAM)L"  Added to favorites! \u2605");
    }

    AppSettings::GetInstance().Save();

    // Repopulate so the ★ prefix and ordering update immediately
    if (isKeyboard)
        g_kbProfiles = AudioEngine::GetInstance().ScanKeyboardProfiles();
    else
        g_mouseProfiles = AudioEngine::GetInstance().ScanMouseProfiles();

    PopulatePresets();
}

void ShowProfileInfo(bool isKeyboard) {
    std::string name, author, description;
    int soundCount = 0;

    if (isKeyboard) {
        const auto& p = AudioEngine::GetInstance().GetCurrentKeyboardProfile();
        name        = p.name;
        author      = p.author;
        description = p.description;
        soundCount  = (int)p.sources.size();
    } else {
        const auto& p = AudioEngine::GetInstance().GetCurrentMouseProfile();
        name        = p.name;
        author      = p.author;
        description = p.description;
        soundCount  = (int)p.sources.size();
    }

    // Build info string (wide)
    auto toW = [](const std::string& s) -> std::wstring {
        if (s.empty()) return L"—";
        int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
        std::wstring w(n, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &w[0], n);
        if (!w.empty() && w.back() == L'\0') w.pop_back();
        return w;
    };

    std::wstring wName   = toW(name);
    std::wstring wAuthor = toW(author);
    std::wstring wDesc   = toW(description);

    WCHAR buf[1024];
    swprintf_s(buf,
        L"Name:         %s\n"
        L"Author:       %s\n"
        L"Sounds:       %d\n"
        L"\n"
        L"%s",
        wName.c_str(),
        wAuthor.c_str(),
        soundCount,
        wDesc.empty() ? L"" : wDesc.c_str()
    );

    std::wstring title = (isKeyboard ? L"Keyboard" : L"Mouse");
    title += L" Profile Info";
    MessageBoxW(g_hMainWnd, buf, title.c_str(), MB_OK | MB_ICONINFORMATION);
}

void TestCurrentProfile(bool isKeyboard) {
    if (isKeyboard) {
        AudioEngine::GetInstance().PlayKey('A', true);
        AudioEngine::GetInstance().PlayKey(VK_SPACE, true);
        AudioEngine::GetInstance().PlayKey(VK_RETURN, true);
    } else {
        AudioEngine::GetInstance().PlayMouse("left", true);
        AudioEngine::GetInstance().PlayMouse("right", true);
    }
    VuPulse();
}

// ---------------------------------------------------------------------------
// VU Meter — pulse, decay, draw
// ---------------------------------------------------------------------------

void VuPulse() {
    // Spike bars to random heights, weighted towards centre for a bell shape
    for (int i = 0; i < NUM_VU_BARS; ++i) {
        // Distance from centre (0–1)
        float centre = (float)(NUM_VU_BARS - 1) / 2.0f;
        float dist   = fabsf((float)i - centre) / centre;        // 0 at centre, 1 at edges
        float weight = 1.0f - dist * 0.55f;                      // centre bars spike higher

        float spike = weight * (0.55f + (rand() % 100) / 220.0f); // 0.55–1.0 range at centre
        if (spike > 1.0f) spike = 1.0f;

        if (spike > g_vuLevels[i])
            g_vuLevels[i] = spike;

        // Per-bar decay speed: edge bars decay faster for a natural tail
        g_vuDecay[i] = 0.032f + dist * 0.018f;

        // Update peak hold
        if (g_vuLevels[i] > g_vuPeak[i])
            g_vuPeak[i] = g_vuLevels[i];
    }
    if (g_hVuWnd) InvalidateRect(g_hVuWnd, NULL, FALSE);
}

void VuDecay() {
    bool changed = false;
    for (int i = 0; i < NUM_VU_BARS; ++i) {
        if (g_vuLevels[i] > 0.0f) {
            g_vuLevels[i] -= g_vuDecay[i];
            if (g_vuLevels[i] < 0.0f) g_vuLevels[i] = 0.0f;
            changed = true;
        }
        // Peak hold decays slower
        if (g_vuPeak[i] > 0.0f) {
            g_vuPeak[i] -= g_vuDecay[i] * 0.35f;
            if (g_vuPeak[i] < 0.0f) g_vuPeak[i] = 0.0f;
            changed = true;
        }
    }
    if (changed && g_hVuWnd) InvalidateRect(g_hVuWnd, NULL, FALSE);
}

void VuDraw(HDC hdc, const RECT& rc) {
    int w = rc.right  - rc.left;
    int h = rc.bottom - rc.top;

    // Background — dark panel matching the status bar
    HBRUSH hBg = CreateSolidBrush(RGB(249, 249, 249));
    FillRect(hdc, &rc, hBg);
    DeleteObject(hBg);

    if (w <= 0 || h <= 0) return;

    const int gap     = 2;                          // px between bars
    int barW = (w - gap * (NUM_VU_BARS + 1)) / NUM_VU_BARS;
    if (barW < 1) barW = 1;
    int totalW = NUM_VU_BARS * barW + gap * (NUM_VU_BARS + 1);
    int offsetX = rc.left + (w - totalW) / 2;       // centre the bar group

    for (int i = 0; i < NUM_VU_BARS; ++i) {
        float level = g_vuLevels[i];
        int barH    = (int)(level * (h - 2));
        int x       = offsetX + gap + i * (barW + gap);
        int y       = rc.top + (h - barH);

        if (barH > 0) {
            // Colour: green (low) → yellow (mid) → red (high)
            int r, g_c, b;
            if (level < 0.5f) {
                // green → yellow
                float t = level / 0.5f;
                r   = (int)(t * 255);
                g_c = 200;
                b   = 0;
            } else {
                // yellow → red
                float t = (level - 0.5f) / 0.5f;
                r   = 255;
                g_c = (int)((1.0f - t) * 200);
                b   = 0;
            }
            // Slight brightness boost at the top segment
            RECT rcBar = { x, y, x + barW, rc.bottom - 1 };
            HBRUSH hBar = CreateSolidBrush(RGB(r, g_c, b));
            FillRect(hdc, &rcBar, hBar);
            DeleteObject(hBar);

            // Bright top pixel on the bar
            RECT rcTop = { x, y, x + barW, y + 2 };
            HBRUSH hTop = CreateSolidBrush(RGB(
                std::min(r + 60, 255),
                std::min(g_c + 60, 255),
                std::min(b + 40, 255)));
            FillRect(hdc, &rcTop, hTop);
            DeleteObject(hTop);
        }

        // Peak hold marker — single bright line
        if (g_vuPeak[i] > 0.02f) {
            int peakY = rc.top + (h - (int)(g_vuPeak[i] * (h - 2))) - 1;
            RECT rcPeak = { x, peakY, x + barW, peakY + 2 };
            HBRUSH hPeak = CreateSolidBrush(RGB(255, 255, 180));
            FillRect(hdc, &rcPeak, hPeak);
            DeleteObject(hPeak);
        }
    }
}

void UpdateCpuUsage() {
    FILETIME idleTime, kernelTime, userTime;
    if (GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        ULONGLONG idle = Utils::FileTimeToULL(idleTime);
        ULONGLONG kernel = Utils::FileTimeToULL(kernelTime);
        ULONGLONG user = Utils::FileTimeToULL(userTime);

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
                swprintf_s(buf, L" CPU: %d%%", cpuPercent);
                SendMessageW(g_hStatusBar, SB_SETTEXTW, 2, (LPARAM)buf);
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

        UINT cmd = TrackPopupMenu(hSubMenu, TPM_RIGHTBUTTON | TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, NULL);
        PostMessage(hWnd, WM_NULL, 0, 0);
        DestroyMenu(hMenu);

        if (cmd == IDM_TRAY_SHOW) {
            ShowWindow(hWnd, SW_SHOW);
            SetForegroundWindow(hWnd);
        } else if (cmd == IDM_TRAY_MUTE_KB) {
            bool nextState = !AudioEngine::GetInstance().IsKeyboardEnabled();
            AudioEngine::GetInstance().SetKeyboardEnabled(nextState);
            Button_SetCheck(g_hKbEnable, nextState ? BST_CHECKED : BST_UNCHECKED);
            SaveCurrentSettings();
        } else if (cmd == IDM_TRAY_MUTE_MOUSE) {
            bool nextState = !AudioEngine::GetInstance().IsMouseEnabled();
            AudioEngine::GetInstance().SetMouseEnabled(nextState);
            Button_SetCheck(g_hMouseEnable, nextState ? BST_CHECKED : BST_UNCHECKED);
            SaveCurrentSettings();
        } else if (cmd == IDM_TRAY_EXIT) {
            DestroyWindow(hWnd);
        }
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        CreateControls(hWnd);
        LoadSettingsToUI();
        UpdateTabVisibility(0);
        SetTimer(hWnd, TIMER_CPU_ID, 1000, NULL);
        SetTimer(hWnd, TIMER_VU_ID,    30, NULL);   // ~33 fps decay
        UpdateCpuUsage();
        // Reposition VU window to sit over the middle status bar pane
        {
            RECT rcSb{};
            GetWindowRect(g_hStatusBar, &rcSb);
            POINT pt{ rcSb.left, rcSb.top };
            ScreenToClient(hWnd, &pt);
            int sbH = (int)(rcSb.bottom - rcSb.top);
            RECT paneRect{};
            SendMessageW(g_hStatusBar, SB_GETRECT, 1, (LPARAM)&paneRect);
            SetWindowPos(g_hVuWnd, HWND_TOP,
                pt.x + paneRect.left + 2,
                pt.y + 2,
                paneRect.right - paneRect.left - 4,
                sbH - 4,
                SWP_NOZORDER);
        }
        break;

    case WM_DRAWITEM: {
        LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
        if (pDIS->CtlID == IDC_VU_METER) {
            VuDraw(pDIS->hDC, pDIS->rcItem);
            return TRUE;
        }
        if (pDIS->CtlID == IDC_STATIC_ABOUT_LOGO) {
            HDC hdc = pDIS->hDC;
            RECT rc = pDIS->rcItem;
            int w = rc.right - rc.left;
            int h = rc.bottom - rc.top;

            // Fill background with tab color
            FillRect(hdc, &rc, g_hTabBgBrush);

            int badgeW = 76;
            int badgeH = 76;
            int badgeX = (w - badgeW) / 2;
            int badgeY = (h - badgeH) / 2;

            if (g_pMonkeySoundsImage) {
                Gdiplus::Graphics graphics(hdc);
                graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
                graphics.DrawImage(g_pMonkeySoundsImage, badgeX, badgeY, badgeW, badgeH);
            } else if (g_hAppIcon) {
                DrawIconEx(hdc, badgeX + (badgeW - 48) / 2, badgeY + (badgeH - 48) / 2, g_hAppIcon, 48, 48, 0, NULL, DI_NORMAL);
            }
            return TRUE;
        }
        break;
    }

    case WM_CTLCOLORSTATIC: {
        HDC hdcStatic = (HDC)wParam;
        HWND hwndStatic = (HWND)lParam;
        if (hwndStatic == g_hAboutTitle) {
            SetTextColor(hdcStatic, RGB(20, 20, 20));
        } else if (hwndStatic == g_hAboutDesc1 || hwndStatic == g_hAboutDesc2) {
            SetTextColor(hdcStatic, RGB(60, 60, 60));
        } else if (hwndStatic == g_hAboutCopy) {
            SetTextColor(hdcStatic, RGB(90, 90, 90));
        }
        SetBkColor(hdcStatic, RGB(249, 249, 249));
        SetBkMode(hdcStatic, TRANSPARENT);
        return (INT_PTR)g_hTabBgBrush;
    }

    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG: {
        HDC hdcDlg = (HDC)wParam;
        SetBkColor(hdcDlg, RGB(249, 249, 249));
        SetBkMode(hdcDlg, TRANSPARENT);
        return (INT_PTR)g_hTabBgBrush;
    }

    case WM_TIMER:
        if (wParam == TIMER_CPU_ID) {
            UpdateCpuUsage();
        } else if (wParam == TIMER_VU_ID) {
            VuDecay();
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

    case WM_NCHITTEST:
        return DefWindowProc(hWnd, message, wParam, lParam);

    case WM_CLOSE:
        ShowWindow(hWnd, SW_HIDE);
        return 0;

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
            SaveCurrentSettings();
        } else if (hScrollBar == g_hMouseVolSlider) {
            int pos = (int)SendMessageW(g_hMouseVolSlider, TBM_GETPOS, 0, 0);
            AudioEngine::GetInstance().SetMouseVolume((float)pos / 100.0f);
            SaveCurrentSettings();
        }
        break;
    }

    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        int wmEvent = HIWORD(wParam);

        switch (wmId) {
        case IDC_CHK_KB_ENABLE:
            AudioEngine::GetInstance().SetKeyboardEnabled(Button_GetCheck(g_hKbEnable) == BST_CHECKED);
            SaveCurrentSettings();
            break;

        case IDC_CHK_MOUSE_ENABLE:
            AudioEngine::GetInstance().SetMouseEnabled(Button_GetCheck(g_hMouseEnable) == BST_CHECKED);
            SaveCurrentSettings();
            break;

        case IDC_COMBO_KB_PRESET:
            if (wmEvent == CBN_SELCHANGE) {
                int sel = (int)SendMessageW(g_hKbPresetCombo, CB_GETCURSEL, 0, 0);
                if (sel >= 0) {
                    // CB_GETITEMDATA holds the original g_kbProfiles index
                    LRESULT profileIdx = SendMessageW(g_hKbPresetCombo, CB_GETITEMDATA, sel, 0);
                    if (profileIdx != CB_ERR && profileIdx < (LRESULT)g_kbProfiles.size()) {
                        AudioEngine::GetInstance().LoadKeyboardProfile(g_kbProfiles[profileIdx].profileJsonPath);
                        AudioEngine::GetInstance().PlayKey(VK_SPACE, true);
                        SaveCurrentSettings();
                        UpdateFavoriteButton(true);
                    }
                }
            }
            break;

        case IDC_COMBO_MOUSE_PRESET:
            if (wmEvent == CBN_SELCHANGE) {
                int sel = (int)SendMessageW(g_hMousePresetCombo, CB_GETCURSEL, 0, 0);
                if (sel >= 0) {
                    LRESULT profileIdx = SendMessageW(g_hMousePresetCombo, CB_GETITEMDATA, sel, 0);
                    if (profileIdx != CB_ERR && profileIdx < (LRESULT)g_mouseProfiles.size()) {
                        AudioEngine::GetInstance().LoadMouseProfile(g_mouseProfiles[profileIdx].profileJsonPath);
                        AudioEngine::GetInstance().PlayMouse("left", true);
                        SaveCurrentSettings();
                        UpdateFavoriteButton(false);
                    }
                }
            }
            break;

        case IDC_BTN_KB_NEW: {
            std::wstring createdPath;
            if (ProfileWizard::Show(hWnd, true, createdPath)) {
                g_kbProfiles = AudioEngine::GetInstance().ScanKeyboardProfiles();
                if (!createdPath.empty() && fs::exists(createdPath)) {
                    AudioEngine::GetInstance().LoadKeyboardProfile(createdPath);
                    AudioEngine::GetInstance().PlayKey(VK_SPACE, true);
                }
                PopulatePresets();
                SaveCurrentSettings();
                SendMessageW(g_hStatusBar, SB_SETTEXTW, 0, (LPARAM)L"  New keyboard profile created!");
            }
            break;
        }

        case IDC_BTN_MOUSE_NEW: {
            std::wstring createdPath;
            if (ProfileWizard::Show(hWnd, false, createdPath)) {
                g_mouseProfiles = AudioEngine::GetInstance().ScanMouseProfiles();
                if (!createdPath.empty() && fs::exists(createdPath)) {
                    AudioEngine::GetInstance().LoadMouseProfile(createdPath);
                    AudioEngine::GetInstance().PlayMouse("left", true);
                }
                PopulatePresets();
                SaveCurrentSettings();
                SendMessageW(g_hStatusBar, SB_SETTEXTW, 0, (LPARAM)L"  New mouse profile created!");
            }
            break;
        }

        case IDC_BTN_KB_CUSTOM:
            ChooseCustomProfile(true);
            break;

        case IDC_BTN_MOUSE_CUSTOM:
            ChooseCustomProfile(false);
            break;

        case IDC_BTN_KB_TEST:
            TestCurrentProfile(true);
            break;

        case IDC_BTN_MOUSE_TEST:
            TestCurrentProfile(false);
            break;

        case IDC_BTN_KB_INFO:
            ShowProfileInfo(true);
            break;

        case IDC_BTN_MOUSE_INFO:
            ShowProfileInfo(false);
            break;

        case IDC_BTN_KB_EXPORT:
            ExportCurrentProfile(true);
            break;

        case IDC_BTN_MOUSE_EXPORT:
            ExportCurrentProfile(false);
            break;

        case IDC_BTN_KB_FAVORITE:
            ToggleFavorite(true);
            break;

        case IDC_BTN_MOUSE_FAVORITE:
            ToggleFavorite(false);
            break;

        case IDC_BTN_CHECK_UPDATES:
            ShellExecuteW(hWnd, L"open",
                L"https://github.com/NSTechBytes/MonkeySounds/releases/latest",
                nullptr, nullptr, SW_SHOWNORMAL);
            break;

        case IDC_CHK_AUTOSTART:
            SaveCurrentSettings();
            break;

        case IDC_CHK_STARTUP_NOTIF:
            SaveCurrentSettings();
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
            SaveCurrentSettings();
            break;
        }

        case IDM_TRAY_MUTE_MOUSE: {
            bool nextState = !AudioEngine::GetInstance().IsMouseEnabled();
            AudioEngine::GetInstance().SetMouseEnabled(nextState);
            Button_SetCheck(g_hMouseEnable, nextState ? BST_CHECKED : BST_UNCHECKED);
            SaveCurrentSettings();
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

    case WM_VU_PULSE:
        VuPulse();
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;
    }

    case WM_DESTROY: {
        NOTIFYICONDATA nid = {};
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hWnd;
        nid.uID = 1;
        Shell_NotifyIconW(NIM_DELETE, &nid);
        InputHook::GetInstance().UninstallHooks();
        AudioEngine::GetInstance().Shutdown();
        KillTimer(hWnd, TIMER_CPU_ID);
        KillTimer(hWnd, TIMER_VU_ID);
        if (g_hVuBgBrush) { DeleteObject(g_hVuBgBrush); g_hVuBgBrush = NULL; }
        PostQuitMessage(0);
        ExitProcess(0);
        break;
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
