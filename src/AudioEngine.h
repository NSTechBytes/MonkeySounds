#pragma once
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <random>

struct SoundSource {
    std::string id;
    std::wstring pressFilePath;
    std::wstring releaseFilePath;
};

struct SoundProfileInfo {
    std::wstring profileJsonPath;
    std::wstring folderPath;
    std::string name;
    std::string author;
    std::string description;
    std::string deviceType; // "keyboard" or "mouse"
};

struct KeyboardProfile {
    std::wstring folderPath;
    std::string name;
    std::string author;
    std::string description;
    std::vector<std::string> defaultKeys;
    std::unordered_map<std::string, std::vector<std::string>> keyToSoundsMap; // normalized keyName -> list of soundIds
    std::unordered_map<std::string, SoundSource> sources;                     // normalized soundId -> SoundSource
};

struct MouseProfile {
    std::wstring folderPath;
    std::string name;
    std::string author;
    std::string description;
    std::vector<std::string> defaultButtons;
    std::unordered_map<std::string, std::vector<std::string>> buttonToSoundsMap; // normalized buttonName -> list of soundIds
    std::unordered_map<std::string, SoundSource> sources;                        // normalized soundId -> SoundSource
};

class AudioEngine {
public:
    static AudioEngine& GetInstance();

    bool Initialize();
    void Shutdown();

    // Profile scanning & loading
    std::vector<SoundProfileInfo> ScanKeyboardProfiles();
    std::vector<SoundProfileInfo> ScanMouseProfiles();
    bool LoadKeyboardProfile(const std::wstring& profileJsonPath);
    bool LoadMouseProfile(const std::wstring& profileJsonPath);

    // Playback methods
    void PlayKey(int vkCode, bool isPress);
    void PlayMouse(const std::string& buttonName, bool isPress);

    // Settings
    void SetKeyboardEnabled(bool enabled) { m_keyboardEnabled = enabled; }
    bool IsKeyboardEnabled() const { return m_keyboardEnabled; }

    void SetMouseEnabled(bool enabled) { m_mouseEnabled = enabled; }
    bool IsMouseEnabled() const { return m_mouseEnabled; }

    void SetKeyboardVolume(float vol); // 0.0f to 1.0f
    float GetKeyboardVolume() const { return m_keyboardVolume; }

    void SetMouseVolume(float vol); // 0.0f to 1.0f
    float GetMouseVolume() const { return m_mouseVolume; }

    const KeyboardProfile& GetCurrentKeyboardProfile() const { return m_currentKbProfile; }
    const MouseProfile& GetCurrentMouseProfile() const { return m_currentMouseProfile; }

    std::wstring GetCurrentKeyboardProfilePath() const { return m_currentKbPath; }
    std::wstring GetCurrentMouseProfilePath() const { return m_currentMousePath; }

private:
    AudioEngine();
    ~AudioEngine();

    void PlaySoundInternal(const std::wstring& filePath, bool isKeyboard);
    std::string MapVkCodeToKeyName(int vkCode);
    void ScanProfilesInDirectory(const std::wstring& dir, const std::string& expectedDevice, std::vector<SoundProfileInfo>& outList);

    bool m_initialized = false;
    bool m_keyboardEnabled = true;
    bool m_mouseEnabled = true;
    float m_keyboardVolume = 0.8f;
    float m_mouseVolume = 0.8f;

    std::wstring m_currentKbPath;
    std::wstring m_currentMousePath;
    KeyboardProfile m_currentKbProfile;
    MouseProfile m_currentMouseProfile;

    // Track which sound ID was selected for a key while held down
    std::unordered_map<int, std::string> m_activeKeySounds;
    std::mutex m_audioMutex;

    std::mt19937 m_rng;
};
