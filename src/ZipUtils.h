#pragma once
#include <string>

namespace ZipUtils {

    // Extract a .zip file into destDir (creates destDir if needed).
    // Returns true on success.
    bool ExtractZip(const std::wstring& zipPath, const std::wstring& destDir);

    // Pack all files in srcDir (flat — no subdirs) into a .zip at zipPath.
    // Returns true on success.
    bool CreateZipFromDir(const std::wstring& srcDir, const std::wstring& zipPath);

} // namespace ZipUtils
