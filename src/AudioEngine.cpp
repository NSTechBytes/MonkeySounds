#include "framework.h"
#include "AudioEngine.h"
#include "json.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <shlwapi.h>
#include <shlobj.h>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#pragma comment(lib, "shlwapi.lib")

namespace fs = std::filesystem;
using json = nlohmann::json;

static ma_engine g_maEngine;
static ma_sound_group g_kbSoundGroup;
static ma_sound_group g_mouseSoundGroup;
static bool g_engineReady = false;

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

static std::wstring GetAppDataSoundsPath() {
    PWSTR ppszPath = NULL;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &ppszPath))) {
        std::wstring path = ppszPath;
        CoTaskMemFree(ppszPath);
        return path + L"\\MonkeySounds\\Sounds";
    }
    return L"C:\\Users\\nasirshahbaz\\AppData\\Roaming\\MonkeySounds\\Sounds";
}

AudioEngine& AudioEngine::GetInstance() {
    static AudioEngine instance;
    return instance;
}

AudioEngine::AudioEngine() : m_rng(std::random_device{}()) {
}

AudioEngine::~AudioEngine() {
    Shutdown();
}

bool AudioEngine::Initialize() {
    if (m_initialized) return true;

    ma_engine_config engineConfig = ma_engine_config_init();
    engineConfig.channels = 2;
    engineConfig.sampleRate = 44100;

    ma_result result = ma_engine_init(&engineConfig, &g_maEngine);
    if (result != MA_SUCCESS) {
        result = ma_engine_init(NULL, &g_maEngine);
        if (result != MA_SUCCESS) {
            return false;
        }
    }

    ma_sound_group_init(&g_maEngine, 0, NULL, &g_kbSoundGroup);
    ma_sound_group_init(&g_maEngine, 0, NULL, &g_mouseSoundGroup);
    ma_sound_group_set_volume(&g_kbSoundGroup, m_keyboardVolume);
    ma_sound_group_set_volume(&g_mouseSoundGroup, m_mouseVolume);

    g_engineReady = true;
    m_initialized = true;
    return true;
}

void AudioEngine::Shutdown() {
    if (!m_initialized) return;
    g_engineReady = false;
    m_initialized = false;
    ma_sound_group_uninit(&g_kbSoundGroup);
    ma_sound_group_uninit(&g_mouseSoundGroup);
    ma_engine_uninit(&g_maEngine);
}

void AudioEngine::SetKeyboardVolume(float vol) {
    if (vol < 0.0f) vol = 0.0f;
    if (vol > 1.0f) vol = 1.0f;
    m_keyboardVolume = vol;
    if (g_engineReady) {
        ma_sound_group_set_volume(&g_kbSoundGroup, m_keyboardVolume);
    }
}

void AudioEngine::SetMouseVolume(float vol) {
    if (vol < 0.0f) vol = 0.0f;
    if (vol > 1.0f) vol = 1.0f;
    m_mouseVolume = vol;
    if (g_engineReady) {
        ma_sound_group_set_volume(&g_mouseSoundGroup, m_mouseVolume);
    }
}

void AudioEngine::PlaySoundInternal(const std::wstring& filePath, bool isKeyboard) {
    if (!m_initialized || !g_engineReady || filePath.empty()) return;
    if (!fs::exists(filePath)) return;

    std::string utf8Path = WideToUtf8(filePath);
    ma_sound_group* parentGroup = isKeyboard ? &g_kbSoundGroup : &g_mouseSoundGroup;
    ma_engine_play_sound(&g_maEngine, utf8Path.c_str(), parentGroup);
}

void AudioEngine::ScanProfilesInDirectory(const std::wstring& dir, const std::string& expectedDevice, std::vector<SoundProfileInfo>& outList) {
    if (!fs::exists(dir)) return;

    try {
        for (const auto& entry : fs::recursive_directory_iterator(dir, fs::directory_options::skip_permission_denied)) {
            if (entry.is_regular_file() && entry.path().filename() == L"profile.json") {
                try {
                    std::ifstream f(entry.path());
                    if (!f.is_open()) continue;
                    json j;
                    f >> j;

                    SoundProfileInfo info;
                    info.profileJsonPath = entry.path().wstring();
                    info.folderPath = entry.path().parent_path().wstring();

                    if (j.contains("profile")) {
                        auto& p = j["profile"];
                        if (p.contains("name")) info.name = p["name"].get<std::string>();
                        if (p.contains("author")) info.author = p["author"].get<std::string>();
                        if (p.contains("description")) info.description = p["description"].get<std::string>();
                        if (p.contains("device")) info.deviceType = p["device"].get<std::string>();
                    }

                    if (info.name.empty()) {
                        info.name = entry.path().parent_path().filename().string();
                    }

                    if (info.deviceType.empty()) {
                        info.deviceType = expectedDevice;
                    }

                    outList.push_back(info);
                } catch (...) {}
            }
        }
    } catch (...) {}
}

std::vector<SoundProfileInfo> AudioEngine::ScanKeyboardProfiles() {
    std::vector<SoundProfileInfo> list;
    std::wstring appDataSounds = GetAppDataSoundsPath() + L"\\Keyboard";
    ScanProfilesInDirectory(appDataSounds, "keyboard", list);

    // Also check current folder / Sounds / Keyboard if any
    ScanProfilesInDirectory(L"Sounds\\Keyboard", "keyboard", list);

    return list;
}

std::vector<SoundProfileInfo> AudioEngine::ScanMouseProfiles() {
    std::vector<SoundProfileInfo> list;
    std::wstring appDataSounds = GetAppDataSoundsPath() + L"\\Mouse";
    ScanProfilesInDirectory(appDataSounds, "mouse", list);

    // Also check current folder / Sounds / Mouse if any
    ScanProfilesInDirectory(L"Sounds\\Mouse", "mouse", list);

    return list;
}

bool AudioEngine::LoadKeyboardProfile(const std::wstring& profileJsonPath) {
    std::lock_guard<std::mutex> lock(m_audioMutex);
    try {
        std::ifstream file(profileJsonPath);
        if (!file.is_open()) return false;

        json j;
        file >> j;

        KeyboardProfile profile;
        profile.folderPath = fs::path(profileJsonPath).parent_path().wstring();

        if (j.contains("profile")) {
            auto& p = j["profile"];
            if (p.contains("name")) profile.name = p["name"].get<std::string>();
            if (p.contains("author")) profile.author = p["author"].get<std::string>();
            if (p.contains("description")) profile.description = p["description"].get<std::string>();
        }

        if (profile.name.empty()) {
            profile.name = fs::path(profileJsonPath).parent_path().filename().string();
        }

        if (j.contains("keys")) {
            auto& k = j["keys"];
            if (k.contains("default") && k["default"].is_array()) {
                for (auto& item : k["default"]) {
                    profile.defaultKeys.push_back(item.get<std::string>());
                }
            }
            if (k.contains("other") && k["other"].is_array()) {
                for (auto& rule : k["other"]) {
                    if (rule.contains("sound") && rule.contains("keys") && rule["keys"].is_array()) {
                        std::string soundId = rule["sound"].get<std::string>();
                        for (auto& keyName : rule["keys"]) {
                            profile.keyToSoundMap[keyName.get<std::string>()] = soundId;
                        }
                    }
                }
            }
        }

        if (j.contains("sources") && j["sources"].is_array()) {
            for (auto& s : j["sources"]) {
                SoundSource src;
                if (s.contains("id")) src.id = s["id"].get<std::string>();
                if (s.contains("source")) {
                    auto& srcObj = s["source"];
                    if (srcObj.contains("press")) {
                        std::string pressRel = srcObj["press"].get<std::string>();
                        fs::path fullPath = fs::path(profile.folderPath) / Utf8ToWide(pressRel);
                        src.pressFilePath = fullPath.lexically_normal().wstring();
                    }
                    if (srcObj.contains("release")) {
                        std::string relRel = srcObj["release"].get<std::string>();
                        fs::path fullPath = fs::path(profile.folderPath) / Utf8ToWide(relRel);
                        src.releaseFilePath = fullPath.lexically_normal().wstring();
                    }
                }
                profile.sources[src.id] = src;
            }
        }

        m_currentKbProfile = profile;
        m_currentKbPath = profileJsonPath;
        m_activeKeySounds.clear();
        return true;
    } catch (...) {
        return false;
    }
}

bool AudioEngine::LoadMouseProfile(const std::wstring& profileJsonPath) {
    std::lock_guard<std::mutex> lock(m_audioMutex);
    try {
        std::ifstream file(profileJsonPath);
        if (!file.is_open()) return false;

        json j;
        file >> j;

        MouseProfile profile;
        profile.folderPath = fs::path(profileJsonPath).parent_path().wstring();

        if (j.contains("profile")) {
            auto& p = j["profile"];
            if (p.contains("name")) profile.name = p["name"].get<std::string>();
            if (p.contains("author")) profile.author = p["author"].get<std::string>();
            if (p.contains("description")) profile.description = p["description"].get<std::string>();
        }

        if (profile.name.empty()) {
            profile.name = fs::path(profileJsonPath).parent_path().filename().string();
        }

        if (j.contains("buttons")) {
            auto& b = j["buttons"];
            if (b.contains("default") && b["default"].is_string()) {
                profile.defaultButton = b["default"].get<std::string>();
            }
            if (b.contains("other") && b["other"].is_array()) {
                for (auto& rule : b["other"]) {
                    if (rule.contains("sound") && rule.contains("buttons") && rule["buttons"].is_array()) {
                        std::string soundId = rule["sound"].get<std::string>();
                        for (auto& btnName : rule["buttons"]) {
                            profile.buttonToSoundMap[btnName.get<std::string>()] = soundId;
                        }
                    }
                }
            }
        }

        if (j.contains("sources") && j["sources"].is_array()) {
            for (auto& s : j["sources"]) {
                SoundSource src;
                if (s.contains("id")) src.id = s["id"].get<std::string>();
                if (s.contains("source")) {
                    auto& srcObj = s["source"];
                    if (srcObj.contains("press")) {
                        std::string pressRel = srcObj["press"].get<std::string>();
                        fs::path fullPath = fs::path(profile.folderPath) / Utf8ToWide(pressRel);
                        src.pressFilePath = fullPath.lexically_normal().wstring();
                    }
                    if (srcObj.contains("release")) {
                        std::string relRel = srcObj["release"].get<std::string>();
                        fs::path fullPath = fs::path(profile.folderPath) / Utf8ToWide(relRel);
                        src.releaseFilePath = fullPath.lexically_normal().wstring();
                    }
                }
                profile.sources[src.id] = src;
            }
        }

        m_currentMouseProfile = profile;
        m_currentMousePath = profileJsonPath;
        return true;
    } catch (...) {
        return false;
    }
}

std::string AudioEngine::MapVkCodeToKeyName(int vkCode) {
    switch (vkCode) {
    case VK_BACK: return "backspace";
    case VK_DELETE: return "delete";
    case VK_RETURN: return "enter";
    case VK_SPACE: return "space";
    case VK_TAB: return "tab";
    case VK_ESCAPE: return "escape";
    case VK_CAPITAL: return "capslock";
    case VK_SHIFT:
    case VK_LSHIFT:
    case VK_RSHIFT: return "shift";
    case VK_CONTROL:
    case VK_LCONTROL:
    case VK_RCONTROL: return "ctrl";
    case VK_MENU:
    case VK_LMENU:
    case VK_RMENU: return "alt";
    case VK_LEFT: return "left";
    case VK_RIGHT: return "right";
    case VK_UP: return "up";
    case VK_DOWN: return "down";
    default: return "";
    }
}

void AudioEngine::PlayKey(int vkCode, bool isPress) {
    if (!m_keyboardEnabled || !m_initialized) return;

    std::wstring soundPathToPlay;

    {
        std::lock_guard<std::mutex> lock(m_audioMutex);
        std::string soundId;

        if (isPress) {
            std::string keyName = MapVkCodeToKeyName(vkCode);
            auto itOther = m_currentKbProfile.keyToSoundMap.find(keyName);
            if (itOther != m_currentKbProfile.keyToSoundMap.end()) {
                soundId = itOther->second;
            } else if (!m_currentKbProfile.defaultKeys.empty()) {
                std::uniform_int_distribution<size_t> dist(0, m_currentKbProfile.defaultKeys.size() - 1);
                soundId = m_currentKbProfile.defaultKeys[dist(m_rng)];
            }

            if (!soundId.empty()) {
                m_activeKeySounds[vkCode] = soundId;
                auto itSrc = m_currentKbProfile.sources.find(soundId);
                if (itSrc != m_currentKbProfile.sources.end()) {
                    soundPathToPlay = itSrc->second.pressFilePath;
                }
            }
        } else {
            auto itActive = m_activeKeySounds.find(vkCode);
            if (itActive != m_activeKeySounds.end()) {
                soundId = itActive->second;
                m_activeKeySounds.erase(itActive);
            } else {
                std::string keyName = MapVkCodeToKeyName(vkCode);
                auto itOther = m_currentKbProfile.keyToSoundMap.find(keyName);
                if (itOther != m_currentKbProfile.keyToSoundMap.end()) {
                    soundId = itOther->second;
                } else if (!m_currentKbProfile.defaultKeys.empty()) {
                    soundId = m_currentKbProfile.defaultKeys[0];
                }
            }

            if (!soundId.empty()) {
                auto itSrc = m_currentKbProfile.sources.find(soundId);
                if (itSrc != m_currentKbProfile.sources.end()) {
                    soundPathToPlay = itSrc->second.releaseFilePath;
                }
            }
        }
    }

    if (!soundPathToPlay.empty()) {
        PlaySoundInternal(soundPathToPlay, true);
    }
}

void AudioEngine::PlayMouse(const std::string& buttonName, bool isPress) {
    if (!m_mouseEnabled || !m_initialized) return;

    std::wstring soundPathToPlay;

    {
        std::lock_guard<std::mutex> lock(m_audioMutex);
        std::string soundId;

        auto itOther = m_currentMouseProfile.buttonToSoundMap.find(buttonName);
        if (itOther != m_currentMouseProfile.buttonToSoundMap.end()) {
            soundId = itOther->second;
        } else if (!m_currentMouseProfile.defaultButton.empty()) {
            soundId = m_currentMouseProfile.defaultButton;
        }

        if (!soundId.empty()) {
            auto itSrc = m_currentMouseProfile.sources.find(soundId);
            if (itSrc != m_currentMouseProfile.sources.end()) {
                soundPathToPlay = isPress ? itSrc->second.pressFilePath : itSrc->second.releaseFilePath;
            }
        }
    }

    if (!soundPathToPlay.empty()) {
        PlaySoundInternal(soundPathToPlay, false);
    }
}
