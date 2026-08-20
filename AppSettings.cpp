#include "framework.h"
#include "AppSettings.h"
#include "json.hpp"
#include <fstream>
#include <filesystem>
#include <shlobj.h>

namespace fs = std::filesystem;
using json = nlohmann::json;

#define REG_RUN_KEY L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define REG_APP_NAME L"MonkeySounds"

static std::string WideToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

static std::wstring Utf8ToWide(const std::string& str) {
    if (str.empty()) return L"";
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

AppSettings& AppSettings::GetInstance() {
    static AppSettings instance;
    return instance;
}

AppSettings::AppSettings() {
    m_config.autoStart = IsAutoStartEnabled();
}

AppSettings::~AppSettings() {
}

std::wstring AppSettings::GetConfigFilePath() const {
    PWSTR ppszPath = NULL;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &ppszPath))) {
        std::wstring path = ppszPath;
        CoTaskMemFree(ppszPath);
        fs::create_directories(path + L"\\MonkeySounds");
        return path + L"\\MonkeySounds\\settings.json";
    }
    return L"settings.json";
}

void AppSettings::Load() {
    std::wstring path = GetConfigFilePath();
    try {
        std::ifstream file(path);
        if (!file.is_open()) return;

        json j;
        file >> j;

        if (j.contains("keyboardEnabled")) m_config.keyboardEnabled = j["keyboardEnabled"].get<bool>();
        if (j.contains("mouseEnabled")) m_config.mouseEnabled = j["mouseEnabled"].get<bool>();
        if (j.contains("keyboardVolume")) m_config.keyboardVolume = j["keyboardVolume"].get<float>();
        if (j.contains("mouseVolume")) m_config.mouseVolume = j["mouseVolume"].get<float>();
        if (j.contains("keyboardProfilePath")) m_config.keyboardProfilePath = Utf8ToWide(j["keyboardProfilePath"].get<std::string>());
        if (j.contains("mouseProfilePath")) m_config.mouseProfilePath = Utf8ToWide(j["mouseProfilePath"].get<std::string>());
    } catch (...) {}

    m_config.autoStart = IsAutoStartEnabled();
}

void AppSettings::Save() {
    std::wstring path = GetConfigFilePath();
    try {
        json j;
        j["keyboardEnabled"] = m_config.keyboardEnabled;
        j["mouseEnabled"] = m_config.mouseEnabled;
        j["keyboardVolume"] = m_config.keyboardVolume;
        j["mouseVolume"] = m_config.mouseVolume;
        j["keyboardProfilePath"] = WideToUtf8(m_config.keyboardProfilePath);
        j["mouseProfilePath"] = WideToUtf8(m_config.mouseProfilePath);

        std::ofstream file(path);
        if (file.is_open()) {
            file << j.dump(4);
        }
    } catch (...) {}

    SetAutoStart(m_config.autoStart);
}

bool AppSettings::IsAutoStartEnabled() const {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, REG_RUN_KEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        WCHAR szBuffer[MAX_PATH];
        DWORD dwSize = sizeof(szBuffer);
        DWORD dwType = REG_SZ;
        LONG lRes = RegQueryValueExW(hKey, REG_APP_NAME, NULL, &dwType, (LPBYTE)szBuffer, &dwSize);
        RegCloseKey(hKey);
        return (lRes == ERROR_SUCCESS);
    }
    return false;
}

void AppSettings::SetAutoStart(bool enable) {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, REG_RUN_KEY, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        if (enable) {
            WCHAR szExePath[MAX_PATH];
            GetModuleFileNameW(NULL, szExePath, MAX_PATH);
            RegSetValueExW(hKey, REG_APP_NAME, 0, REG_SZ, (const BYTE*)szExePath, (DWORD)((wcslen(szExePath) + 1) * sizeof(WCHAR)));
        } else {
            RegDeleteValueW(hKey, REG_APP_NAME);
        }
        RegCloseKey(hKey);
    }
}
