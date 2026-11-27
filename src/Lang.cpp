// ---------------------------------------------------------------------------
// Lang.cpp  –  MonkeySounds Localization System
// ---------------------------------------------------------------------------
#include "framework.h"
#include "Lang.h"
#include "json.hpp"
#include <fstream>
#include <filesystem>
#include <shlobj.h>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace Lang {

// ---------------------------------------------------------------------------
// Supported locale table (code → display name)
// ---------------------------------------------------------------------------
const char* const kSupportedLocales[] = {
    "en",      "ar",      "bg",      "cs",      "da",
    "de",      "el",      "es",      "et",      "fi",
    "fr",      "he",      "hr",      "hu",      "id",
    "it",      "ja",      "ko",      "lt",      "lv",
    "ms",      "nb",      "nl",      "pl",      "pt-BR",
    "pt-PT",   "ro",      "ru",      "sk",      "sl",
    "sr-Cyrl", "sr-Latn", "sv",      "th",      "tr",
    "uk",      "vi",      "zh-Hans", "zh-Hant"
};
const wchar_t* const kSupportedLocaleNames[] = {
    L"English",
    L"\u0627\u0644\u0639\u0631\u0628\u064a\u0629 (Arabic)",
    L"\u0411\u044a\u043b\u0433\u0430\u0440\u0441\u043a\u0438 (Bulgarian)",
    L"\u010ce\u0161tina (Czech)",
    L"Dansk (Danish)",
    L"Deutsch (German)",
    L"\u0395\u03bb\u03bb\u03b7\u03bd\u03b9\u03ba\u03ac (Greek)",
    L"Espa\u00f1ol (Spanish)",
    L"Eesti (Estonian)",
    L"Suomi (Finnish)",
    L"Fran\u00e7ais (French)",
    L"\u05e2\u05d1\u05e8\u05d9\u05ea (Hebrew)",
    L"Hrvatski (Croatian)",
    L"Magyar (Hungarian)",
    L"Indonesia (Indonesian)",
    L"Italiano (Italian)",
    L"\u65e5\u672c\u8a9e (Japanese)",
    L"\ud55c\uad6d\uc5b4 (Korean)",
    L"Lietuvi\u0173 (Lithuanian)",
    L"Latvie\u0161u (Latvian)",
    L"Melayu (Malay)",
    L"Norsk (Norwegian)",
    L"Nederlands (Dutch)",
    L"Polski (Polish)",
    L"Portugu\u00eas (Brasil) (Portuguese Brazil)",
    L"Portugu\u00eas (Portugal) (Portuguese Portugal)",
    L"Rom\u00e2n\u0103 (Romanian)",
    L"\u0420\u0443\u0441\u0441\u043a\u0438\u0439 (Russian)",
    L"Sloven\u010dina (Slovak)",
    L"Sloven\u0161\u010dina (Slovenian)",
    L"\u0421\u0440\u043f\u0441\u043a\u0438 (\u045b\u0438\u0440\u0438\u043b\u0438\u0446\u0430) (Serbian Cyrillic)",
    L"\u0421\u0440\u043f\u0441\u043a\u0438 (latinica) (Serbian Latin)",
    L"Svenska (Swedish)",
    L"\u0e44\u0e17\u0e22 (Thai)",
    L"T\u00fcrk\u00e7e (Turkish)",
    L"\u0423\u043a\u0440\u0430\u0457\u043d\u0441\u044c\u043a\u0430 (Ukrainian)",
    L"Ti\u1ebfng Vi\u1ec7t (Vietnamese)",
    L"\u7b80\u4f53\u4e2d\u6587 (Chinese Simplified)",
    L"\u7e41\u9ad4\u4e2d\u6587 (Chinese Traditional)"
};
const int kLocaleCount = (int)(sizeof(kSupportedLocales) / sizeof(kSupportedLocales[0]));

// ---------------------------------------------------------------------------
// Built-in English fallback strings
// (populated at first Load() / static init)
// ---------------------------------------------------------------------------
static const std::unordered_map<std::string, std::wstring> kFallback = {
    // Tabs
    {"tab.sounds",      L"Sounds"},
    {"tab.settings",    L"Settings"},
    {"tab.about",       L"About"},
    // Sounds tab
    {"sounds.keyboard.group",     L"Keyboard Sounds"},
    {"sounds.keyboard.enable",    L"Enable Keyboard Sounds"},
    {"sounds.keyboard.newprofile",L"+ New Profile..."},
    {"sounds.keyboard.preset",    L"Preset:"},
    {"sounds.keyboard.volume",    L"Volume:"},
    {"sounds.keyboard.export",    L"Export"},
    {"sounds.keyboard.import",    L"Import ZIP..."},
    {"sounds.mouse.group",        L"Mouse Sounds"},
    {"sounds.mouse.enable",       L"Enable Mouse Sounds"},
    {"sounds.mouse.newprofile",   L"+ New Profile..."},
    {"sounds.mouse.preset",       L"Preset:"},
    {"sounds.mouse.volume",       L"Volume:"},
    {"sounds.mouse.export",       L"Export"},
    {"sounds.mouse.import",       L"Import ZIP..."},
    // Settings tab
    {"settings.group.app",          L"Application"},
    {"settings.version",            L"Current Version: "},
    {"settings.checkUpdates",       L"Check for Updates"},
    {"settings.autoStart",          L"Auto-start on Windows startup"},
    {"settings.startupNotif",       L"Show notification on startup"},
    {"settings.ignoreAutorepeat",   L"Ignore key autorepeat (typing & gaming)"},
    {"settings.updateOnStartup",    L"Check for updates on startup"},
    {"settings.updateInBackground", L"Check for updates in background"},
    {"settings.theme",              L"Theme:"},
    {"settings.theme.system",       L"System"},
    {"settings.theme.light",        L"Light"},
    {"settings.theme.dark",         L"Dark"},
    {"settings.language",           L"Language:"},
    {"settings.language.system",    L"System (Auto)"},
    {"settings.languageRestart",    L"Language changes take effect after restarting MonkeySounds."},
    // About tab
    {"about.title",    L"MonkeySounds"},
    {"about.desc1",    L"Professional system sound"},
    {"about.desc2",    L"customization utility."},
    {"about.copy",     L"\u00A9 2026 MonkeySounds. All rights reserved."},
    {"about.github",   L"GitHub"},
    {"about.discord",  L"Discord"},
    {"about.patreon",  L"Support on Patreon"},
    // Tray menu (from IDR_TRAY_MENU resource strings — these override at runtime)
    {"tray.show",       L"Show MonkeySounds"},
    {"tray.muteKb",     L"Mute Keyboard"},
    {"tray.muteMouse",  L"Mute Mouse"},
    {"tray.exit",       L"Exit"},
    // Startup balloon
    {"tray.balloon.title", L"MonkeySounds is running"},
    {"tray.balloon.body",  L"Keyboard & mouse sounds are active.\nRight-click the tray icon to mute or open settings."},
    // Status bar
    {"status.ready",          L"  Ready"},
    {"status.importing",      L"  Importing sound pack..."},
    {"status.importFailed",   L"  Import failed."},
    {"status.importBadProf",  L"  Import failed \u2013 invalid profile."},
    {"status.exporting",      L"  Exporting sound pack..."},
    {"status.exportFailed",   L"  Export failed."},
    {"status.addedFav",       L"  Added to favorites! \u2605"},
    {"status.removedFav",     L"  Removed from favorites."},
    // Update checker
    {"update.checking",      L"Checking..."},
    {"update.checkUpdates",  L"Check for Updates"},
    {"update.available.title",  L"Update Available"},
    {"update.available.body",   L"A new version of MonkeySounds is available!\n\nCurrent: {0}\nLatest:  {1}\n\nWould you like to open the releases page?"},
    {"update.uptodate.title",   L"Up to Date"},
    {"update.uptodate.body",    L"MonkeySounds is up to date.\n\nVersion: {0}"},
    {"update.failed.title",     L"Update Check Failed"},
    {"update.failed.body",      L"Could not connect to GitHub to check for updates.\nPlease check your internet connection."},
    // Profile info dialog
    {"profile.info.keyboard",   L"Keyboard Profile Info"},
    {"profile.info.mouse",      L"Mouse Profile Info"},
    {"profile.info.noProfile",  L"No profile is currently loaded."},
    // Import / Export
    {"import.title.keyboard",   L"Import Keyboard Sound Pack (.zip)"},
    {"import.title.mouse",      L"Import Mouse Sound Pack (.zip)"},
    {"import.filter",           L"Sound Pack ZIP (*.zip)\0*.zip\0All Files (*.*)\0*.*\0"},
    {"import.error.extract",    L"Failed to extract the ZIP file.\nMake sure it contains a valid profile.json."},
    {"import.error.noJson",     L"No profile.json found in the extracted ZIP.\nThe sound pack must contain a profile.json at its root."},
    {"import.error.badJson",    L"The profile.json was found but could not be loaded.\nPlease check that the file is valid."},
    {"import.error.title",      L"Import Error"},
    {"export.title.keyboard",   L"Export Keyboard Sound Pack"},
    {"export.title.mouse",      L"Export Mouse Sound Pack"},
    {"export.filter",           L"ZIP Archive (*.zip)\0*.zip\0"},
    {"export.error.noProfile",  L"No profile is currently loaded."},
    {"export.error.failed",     L"Failed to create ZIP archive."},
    {"export.error.title",      L"Export Error"},
    {"export.dialog.title",     L"Export"},
    // Wizard
    {"wizard.title",              L"Create Sound Profile Wizard"},
    {"wizard.step1.title",        L"Profile Information"},
    {"wizard.step1.sub",          L"Choose device type and provide profile name, author, and description."},
    {"wizard.step2.title.kb",     L"Default Keyboard Sounds"},
    {"wizard.step2.title.mouse",  L"Default Mouse Sounds"},
    {"wizard.step2.sub",          L"Select default press audio files (multiple for variations) and optional release audio."},
    {"wizard.step3.title.kb",     L"Special Key Sounds"},
    {"wizard.step3.title.mouse",  L"Button Specific Sounds"},
    {"wizard.step3.sub",          L"Configure dedicated sound effects for Spacebar, Enter, Backspace, Modifiers, or Mouse clicks."},
    {"wizard.step4.title",        L"Test & Create Profile"},
    {"wizard.step4.sub",          L"Test your new profile in real-time, then save or export it."},
    {"wizard.step.of",            L"Step {0} of 4: "},
    {"wizard.btn.back",           L"< Back"},
    {"wizard.btn.next",           L"Next >"},
    {"wizard.btn.finish",         L"Finish & Create"},
    {"wizard.btn.cancel",         L"Cancel"},
    {"wizard.grp.deviceType",     L"Profile Device Type"},
    {"wizard.rad.keyboard",       L"Keyboard Profile"},
    {"wizard.rad.mouse",          L"Mouse Profile"},
    {"wizard.lbl.name",           L"Profile Name:*"},
    {"wizard.lbl.author",         L"Author / Creator Name:"},
    {"wizard.lbl.desc",           L"Description / Switch Info:"},
    {"wizard.grp.defPress",       L"Default Press Sound(s) - Base Typing / Click"},
    {"wizard.grp.defRel",         L"Default Release Sound (Optional)"},
    {"wizard.grp.keySelect",      L"1. Select Key or Button"},
    {"wizard.grp.keySounds",      L"2. Assign Sounds for Selected Key"},
    {"wizard.grp.summary",        L"Profile Configuration Review"},
    {"wizard.grp.test",           L"Interactive Live Sound Test"},
    {"wizard.lbl.quickKey",       L"Standard Key:"},
    {"wizard.lbl.customKey",      L"Or Custom:"},
    {"wizard.lbl.keyPress",       L"Press Sound:"},
    {"wizard.lbl.keyRel",         L"Release Sound:"},
    {"wizard.lbl.assigned",       L"Configured Key / Button Mappings:"},
    {"wizard.lbl.testNote.kb",    L"Type below to test your new sound profile live before creating:"},
    {"wizard.lbl.testNote.mouse", L"Click the buttons below to test mouse clicks live before creating:"},
    {"wizard.chk.activate",       L"Activate this profile immediately in MonkeySounds"},
    {"wizard.chk.exportZip",      L"Also export as shareable Sound Pack (.zip)"},
    {"wizard.tip.multiPress",     L"Tip: Adding multiple press audio files enables natural round-robin variation on every keystroke."},
    {"wizard.btn.add",            L"Add..."},
    {"wizard.btn.remove",         L"Remove"},
    {"wizard.btn.play",           L"Play \u25B6"},
    {"wizard.btn.browse",         L"Browse..."},
    {"wizard.btn.clear",          L"\u2715"},
    {"wizard.btn.set",            L"Set"},
    {"wizard.btn.deleteMapping",  L"Delete Mapping"},
    {"wizard.btn.testLeft",       L"Test Left Click"},
    {"wizard.btn.testRight",      L"Test Right Click"},
    {"wizard.btn.testMid",        L"Test Middle Click"},
    {"wizard.validate.nameEmpty", L"Please enter a Profile Name."},
    {"wizard.validate.nameTitle", L"Required Field"},
    {"wizard.validate.noPress",   L"Please add at least one default press sound file."},
    {"wizard.validate.noPressTtl",L"Missing Sound"},
    {"wizard.created.msg",        L"Sound profile created successfully!"},
    {"wizard.created.title",      L"Profile Wizard"},
    {"wizard.failed.msg",         L"Failed to create the sound profile. Please check directory permissions."},
    {"wizard.failed.title",       L"Creation Error"},
    {"wizard.export.title",       L"Export Created Sound Pack ZIP"},
    {"wizard.audio.filter",       L"Audio Files (*.wav;*.mp3;*.ogg;*.flac)\0*.wav;*.mp3;*.ogg;*.flac\0All Files (*.*)\0*.*\0"},
    {"wizard.zip.filter",         L"ZIP Archive (*.zip)\0*.zip\0"},
    // Summary labels
    {"wizard.sum.device",         L"Device: "},
    {"wizard.sum.name",           L"Name: "},
    {"wizard.sum.author",         L"Author: "},
    {"wizard.sum.desc",           L"Description: "},
    {"wizard.sum.defPress",       L"Default Press Sounds: "},
    {"wizard.sum.defPressFmt",    L" file(s)"},
    {"wizard.sum.defRel",         L"Default Release Sound: "},
    {"wizard.sum.bindings",       L"\r\nSpecific Key / Button Mappings: "},
    {"wizard.sum.keyboard",       L"Keyboard"},
    {"wizard.sum.mouse",          L"Mouse"},
    {"wizard.sum.untitled",       L"(Untitled)"},
    {"wizard.sum.anon",           L"Anonymous"},
    {"wizard.sum.header",         L"=== Profile Summary ===\r\n\r\n"},
    {"wizard.sum.press",          L"press="},
    {"wizard.sum.release",        L"release="},
};

// ---------------------------------------------------------------------------
// Runtime state
// ---------------------------------------------------------------------------
static std::unordered_map<std::string, std::wstring> g_strings;
static std::string g_currentLocale = "en";

// ---------------------------------------------------------------------------
// Helper: get EXE directory
// ---------------------------------------------------------------------------
static std::wstring ExeDir() {
    WCHAR buf[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, buf, MAX_PATH);
    return fs::path(buf).parent_path().wstring();
}

// ---------------------------------------------------------------------------
// Helper: UTF-8 string → wstring
// ---------------------------------------------------------------------------
static std::wstring Utf8ToWide(const std::string& s) {
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &w[0], n);
    if (!w.empty() && w.back() == L'\0') w.pop_back();
    return w;
}

// ---------------------------------------------------------------------------
// Load
// ---------------------------------------------------------------------------
bool Load(const std::string& locale) {
    g_currentLocale = (locale.empty() || locale == "system") ? "en" : locale;

    // Update process default layout direction for newly created dialogs/MessageBoxes
    SetProcessDefaultLayout(IsRTL(g_currentLocale) ? LAYOUT_RTL : 0);

    // Always seed with English fallback first so missing keys degrade gracefully
    g_strings.clear();
    for (auto& kv : kFallback)
        g_strings[kv.first] = kv.second;

    if (g_currentLocale == "en") return true; // nothing more to do

    std::wstring path = fs::path(ExeDir()) / L"locales" / Utf8ToWide(g_currentLocale + ".json");
    std::ifstream f(path);
    if (!f.is_open()) return false;

    try {
        json j;
        f >> j;
        for (auto it = j.begin(); it != j.end(); ++it) {
            if (it.key() == "locale") continue;
            if (!it.value().is_string()) continue;
            g_strings[it.key()] = Utf8ToWide(it.value().get<std::string>());
        }
    } catch (...) {
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------
const std::string& CurrentLocale() { return g_currentLocale; }

bool IsRTL(const std::string& locale) {
    const std::string& loc = locale.empty() ? g_currentLocale : locale;
    return (loc == "ar" || loc == "he");
}

const wchar_t* Get(const char* key) {
    auto it = g_strings.find(key);
    if (it != g_strings.end()) return it->second.c_str();
    // Key missing from loaded locale: try fallback
    auto fb = kFallback.find(key);
    if (fb != kFallback.end()) return fb->second.c_str();
    // Completely unknown: return key itself as a debugging aid
    static thread_local std::wstring tls;
    tls = Utf8ToWide(key);
    return tls.c_str();
}

// ---------------------------------------------------------------------------
// System locale detection
// ---------------------------------------------------------------------------
std::string SystemLocale() {
    WCHAR buf[LOCALE_NAME_MAX_LENGTH] = {};
    if (GetUserDefaultLocaleName(buf, LOCALE_NAME_MAX_LENGTH) == 0)
        return "en";

    // Convert to narrow string
    char narrow[LOCALE_NAME_MAX_LENGTH] = {};
    WideCharToMultiByte(CP_ACP, 0, buf, -1, narrow, sizeof(narrow), nullptr, nullptr);
    std::string loc = narrow; // e.g. "de-DE", "zh-Hans-CN", "pt-BR"

    // Try exact match first
    for (int i = 0; i < kLocaleCount; ++i)
        if (_stricmp(kSupportedLocales[i], loc.c_str()) == 0) return kSupportedLocales[i];

    // Try language prefix (e.g. "de-DE" → "de")
    std::string prefix = loc.substr(0, loc.find('-'));
    for (int i = 0; i < kLocaleCount; ++i)
        if (_stricmp(kSupportedLocales[i], prefix.c_str()) == 0) return kSupportedLocales[i];

    // Handle zh variants
    if (loc.find("zh-Hant") != std::string::npos || loc.find("zh-TW") != std::string::npos ||
        loc.find("zh-HK") != std::string::npos || loc.find("zh-MO") != std::string::npos)
        return "zh-Hant";
    if (loc.find("zh") != std::string::npos)
        return "zh-Hans";

    return "en";
}

} // namespace Lang
