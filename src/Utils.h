#pragma once
#include <string>
#include <filesystem>
#include <windows.h>

namespace Utils {
    // String encoding helpers
    std::string WideToUtf8(const std::wstring& wstr);
    std::wstring Utf8ToWide(const std::string& str);

    // Performance & time helpers
    ULONGLONG FileTimeToULL(const FILETIME& ft);

    // Path resolution helpers
    std::wstring GetExeDirectory();
    bool IsPortableMode();
    std::wstring GetAppDataDirectory();
    std::wstring GetSettingsFilePath();
    std::wstring GetSoundsDirectory();
    std::wstring GetAssetPath(const std::wstring& filename);
}
