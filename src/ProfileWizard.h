#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <unordered_map>

struct KeySoundBinding {
    std::string keyName;        // e.g. "space", "enter", "backspace", "shift", "left", etc.
    std::wstring pressPath;     // absolute path to source audio file
    std::wstring releasePath;   // optional absolute path to source audio file
};

struct WizardProfileData {
    std::string deviceType = "keyboard"; // "keyboard" or "mouse"
    std::string name;
    std::string author;
    std::string description;

    // Default sounds (can have multiple press files for round-robin variation)
    std::vector<std::wstring> defaultPressFiles;
    std::vector<std::wstring> defaultReleaseFiles;

    // Specific key / button bindings
    std::vector<KeySoundBinding> specificBindings;

    // Options on save
    bool activateImmediately = true;
    bool exportZip = false;
    std::wstring exportZipPath;
};

class ProfileWizard {
public:
    // Launch the modal Profile Wizard. Returns true if profile was created successfully.
    // outCreatedProfilePath receives the path to the newly generated profile.json.
    static bool Show(HWND hParentWnd, bool isKeyboard, std::wstring& outCreatedProfilePath);
};
