#include "framework.h"
#include "ProfileWizard.h"
#include "Utils.h"
#include "ZipUtils.h"
#include "Resource.h"
#include "WindowsTheme.h"
#include "DpiUtil.h"
#include "Lang.h"
#include "json.hpp"
#include <commctrl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <random>

#pragma warning(push)
#pragma warning(disable: 4244 4267)
#include "miniaudio.h"
#pragma warning(pop)

namespace fs = std::filesystem;
using json = nlohmann::json;

#define WIZARD_WIDTH   580
#define WIZARD_HEIGHT  500

enum WizardStep {
    STEP_INFO = 0,
    STEP_DEFAULT_SOUNDS = 1,
    STEP_SPECIFIC_KEYS = 2,
    STEP_TEST_SAVE = 3,
    STEP_COUNT = 4
};

// Wizard Control IDs
#define IDC_WIZ_BTN_BACK        2001
#define IDC_WIZ_BTN_NEXT        2002
#define IDC_WIZ_BTN_CANCEL      2003

// Step 1 IDs
#define IDC_WIZ_RAD_KEYBOARD    2101
#define IDC_WIZ_RAD_MOUSE       2102
#define IDC_WIZ_EDIT_NAME       2103
#define IDC_WIZ_EDIT_AUTHOR     2104
#define IDC_WIZ_EDIT_DESC       2105

// Step 2 IDs
#define IDC_WIZ_LIST_DEF_PRESS  2201
#define IDC_WIZ_BTN_ADD_PRESS   2202
#define IDC_WIZ_BTN_DEL_PRESS   2203
#define IDC_WIZ_BTN_PLAY_PRESS  2204
#define IDC_WIZ_EDIT_DEF_REL    2205
#define IDC_WIZ_BTN_BROWSE_REL  2206
#define IDC_WIZ_BTN_CLEAR_REL   2207
#define IDC_WIZ_BTN_PLAY_REL    2208

// Step 3 IDs
#define IDC_WIZ_COMBO_KEY_LIST  2301
#define IDC_WIZ_EDIT_CUSTOM_KEY 2302
#define IDC_WIZ_BTN_ADD_KEY     2303
#define IDC_WIZ_EDIT_KEY_PRESS  2304
#define IDC_WIZ_BTN_BROWSE_KP   2305
#define IDC_WIZ_BTN_CLEAR_KP    2306
#define IDC_WIZ_BTN_PLAY_KP     2307
#define IDC_WIZ_EDIT_KEY_REL    2308
#define IDC_WIZ_BTN_BROWSE_KR   2309
#define IDC_WIZ_BTN_CLEAR_KR    2310
#define IDC_WIZ_BTN_PLAY_KR     2311
#define IDC_WIZ_LIST_ASSIGNED   2312
#define IDC_WIZ_BTN_REMOVE_BIND 2313

// Step 4 IDs
#define IDC_WIZ_EDIT_SUMMARY    2401
#define IDC_WIZ_EDIT_TEST_PAD   2402
#define IDC_WIZ_BTN_TEST_LEFT   2403
#define IDC_WIZ_BTN_TEST_RIGHT  2404
#define IDC_WIZ_BTN_TEST_MID    2405
#define IDC_WIZ_CHK_ACTIVATE    2406
#define IDC_WIZ_CHK_EXPORT_ZIP  2407

struct WizardDialogState {
    HWND hWnd = NULL;
    HWND hParent = NULL;
    int currentStep = STEP_INFO;
    WizardProfileData data;
    bool success = false;
    bool isRunning = true;
    std::wstring createdProfilePath;
    UINT dpi = 96;

    // Mini preview audio engine
    ma_engine previewEngine;
    bool previewEngineReady = false;

    // Fonts & Brushes
    HFONT hFontNormal = NULL;
    HFONT hFontBold = NULL;
    HFONT hFontTitle = NULL;
    HFONT hFontHeader = NULL;
    HBRUSH hBgBrush = NULL;
    HBRUSH hHeaderBrush = NULL;
    HBRUSH hWhiteBrush = NULL;

    // Common Nav Controls
    HWND hBtnBack = NULL;
    HWND hBtnNext = NULL;
    HWND hBtnCancel = NULL;
    HWND hLblStepTitle = NULL;
    HWND hLblStepSub = NULL;

    // Step 1 Controls
    HWND hGrpType = NULL;
    HWND hRadKb = NULL;
    HWND hRadMouse = NULL;
    HWND hLblName = NULL;
    HWND hEditName = NULL;
    HWND hLblAuthor = NULL;
    HWND hEditAuthor = NULL;
    HWND hLblDesc = NULL;
    HWND hEditDesc = NULL;

    // Step 2 Controls
    HWND hGrpDefPress = NULL;
    HWND hListDefPress = NULL;
    HWND hBtnAddPress = NULL;
    HWND hBtnDelPress = NULL;
    HWND hBtnPlayPress = NULL;
    HWND hGrpDefRel = NULL;
    HWND hEditDefRel = NULL;
    HWND hBtnBrowseRel = NULL;
    HWND hBtnClearRel = NULL;
    HWND hBtnPlayRel = NULL;
    HWND hLblDefNote = NULL;

    // Step 3 Controls
    HWND hGrpKeySelect = NULL;
    HWND hLblQuickKey = NULL;
    HWND hComboKeyList = NULL;
    HWND hLblCustomKey = NULL;
    HWND hEditCustomKey = NULL;
    HWND hBtnAddKey = NULL;
    HWND hGrpKeySounds = NULL;
    HWND hLblKeyPress = NULL;
    HWND hEditKeyPress = NULL;
    HWND hBtnBrowseKP = NULL;
    HWND hBtnClearKP = NULL;
    HWND hBtnPlayKP = NULL;
    HWND hLblKeyRel = NULL;
    HWND hEditKeyRel = NULL;
    HWND hBtnBrowseKR = NULL;
    HWND hBtnClearKR = NULL;
    HWND hBtnPlayKR = NULL;
    HWND hLblAssigned = NULL;
    HWND hListAssigned = NULL;
    HWND hBtnRemoveBind = NULL;

    // Step 4 Controls
    HWND hGrpSummary = NULL;
    HWND hEditSummary = NULL;
    HWND hGrpTest = NULL;
    HWND hEditTestPad = NULL;
    HWND hBtnTestLeft = NULL;
    HWND hBtnTestRight = NULL;
    HWND hBtnTestMid = NULL;
    HWND hChkActivate = NULL;
    HWND hChkExportZip = NULL;
    HWND hLblTestNote = NULL;

    // Keep track of currently edited key in Step 3
    std::string selectedKeyName;

    // Track held keys in test pad for press/release
    std::mt19937 rng;
};

static WizardDialogState* g_pWiz = nullptr;

// Forward declarations
static void FixWizardGroupBoxZOrder(WizardDialogState* pWiz);

// ---------------------------------------------------------------------------
// Wizard DPI-aware font and layout helpers
// ---------------------------------------------------------------------------

static void CreateWizardFonts(WizardDialogState* pWiz, UINT dpi) {
    if (pWiz->hFontNormal) { DeleteObject(pWiz->hFontNormal); pWiz->hFontNormal = NULL; }
    if (pWiz->hFontBold)   { DeleteObject(pWiz->hFontBold);   pWiz->hFontBold   = NULL; }
    if (pWiz->hFontTitle)  { DeleteObject(pWiz->hFontTitle);  pWiz->hFontTitle  = NULL; }
    if (pWiz->hFontHeader) { DeleteObject(pWiz->hFontHeader); pWiz->hFontHeader = NULL; }
    pWiz->dpi = dpi;
    pWiz->hFontNormal = CreateFontW(-DpiUtil::Scale(12, dpi), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    pWiz->hFontBold = CreateFontW(-DpiUtil::Scale(12, dpi), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    pWiz->hFontHeader = CreateFontW(-DpiUtil::Scale(15, dpi), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    pWiz->hFontTitle = CreateFontW(-DpiUtil::Scale(13, dpi), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
}

static void ApplyWizardFonts(WizardDialogState* pWiz) {
    auto sf = [](HWND h, HFONT f) { if (h && f) SendMessageW(h, WM_SETFONT, (WPARAM)f, TRUE); };
    sf(pWiz->hLblStepTitle,  pWiz->hFontHeader);
    sf(pWiz->hLblStepSub,    pWiz->hFontTitle);
    sf(pWiz->hBtnBack,       pWiz->hFontNormal);
    sf(pWiz->hBtnNext,       pWiz->hFontBold);
    sf(pWiz->hBtnCancel,     pWiz->hFontNormal);
    // Step 1
    sf(pWiz->hGrpType,       pWiz->hFontBold);
    sf(pWiz->hRadKb,         pWiz->hFontNormal);
    sf(pWiz->hRadMouse,      pWiz->hFontNormal);
    sf(pWiz->hLblName,       pWiz->hFontBold);
    sf(pWiz->hEditName,      pWiz->hFontNormal);
    sf(pWiz->hLblAuthor,     pWiz->hFontNormal);
    sf(pWiz->hEditAuthor,    pWiz->hFontNormal);
    sf(pWiz->hLblDesc,       pWiz->hFontNormal);
    sf(pWiz->hEditDesc,      pWiz->hFontNormal);
    // Step 2
    sf(pWiz->hGrpDefPress,   pWiz->hFontBold);
    sf(pWiz->hListDefPress,  pWiz->hFontNormal);
    sf(pWiz->hBtnAddPress,   pWiz->hFontNormal);
    sf(pWiz->hBtnDelPress,   pWiz->hFontNormal);
    sf(pWiz->hBtnPlayPress,  pWiz->hFontNormal);
    sf(pWiz->hGrpDefRel,     pWiz->hFontBold);
    sf(pWiz->hEditDefRel,    pWiz->hFontNormal);
    sf(pWiz->hBtnBrowseRel,  pWiz->hFontNormal);
    sf(pWiz->hBtnClearRel,   pWiz->hFontNormal);
    sf(pWiz->hBtnPlayRel,    pWiz->hFontNormal);
    sf(pWiz->hLblDefNote,    pWiz->hFontNormal);
    // Step 3
    sf(pWiz->hGrpKeySelect,  pWiz->hFontBold);
    sf(pWiz->hLblQuickKey,   pWiz->hFontNormal);
    sf(pWiz->hComboKeyList,  pWiz->hFontNormal);
    sf(pWiz->hLblCustomKey,  pWiz->hFontNormal);
    sf(pWiz->hEditCustomKey, pWiz->hFontNormal);
    sf(pWiz->hBtnAddKey,     pWiz->hFontNormal);
    sf(pWiz->hGrpKeySounds,  pWiz->hFontBold);
    sf(pWiz->hLblKeyPress,   pWiz->hFontNormal);
    sf(pWiz->hEditKeyPress,  pWiz->hFontNormal);
    sf(pWiz->hBtnBrowseKP,   pWiz->hFontNormal);
    sf(pWiz->hBtnClearKP,    pWiz->hFontNormal);
    sf(pWiz->hBtnPlayKP,     pWiz->hFontNormal);
    sf(pWiz->hLblKeyRel,     pWiz->hFontNormal);
    sf(pWiz->hEditKeyRel,    pWiz->hFontNormal);
    sf(pWiz->hBtnBrowseKR,   pWiz->hFontNormal);
    sf(pWiz->hBtnClearKR,    pWiz->hFontNormal);
    sf(pWiz->hBtnPlayKR,     pWiz->hFontNormal);
    sf(pWiz->hLblAssigned,   pWiz->hFontBold);
    sf(pWiz->hListAssigned,  pWiz->hFontNormal);
    sf(pWiz->hBtnRemoveBind, pWiz->hFontNormal);
    // Step 4
    sf(pWiz->hGrpSummary,    pWiz->hFontBold);
    sf(pWiz->hEditSummary,   pWiz->hFontNormal);
    sf(pWiz->hGrpTest,       pWiz->hFontBold);
    sf(pWiz->hLblTestNote,   pWiz->hFontNormal);
    sf(pWiz->hEditTestPad,   pWiz->hFontTitle);
    sf(pWiz->hBtnTestLeft,   pWiz->hFontNormal);
    sf(pWiz->hBtnTestRight,  pWiz->hFontNormal);
    sf(pWiz->hBtnTestMid,    pWiz->hFontNormal);
    sf(pWiz->hChkActivate,   pWiz->hFontNormal);
    sf(pWiz->hChkExportZip,  pWiz->hFontNormal);
}

static void LayoutWizardControls(WizardDialogState* pWiz, UINT dpi) {
    HWND hWnd = pWiz->hWnd;
    if (!hWnd || !IsWindow(hWnd)) return;
    auto S = [dpi](int v) { return DpiUtil::Scale(v, dpi); };
    const int W = S(WIZARD_WIDTH);
    const int H = S(WIZARD_HEIGHT);
    const int btnNavY = H - S(75);

    // Navigation & header
    SetWindowPos(pWiz->hLblStepTitle, NULL, S(20), S(12),  W-S(40), S(20), SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hLblStepSub,   NULL, S(20), S(34),  W-S(40), S(20), SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hBtnBack,      NULL, W-S(300), btnNavY, S(80),  S(28), SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hBtnNext,      NULL, W-S(212), btnNavY, S(110), S(28), SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hBtnCancel,    NULL, W-S(94),  btnNavY, S(74),  S(28), SWP_NOZORDER | SWP_NOACTIVATE);

    // Step 1
    SetWindowPos(pWiz->hGrpType,    NULL, S(20), S(68), W-S(40), S(56), SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hRadKb,      NULL, S(36), S(90), S(160),  S(20), SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hRadMouse,   NULL, S(210),S(90), S(160),  S(20), SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hLblName,    NULL, S(20), S(140), S(120), S(18), SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hEditName,   NULL, S(20), S(160), W-S(40),S(24), SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hLblAuthor,  NULL, S(20), S(195), S(180), S(18), SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hEditAuthor, NULL, S(20), S(215), W-S(40),S(24), SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hLblDesc,    NULL, S(20), S(250), S(200), S(18), SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hEditDesc,   NULL, S(20), S(270), W-S(40),S(60), SWP_NOZORDER | SWP_NOACTIVATE);

    // Step 2
    SetWindowPos(pWiz->hGrpDefPress,  NULL, S(20),     S(65),  W-S(40),    S(175), SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hListDefPress, NULL, S(36),     S(90),  W-S(170),   S(135), SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hBtnAddPress,  NULL, W-S(124),  S(90),  S(84),      S(26),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hBtnDelPress,  NULL, W-S(124),  S(122), S(84),      S(26),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hBtnPlayPress, NULL, W-S(124),  S(154), S(84),      S(26),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hGrpDefRel,    NULL, S(20),     S(248), W-S(40),    S(75),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hEditDefRel,   NULL, S(36),     S(275), W-S(210),   S(24),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hBtnBrowseRel, NULL, W-S(165),  S(274), S(65),      S(26),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hBtnClearRel,  NULL, W-S(96),   S(274), S(26),      S(26),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hBtnPlayRel,   NULL, W-S(66),   S(274), S(26),      S(26),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hLblDefNote,   NULL, S(20),     S(330), W-S(40),    S(20),  SWP_NOZORDER | SWP_NOACTIVATE);

    // Step 3
    SetWindowPos(pWiz->hGrpKeySelect,  NULL, S(20),   S(65),  W-S(40), S(70),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hLblQuickKey,   NULL, S(36),   S(92),  S(85),   S(18),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hComboKeyList,  NULL, S(125),  S(89),  S(180),  S(160), SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hLblCustomKey,  NULL, S(315),  S(92),  S(65),   S(18),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hEditCustomKey, NULL, S(382),  S(89),  S(70),   S(24),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hBtnAddKey,     NULL, S(458),  S(88),  S(45),   S(26),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hGrpKeySounds,  NULL, S(20),   S(140), W-S(40), S(115), SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hLblKeyPress,   NULL, S(36),   S(166), S(85),   S(18),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hEditKeyPress,  NULL, S(125),  S(163), S(292),  S(24),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hBtnBrowseKP,   NULL, S(421),  S(162), S(65),   S(26),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hBtnClearKP,    NULL, S(490),  S(162), S(26),   S(26),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hBtnPlayKP,     NULL, S(520),  S(162), S(26),   S(26),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hLblKeyRel,     NULL, S(36),   S(206), S(85),   S(18),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hEditKeyRel,    NULL, S(125),  S(203), S(292),  S(24),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hBtnBrowseKR,   NULL, S(421),  S(202), S(65),   S(26),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hBtnClearKR,    NULL, S(490),  S(202), S(26),   S(26),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hBtnPlayKR,     NULL, S(520),  S(202), S(26),   S(26),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hLblAssigned,   NULL, S(20),   S(258), S(260),  S(18),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hListAssigned,  NULL, S(20),   S(278), W-S(140),S(80),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hBtnRemoveBind, NULL, W-S(115),S(278), S(95),   S(28),  SWP_NOZORDER | SWP_NOACTIVATE);

    // Step 4
    SetWindowPos(pWiz->hGrpSummary,   NULL, S(20),  S(65),  W-S(40), S(140), SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hEditSummary,  NULL, S(36),  S(88),  W-S(72), S(105), SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hGrpTest,      NULL, S(20),  S(212), W-S(40), S(105), SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hLblTestNote,  NULL, S(36),  S(232), W-S(72), S(18),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hEditTestPad,  NULL, S(36),  S(254), W-S(72), S(28),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hBtnTestLeft,  NULL, S(36),  S(254), S(140),  S(32),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hBtnTestRight, NULL, S(190), S(254), S(140),  S(32),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hBtnTestMid,   NULL, S(345), S(254), S(140),  S(32),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hChkActivate,  NULL, S(24),  S(325), S(360),  S(20),  SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(pWiz->hChkExportZip, NULL, S(24),  S(348), S(360),  S(20),  SWP_NOZORDER | SWP_NOACTIVATE);

    // Keep GroupBoxes at HWND_BOTTOM so controls inside them are never clipped out by WS_CLIPSIBLINGS
    FixWizardGroupBoxZOrder(pWiz);

    InvalidateRect(hWnd, NULL, TRUE);
}

// Push all GroupBox controls to HWND_BOTTOM so sibling controls render on top of them.
// Must be called any time ShowWindow is used on a GroupBox (which pulls it to Z-order top).
static void FixWizardGroupBoxZOrder(WizardDialogState* pWiz) {
    HWND groups[] = {
        pWiz->hGrpType, pWiz->hGrpDefPress, pWiz->hGrpDefRel,
        pWiz->hGrpKeySelect, pWiz->hGrpKeySounds, pWiz->hGrpSummary, pWiz->hGrpTest
    };
    for (HWND grp : groups) {
        if (grp) {
            SetWindowPos(grp, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
    }
}

static void PlayAudioPreview(const std::wstring& filePath) {
    if (!g_pWiz || !g_pWiz->previewEngineReady || filePath.empty()) return;
    if (!fs::exists(filePath)) return;
    std::string utf8 = Utils::WideToUtf8(filePath);
    ma_engine_play_sound(&g_pWiz->previewEngine, utf8.c_str(), NULL);
}

static std::wstring BrowseAudioFile(HWND hWndOwner, const wchar_t* title) {
    WCHAR szFile[MAX_PATH] = {};
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = hWndOwner;
    ofn.lpstrFilter = L"Audio Files (*.wav;*.mp3;*.ogg;*.flac)\0*.wav;*.mp3;*.ogg;*.flac\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrTitle = title;

    if (GetOpenFileNameW(&ofn)) {
        return std::wstring(szFile);
    }
    return L"";
}

static std::vector<std::wstring> BrowseMultipleAudioFiles(HWND hWndOwner, const wchar_t* title) {
    std::vector<std::wstring> result;
    const DWORD bufSize = 32768;
    std::vector<WCHAR> buffer(bufSize, 0);

    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = hWndOwner;
    ofn.lpstrFilter = L"Audio Files (*.wav;*.mp3;*.ogg;*.flac)\0*.wav;*.mp3;*.ogg;*.flac\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = buffer.data();
    ofn.nMaxFile = bufSize;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY;
    ofn.lpstrTitle = title;

    if (GetOpenFileNameW(&ofn)) {
        WCHAR* p = buffer.data();
        std::wstring dirOrFile = p;
        p += dirOrFile.length() + 1;
        if (*p == L'\0') {
            // Single file selected
            result.push_back(dirOrFile);
        } else {
            // Multiple files selected: first string is directory, followed by filenames
            std::wstring dir = dirOrFile;
            while (*p != L'\0') {
                std::wstring fileName = p;
                fs::path fullPath = fs::path(dir) / fileName;
                result.push_back(fullPath.wstring());
                p += fileName.length() + 1;
            }
        }
    }
    return result;
}

static void PopulateKeyPresets(WizardDialogState* pWiz) {
    SendMessageW(pWiz->hComboKeyList, CB_RESETCONTENT, 0, 0);
    if (pWiz->data.deviceType == "keyboard") {
        const wchar_t* kbKeys[] = {
            L"space (Spacebar)",
            L"enter (Enter / Return)",
            L"backspace (Backspace / Delete)",
            L"shift (Shift Keys)",
            L"ctrl (Control Keys)",
            L"alt (Alt Keys)",
            L"tab (Tab Key)",
            L"capslock (Caps Lock)",
            L"escape (Escape)",
            L"up (Arrow Up)",
            L"down (Arrow Down)",
            L"left (Arrow Left)",
            L"right (Arrow Right)",
            L"win (Windows Key)"
        };
        for (auto k : kbKeys) {
            SendMessageW(pWiz->hComboKeyList, CB_ADDSTRING, 0, (LPARAM)k);
        }
    } else {
        const wchar_t* mouseKeys[] = {
            L"left (Left Click)",
            L"right (Right Click)",
            L"middle (Middle Click / Wheel)"
        };
        for (auto m : mouseKeys) {
            SendMessageW(pWiz->hComboKeyList, CB_ADDSTRING, 0, (LPARAM)m);
        }
    }
    SendMessageW(pWiz->hComboKeyList, CB_SETCURSEL, 0, 0);
}

static std::string ExtractKeyNameFromCombo(const std::wstring& comboText) {
    size_t spacePos = comboText.find(L' ');
    std::wstring keyW = (spacePos != std::wstring::npos) ? comboText.substr(0, spacePos) : comboText;
    return Utils::WideToUtf8(keyW);
}

static KeySoundBinding* FindBinding(WizardProfileData& data, const std::string& keyName) {
    for (auto& b : data.specificBindings) {
        if (_stricmp(b.keyName.c_str(), keyName.c_str()) == 0) {
            return &b;
        }
    }
    return nullptr;
}

static void RefreshAssignedList(WizardDialogState* pWiz) {
    SendMessageW(pWiz->hListAssigned, LB_RESETCONTENT, 0, 0);
    for (size_t i = 0; i < pWiz->data.specificBindings.size(); ++i) {
        const auto& b = pWiz->data.specificBindings[i];
        std::wstring item = Utils::Utf8ToWide(b.keyName) + L" : ";
        if (!b.pressPath.empty()) {
            item += L"Press [" + fs::path(b.pressPath).filename().wstring() + L"] ";
        }
        if (!b.releasePath.empty()) {
            item += L"Release [" + fs::path(b.releasePath).filename().wstring() + L"]";
        }
        SendMessageW(pWiz->hListAssigned, LB_ADDSTRING, 0, (LPARAM)item.c_str());
    }
}

static void UpdateKeyEditFields(WizardDialogState* pWiz) {
    KeySoundBinding* pBind = FindBinding(pWiz->data, pWiz->selectedKeyName);
    if (pBind) {
        SetWindowTextW(pWiz->hEditKeyPress, pBind->pressPath.c_str());
        SetWindowTextW(pWiz->hEditKeyRel, pBind->releasePath.c_str());
    } else {
        SetWindowTextW(pWiz->hEditKeyPress, L"");
        SetWindowTextW(pWiz->hEditKeyRel, L"");
    }
}

static void RefreshSummary(WizardDialogState* pWiz) {
    std::wstring sum = TRS("wizard.sum.header");
    sum += TRS("wizard.sum.device") + (pWiz->data.deviceType == "keyboard" ? TRS("wizard.sum.keyboard") : TRS("wizard.sum.mouse")) + L"\r\n";
    sum += TRS("wizard.sum.name") + (pWiz->data.name.empty() ? TRS("wizard.sum.untitled") : Utils::Utf8ToWide(pWiz->data.name)) + L"\r\n";
    sum += TRS("wizard.sum.author") + (pWiz->data.author.empty() ? TRS("wizard.sum.anon") : Utils::Utf8ToWide(pWiz->data.author)) + L"\r\n";
    sum += TRS("wizard.sum.desc") + Utils::Utf8ToWide(pWiz->data.description) + L"\r\n\r\n";

    sum += TRS("wizard.sum.defPress") + std::to_wstring(pWiz->data.defaultPressFiles.size()) + TRS("wizard.sum.defPressFmt") + L"\r\n";
    for (size_t i = 0; i < pWiz->data.defaultPressFiles.size(); ++i) {
        sum += L"  - " + fs::path(pWiz->data.defaultPressFiles[i]).filename().wstring() + L"\r\n";
    }
    if (!pWiz->data.defaultReleaseFiles.empty()) {
        sum += TRS("wizard.sum.defRel") + fs::path(pWiz->data.defaultReleaseFiles[0]).filename().wstring() + L"\r\n";
    }

    sum += TRS("wizard.sum.bindings") + std::to_wstring(pWiz->data.specificBindings.size()) + L"\r\n";
    for (const auto& b : pWiz->data.specificBindings) {
        sum += L"  * " + Utils::Utf8ToWide(b.keyName) + L": ";
        if (!b.pressPath.empty()) sum += TRS("wizard.sum.press") + fs::path(b.pressPath).filename().wstring() + L" ";
        if (!b.releasePath.empty()) sum += TRS("wizard.sum.release") + fs::path(b.releasePath).filename().wstring();
        sum += L"\r\n";
    }

    SetWindowTextW(pWiz->hEditSummary, sum.c_str());

    // Update test pad buttons visibility
    bool isKb = (pWiz->data.deviceType == "keyboard");
    ShowWindow(pWiz->hEditTestPad, isKb ? SW_SHOW : SW_HIDE);
    ShowWindow(pWiz->hBtnTestLeft, isKb ? SW_HIDE : SW_SHOW);
    ShowWindow(pWiz->hBtnTestRight, isKb ? SW_HIDE : SW_SHOW);
    ShowWindow(pWiz->hBtnTestMid, isKb ? SW_HIDE : SW_SHOW);
    SetWindowTextW(pWiz->hLblTestNote, isKb
        ? TR("wizard.lbl.testNote.kb")
        : TR("wizard.lbl.testNote.mouse"));
}

static void HideAllStepControls(WizardDialogState* pWiz) {
    HWND allControls[] = {
        // Step 1
        pWiz->hGrpType, pWiz->hRadKb, pWiz->hRadMouse, pWiz->hLblName, pWiz->hEditName,
        pWiz->hLblAuthor, pWiz->hEditAuthor, pWiz->hLblDesc, pWiz->hEditDesc,
        // Step 2
        pWiz->hGrpDefPress, pWiz->hListDefPress, pWiz->hBtnAddPress, pWiz->hBtnDelPress,
        pWiz->hBtnPlayPress, pWiz->hGrpDefRel, pWiz->hEditDefRel, pWiz->hBtnBrowseRel,
        pWiz->hBtnClearRel, pWiz->hBtnPlayRel, pWiz->hLblDefNote,
        // Step 3
        pWiz->hGrpKeySelect, pWiz->hLblQuickKey, pWiz->hComboKeyList, pWiz->hLblCustomKey,
        pWiz->hEditCustomKey, pWiz->hBtnAddKey, pWiz->hGrpKeySounds, pWiz->hLblKeyPress,
        pWiz->hEditKeyPress, pWiz->hBtnBrowseKP, pWiz->hBtnClearKP, pWiz->hBtnPlayKP,
        pWiz->hLblKeyRel, pWiz->hEditKeyRel, pWiz->hBtnBrowseKR, pWiz->hBtnClearKR,
        pWiz->hBtnPlayKR, pWiz->hLblAssigned, pWiz->hListAssigned, pWiz->hBtnRemoveBind,
        // Step 4
        pWiz->hGrpSummary, pWiz->hEditSummary, pWiz->hGrpTest, pWiz->hLblTestNote,
        pWiz->hEditTestPad, pWiz->hBtnTestLeft, pWiz->hBtnTestRight, pWiz->hBtnTestMid,
        pWiz->hChkActivate, pWiz->hChkExportZip
    };
    for (HWND h : allControls) {
        if (h) ShowWindow(h, SW_HIDE);
    }
}

static void ShowStep(WizardDialogState* pWiz, int step) {
    pWiz->currentStep = step;

    // 1. Hide all controls first
    HideAllStepControls(pWiz);

    // 2. Update Header
    std::wstring stepTitle = TRS("wizard.step.of");
    {
        size_t pos = stepTitle.find(L"{0}");
        if (pos != std::wstring::npos) {
            stepTitle.replace(pos, 3, std::to_wstring(step + 1));
        }
    }
    std::wstring stepSub = L"";
    switch (step) {
    case STEP_INFO:
        stepTitle += TRS("wizard.step1.title");
        stepSub = TRS("wizard.step1.sub");
        break;
    case STEP_DEFAULT_SOUNDS:
        stepTitle += (pWiz->data.deviceType == "keyboard" ? TRS("wizard.step2.title.kb") : TRS("wizard.step2.title.mouse"));
        stepSub = TRS("wizard.step2.sub");
        break;
    case STEP_SPECIFIC_KEYS:
        stepTitle += (pWiz->data.deviceType == "keyboard" ? TRS("wizard.step3.title.kb") : TRS("wizard.step3.title.mouse"));
        stepSub = TRS("wizard.step3.sub");
        break;
    case STEP_TEST_SAVE:
        stepTitle += TRS("wizard.step4.title");
        stepSub = TRS("wizard.step4.sub");
        break;
    }

    SetWindowTextW(pWiz->hLblStepTitle, stepTitle.c_str());
    SetWindowTextW(pWiz->hLblStepSub, stepSub.c_str());

    // 3. Nav Buttons
    EnableWindow(pWiz->hBtnBack, step > STEP_INFO);
    SetWindowTextW(pWiz->hBtnNext, (step == STEP_TEST_SAVE) ? TR("wizard.btn.finish") : TR("wizard.btn.next"));

    // 4. Show only current step controls
    switch (step) {
    case STEP_INFO:
        ShowWindow(pWiz->hGrpType, SW_SHOW);
        ShowWindow(pWiz->hRadKb, SW_SHOW);
        ShowWindow(pWiz->hRadMouse, SW_SHOW);
        ShowWindow(pWiz->hLblName, SW_SHOW);
        ShowWindow(pWiz->hEditName, SW_SHOW);
        ShowWindow(pWiz->hLblAuthor, SW_SHOW);
        ShowWindow(pWiz->hEditAuthor, SW_SHOW);
        ShowWindow(pWiz->hLblDesc, SW_SHOW);
        ShowWindow(pWiz->hEditDesc, SW_SHOW);

        SendMessageW(pWiz->hRadKb, BM_SETCHECK, (pWiz->data.deviceType == "keyboard") ? BST_CHECKED : BST_UNCHECKED, 0);
        SendMessageW(pWiz->hRadMouse, BM_SETCHECK, (pWiz->data.deviceType == "mouse") ? BST_CHECKED : BST_UNCHECKED, 0);
        break;

    case STEP_DEFAULT_SOUNDS:
        ShowWindow(pWiz->hGrpDefPress, SW_SHOW);
        ShowWindow(pWiz->hListDefPress, SW_SHOW);
        ShowWindow(pWiz->hBtnAddPress, SW_SHOW);
        ShowWindow(pWiz->hBtnDelPress, SW_SHOW);
        ShowWindow(pWiz->hBtnPlayPress, SW_SHOW);
        ShowWindow(pWiz->hGrpDefRel, SW_SHOW);
        ShowWindow(pWiz->hEditDefRel, SW_SHOW);
        ShowWindow(pWiz->hBtnBrowseRel, SW_SHOW);
        ShowWindow(pWiz->hBtnClearRel, SW_SHOW);
        ShowWindow(pWiz->hBtnPlayRel, SW_SHOW);
        ShowWindow(pWiz->hLblDefNote, SW_SHOW);

        SendMessageW(pWiz->hListDefPress, LB_RESETCONTENT, 0, 0);
        for (const auto& f : pWiz->data.defaultPressFiles) {
            SendMessageW(pWiz->hListDefPress, LB_ADDSTRING, 0, (LPARAM)fs::path(f).filename().wstring().c_str());
        }
        if (!pWiz->data.defaultPressFiles.empty()) {
            SendMessageW(pWiz->hListDefPress, LB_SETCURSEL, 0, 0);
        }
        if (!pWiz->data.defaultReleaseFiles.empty()) {
            SetWindowTextW(pWiz->hEditDefRel, pWiz->data.defaultReleaseFiles[0].c_str());
        } else {
            SetWindowTextW(pWiz->hEditDefRel, L"");
        }
        break;

    case STEP_SPECIFIC_KEYS:
        ShowWindow(pWiz->hGrpKeySelect, SW_SHOW);
        ShowWindow(pWiz->hLblQuickKey, SW_SHOW);
        ShowWindow(pWiz->hComboKeyList, SW_SHOW);
        ShowWindow(pWiz->hLblCustomKey, SW_SHOW);
        ShowWindow(pWiz->hEditCustomKey, SW_SHOW);
        ShowWindow(pWiz->hBtnAddKey, SW_SHOW);
        ShowWindow(pWiz->hGrpKeySounds, SW_SHOW);
        ShowWindow(pWiz->hLblKeyPress, SW_SHOW);
        ShowWindow(pWiz->hEditKeyPress, SW_SHOW);
        ShowWindow(pWiz->hBtnBrowseKP, SW_SHOW);
        ShowWindow(pWiz->hBtnClearKP, SW_SHOW);
        ShowWindow(pWiz->hBtnPlayKP, SW_SHOW);
        ShowWindow(pWiz->hLblKeyRel, SW_SHOW);
        ShowWindow(pWiz->hEditKeyRel, SW_SHOW);
        ShowWindow(pWiz->hBtnBrowseKR, SW_SHOW);
        ShowWindow(pWiz->hBtnClearKR, SW_SHOW);
        ShowWindow(pWiz->hBtnPlayKR, SW_SHOW);
        ShowWindow(pWiz->hLblAssigned, SW_SHOW);
        ShowWindow(pWiz->hListAssigned, SW_SHOW);
        ShowWindow(pWiz->hBtnRemoveBind, SW_SHOW);

        PopulateKeyPresets(pWiz);
        {
            WCHAR curText[128] = {};
            SendMessageW(pWiz->hComboKeyList, WM_GETTEXT, 128, (LPARAM)curText);
            pWiz->selectedKeyName = ExtractKeyNameFromCombo(curText);
        }
        UpdateKeyEditFields(pWiz);
        RefreshAssignedList(pWiz);
        break;

    case STEP_TEST_SAVE:
        ShowWindow(pWiz->hGrpSummary, SW_SHOW);
        ShowWindow(pWiz->hEditSummary, SW_SHOW);
        ShowWindow(pWiz->hGrpTest, SW_SHOW);
        ShowWindow(pWiz->hLblTestNote, SW_SHOW);
        ShowWindow(pWiz->hChkActivate, SW_SHOW);
        ShowWindow(pWiz->hChkExportZip, SW_SHOW);

        if (pWiz->data.deviceType == "keyboard") {
            ShowWindow(pWiz->hEditTestPad, SW_SHOW);
            ShowWindow(pWiz->hBtnTestLeft, SW_HIDE);
            ShowWindow(pWiz->hBtnTestRight, SW_HIDE);
            ShowWindow(pWiz->hBtnTestMid, SW_HIDE);
        } else {
            ShowWindow(pWiz->hEditTestPad, SW_HIDE);
            ShowWindow(pWiz->hBtnTestLeft, SW_SHOW);
            ShowWindow(pWiz->hBtnTestRight, SW_SHOW);
            ShowWindow(pWiz->hBtnTestMid, SW_SHOW);
        }
        RefreshSummary(pWiz);
        break;
    }

    // 5. Re-apply GroupBox Z-order fix (ShowWindow pulls groupboxes to Z-order top)
    //    then invalidate and immediately repaint whole dialog window
    FixWizardGroupBoxZOrder(pWiz);
    InvalidateRect(pWiz->hWnd, NULL, TRUE);
    UpdateWindow(pWiz->hWnd);
}

static bool ValidateAndSaveStep(WizardDialogState* pWiz, int step) {
    if (step == STEP_INFO) {
        WCHAR nameBuf[256] = {};
        GetWindowTextW(pWiz->hEditName, nameBuf, 256);
        std::wstring wName = nameBuf;
        // Trim
        size_t first = wName.find_first_not_of(L" \t\r\n");
        if (first == std::wstring::npos) {
            MessageBoxW(pWiz->hWnd, TR("wizard.validate.nameEmpty"), TR("wizard.validate.nameTitle"), MB_OK | MB_ICONWARNING);
            SetFocus(pWiz->hEditName);
            return false;
        }
        size_t last = wName.find_last_not_of(L" \t\r\n");
        wName = wName.substr(first, last - first + 1);

        pWiz->data.name = Utils::WideToUtf8(wName);

        WCHAR authorBuf[256] = {};
        GetWindowTextW(pWiz->hEditAuthor, authorBuf, 256);
        pWiz->data.author = Utils::WideToUtf8(authorBuf);

        WCHAR descBuf[512] = {};
        GetWindowTextW(pWiz->hEditDesc, descBuf, 512);
        pWiz->data.description = Utils::WideToUtf8(descBuf);

        pWiz->data.deviceType = (SendMessageW(pWiz->hRadKb, BM_GETCHECK, 0, 0) == BST_CHECKED) ? "keyboard" : "mouse";
        return true;
    } else if (step == STEP_DEFAULT_SOUNDS) {
        if (pWiz->data.defaultPressFiles.empty()) {
            MessageBoxW(pWiz->hWnd, TR("wizard.validate.noPress"), TR("wizard.validate.noPressTtl"), MB_OK | MB_ICONWARNING);
            return false;
        }
        return true;
    }
    return true;
}

static void PlayWizardTestSound(WizardDialogState* pWiz, const std::string& keyName, bool isPress) {
    if (!pWiz) return;
    std::wstring soundToPlay;

    // Check specific bindings
    KeySoundBinding* pBind = FindBinding(pWiz->data, keyName);
    if (pBind) {
        if (isPress && !pBind->pressPath.empty()) soundToPlay = pBind->pressPath;
        else if (!isPress && !pBind->releasePath.empty()) soundToPlay = pBind->releasePath;
    }

    // Fallback to default sounds
    if (soundToPlay.empty()) {
        if (isPress && !pWiz->data.defaultPressFiles.empty()) {
            std::uniform_int_distribution<size_t> dist(0, pWiz->data.defaultPressFiles.size() - 1);
            soundToPlay = pWiz->data.defaultPressFiles[dist(pWiz->rng)];
        } else if (!isPress && !pWiz->data.defaultReleaseFiles.empty()) {
            soundToPlay = pWiz->data.defaultReleaseFiles[0];
        }
    }

    if (!soundToPlay.empty()) {
        PlayAudioPreview(soundToPlay);
    }
}

// Subclass procedure for the test typing edit control to intercept keystrokes
static WNDPROC g_oldTestPadProc = NULL;
static LRESULT CALLBACK TestPadSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_KEYDOWN) {
        int vk = (int)wParam;
        std::string keyName;
        switch (vk) {
        case VK_SPACE: keyName = "space"; break;
        case VK_RETURN: keyName = "enter"; break;
        case VK_BACK: keyName = "backspace"; break;
        case VK_DELETE: keyName = "delete"; break;
        case VK_TAB: keyName = "tab"; break;
        case VK_CAPITAL: keyName = "capslock"; break;
        case VK_SHIFT:
        case VK_LSHIFT:
        case VK_RSHIFT: keyName = "shift"; break;
        case VK_CONTROL:
        case VK_LCONTROL:
        case VK_RCONTROL: keyName = "ctrl"; break;
        case VK_MENU:
        case VK_LMENU:
        case VK_RMENU: keyName = "alt"; break;
        case VK_LEFT: keyName = "left"; break;
        case VK_RIGHT: keyName = "right"; break;
        case VK_UP: keyName = "up"; break;
        case VK_DOWN: keyName = "down"; break;
        case VK_ESCAPE: keyName = "escape"; break;
        default: break;
        }

        if (g_pWiz) {
            PlayWizardTestSound(g_pWiz, keyName, true);
        }
    } else if (uMsg == WM_KEYUP) {
        int vk = (int)wParam;
        std::string keyName;
        switch (vk) {
        case VK_SPACE: keyName = "space"; break;
        case VK_RETURN: keyName = "enter"; break;
        case VK_BACK: keyName = "backspace"; break;
        case VK_DELETE: keyName = "delete"; break;
        case VK_TAB: keyName = "tab"; break;
        case VK_CAPITAL: keyName = "capslock"; break;
        case VK_SHIFT:
        case VK_LSHIFT:
        case VK_RSHIFT: keyName = "shift"; break;
        case VK_CONTROL:
        case VK_LCONTROL:
        case VK_RCONTROL: keyName = "ctrl"; break;
        case VK_MENU:
        case VK_LMENU:
        case VK_RMENU: keyName = "alt"; break;
        case VK_LEFT: keyName = "left"; break;
        case VK_RIGHT: keyName = "right"; break;
        case VK_UP: keyName = "up"; break;
        case VK_DOWN: keyName = "down"; break;
        case VK_ESCAPE: keyName = "escape"; break;
        default: break;
        }

        if (g_pWiz) {
            PlayWizardTestSound(g_pWiz, keyName, false);
        }
    }

    return CallWindowProcW(g_oldTestPadProc, hWnd, uMsg, wParam, lParam);
}

static bool CreateAndWriteProfile(WizardDialogState* pWiz) {
    try {
        // Sanitize profile folder name
        std::string safeName = pWiz->data.name;
        for (char& c : safeName) {
            if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {
                c = '_';
            }
        }
        std::wstring wSafeName = Utils::Utf8ToWide(safeName);

        // Destination folder: Sounds/<Keyboard|Mouse>/Custom/<safeName>
        std::wstring soundsRoot = Utils::GetSoundsDirectory();
        std::wstring subDir = (pWiz->data.deviceType == "keyboard") ? L"Keyboard\\Custom" : L"Mouse\\Custom";
        fs::path destDir = fs::path(soundsRoot) / subDir / wSafeName;

        // If folder exists, create unique name
        int suffix = 1;
        fs::path finalDestDir = destDir;
        while (fs::exists(finalDestDir)) {
            finalDestDir = fs::path(soundsRoot) / subDir / (wSafeName + L"_" + std::to_wstring(suffix++));
        }

        fs::create_directories(finalDestDir);

        json j;
        j["profile"] = {
            { "name", pWiz->data.name },
            { "author", pWiz->data.author },
            { "description", pWiz->data.description },
            { "device", pWiz->data.deviceType }
        };

        std::vector<json> sourcesJson;
        std::unordered_map<std::wstring, std::string> copiedFilesToRelName;
        int soundIdCounter = 1;

        auto copySoundFile = [&](const std::wstring& srcPath) -> std::string {
            if (srcPath.empty() || !fs::exists(srcPath)) return "";
            auto it = copiedFilesToRelName.find(srcPath);
            if (it != copiedFilesToRelName.end()) return it->second;

            fs::path src(srcPath);
            std::wstring filename = src.filename().wstring();
            fs::path targetPath = finalDestDir / filename;

            int copyIdx = 1;
            while (fs::exists(targetPath)) {
                targetPath = finalDestDir / (src.stem().wstring() + L"_" + std::to_wstring(copyIdx++) + src.extension().wstring());
            }

            fs::copy_file(srcPath, targetPath, fs::copy_options::overwrite_existing);
            std::string relName = Utils::WideToUtf8(targetPath.filename().wstring());
            copiedFilesToRelName[srcPath] = relName;
            return relName;
        };

        // 1. Default sound sources
        std::vector<std::string> defaultIds;
        for (size_t i = 0; i < pWiz->data.defaultPressFiles.size(); ++i) {
            std::string sId = (pWiz->data.deviceType == "keyboard") ? ("key" + std::to_string(soundIdCounter++))
                                                                    : ("click_" + std::to_string(soundIdCounter++));
            defaultIds.push_back(sId);

            std::string pressRel = copySoundFile(pWiz->data.defaultPressFiles[i]);
            std::string releaseRel = "";
            if (!pWiz->data.defaultReleaseFiles.empty()) {
                releaseRel = copySoundFile(pWiz->data.defaultReleaseFiles[0]);
            }

            json sObj;
            sObj["id"] = sId;
            if (!releaseRel.empty()) {
                sObj["source"] = { { "press", pressRel }, { "release", releaseRel } };
            } else {
                sObj["source"] = pressRel;
            }
            sourcesJson.push_back(sObj);
        }

        // 2. Specific key sound sources
        std::vector<json> otherJson;
        for (const auto& b : pWiz->data.specificBindings) {
            if (b.pressPath.empty() && b.releasePath.empty()) continue;

            std::string sId = "sound_" + b.keyName;
            std::string pressRel = copySoundFile(b.pressPath);
            std::string releaseRel = copySoundFile(b.releasePath);

            json sObj;
            sObj["id"] = sId;
            if (!releaseRel.empty() && !pressRel.empty()) {
                sObj["source"] = { { "press", pressRel }, { "release", releaseRel } };
            } else if (!pressRel.empty()) {
                sObj["source"] = pressRel;
            } else {
                sObj["source"] = { { "release", releaseRel } };
            }
            sourcesJson.push_back(sObj);

            json otherRule;
            if (pWiz->data.deviceType == "keyboard") {
                otherRule["keys"] = { b.keyName };
            } else {
                otherRule["buttons"] = { b.keyName };
            }
            otherRule["sound"] = sId;
            otherJson.push_back(otherRule);
        }

        if (pWiz->data.deviceType == "keyboard") {
            j["keys"] = {
                { "default", defaultIds },
                { "other", otherJson }
            };
        } else {
            j["buttons"] = {
                { "default", defaultIds },
                { "other", otherJson }
            };
        }

        j["sources"] = sourcesJson;

        // Write profile.json
        fs::path jsonFilePath = finalDestDir / L"profile.json";
        std::ofstream outFile(jsonFilePath);
        if (!outFile.is_open()) return false;
        outFile << j.dump(2);
        outFile.close();

        pWiz->createdProfilePath = jsonFilePath.wstring();
        pWiz->data.activateImmediately = (SendMessageW(pWiz->hChkActivate, BM_GETCHECK, 0, 0) == BST_CHECKED);
        pWiz->data.exportZip = (SendMessageW(pWiz->hChkExportZip, BM_GETCHECK, 0, 0) == BST_CHECKED);

        if (pWiz->data.exportZip) {
            WCHAR szZipFile[MAX_PATH] = {};
            wcscpy_s(szZipFile, (wSafeName + L".zip").c_str());

            OPENFILENAMEW ofn = {};
            ofn.lStructSize = sizeof(OPENFILENAMEW);
            ofn.hwndOwner = pWiz->hWnd;
            ofn.lpstrFilter = L"ZIP Archive (*.zip)\0*.zip\0";
            ofn.lpstrFile = szZipFile;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrDefExt = L"zip";
            ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT;
            ofn.lpstrTitle = TR("wizard.export.title");

            if (GetSaveFileNameW(&ofn)) {
                ZipUtils::CreateZipFromDir(finalDestDir.wstring(), szZipFile);
            }
        }

        return true;
    } catch (...) {
        return false;
    }
}

static LRESULT CALLBACK WizardWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    WizardDialogState* pWiz = g_pWiz;

    switch (uMsg) {
    case WM_CREATE: {
        pWiz->hWnd = hWnd;

        // Create DPI-aware fonts
        CreateWizardFonts(pWiz, DpiUtil::GetWindowDpi(hWnd));

        bool dark = WindowsTheme::IsUsingDarkMode();
        pWiz->hBgBrush = CreateSolidBrush(dark ? RGB(32, 32, 32) : RGB(240, 240, 240));
        pWiz->hHeaderBrush = CreateSolidBrush(dark ? RGB(42, 42, 42) : RGB(240, 243, 246));
        pWiz->hWhiteBrush = CreateSolidBrush(dark ? RGB(32, 32, 32) : RGB(255, 255, 255));

        // Header Labels
        pWiz->hLblStepTitle = CreateWindowExW(0, L"STATIC", L"Step Title", WS_CHILD | WS_VISIBLE | SS_LEFT,
            20, 12, WIZARD_WIDTH - 40, 20, hWnd, NULL, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hLblStepTitle, WM_SETFONT, (WPARAM)pWiz->hFontHeader, TRUE);

        pWiz->hLblStepSub = CreateWindowExW(0, L"STATIC", L"Step description...", WS_CHILD | WS_VISIBLE | SS_LEFT,
            20, 34, WIZARD_WIDTH - 40, 20, hWnd, NULL, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hLblStepSub, WM_SETFONT, (WPARAM)pWiz->hFontTitle, TRUE);

        // Navigation Footer Buttons
        int btnY = WIZARD_HEIGHT - 75;
        pWiz->hBtnBack = CreateWindowExW(0, L"BUTTON", TR("wizard.btn.back"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            WIZARD_WIDTH - 300, btnY, 80, 28, hWnd, (HMENU)IDC_WIZ_BTN_BACK, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hBtnBack, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hBtnNext = CreateWindowExW(0, L"BUTTON", TR("wizard.btn.next"), WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            WIZARD_WIDTH - 212, btnY, 110, 28, hWnd, (HMENU)IDC_WIZ_BTN_NEXT, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hBtnNext, WM_SETFONT, (WPARAM)pWiz->hFontBold, TRUE);

        pWiz->hBtnCancel = CreateWindowExW(0, L"BUTTON", TR("wizard.btn.cancel"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            WIZARD_WIDTH - 94, btnY, 74, 28, hWnd, (HMENU)IDC_WIZ_BTN_CANCEL, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hBtnCancel, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        // ==================== STEP 1 CONTROLS ====================
        pWiz->hGrpType = CreateWindowExW(0, L"BUTTON", TR("wizard.grp.deviceType"), WS_CHILD | BS_GROUPBOX,
            20, 68, WIZARD_WIDTH - 40, 56, hWnd, NULL, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hGrpType, WM_SETFONT, (WPARAM)pWiz->hFontBold, TRUE);

        pWiz->hRadKb = CreateWindowExW(0, L"BUTTON", TR("wizard.rad.keyboard"), WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP,
            36, 90, 160, 20, hWnd, (HMENU)IDC_WIZ_RAD_KEYBOARD, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hRadKb, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hRadMouse = CreateWindowExW(0, L"BUTTON", TR("wizard.rad.mouse"), WS_CHILD | BS_AUTORADIOBUTTON,
            210, 90, 160, 20, hWnd, (HMENU)IDC_WIZ_RAD_MOUSE, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hRadMouse, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        if (pWiz->data.deviceType == "keyboard") {
            SendMessageW(pWiz->hRadKb, BM_SETCHECK, BST_CHECKED, 0);
        } else {
            SendMessageW(pWiz->hRadMouse, BM_SETCHECK, BST_CHECKED, 0);
        }

        pWiz->hLblName = CreateWindowExW(0, L"STATIC", TR("wizard.lbl.name"), WS_CHILD | SS_LEFT,
            20, 140, 120, 18, hWnd, NULL, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hLblName, WM_SETFONT, (WPARAM)pWiz->hFontBold, TRUE);

        pWiz->hEditName = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL,
            20, 160, WIZARD_WIDTH - 40, 24, hWnd, (HMENU)IDC_WIZ_EDIT_NAME, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hEditName, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hLblAuthor = CreateWindowExW(0, L"STATIC", TR("wizard.lbl.author"), WS_CHILD | SS_LEFT,
            20, 195, 180, 18, hWnd, NULL, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hLblAuthor, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hEditAuthor = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL,
            20, 215, WIZARD_WIDTH - 40, 24, hWnd, (HMENU)IDC_WIZ_EDIT_AUTHOR, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hEditAuthor, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hLblDesc = CreateWindowExW(0, L"STATIC", TR("wizard.lbl.desc"), WS_CHILD | SS_LEFT,
            20, 250, 200, 18, hWnd, NULL, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hLblDesc, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hEditDesc = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_TABSTOP | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
            20, 270, WIZARD_WIDTH - 40, 60, hWnd, (HMENU)IDC_WIZ_EDIT_DESC, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hEditDesc, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        // ==================== STEP 2 CONTROLS ====================
        pWiz->hGrpDefPress = CreateWindowExW(0, L"BUTTON", TR("wizard.grp.defPress"), WS_CHILD | BS_GROUPBOX,
            20, 65, WIZARD_WIDTH - 40, 175, hWnd, NULL, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hGrpDefPress, WM_SETFONT, (WPARAM)pWiz->hFontBold, TRUE);

        pWiz->hListDefPress = CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"",
            WS_CHILD | LBS_NOTIFY | WS_VSCROLL | WS_TABSTOP,
            36, 90, WIZARD_WIDTH - 170, 135, hWnd, (HMENU)IDC_WIZ_LIST_DEF_PRESS, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hListDefPress, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hBtnAddPress = CreateWindowExW(0, L"BUTTON", TR("wizard.btn.add"), WS_CHILD | BS_PUSHBUTTON,
            WIZARD_WIDTH - 124, 90, 84, 26, hWnd, (HMENU)IDC_WIZ_BTN_ADD_PRESS, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hBtnAddPress, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hBtnDelPress = CreateWindowExW(0, L"BUTTON", TR("wizard.btn.remove"), WS_CHILD | BS_PUSHBUTTON,
            WIZARD_WIDTH - 124, 122, 84, 26, hWnd, (HMENU)IDC_WIZ_BTN_DEL_PRESS, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hBtnDelPress, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hBtnPlayPress = CreateWindowExW(0, L"BUTTON", TR("wizard.btn.play"), WS_CHILD | BS_PUSHBUTTON,
            WIZARD_WIDTH - 124, 154, 84, 26, hWnd, (HMENU)IDC_WIZ_BTN_PLAY_PRESS, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hBtnPlayPress, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hGrpDefRel = CreateWindowExW(0, L"BUTTON", TR("wizard.grp.defRel"), WS_CHILD | BS_GROUPBOX,
            20, 248, WIZARD_WIDTH - 40, 75, hWnd, NULL, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hGrpDefRel, WM_SETFONT, (WPARAM)pWiz->hFontBold, TRUE);

        pWiz->hEditDefRel = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | ES_READONLY | ES_AUTOHSCROLL,
            36, 275, WIZARD_WIDTH - 210, 24, hWnd, (HMENU)IDC_WIZ_EDIT_DEF_REL, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hEditDefRel, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hBtnBrowseRel = CreateWindowExW(0, L"BUTTON", TR("wizard.btn.browse"), WS_CHILD | BS_PUSHBUTTON,
            WIZARD_WIDTH - 165, 274, 65, 26, hWnd, (HMENU)IDC_WIZ_BTN_BROWSE_REL, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hBtnBrowseRel, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hBtnClearRel = CreateWindowExW(0, L"BUTTON", TR("wizard.btn.clear"), WS_CHILD | BS_PUSHBUTTON,
            WIZARD_WIDTH - 96, 274, 26, 26, hWnd, (HMENU)IDC_WIZ_BTN_CLEAR_REL, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hBtnClearRel, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hBtnPlayRel = CreateWindowExW(0, L"BUTTON", L"\u25B6", WS_CHILD | BS_PUSHBUTTON,
            WIZARD_WIDTH - 66, 274, 26, 26, hWnd, (HMENU)IDC_WIZ_BTN_PLAY_REL, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hBtnPlayRel, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hLblDefNote = CreateWindowExW(0, L"STATIC",
            TR("wizard.tip.multiPress"),
            WS_CHILD | SS_LEFT, 20, 330, WIZARD_WIDTH - 40, 20, hWnd, NULL, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hLblDefNote, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        // ==================== STEP 3 CONTROLS ====================
        pWiz->hGrpKeySelect = CreateWindowExW(0, L"BUTTON", TR("wizard.grp.keySelect"), WS_CHILD | BS_GROUPBOX,
            20, 65, WIZARD_WIDTH - 40, 70, hWnd, NULL, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hGrpKeySelect, WM_SETFONT, (WPARAM)pWiz->hFontBold, TRUE);

        pWiz->hLblQuickKey = CreateWindowExW(0, L"STATIC", TR("wizard.lbl.quickKey"), WS_CHILD | SS_LEFT,
            36, 92, 85, 18, hWnd, NULL, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hLblQuickKey, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hComboKeyList = CreateWindowExW(0, L"COMBOBOX", L"",
            WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
            125, 89, 180, 160, hWnd, (HMENU)IDC_WIZ_COMBO_KEY_LIST, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hComboKeyList, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hLblCustomKey = CreateWindowExW(0, L"STATIC", TR("wizard.lbl.customKey"), WS_CHILD | SS_LEFT,
            315, 92, 65, 18, hWnd, NULL, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hLblCustomKey, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hEditCustomKey = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL,
            382, 89, 70, 24, hWnd, (HMENU)IDC_WIZ_EDIT_CUSTOM_KEY, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hEditCustomKey, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hBtnAddKey = CreateWindowExW(0, L"BUTTON", TR("wizard.btn.set"), WS_CHILD | BS_PUSHBUTTON,
            458, 88, 45, 26, hWnd, (HMENU)IDC_WIZ_BTN_ADD_KEY, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hBtnAddKey, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hGrpKeySounds = CreateWindowExW(0, L"BUTTON", TR("wizard.grp.keySounds"), WS_CHILD | BS_GROUPBOX,
            20, 140, WIZARD_WIDTH - 40, 115, hWnd, NULL, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hGrpKeySounds, WM_SETFONT, (WPARAM)pWiz->hFontBold, TRUE);

        pWiz->hLblKeyPress = CreateWindowExW(0, L"STATIC", TR("wizard.lbl.keyPress"), WS_CHILD | SS_LEFT,
            36, 166, 85, 18, hWnd, NULL, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hLblKeyPress, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hEditKeyPress = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | ES_READONLY | ES_AUTOHSCROLL,
            125, 163, 292, 24, hWnd, (HMENU)IDC_WIZ_EDIT_KEY_PRESS, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hEditKeyPress, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hBtnBrowseKP = CreateWindowExW(0, L"BUTTON", TR("wizard.btn.browse"), WS_CHILD | BS_PUSHBUTTON,
            421, 162, 65, 26, hWnd, (HMENU)IDC_WIZ_BTN_BROWSE_KP, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hBtnBrowseKP, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hBtnClearKP = CreateWindowExW(0, L"BUTTON", TR("wizard.btn.clear"), WS_CHILD | BS_PUSHBUTTON,
            490, 162, 26, 26, hWnd, (HMENU)IDC_WIZ_BTN_CLEAR_KP, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hBtnClearKP, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hBtnPlayKP = CreateWindowExW(0, L"BUTTON", L"\u25B6", WS_CHILD | BS_PUSHBUTTON,
            520, 162, 26, 26, hWnd, (HMENU)IDC_WIZ_BTN_PLAY_KP, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hBtnPlayKP, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hLblKeyRel = CreateWindowExW(0, L"STATIC", TR("wizard.lbl.keyRel"), WS_CHILD | SS_LEFT,
            36, 206, 85, 18, hWnd, NULL, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hLblKeyRel, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hEditKeyRel = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | ES_READONLY | ES_AUTOHSCROLL,
            125, 203, 292, 24, hWnd, (HMENU)IDC_WIZ_EDIT_KEY_REL, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hEditKeyRel, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hBtnBrowseKR = CreateWindowExW(0, L"BUTTON", TR("wizard.btn.browse"), WS_CHILD | BS_PUSHBUTTON,
            421, 202, 65, 26, hWnd, (HMENU)IDC_WIZ_BTN_BROWSE_KR, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hBtnBrowseKR, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hBtnClearKR = CreateWindowExW(0, L"BUTTON", TR("wizard.btn.clear"), WS_CHILD | BS_PUSHBUTTON,
            490, 202, 26, 26, hWnd, (HMENU)IDC_WIZ_BTN_CLEAR_KR, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hBtnClearKR, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hBtnPlayKR = CreateWindowExW(0, L"BUTTON", L"\u25B6", WS_CHILD | BS_PUSHBUTTON,
            520, 202, 26, 26, hWnd, (HMENU)IDC_WIZ_BTN_PLAY_KR, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hBtnPlayKR, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hLblAssigned = CreateWindowExW(0, L"STATIC", TR("wizard.lbl.assigned"), WS_CHILD | SS_LEFT,
            20, 258, 260, 18, hWnd, NULL, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hLblAssigned, WM_SETFONT, (WPARAM)pWiz->hFontBold, TRUE);

        pWiz->hListAssigned = CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"",
            WS_CHILD | LBS_NOTIFY | WS_VSCROLL | WS_TABSTOP,
            20, 278, WIZARD_WIDTH - 140, 80, hWnd, (HMENU)IDC_WIZ_LIST_ASSIGNED, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hListAssigned, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hBtnRemoveBind = CreateWindowExW(0, L"BUTTON", TR("wizard.btn.deleteMapping"), WS_CHILD | BS_PUSHBUTTON,
            WIZARD_WIDTH - 115, 278, 95, 28, hWnd, (HMENU)IDC_WIZ_BTN_REMOVE_BIND, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hBtnRemoveBind, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        // ==================== STEP 4 CONTROLS ====================
        pWiz->hGrpSummary = CreateWindowExW(0, L"BUTTON", TR("wizard.grp.summary"), WS_CHILD | BS_GROUPBOX,
            20, 65, WIZARD_WIDTH - 40, 140, hWnd, NULL, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hGrpSummary, WM_SETFONT, (WPARAM)pWiz->hFontBold, TRUE);

        pWiz->hEditSummary = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | ES_MULTILINE | ES_READONLY | WS_VSCROLL,
            36, 88, WIZARD_WIDTH - 72, 105, hWnd, (HMENU)IDC_WIZ_EDIT_SUMMARY, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hEditSummary, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hGrpTest = CreateWindowExW(0, L"BUTTON", TR("wizard.grp.test"), WS_CHILD | BS_GROUPBOX,
            20, 212, WIZARD_WIDTH - 40, 105, hWnd, NULL, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hGrpTest, WM_SETFONT, (WPARAM)pWiz->hFontBold, TRUE);

        pWiz->hLblTestNote = CreateWindowExW(0, L"STATIC", TR("wizard.lbl.testNote.kb"), WS_CHILD | SS_LEFT,
            36, 232, WIZARD_WIDTH - 72, 18, hWnd, NULL, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hLblTestNote, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hEditTestPad = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL,
            36, 254, WIZARD_WIDTH - 72, 28, hWnd, (HMENU)IDC_WIZ_EDIT_TEST_PAD, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hEditTestPad, WM_SETFONT, (WPARAM)pWiz->hFontTitle, TRUE);

        // Subclass test pad edit control to capture raw WM_KEYDOWN and WM_KEYUP
        g_oldTestPadProc = (WNDPROC)SetWindowLongPtrW(pWiz->hEditTestPad, GWLP_WNDPROC, (LONG_PTR)TestPadSubclassProc);

        pWiz->hBtnTestLeft = CreateWindowExW(0, L"BUTTON", TR("wizard.btn.testLeft"), WS_CHILD | BS_PUSHBUTTON,
            36, 254, 140, 32, hWnd, (HMENU)IDC_WIZ_BTN_TEST_LEFT, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hBtnTestLeft, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hBtnTestRight = CreateWindowExW(0, L"BUTTON", TR("wizard.btn.testRight"), WS_CHILD | BS_PUSHBUTTON,
            190, 254, 140, 32, hWnd, (HMENU)IDC_WIZ_BTN_TEST_RIGHT, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hBtnTestRight, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hBtnTestMid = CreateWindowExW(0, L"BUTTON", TR("wizard.btn.testMid"), WS_CHILD | BS_PUSHBUTTON,
            345, 254, 140, 32, hWnd, (HMENU)IDC_WIZ_BTN_TEST_MID, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hBtnTestMid, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        pWiz->hChkActivate = CreateWindowExW(0, L"BUTTON", TR("wizard.chk.activate"),
            WS_CHILD | BS_AUTOCHECKBOX, 24, 325, 360, 20, hWnd, (HMENU)IDC_WIZ_CHK_ACTIVATE, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hChkActivate, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);
        SendMessageW(pWiz->hChkActivate, BM_SETCHECK, BST_CHECKED, 0);

        pWiz->hChkExportZip = CreateWindowExW(0, L"BUTTON", TR("wizard.chk.exportZip"),
            WS_CHILD | BS_AUTOCHECKBOX, 24, 348, 360, 20, hWnd, (HMENU)IDC_WIZ_CHK_EXPORT_ZIP, GetModuleHandle(NULL), NULL);
        SendMessageW(pWiz->hChkExportZip, WM_SETFONT, (WPARAM)pWiz->hFontNormal, TRUE);

        ShowStep(pWiz, STEP_INFO);
        LayoutWizardControls(pWiz, pWiz->dpi);
        ApplyWizardFonts(pWiz);
        WindowsTheme::ApplyToControlTree(hWnd);
        LayoutWizardControls(pWiz, pWiz->dpi);
        break;
    }

    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        int wmEvent = HIWORD(wParam);

        switch (wmId) {
        case IDC_WIZ_BTN_NEXT: {
            if (!ValidateAndSaveStep(pWiz, pWiz->currentStep)) {
                break;
            }
            if (pWiz->currentStep < STEP_TEST_SAVE) {
                ShowStep(pWiz, pWiz->currentStep + 1);
            } else {
                // Finish & Create Profile
                if (CreateAndWriteProfile(pWiz)) {
                    pWiz->success = true;
                    MessageBoxW(hWnd, TR("wizard.created.msg"), TR("wizard.created.title"), MB_OK | MB_ICONINFORMATION);
                    DestroyWindow(hWnd);
                } else {
                    MessageBoxW(hWnd, TR("wizard.failed.msg"), TR("wizard.failed.title"), MB_OK | MB_ICONERROR);
                }
            }
            break;
        }

        case IDC_WIZ_BTN_BACK: {
            if (pWiz->currentStep > STEP_INFO) {
                ShowStep(pWiz, pWiz->currentStep - 1);
            }
            break;
        }

        case IDC_WIZ_BTN_CANCEL: {
            DestroyWindow(hWnd);
            break;
        }

        case IDC_WIZ_RAD_KEYBOARD:
        case IDC_WIZ_RAD_MOUSE: {
            pWiz->data.deviceType = (SendMessageW(pWiz->hRadKb, BM_GETCHECK, 0, 0) == BST_CHECKED) ? "keyboard" : "mouse";
            break;
        }

        // Step 2 Actions
        case IDC_WIZ_BTN_ADD_PRESS: {
            std::vector<std::wstring> files = BrowseMultipleAudioFiles(hWnd, L"Select Default Press Sound File(s)");
            for (const auto& f : files) {
                if (std::find(pWiz->data.defaultPressFiles.begin(), pWiz->data.defaultPressFiles.end(), f) == pWiz->data.defaultPressFiles.end()) {
                    pWiz->data.defaultPressFiles.push_back(f);
                    SendMessageW(pWiz->hListDefPress, LB_ADDSTRING, 0, (LPARAM)fs::path(f).filename().wstring().c_str());
                }
            }
            if (SendMessageW(pWiz->hListDefPress, LB_GETCURSEL, 0, 0) == LB_ERR && !pWiz->data.defaultPressFiles.empty()) {
                SendMessageW(pWiz->hListDefPress, LB_SETCURSEL, 0, 0);
            }
            break;
        }

        case IDC_WIZ_BTN_DEL_PRESS: {
            int sel = (int)SendMessageW(pWiz->hListDefPress, LB_GETCURSEL, 0, 0);
            if (sel >= 0 && sel < (int)pWiz->data.defaultPressFiles.size()) {
                pWiz->data.defaultPressFiles.erase(pWiz->data.defaultPressFiles.begin() + sel);
                SendMessageW(pWiz->hListDefPress, LB_DELETESTRING, sel, 0);
                if (sel < (int)pWiz->data.defaultPressFiles.size()) {
                    SendMessageW(pWiz->hListDefPress, LB_SETCURSEL, sel, 0);
                } else if (!pWiz->data.defaultPressFiles.empty()) {
                    SendMessageW(pWiz->hListDefPress, LB_SETCURSEL, (int)pWiz->data.defaultPressFiles.size() - 1, 0);
                }
            }
            break;
        }

        case IDC_WIZ_BTN_PLAY_PRESS: {
            int sel = (int)SendMessageW(pWiz->hListDefPress, LB_GETCURSEL, 0, 0);
            if (sel >= 0 && sel < (int)pWiz->data.defaultPressFiles.size()) {
                PlayAudioPreview(pWiz->data.defaultPressFiles[sel]);
            }
            break;
        }

        case IDC_WIZ_BTN_BROWSE_REL: {
            std::wstring f = BrowseAudioFile(hWnd, L"Select Default Release Sound File");
            if (!f.empty()) {
                pWiz->data.defaultReleaseFiles = { f };
                SetWindowTextW(pWiz->hEditDefRel, f.c_str());
            }
            break;
        }

        case IDC_WIZ_BTN_CLEAR_REL: {
            pWiz->data.defaultReleaseFiles.clear();
            SetWindowTextW(pWiz->hEditDefRel, L"");
            break;
        }

        case IDC_WIZ_BTN_PLAY_REL: {
            if (!pWiz->data.defaultReleaseFiles.empty()) {
                PlayAudioPreview(pWiz->data.defaultReleaseFiles[0]);
            }
            break;
        }

        // Step 3 Actions
        case IDC_WIZ_COMBO_KEY_LIST: {
            if (wmEvent == CBN_SELCHANGE) {
                WCHAR curText[128] = {};
                SendMessageW(pWiz->hComboKeyList, WM_GETTEXT, 128, (LPARAM)curText);
                pWiz->selectedKeyName = ExtractKeyNameFromCombo(curText);
                SetWindowTextW(pWiz->hEditCustomKey, L"");
                UpdateKeyEditFields(pWiz);
            }
            break;
        }

        case IDC_WIZ_BTN_ADD_KEY: {
            WCHAR customKeyBuf[64] = {};
            GetWindowTextW(pWiz->hEditCustomKey, customKeyBuf, 64);
            std::wstring customW = customKeyBuf;
            size_t first = customW.find_first_not_of(L" \t\r\n");
            if (first != std::wstring::npos) {
                size_t last = customW.find_last_not_of(L" \t\r\n");
                customW = customW.substr(first, last - first + 1);
                pWiz->selectedKeyName = Utils::WideToUtf8(customW);
                UpdateKeyEditFields(pWiz);
            }
            break;
        }

        case IDC_WIZ_BTN_BROWSE_KP: {
            if (pWiz->selectedKeyName.empty()) break;
            std::wstring f = BrowseAudioFile(hWnd, L"Select Key Press Sound File");
            if (!f.empty()) {
                KeySoundBinding* pBind = FindBinding(pWiz->data, pWiz->selectedKeyName);
                if (!pBind) {
                    KeySoundBinding newB;
                    newB.keyName = pWiz->selectedKeyName;
                    newB.pressPath = f;
                    pWiz->data.specificBindings.push_back(newB);
                } else {
                    pBind->pressPath = f;
                }
                UpdateKeyEditFields(pWiz);
                RefreshAssignedList(pWiz);
            }
            break;
        }

        case IDC_WIZ_BTN_CLEAR_KP: {
            KeySoundBinding* pBind = FindBinding(pWiz->data, pWiz->selectedKeyName);
            if (pBind) {
                pBind->pressPath = L"";
                if (pBind->pressPath.empty() && pBind->releasePath.empty()) {
                    for (auto it = pWiz->data.specificBindings.begin(); it != pWiz->data.specificBindings.end(); ++it) {
                        if (_stricmp(it->keyName.c_str(), pWiz->selectedKeyName.c_str()) == 0) {
                            pWiz->data.specificBindings.erase(it);
                            break;
                        }
                    }
                }
            }
            UpdateKeyEditFields(pWiz);
            RefreshAssignedList(pWiz);
            break;
        }

        case IDC_WIZ_BTN_PLAY_KP: {
            KeySoundBinding* pBind = FindBinding(pWiz->data, pWiz->selectedKeyName);
            if (pBind && !pBind->pressPath.empty()) {
                PlayAudioPreview(pBind->pressPath);
            }
            break;
        }

        case IDC_WIZ_BTN_BROWSE_KR: {
            if (pWiz->selectedKeyName.empty()) break;
            std::wstring f = BrowseAudioFile(hWnd, L"Select Key Release Sound File");
            if (!f.empty()) {
                KeySoundBinding* pBind = FindBinding(pWiz->data, pWiz->selectedKeyName);
                if (!pBind) {
                    KeySoundBinding newB;
                    newB.keyName = pWiz->selectedKeyName;
                    newB.releasePath = f;
                    pWiz->data.specificBindings.push_back(newB);
                } else {
                    pBind->releasePath = f;
                }
                UpdateKeyEditFields(pWiz);
                RefreshAssignedList(pWiz);
            }
            break;
        }

        case IDC_WIZ_BTN_CLEAR_KR: {
            KeySoundBinding* pBind = FindBinding(pWiz->data, pWiz->selectedKeyName);
            if (pBind) {
                pBind->releasePath = L"";
                if (pBind->pressPath.empty() && pBind->releasePath.empty()) {
                    for (auto it = pWiz->data.specificBindings.begin(); it != pWiz->data.specificBindings.end(); ++it) {
                        if (_stricmp(it->keyName.c_str(), pWiz->selectedKeyName.c_str()) == 0) {
                            pWiz->data.specificBindings.erase(it);
                            break;
                        }
                    }
                }
            }
            UpdateKeyEditFields(pWiz);
            RefreshAssignedList(pWiz);
            break;
        }

        case IDC_WIZ_BTN_PLAY_KR: {
            KeySoundBinding* pBind = FindBinding(pWiz->data, pWiz->selectedKeyName);
            if (pBind && !pBind->releasePath.empty()) {
                PlayAudioPreview(pBind->releasePath);
            }
            break;
        }

        case IDC_WIZ_BTN_REMOVE_BIND: {
            int sel = (int)SendMessageW(pWiz->hListAssigned, LB_GETCURSEL, 0, 0);
            if (sel >= 0 && sel < (int)pWiz->data.specificBindings.size()) {
                pWiz->data.specificBindings.erase(pWiz->data.specificBindings.begin() + sel);
                UpdateKeyEditFields(pWiz);
                RefreshAssignedList(pWiz);
            }
            break;
        }

        // Step 4 Test Buttons
        case IDC_WIZ_BTN_TEST_LEFT:
            PlayWizardTestSound(pWiz, "left", true);
            break;
        case IDC_WIZ_BTN_TEST_RIGHT:
            PlayWizardTestSound(pWiz, "right", true);
            break;
        case IDC_WIZ_BTN_TEST_MID:
            PlayWizardTestSound(pWiz, "middle", true);
            break;

        default:
            break;
        }
        break;
    }

    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wParam;
        RECT rcClient;
        GetClientRect(hWnd, &rcClient);
        FillRect(hdc, &rcClient, pWiz->hBgBrush);
        return 1;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rcClient;
        GetClientRect(hWnd, &rcClient);

        // Fill entire window client area
        FillRect(hdc, &rcClient, pWiz->hBgBrush);

        // Header Background
        const int headerH = DpiUtil::Scale(58, pWiz->dpi);
        RECT rcHeader = { 0, 0, rcClient.right, headerH };
        FillRect(hdc, &rcHeader, pWiz->hHeaderBrush);

        // Divider under header
        HPEN hPen = CreatePen(PS_SOLID, 1, WindowsTheme::IsUsingDarkMode() ? RGB(75, 75, 75) : RGB(215, 220, 225));
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        MoveToEx(hdc, 0, headerH, NULL);
        LineTo(hdc, rcClient.right, headerH);

        // Divider above footer
        int footerY = rcClient.bottom - DpiUtil::Scale(78, pWiz->dpi);
        MoveToEx(hdc, 0, footerY, NULL);
        LineTo(hdc, rcClient.right, footerY);

        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);

        // If on step 1 (Info), fill background for Profile Device Type group with white - not needed
        // Group box draws naturally on the gray background

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_CTLCOLORSTATIC: {
        HDC hdcStatic = (HDC)wParam;
        HWND hwndStatic = (HWND)lParam;
        bool dark = WindowsTheme::IsUsingDarkMode();

        if (hwndStatic == pWiz->hLblStepTitle || hwndStatic == pWiz->hLblStepSub) {
            SetBkMode(hdcStatic, TRANSPARENT);
            if (dark) {
                SetTextColor(hdcStatic, (hwndStatic == pWiz->hLblStepTitle) ? RGB(224, 224, 224) : RGB(170, 170, 170));
            } else {
                SetTextColor(hdcStatic, (hwndStatic == pWiz->hLblStepTitle) ? RGB(20, 30, 45) : RGB(80, 90, 100));
            }
            return (INT_PTR)pWiz->hHeaderBrush;
        }

        SetTextColor(hdcStatic, dark ? RGB(224, 224, 224) : GetSysColor(COLOR_WINDOWTEXT));
        SetBkColor(hdcStatic, dark ? RGB(32, 32, 32) : RGB(240, 240, 240));
        SetBkMode(hdcStatic, TRANSPARENT);
        return (INT_PTR)pWiz->hBgBrush;
    }

    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX: {
        if (WindowsTheme::IsUsingDarkMode()) {
            HDC hdcCtrl = (HDC)wParam;
            SetTextColor(hdcCtrl, RGB(224, 224, 224));
            SetBkColor(hdcCtrl, RGB(32, 32, 32));
            return (INT_PTR)pWiz->hBgBrush;
        }
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }

    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG: {
        HDC hdcDlg = (HDC)wParam;
        HWND hwndDlg = (HWND)lParam;
        bool dark = WindowsTheme::IsUsingDarkMode();

        if (!dark && (hwndDlg == pWiz->hRadKb || hwndDlg == pWiz->hRadMouse || hwndDlg == pWiz->hGrpType)) {
            SetBkColor(hdcDlg, RGB(255, 255, 255));
            SetBkMode(hdcDlg, TRANSPARENT);
            return (INT_PTR)pWiz->hWhiteBrush;
        }

        SetTextColor(hdcDlg, dark ? RGB(224, 224, 224) : GetSysColor(COLOR_WINDOWTEXT));
        SetBkColor(hdcDlg, dark ? RGB(32, 32, 32) : RGB(240, 240, 240));
        SetBkMode(hdcDlg, TRANSPARENT);
        return (INT_PTR)pWiz->hBgBrush;
    }

    case WM_DPICHANGED: {
        const RECT* suggested = reinterpret_cast<const RECT*>(lParam);
        SetWindowPos(hWnd, nullptr,
            suggested->left, suggested->top,
            suggested->right  - suggested->left,
            suggested->bottom - suggested->top,
            SWP_NOZORDER | SWP_NOACTIVATE);
        CreateWizardFonts(pWiz, HIWORD(wParam));
        LayoutWizardControls(pWiz, pWiz->dpi);
        ApplyWizardFonts(pWiz);
        InvalidateRect(hWnd, NULL, TRUE);
        return 0;
    }

    case WM_CLOSE:
        DestroyWindow(hWnd);
        return 0;

    case WM_DESTROY: {
        if (pWiz->hFontNormal) DeleteObject(pWiz->hFontNormal);
        if (pWiz->hFontBold) DeleteObject(pWiz->hFontBold);
        if (pWiz->hFontTitle) DeleteObject(pWiz->hFontTitle);
        if (pWiz->hFontHeader) DeleteObject(pWiz->hFontHeader);
        if (pWiz->hBgBrush) DeleteObject(pWiz->hBgBrush);
        if (pWiz->hHeaderBrush) DeleteObject(pWiz->hHeaderBrush);
        if (pWiz->hWhiteBrush) DeleteObject(pWiz->hWhiteBrush);

        pWiz->isRunning = false;
        return 0;
    }

    default:
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

bool ProfileWizard::Show(HWND hParentWnd, bool isKeyboard, std::wstring& outCreatedProfilePath) {
    WizardDialogState wizState;
    wizState.hParent = hParentWnd;
    wizState.data.deviceType = isKeyboard ? "keyboard" : "mouse";
    wizState.rng.seed((unsigned int)GetTickCount64());

    // Initialize temporary miniaudio engine for live sound previews
    ma_result res = ma_engine_init(NULL, &wizState.previewEngine);
    if (res == MA_SUCCESS) {
        wizState.previewEngineReady = true;
    }

    g_pWiz = &wizState;

    // Register Wizard Window Class
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WizardWndProc;
    wcex.hInstance = GetModuleHandle(NULL);
    wcex.hIcon = (HICON)GetClassLongPtrW(hParentWnd, GCLP_HICON);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    HBRUSH hClassBgBrush = CreateSolidBrush(WindowsTheme::IsUsingDarkMode() ? RGB(32, 32, 32) : RGB(240, 240, 240));
    wcex.hbrBackground = hClassBgBrush;
    wcex.lpszClassName = L"MonkeySoundsProfileWizard";

    RegisterClassExW(&wcex);

    // Calculate center relative to parent or screen, scaled to parent's DPI
    RECT rcParent;
    GetWindowRect(hParentWnd ? hParentWnd : GetDesktopWindow(), &rcParent);
    UINT parentDpi = hParentWnd ? DpiUtil::GetWindowDpi(hParentWnd) : GetDpiForSystem();
    int scaledWizW = DpiUtil::Scale(WIZARD_WIDTH,  parentDpi);
    int scaledWizH = DpiUtil::Scale(WIZARD_HEIGHT, parentDpi);
    int x = rcParent.left + ((rcParent.right - rcParent.left) - scaledWizW) / 2;
    int y = rcParent.top  + ((rcParent.bottom - rcParent.top) - scaledWizH) / 2;

    HWND hWnd = CreateWindowExW(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST | (Lang::IsRTL() ? WS_EX_LAYOUTRTL : 0),
        L"MonkeySoundsProfileWizard",
        TR("wizard.title"),
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        x, y, scaledWizW, scaledWizH,
        hParentWnd, NULL, GetModuleHandle(NULL), NULL
    );

    if (!hWnd) {
        if (wizState.previewEngineReady) {
            ma_engine_uninit(&wizState.previewEngine);
        }
        g_pWiz = nullptr;
        return false;
    }

    // Modal loop
    if (hParentWnd) {
        EnableWindow(hParentWnd, FALSE);
    }

    MSG msg;
    while (wizState.isRunning && GetMessageW(&msg, NULL, 0, 0)) {
        if (!IsDialogMessageW(hWnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (!IsWindow(hWnd)) break;
    }

    if (hParentWnd) {
        EnableWindow(hParentWnd, TRUE);
        SetForegroundWindow(hParentWnd);
    }

    if (wizState.previewEngineReady) {
        ma_engine_uninit(&wizState.previewEngine);
    }

    UnregisterClassW(L"MonkeySoundsProfileWizard", GetModuleHandle(NULL));
    DeleteObject(hClassBgBrush);

    g_pWiz = nullptr;
    outCreatedProfilePath = wizState.createdProfilePath;
    return wizState.success;
}
