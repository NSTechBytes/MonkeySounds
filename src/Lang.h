#pragma once
// ---------------------------------------------------------------------------
// Lang.h  –  MonkeySounds Localization System
//
// Usage:
//   Lang::Load("de");          // load a locale once at startup / on change
//   const wchar_t* s = TR("tab.sounds");  // retrieve a translated string
//   std::wstring   s = TRS("tab.sounds"); // same but as std::wstring
//
// Locale JSON files live next to the EXE in:
//   <ExeDir>\locales\<code>.json
//
// Falls back to the built-in English strings if a key is missing or the
// locale file cannot be loaded.
// ---------------------------------------------------------------------------

#include <string>
#include <unordered_map>

namespace Lang {

// Load a locale by its BCP-47 code, e.g. "en", "de", "zh-Hans".
// Call once on startup and again whenever the user changes the language.
// Returns true on success, false if the file was not found (falls back to en).
bool Load(const std::string& locale);

// Return the current locale code (e.g. "en", "de").
const std::string& CurrentLocale();

// Return true if the given locale (or current locale when empty) is RTL.
bool IsRTL(const std::string& locale = "");

// Translate a key.  Returns the English fallback when the key is missing.
const wchar_t* Get(const char* key);

// Convenience:  wstring version.
inline std::wstring GetS(const char* key) { return Get(key); }

// Detect the system UI language and return its BCP-47 code.
std::string SystemLocale();

// All locale codes shipped with MonkeySounds (must match files in locales/).
extern const char* const kSupportedLocales[];
extern const wchar_t* const kSupportedLocaleNames[];  // display names
extern const int kLocaleCount;

} // namespace Lang

// Convenience short-hand macros used throughout the source.
#define TR(key)  Lang::Get(key)
#define TRS(key) Lang::GetS(key)
