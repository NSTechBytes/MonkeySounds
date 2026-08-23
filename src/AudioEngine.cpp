#include "framework.h"
#include "AudioEngine.h"
#include "Utils.h"
#include "json.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <shlwapi.h>
#include <shlobj.h>

#pragma warning(push)
#pragma warning(disable: 4244 4267)

// ---------------------------------------------------------------------------
// miniaudio — strip unused backends and decoders to reduce EXE size.
//
// Safe to disable: all non-Windows audio backends, FLAC decoder, encoding.
// Must keep: WASAPI, WAV, MP3, OGG — these are the only ones we use.
// Do NOT disable: MA_NO_NODE_GRAPH, MA_NO_ENGINE — ma_engine and
// ma_sound_group depend on the node graph internally.
// ---------------------------------------------------------------------------

// Non-Windows backends — never compiled on Windows anyway, but explicit
// defines prevent miniaudio from even trying to detect/include them.
#define MA_NO_DSOUND        // Skip DirectSound (we use WASAPI)
#define MA_NO_WINMM         // Skip WinMM legacy backend
#define MA_NO_PULSEAUDIO    // Linux
#define MA_NO_ALSA          // Linux
#define MA_NO_COREAUDIO     // macOS
#define MA_NO_SNDIO         // OpenBSD
#define MA_NO_AUDIO4        // NetBSD
#define MA_NO_OSS           // BSD
#define MA_NO_AAUDIO        // Android
#define MA_NO_OPENSL        // Android
#define MA_NO_WEBAUDIO      // WebAssembly
#define MA_NO_NULL          // Null/silent backend

// Decoders we don't use
#define MA_NO_FLAC          // FLAC not needed

// We never write audio data
#define MA_NO_ENCODING

// No waveform/noise synthesis
#define MA_NO_GENERATION

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#pragma warning(pop)

#pragma comment(lib, "shlwapi.lib")

namespace fs = std::filesystem;
using json = nlohmann::json;

static ma_engine g_maEngine;
static ma_sound_group g_kbSoundGroup;
static ma_sound_group g_mouseSoundGroup;
static bool g_engineReady = false;

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

    std::string utf8Path = Utils::WideToUtf8(filePath);
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
    std::wstring soundsDir = Utils::GetSoundsDirectory();
    std::wstring kbSounds = (fs::path(soundsDir) / L"Keyboard").wstring();
    ScanProfilesInDirectory(kbSounds, "keyboard", list);

    std::wstring exeSounds = (fs::path(Utils::GetExeDirectory()) / L"Sounds" / L"Keyboard").wstring();
    if (_wcsicmp(exeSounds.c_str(), kbSounds.c_str()) != 0 && fs::exists(exeSounds)) {
        ScanProfilesInDirectory(exeSounds, "keyboard", list);
    }

    return list;
}

std::vector<SoundProfileInfo> AudioEngine::ScanMouseProfiles() {
    std::vector<SoundProfileInfo> list;
    std::wstring soundsDir = Utils::GetSoundsDirectory();
    std::wstring mouseSounds = (fs::path(soundsDir) / L"Mouse").wstring();
    ScanProfilesInDirectory(mouseSounds, "mouse", list);

    std::wstring exeSounds = (fs::path(Utils::GetExeDirectory()) / L"Sounds" / L"Mouse").wstring();
    if (_wcsicmp(exeSounds.c_str(), mouseSounds.c_str()) != 0 && fs::exists(exeSounds)) {
        ScanProfilesInDirectory(exeSounds, "mouse", list);
    }

    return list;
}

static std::string ToLower(const std::string& str) {
    std::string s = str;
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)::tolower(c); });
    return s;
}

static std::string NormalizeKeyOrId(const std::string& str) {
    std::string s = ToLower(str);
    // Remove all asterisks (Mechvibes wildcard notation: *key*1 → key1)
    s.erase(std::remove(s.begin(), s.end(), '*'), s.end());
    // Strip leading/trailing whitespace
    size_t first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = s.find_last_not_of(" \t\r\n");
    s = s.substr(first, (last - first + 1));
    return s;
}

static const SoundSource* FindSoundSource(const std::unordered_map<std::string, SoundSource>& sources, const std::string& soundId) {
    auto it = sources.find(soundId);
    if (it != sources.end()) return &it->second;

    std::string norm = NormalizeKeyOrId(soundId);
    it = sources.find(norm);
    if (it != sources.end()) return &it->second;

    for (const auto& pair : sources) {
        if (NormalizeKeyOrId(pair.first) == norm) {
            return &pair.second;
        }
    }
    return nullptr;
}

static std::vector<std::string> GetVkCodeAliases(int vkCode) {
    std::vector<std::string> aliases;
    switch (vkCode) {
    case VK_SPACE:
        aliases = { "space", "*space*" };
        break;
    case VK_RETURN:
        aliases = { "enter", "return", "*enter*", "*return*" };
        break;
    case VK_BACK:
        aliases = { "backspace", "back", "*backspace*", "*back*", "delete" };
        break;
    case VK_DELETE:
        aliases = { "delete", "del", "*delete*", "*del*" };
        break;
    case VK_TAB:
        aliases = { "tab", "*tab*" };
        break;
    case VK_ESCAPE:
        aliases = { "escape", "esc", "*escape*", "*esc*" };
        break;
    case VK_CAPITAL:
        aliases = { "capslock", "caps_lock", "caps", "*capslock*", "*caps_lock*" };
        break;
    case VK_LSHIFT:
        aliases = { "shift_l", "lshift", "shift", "*shift_l*", "*shift*" };
        break;
    case VK_RSHIFT:
        aliases = { "shift_r", "rshift", "shift", "*shift_r*", "*shift*" };
        break;
    case VK_SHIFT:
        aliases = { "shift", "shift_l", "shift_r", "*shift*" };
        break;
    case VK_LCONTROL:
        aliases = { "ctrl_l", "lctrl", "ctrl", "control", "*ctrl_l*", "*ctrl*" };
        break;
    case VK_RCONTROL:
        aliases = { "ctrl_r", "rctrl", "ctrl", "control", "*ctrl_r*", "*ctrl*" };
        break;
    case VK_CONTROL:
        aliases = { "ctrl", "ctrl_l", "ctrl_r", "control", "*ctrl*" };
        break;
    case VK_LMENU:
        aliases = { "alt_l", "lalt", "alt", "*alt_l*", "*alt*" };
        break;
    case VK_RMENU:
        aliases = { "alt_r", "ralt", "alt_gr", "alt", "*alt_r*", "*alt*" };
        break;
    case VK_MENU:
        aliases = { "alt", "alt_l", "alt_r", "alt_gr", "*alt*" };
        break;
    case VK_LWIN:
    case VK_RWIN:
        aliases = { "cmd", "win", "windows", "meta", "super", "*cmd*", "*win*" };
        break;
    case VK_APPS:
        aliases = { "menu", "apps", "context_menu", "*menu*" };
        break;
    case VK_INSERT:
        aliases = { "insert", "ins", "*insert*" };
        break;
    case VK_HOME:
        aliases = { "home", "*home*" };
        break;
    case VK_END:
        aliases = { "end", "*end*" };
        break;
    case VK_PRIOR:
        aliases = { "page_up", "pageup", "pgup", "*page_up*", "*pageup*" };
        break;
    case VK_NEXT:
        aliases = { "page_down", "pagedown", "pgdn", "*page_down*", "*pagedown*" };
        break;
    case VK_LEFT:
        aliases = { "left", "arrow_left", "leftarrow", "*left*" };
        break;
    case VK_RIGHT:
        aliases = { "right", "arrow_right", "rightarrow", "*right*" };
        break;
    case VK_UP:
        aliases = { "up", "arrow_up", "uparrow", "*up*" };
        break;
    case VK_DOWN:
        aliases = { "down", "arrow_down", "downarrow", "*down*" };
        break;
    case VK_NUMLOCK:
        aliases = { "numlock", "num_lock", "*numlock*" };
        break;
    case VK_SNAPSHOT:
        aliases = { "printscreen", "print_screen", "prtscn", "*printscreen*" };
        break;
    case VK_SCROLL:
        aliases = { "scrolllock", "scroll_lock", "*scrolllock*" };
        break;
    case VK_PAUSE:
        aliases = { "pause", "*pause*" };
        break;
    default:
        if (vkCode >= 'A' && vkCode <= 'Z') {
            char ch = (char)::tolower(vkCode);
            aliases = { std::string(1, ch) };
        } else if (vkCode >= '0' && vkCode <= '9') {
            aliases = { std::string(1, (char)vkCode) };
        } else if (vkCode >= VK_NUMPAD0 && vkCode <= VK_NUMPAD9) {
            aliases = { "numpad" + std::to_string(vkCode - VK_NUMPAD0) };
        } else if (vkCode >= VK_F1 && vkCode <= VK_F24) {
            aliases = { "f" + std::to_string(vkCode - VK_F1 + 1) };
        }
        break;
    }
    return aliases;
}

static std::vector<std::string> GetMouseAliases(const std::string& buttonName) {
    std::string norm = NormalizeKeyOrId(buttonName);
    if (norm == "left" || norm == "click_left" || norm == "primary" || norm == "lclick") {
        return { "left", "click_left", "primary", "lclick", "button1" };
    }
    if (norm == "right" || norm == "click_right" || norm == "secondary" || norm == "rclick") {
        return { "right", "click_right", "secondary", "rclick", "button2" };
    }
    if (norm == "middle" || norm == "click_middle" || norm == "wheel" || norm == "mclick") {
        return { "middle", "click_middle", "wheel", "mclick", "button3" };
    }
    return { norm };
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

        if (j.contains("profile") && j["profile"].is_object()) {
            auto& p = j["profile"];
            if (p.contains("name") && p["name"].is_string()) profile.name = p["name"].get<std::string>();
            if (p.contains("author") && p["author"].is_string()) profile.author = p["author"].get<std::string>();
            if (p.contains("description") && p["description"].is_string()) profile.description = p["description"].get<std::string>();
        }

        if (profile.name.empty()) {
            profile.name = fs::path(profileJsonPath).parent_path().filename().string();
        }

        if (j.contains("keys") && j["keys"].is_object()) {
            auto& k = j["keys"];
            if (k.contains("default")) {
                if (k["default"].is_array()) {
                    for (auto& item : k["default"]) {
                        if (item.is_string()) profile.defaultKeys.push_back(item.get<std::string>());
                    }
                } else if (k["default"].is_string()) {
                    profile.defaultKeys.push_back(k["default"].get<std::string>());
                }
            }

            if (k.contains("other") && k["other"].is_array()) {
                for (auto& rule : k["other"]) {
                    if (rule.is_object() && rule.contains("sound") && rule["sound"].is_string()) {
                        std::string soundId = rule["sound"].get<std::string>();
                        if (rule.contains("keys")) {
                            if (rule["keys"].is_array()) {
                                for (auto& keyItem : rule["keys"]) {
                                    if (keyItem.is_string()) {
                                        std::string normKey = NormalizeKeyOrId(keyItem.get<std::string>());
                                        profile.keyToSoundsMap[normKey].push_back(soundId);
                                    }
                                }
                            } else if (rule["keys"].is_string()) {
                                std::string normKey = NormalizeKeyOrId(rule["keys"].get<std::string>());
                                profile.keyToSoundsMap[normKey].push_back(soundId);
                            }
                        }
                    }
                }
            }
        }

        if (j.contains("sources") && j["sources"].is_array()) {
            for (auto& s : j["sources"]) {
                if (!s.is_object() || !s.contains("id") || !s["id"].is_string()) continue;
                SoundSource src;
                src.id = s["id"].get<std::string>();

                if (s.contains("source")) {
                    if (s["source"].is_string()) {
                        std::string rel = s["source"].get<std::string>();
                        fs::path fullPath = fs::path(profile.folderPath) / Utils::Utf8ToWide(rel);
                        src.pressFilePath = fullPath.lexically_normal().wstring();
                        src.releaseFilePath = L"";
                    } else if (s["source"].is_object()) {
                        auto& srcObj = s["source"];
                        if (srcObj.contains("press") && srcObj["press"].is_string()) {
                            std::string pressRel = srcObj["press"].get<std::string>();
                            fs::path fullPath = fs::path(profile.folderPath) / Utils::Utf8ToWide(pressRel);
                            src.pressFilePath = fullPath.lexically_normal().wstring();
                        }
                        if (srcObj.contains("release") && srcObj["release"].is_string()) {
                            std::string relRel = srcObj["release"].get<std::string>();
                            fs::path fullPath = fs::path(profile.folderPath) / Utils::Utf8ToWide(relRel);
                            src.releaseFilePath = fullPath.lexically_normal().wstring();
                        }
                    }
                }
                profile.sources[src.id] = src;
                profile.sources[NormalizeKeyOrId(src.id)] = src;
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

        if (j.contains("profile") && j["profile"].is_object()) {
            auto& p = j["profile"];
            if (p.contains("name") && p["name"].is_string()) profile.name = p["name"].get<std::string>();
            if (p.contains("author") && p["author"].is_string()) profile.author = p["author"].get<std::string>();
            if (p.contains("description") && p["description"].is_string()) profile.description = p["description"].get<std::string>();
        }

        if (profile.name.empty()) {
            profile.name = fs::path(profileJsonPath).parent_path().filename().string();
        }

        if (j.contains("buttons") && j["buttons"].is_object()) {
            auto& b = j["buttons"];
            if (b.contains("default")) {
                if (b["default"].is_array()) {
                    for (auto& item : b["default"]) {
                        if (item.is_string()) profile.defaultButtons.push_back(item.get<std::string>());
                    }
                } else if (b["default"].is_string()) {
                    profile.defaultButtons.push_back(b["default"].get<std::string>());
                }
            }

            if (b.contains("other") && b["other"].is_array()) {
                for (auto& rule : b["other"]) {
                    if (rule.is_object() && rule.contains("sound") && rule["sound"].is_string()) {
                        std::string soundId = rule["sound"].get<std::string>();
                        if (rule.contains("buttons")) {
                            if (rule["buttons"].is_array()) {
                                for (auto& btnItem : rule["buttons"]) {
                                    if (btnItem.is_string()) {
                                        std::string normBtn = NormalizeKeyOrId(btnItem.get<std::string>());
                                        profile.buttonToSoundsMap[normBtn].push_back(soundId);
                                    }
                                }
                            } else if (rule["buttons"].is_string()) {
                                std::string normBtn = NormalizeKeyOrId(rule["buttons"].get<std::string>());
                                profile.buttonToSoundsMap[normBtn].push_back(soundId);
                            }
                        }
                    }
                }
            }
        }

        if (j.contains("sources") && j["sources"].is_array()) {
            for (auto& s : j["sources"]) {
                if (!s.is_object() || !s.contains("id") || !s["id"].is_string()) continue;
                SoundSource src;
                src.id = s["id"].get<std::string>();

                if (s.contains("source")) {
                    if (s["source"].is_string()) {
                        std::string rel = s["source"].get<std::string>();
                        fs::path fullPath = fs::path(profile.folderPath) / Utils::Utf8ToWide(rel);
                        src.pressFilePath = fullPath.lexically_normal().wstring();
                        src.releaseFilePath = L"";
                    } else if (s["source"].is_object()) {
                        auto& srcObj = s["source"];
                        if (srcObj.contains("press") && srcObj["press"].is_string()) {
                            std::string pressRel = srcObj["press"].get<std::string>();
                            fs::path fullPath = fs::path(profile.folderPath) / Utils::Utf8ToWide(pressRel);
                            src.pressFilePath = fullPath.lexically_normal().wstring();
                        }
                        if (srcObj.contains("release") && srcObj["release"].is_string()) {
                            std::string relRel = srcObj["release"].get<std::string>();
                            fs::path fullPath = fs::path(profile.folderPath) / Utils::Utf8ToWide(relRel);
                            src.releaseFilePath = fullPath.lexically_normal().wstring();
                        }
                    }
                }
                profile.sources[src.id] = src;
                profile.sources[NormalizeKeyOrId(src.id)] = src;
            }
        }

        m_currentMouseProfile = profile;
        m_currentMousePath = profileJsonPath;
        return true;
    } catch (...) {
        return false;
    }
}

void AudioEngine::PlayKey(int vkCode, bool isPress) {
    if (!m_keyboardEnabled || !m_initialized) return;

    std::wstring soundPathToPlay;

    {
        std::lock_guard<std::mutex> lock(m_audioMutex);
        std::string soundId;

        if (isPress) {
            std::vector<std::string> aliases = GetVkCodeAliases(vkCode);
            for (const auto& alias : aliases) {
                std::string norm = NormalizeKeyOrId(alias);
                auto it = m_currentKbProfile.keyToSoundsMap.find(norm);
                if (it != m_currentKbProfile.keyToSoundsMap.end() && !it->second.empty()) {
                    std::uniform_int_distribution<size_t> dist(0, it->second.size() - 1);
                    soundId = it->second[dist(m_rng)];
                    break;
                }
            }

            if (soundId.empty() && !m_currentKbProfile.defaultKeys.empty()) {
                std::uniform_int_distribution<size_t> dist(0, m_currentKbProfile.defaultKeys.size() - 1);
                soundId = m_currentKbProfile.defaultKeys[dist(m_rng)];
            }

            if (!soundId.empty()) {
                m_activeKeySounds[vkCode] = soundId;
                const SoundSource* pSrc = FindSoundSource(m_currentKbProfile.sources, soundId);
                if (pSrc) {
                    soundPathToPlay = !pSrc->pressFilePath.empty() ? pSrc->pressFilePath : pSrc->releaseFilePath;
                }
            }
        } else {
            auto itActive = m_activeKeySounds.find(vkCode);
            if (itActive != m_activeKeySounds.end()) {
                soundId = itActive->second;
                m_activeKeySounds.erase(itActive);
            } else {
                std::vector<std::string> aliases = GetVkCodeAliases(vkCode);
                for (const auto& alias : aliases) {
                    std::string norm = NormalizeKeyOrId(alias);
                    auto it = m_currentKbProfile.keyToSoundsMap.find(norm);
                    if (it != m_currentKbProfile.keyToSoundsMap.end() && !it->second.empty()) {
                        soundId = it->second[0];
                        break;
                    }
                }
                if (soundId.empty() && !m_currentKbProfile.defaultKeys.empty()) {
                    soundId = m_currentKbProfile.defaultKeys[0];
                }
            }

            if (!soundId.empty()) {
                const SoundSource* pSrc = FindSoundSource(m_currentKbProfile.sources, soundId);
                if (pSrc && !pSrc->releaseFilePath.empty()) {
                    soundPathToPlay = pSrc->releaseFilePath;
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

        std::vector<std::string> aliases = GetMouseAliases(buttonName);
        for (const auto& alias : aliases) {
            std::string norm = NormalizeKeyOrId(alias);
            auto it = m_currentMouseProfile.buttonToSoundsMap.find(norm);
            if (it != m_currentMouseProfile.buttonToSoundsMap.end() && !it->second.empty()) {
                std::uniform_int_distribution<size_t> dist(0, it->second.size() - 1);
                soundId = it->second[dist(m_rng)];
                break;
            }
        }

        if (soundId.empty() && !m_currentMouseProfile.defaultButtons.empty()) {
            std::uniform_int_distribution<size_t> dist(0, m_currentMouseProfile.defaultButtons.size() - 1);
            soundId = m_currentMouseProfile.defaultButtons[dist(m_rng)];
        }

        if (!soundId.empty()) {
            const SoundSource* pSrc = FindSoundSource(m_currentMouseProfile.sources, soundId);
            if (pSrc) {
                if (isPress) {
                    soundPathToPlay = !pSrc->pressFilePath.empty() ? pSrc->pressFilePath : pSrc->releaseFilePath;
                } else if (!pSrc->releaseFilePath.empty()) {
                    soundPathToPlay = pSrc->releaseFilePath;
                }
            }
        }
    }

    if (!soundPathToPlay.empty()) {
        PlaySoundInternal(soundPathToPlay, false);
    }
}
