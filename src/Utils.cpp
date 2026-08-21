#include "framework.h"
#include "Utils.h"
#include <shlobj.h>
#include <filesystem>

namespace fs = std::filesystem;

namespace Utils {

std::string WideToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

std::wstring Utf8ToWide(const std::string& str) {
    if (str.empty()) return L"";
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

ULONGLONG FileTimeToULL(const FILETIME& ft) {
    return ((ULONGLONG)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
}

std::wstring GetExeDirectory() {
    WCHAR szExePath[MAX_PATH] = {};
    GetModuleFileNameW(NULL, szExePath, MAX_PATH);
    return fs::path(szExePath).parent_path().wstring();
}

bool IsPortableMode() {
    std::wstring exeDir = GetExeDirectory();
    fs::path exeSettings = fs::path(exeDir) / L"settings.json";
    return fs::exists(exeSettings);
}

std::wstring GetAppDataDirectory() {
    PWSTR ppszPath = NULL;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &ppszPath))) {
        std::wstring path = ppszPath;
        CoTaskMemFree(ppszPath);
        return (fs::path(path) / L"MonkeySounds").wstring();
    }
    WCHAR szEnv[MAX_PATH] = {};
    DWORD dwRet = GetEnvironmentVariableW(L"APPDATA", szEnv, MAX_PATH);
    if (dwRet > 0 && dwRet < MAX_PATH) {
        return (fs::path(szEnv) / L"MonkeySounds").wstring();
    }
    return GetExeDirectory();
}

std::wstring GetSettingsFilePath() {
    if (IsPortableMode()) {
        return (fs::path(GetExeDirectory()) / L"settings.json").wstring();
    }
    std::wstring appDataDir = GetAppDataDirectory();
    fs::create_directories(appDataDir);
    return (fs::path(appDataDir) / L"settings.json").wstring();
}

std::wstring GetSoundsDirectory() {
    if (IsPortableMode()) {
        std::wstring exeSounds = (fs::path(GetExeDirectory()) / L"Sounds").wstring();
        if (fs::exists(exeSounds)) {
            return exeSounds;
        }
    }
    std::wstring appDataSounds = (fs::path(GetAppDataDirectory()) / L"Sounds").wstring();
    return appDataSounds;
}

std::wstring GetAssetPath(const std::wstring& filename) {
    std::wstring exeDir = GetExeDirectory();
    fs::path p1 = fs::path(exeDir) / L"assets" / filename;
    if (fs::exists(p1)) return p1.wstring();

    fs::path p2 = fs::path(exeDir) / filename;
    if (fs::exists(p2)) return p2.wstring();

    fs::path p3 = fs::path(L"assets") / filename;
    if (fs::exists(p3)) return p3.wstring();

    return filename;
}

} // namespace Utils
