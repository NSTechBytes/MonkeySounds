#include "framework.h"
#include "ZipUtils.h"
#include "Utils.h"

#include <filesystem>
#include <fstream>
#include <vector>
#include <string>

// ---- zlib / minizip --------------------------------------------------------
#include "third_party/zlib/zlib.h"
#include "third_party/zlib/minizip/zip.h"
#include "third_party/zlib/minizip/unzip.h"
#include "third_party/zlib/minizip/iowin32.h"

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::string WideToUtf8(const std::wstring& w) {
    return Utils::WideToUtf8(w);
}

static std::wstring Utf8ToWide(const std::string& s) {
    return Utils::Utf8ToWide(s);
}

// Read entire file into a buffer
static std::vector<char> ReadFileBuf(const std::wstring& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f.is_open()) return {};
    std::streamsize sz = f.tellg();
    f.seekg(0, std::ios::beg);
    std::vector<char> buf(static_cast<size_t>(sz));
    f.read(buf.data(), sz);
    return buf;
}

// ---------------------------------------------------------------------------
// ExtractZip
// ---------------------------------------------------------------------------

bool ZipUtils::ExtractZip(const std::wstring& zipPath, const std::wstring& destDir) {
    // Use Win32 ioapi so paths with Unicode work correctly
    zlib_filefunc64_def ffunc;
    fill_win32_filefunc64W(&ffunc);

    unzFile uf = unzOpen2_64(zipPath.c_str(), &ffunc);
    if (!uf) return false;

    fs::create_directories(destDir);

    unz_global_info64 gi{};
    if (unzGetGlobalInfo64(uf, &gi) != UNZ_OK) {
        unzClose(uf);
        return false;
    }

    for (uLong i = 0; i < gi.number_entry; ++i) {
        char szFilename[512]{};
        unz_file_info64 fi{};
        if (unzGetCurrentFileInfo64(uf, &fi, szFilename, sizeof(szFilename),
                                    nullptr, 0, nullptr, 0) != UNZ_OK) break;

        // Convert stored UTF-8 filename to wide
        std::wstring entryName = Utf8ToWide(szFilename);

        // Strip any leading path separators / drive letters for safety
        fs::path entryPath(entryName);
        fs::path outPath = fs::path(destDir) / entryPath.filename();

        bool isDir = (!entryName.empty() &&
                      (entryName.back() == L'/' || entryName.back() == L'\\'));

        if (isDir) {
            fs::create_directories(fs::path(destDir) / entryPath);
        } else {
            // Ensure parent exists
            fs::create_directories(outPath.parent_path());

            if (unzOpenCurrentFile(uf) == UNZ_OK) {
                std::ofstream ofs(outPath, std::ios::binary);
                if (ofs.is_open()) {
                    std::vector<char> buf(65536);
                    int bytesRead = 0;
                    while ((bytesRead = unzReadCurrentFile(uf, buf.data(),
                                        static_cast<unsigned>(buf.size()))) > 0) {
                        ofs.write(buf.data(), bytesRead);
                    }
                }
                unzCloseCurrentFile(uf);
            }
        }

        if (i + 1 < gi.number_entry) {
            if (unzGoToNextFile(uf) != UNZ_OK) break;
        }
    }

    unzClose(uf);
    return true;
}

// ---------------------------------------------------------------------------
// CreateZipFromDir  — packs every file directly inside srcDir into the zip.
// Sub-directories are skipped intentionally (sound profiles are flat folders).
// ---------------------------------------------------------------------------

bool ZipUtils::CreateZipFromDir(const std::wstring& srcDir, const std::wstring& zipPath) {
    zlib_filefunc64_def ffunc;
    fill_win32_filefunc64W(&ffunc);

    zipFile zf = zipOpen2_64(zipPath.c_str(), APPEND_STATUS_CREATE, nullptr, &ffunc);
    if (!zf) return false;

    bool ok = true;

    try {
        for (const auto& entry : fs::directory_iterator(srcDir)) {
            if (!entry.is_regular_file()) continue;

            std::wstring filePath = entry.path().wstring();
            std::string  arcName  = WideToUtf8(entry.path().filename().wstring());

            std::vector<char> buf = ReadFileBuf(filePath);
            if (buf.empty() && fs::file_size(entry.path()) > 0) { ok = false; continue; }

            zip_fileinfo zfi{};
            // Populate date/time from file metadata
            HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_READ,
                                       FILE_SHARE_READ, nullptr,
                                       OPEN_EXISTING, 0, nullptr);
            if (hFile != INVALID_HANDLE_VALUE) {
                FILETIME ftCreate, ftAccess, ftWrite;
                if (GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite)) {
                    SYSTEMTIME st{};
                    FileTimeToLocalFileTime(&ftWrite, &ftWrite);
                    FileTimeToSystemTime(&ftWrite, &st);
                    zfi.tmz_date.tm_year = st.wYear;
                    zfi.tmz_date.tm_mon  = st.wMonth  - 1;
                    zfi.tmz_date.tm_mday = st.wDay;
                    zfi.tmz_date.tm_hour = st.wHour;
                    zfi.tmz_date.tm_min  = st.wMinute;
                    zfi.tmz_date.tm_sec  = st.wSecond;
                }
                CloseHandle(hFile);
            }

            if (zipOpenNewFileInZip(zf, arcName.c_str(), &zfi,
                                    nullptr, 0, nullptr, 0, nullptr,
                                    Z_DEFLATED, Z_DEFAULT_COMPRESSION) != ZIP_OK) {
                ok = false;
                continue;
            }

            if (!buf.empty()) {
                zipWriteInFileInZip(zf, buf.data(), static_cast<unsigned>(buf.size()));
            }
            zipCloseFileInZip(zf);
        }
    } catch (...) {
        ok = false;
    }

    zipClose(zf, nullptr);
    return ok;
}
