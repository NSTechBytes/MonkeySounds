#include "framework.h"
#include "AppSettings.h"
#include "Utils.h"
#include "json.hpp"
#include <fstream>
#include <filesystem>
#include <shlobj.h>

namespace fs = std::filesystem;
using json = nlohmann::json;

#define REG_RUN_KEY L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define REG_APP_NAME L"MonkeySounds"

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
    return Utils::GetSettingsFilePath();
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
        if (j.contains("keyboardProfilePath")) m_config.keyboardProfilePath = Utils::Utf8ToWide(j["keyboardProfilePath"].get<std::string>());
        if (j.contains("mouseProfilePath")) m_config.mouseProfilePath = Utils::Utf8ToWide(j["mouseProfilePath"].get<std::string>());
        if (j.contains("showStartupNotification")) m_config.showStartupNotification = j["showStartupNotification"].get<bool>();
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
        j["keyboardProfilePath"] = Utils::WideToUtf8(m_config.keyboardProfilePath);
        j["mouseProfilePath"] = Utils::WideToUtf8(m_config.mouseProfilePath);
        j["showStartupNotification"] = m_config.showStartupNotification;

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
