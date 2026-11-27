#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <uxtheme.h>
#include <dwmapi.h>
#include <string>
#include <functional>

namespace WindowsTheme {

enum class Mode {
  System,
  Dark,
  Light
};

using SettingsReader = std::function<std::string()>;
using SettingsWriter = std::function<void(const std::string &)>;

/// Register callbacks for reading and persisting theme setting.
void SetSettingsHandlers(SettingsReader reader, SettingsWriter writer);

/// Initialize theme hooks and uxtheme/dwmapi exports.
void Initialize();

/// Refresh theme mode (e.g. after setting change or WM_SETTINGCHANGE).
void Refresh();

/// Returns whether dark mode is currently effective.
bool IsUsingDarkMode();

/// Returns current configured mode.
Mode GetMode();

/// Sets preferred mode (Mode::System, Mode::Dark, Mode::Light).
void SetMode(Mode mode);

/// Sets mode by string ("system", "dark", "light").
void SetMode(const std::string &modeStr);

/// Apply theme (dark / light) to a standard Win32 window (including titlebar).
void ApplyToWindow(HWND window);

/// Apply theme to a window and every child control WITHOUT installing the
/// WindowsTheme parent subclass — the host WndProc keeps ownership of
/// WM_ERASEBKGND / WM_CTLCOLORSTATIC and can apply per-control colors.
/// Also themes the dark titlebar via DWM.
void ApplyToControlTree(HWND window, HWND progressBar = nullptr);

/// Apply theme to an individual control (ListView, Button, ComboBox, Edit, etc.).
void ApplyToControl(HWND control);

/// Apply theme to a dialog or control hierarchy.
void ApplyToDialog(HWND dialog);

/// Apply theme to a tooltip window (tooltips_class32).
void ApplyToTooltip(HWND tooltipWindow);

/// Prepare and update menu theme right before showing a popup menu.
void PrepareMenu(HWND owner);

COLORREF GetTextColor();
COLORREF GetDisabledTextColor();

/// Returns the dark-mode window background color.
COLORREF GetDarkBackgroundColor();

/// Returns the dark-mode tab/page background color.
COLORREF GetPageBackgroundColor();

/// Returns a shared, persistent brush for the dark-mode background color.
/// @warning Owned by WindowsTheme — do NOT DeleteObject it.
HBRUSH GetDarkBackgroundBrush();

/// Returns a shared, persistent brush for the dark-mode tab/page background color.
/// @warning Owned by WindowsTheme — do NOT DeleteObject it.
HBRUSH GetPageBackgroundBrush();

void EnableDialogTexture(HWND window, bool enable);

/// Helper to resolve whether system uses dark mode directly from registry.
bool IsSystemDarkMode();

/// Scope-based helper to theme dialogs created in the current thread (e.g. MessageBox, FileDialog).
class DialogHookScope {
public:
  DialogHookScope();
  ~DialogHookScope();

  DialogHookScope(const DialogHookScope &) = delete;
  DialogHookScope &operator=(const DialogHookScope &) = delete;

private:
  HHOOK m_Hook = nullptr;
};

} // namespace WindowsTheme
