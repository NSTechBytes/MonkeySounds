#pragma once
#include <string>

struct AppConfig {
    bool keyboardEnabled = true;
    bool mouseEnabled = true;
    float keyboardVolume = 0.8f;
    float mouseVolume = 0.8f;
    std::wstring keyboardProfilePath;
    std::wstring mouseProfilePath;
    bool autoStart = false;
};

class AppSettings {
public:
    static AppSettings& GetInstance();

    void Load();
    void Save();

    AppConfig& GetConfig() { return m_config; }
    const AppConfig& GetConfig() const { return m_config; }

    void SetAutoStart(bool enable);
    bool IsAutoStartEnabled() const;

private:
    AppSettings();
    ~AppSettings();

    std::wstring GetConfigFilePath() const;
    AppConfig m_config;
};
