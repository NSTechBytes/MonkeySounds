/* Derived from Novadesk WindowsTheme (originally derived from win32-darkmodelib) */

#define NOMINMAX
#include "WindowsTheme.h"

#include <atomic>
#include <mutex>
#include <algorithm>
#include <vector>
#include <dwmapi.h>
#include <uxtheme.h>
#include <vsstyle.h>
#include <vssym32.h>
#include <commctrl.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "comctl32.lib")

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1
#define DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 19
#endif

namespace WindowsTheme {

namespace {

// ---------------------------------------------------------------------------
// Color constants
// ---------------------------------------------------------------------------
constexpr COLORREF g_Background     = RGB(32, 32, 32);
constexpr COLORREF g_PageBackground = RGB(37, 37, 37);
constexpr COLORREF g_ControlBg      = RGB(56, 56, 56);
constexpr COLORREF g_HotBg          = RGB(69, 69, 69);
constexpr COLORREF g_Text           = RGB(224, 224, 224);
constexpr COLORREF g_DisabledText   = RGB(128, 128, 128);
constexpr COLORREF g_Edge           = RGB(75, 75, 75);

enum class ControlType { Button, Tab, ComboBox, Header, Edit, Spinner, ListView, ListBox, Separator, StatusBar };

// ---------------------------------------------------------------------------
// Uxtheme ordinal typedefs
// ---------------------------------------------------------------------------
using fnSetPreferredAppMode              = int(WINAPI*)(int);
using fnAllowDarkModeForWindow           = bool(WINAPI*)(HWND, bool);
using fnAllowDarkModeForApp              = bool(WINAPI*)(bool);
using fnRefreshImmersiveColorPolicyState = void(WINAPI*)();
using fnFlushMenuThemes                  = void(WINAPI*)();

// ---------------------------------------------------------------------------
// Global theme state
// ---------------------------------------------------------------------------
struct ThemeState {
  std::atomic<bool> initialized{false};
  std::atomic<bool> isDarkMode{false};
  Mode mode{Mode::System};

  fnSetPreferredAppMode              pfnSetPreferredAppMode = nullptr;
  fnAllowDarkModeForWindow           pfnAllowDarkModeForWindow = nullptr;
  fnAllowDarkModeForApp              pfnAllowDarkModeForApp = nullptr;
  fnRefreshImmersiveColorPolicyState pfnRefreshImmersiveColorPolicyState = nullptr;
  fnFlushMenuThemes                  pfnFlushMenuThemes = nullptr;

  std::mutex     handlerMutex;
  SettingsReader settingsReader = nullptr;
  SettingsWriter settingsWriter = nullptr;

  // Brushes owned by ThemeState
  HBRUSH brBackground     = CreateSolidBrush(g_Background);
  HBRUSH brPageBackground = CreateSolidBrush(g_PageBackground);
  HBRUSH brControlBg      = CreateSolidBrush(g_ControlBg);
  HBRUSH brHotBg          = CreateSolidBrush(g_HotBg);
  HBRUSH brEdge           = CreateSolidBrush(g_Edge);

  ~ThemeState() {
    DeleteObject(brBackground);
    DeleteObject(brPageBackground);
    DeleteObject(brControlBg);
    DeleteObject(brHotBg);
    DeleteObject(brEdge);
  }
};

ThemeState& GetState() {
  static ThemeState state;
  return state;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
bool HasClass(HWND w, const WCHAR* name) {
  WCHAR cls[64] = {};
  return GetClassNameW(w, cls, 64) && _wcsicmp(cls, name) == 0;
}

bool IsTabSurface(HWND ctrl) {
  HWND parent = GetParent(ctrl);
  if (!parent) return false;
  if (HasClass(parent, WC_TABCONTROLW)) return true;
  for (HWND p = parent; p; p = GetParent(p)) {
    if (HasClass(p, L"#32770") && (GetWindowLongPtrW(p, GWL_STYLE) & WS_CHILD)) return true;
  }
  for (HWND sibling = GetWindow(parent, GW_CHILD); sibling; sibling = GetWindow(sibling, GW_HWNDNEXT)) {
    if (sibling == ctrl || !HasClass(sibling, WC_TABCONTROLW)) continue;
    RECT tabRc, ctrlRc;
    GetWindowRect(sibling, &tabRc);
    GetWindowRect(ctrl, &ctrlRc);
    if (ctrlRc.left >= tabRc.left && ctrlRc.right <= tabRc.right &&
        ctrlRc.top >= tabRc.top && ctrlRc.bottom <= tabRc.bottom) {
      return true;
    }
  }
  return false;
}

HBRUSH GetSurfaceBrush(HWND w) {
  return IsTabSurface(w) ? GetState().brPageBackground : GetState().brBackground;
}

void ClipSiblings(HWND window, HDC dc) {
  HWND parent = GetParent(window);
  if (!parent) return;
  for (HWND sibling = GetWindow(parent, GW_CHILD); sibling; sibling = GetWindow(sibling, GW_HWNDNEXT)) {
    if (sibling == window || !IsWindowVisible(sibling)) continue;
    if (HasClass(sibling, WC_TABCONTROLW)) continue;
    if (HasClass(sibling, WC_BUTTON) && (GetWindowLongPtrW(sibling, GWL_STYLE) & BS_TYPEMASK) == BS_GROUPBOX) continue;

    RECT rect;
    GetWindowRect(sibling, &rect);
    MapWindowPoints(nullptr, window, (POINT*)&rect, 2);
    ExcludeClipRect(dc, rect.left, rect.top, rect.right, rect.bottom);
  }
}

void ClipChildren(HWND parent, HDC dc) {
  for (HWND child = GetWindow(parent, GW_CHILD); child; child = GetWindow(child, GW_HWNDNEXT)) {
    if (!IsWindowVisible(child)) continue;
    if (HasClass(child, WC_STATIC)) continue;
    if (HasClass(child, WC_BUTTON) && (GetWindowLongPtrW(child, GWL_STYLE) & BS_TYPEMASK) == BS_GROUPBOX) continue;

    RECT rect;
    GetWindowRect(child, &rect);
    MapWindowPoints(nullptr, parent, (POINT*)&rect, 2);
    ExcludeClipRect(dc, rect.left, rect.top, rect.right, rect.bottom);
  }
}

bool IsHot(HWND w, const RECT& r) {
  POINT pt; GetCursorPos(&pt); ScreenToClient(w, &pt);
  return PtInRect(&r, pt) != FALSE;
}

void DrawArrow(HDC dc, RECT r, bool up, COLORREF color) {
  const int x = (r.left + r.right) / 2, y = (r.top + r.bottom) / 2, s = 3;
  const int d = up ? -1 : 1;
  const POINT pts[] = {{x-s, y-d*s/2},{x+s, y-d*s/2},{x, y+d*s}};
  HBRUSH br = CreateSolidBrush(color);
  HGDIOBJ ob = SelectObject(dc, br), op = SelectObject(dc, GetStockObject(NULL_PEN));
  Polygon(dc, pts, 3);
  SelectObject(dc, op); SelectObject(dc, ob);
  DeleteObject(br);
}

void DrawLabel(HWND w, HDC dc, RECT r, UINT flags) {
  WCHAR text[4096]; GetWindowTextW(w, text, 4096);
  if (SendMessageW(w, WM_QUERYUISTATE, 0, 0) & UISF_HIDEACCEL) flags |= DT_HIDEPREFIX;
  DrawTextW(dc, text, -1, &r, flags);
}

void DrawControlFrame(HWND w, HDC dc, const RECT& r, const WCHAR* cls, int part, int status, HBRUSH bg, RECT* content = nullptr) {
  HTHEME ht = OpenThemeData(w, cls);
  HRGN rgn = nullptr;
  if (ht && SUCCEEDED(GetThemeBackgroundRegion(ht, dc, part, status, &r, &rgn))) {
    FillRgn(dc, rgn, bg);
    FrameRgn(dc, rgn, GetState().brEdge, 1, 1);
    DeleteObject(rgn);
  } else {
    FillRect(dc, &r, bg);
    FrameRect(dc, &r, GetState().brEdge);
  }
  if (content) {
    *content = r;
    if (ht) GetThemeBackgroundContentRect(ht, dc, part, status, &r, content);
  }
  if (ht) CloseThemeData(ht);
}

// ---------------------------------------------------------------------------
// Per-control Draw routines
// ---------------------------------------------------------------------------
void DrawButton(HWND w, HDC dc, RECT r) {
  auto& st = GetState();
  const DWORD style = (DWORD)GetWindowLongPtrW(w, GWL_STYLE);
  const DWORD type  = style & BS_TYPEMASK;
  const bool enabled = IsWindowEnabled(w) != FALSE;
  const LRESULT bstate = SendMessageW(w, BM_GETSTATE, 0, 0);
  const bool pressed = (bstate & BST_PUSHED) != 0;
  const bool hot = enabled && IsHot(w, r);
  if (type == BS_GROUPBOX) {
    TEXTMETRICW tm; GetTextMetricsW(dc, &tm);
    RECT frame = r; frame.top += tm.tmHeight / 2;
    FrameRect(dc, &frame, st.brEdge);
    WCHAR text[4096]; const int len = GetWindowTextW(w, text, 4096);
    SIZE sz; GetTextExtentPoint32W(dc, text, len, &sz);
    RECT label = {8, 0, 12 + sz.cx, tm.tmHeight};
    FillRect(dc, &label, GetSurfaceBrush(w));
    DrawLabel(w, dc, label, DT_LEFT | DT_SINGLELINE);
    return;
  }

  FillRect(dc, &r, GetSurfaceBrush(w));

  const bool radio = type == BS_RADIOBUTTON || type == BS_AUTORADIOBUTTON;
  const bool check = radio || type == BS_CHECKBOX || type == BS_AUTOCHECKBOX || type == BS_3STATE || type == BS_AUTO3STATE;
  if (check) {
    const int sz = MulDiv(13, (int)GetDpiForWindow(w), 96);
    RECT glyph = {r.left, (r.bottom - sz) / 2, r.left + sz, (r.bottom + sz) / 2};
    if (style & BS_LEFTTEXT) OffsetRect(&glyph, r.right - sz, 0);
    const int checked = (int)SendMessageW(w, BM_GETCHECK, 0, 0);
    HTHEME ht = OpenThemeData(w, L"DarkMode_Explorer::Button");
    if (!ht) ht = OpenThemeData(w, L"Button");
    if (ht) {
      const int part   = radio ? BP_RADIOBUTTON : BP_CHECKBOX;
      const int status = 1 + checked * 4 + (!enabled ? 3 : pressed ? 2 : hot ? 1 : 0);
      DrawThemeBackground(ht, dc, part, status, &glyph, nullptr);
      CloseThemeData(ht);
    }
    if (style & BS_LEFTTEXT) r.right -= sz + 5; else r.left += sz + 5;
    DrawLabel(w, dc, r, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
  } else {
    const int status = !enabled ? PBS_DISABLED : pressed ? PBS_PRESSED : hot ? PBS_HOT :
        (type == BS_DEFPUSHBUTTON || GetFocus() == w) ? PBS_DEFAULTED : PBS_NORMAL;
    RECT content;
    DrawControlFrame(w, dc, r, L"Button", BP_PUSHBUTTON, status,
        pressed || hot ? st.brHotBg : st.brControlBg, &content);
    if (pressed) OffsetRect(&content, 1, 1);
    DrawLabel(w, dc, content, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  }
}

void DrawTabs(HWND w, HDC dc, RECT r) {
  auto& st = GetState();
  FillRect(dc, &r, GetSurfaceBrush(w));

  RECT page = r;
  TabCtrl_AdjustRect(w, FALSE, &page);
  const int pad = 2;
  InflateRect(&page, pad, pad);
  const int count = TabCtrl_GetItemCount(w);
  RECT first;
  if (count && TabCtrl_GetItemRect(w, 0, &first)) {
    page.left = first.left - pad;
    page.top  = first.bottom - 1;
  }
  FillRect(dc, &page, st.brPageBackground);
  FrameRect(dc, &page, st.brEdge);
  const int sel = TabCtrl_GetCurSel(w);

  for (int pos = 0; pos <= count; ++pos) {
    const int i = pos == count ? sel : pos;
    if (i < 0 || i >= count || (pos != count && i == sel)) continue;
    RECT item;
    if (!TabCtrl_GetItemRect(w, i, &item)) continue;
    const RECT label = item;
    const bool active = (i == sel);
    if (active) { item.left -= pad; item.right += pad; item.top -= pad; }
    item.bottom = page.top + 1;
    FillRect(dc, &item, active ? st.brPageBackground : IsHot(w, item) ? st.brHotBg : st.brBackground);
    FrameRect(dc, &item, st.brEdge);
    if (active) {
      RECT bot = {item.left + 1, page.top, item.right - 1, page.top + 1};
      FillRect(dc, &bot, st.brPageBackground);
    }
    WCHAR text[512]; TCITEMW entry{}; entry.mask = TCIF_TEXT; entry.pszText = text; entry.cchTextMax = 512;
    RECT tr = label;
    if (TabCtrl_GetItem(w, i, &entry)) DrawTextW(dc, text, -1, &tr, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  }
}

void DrawHeader(HWND w, HDC dc, RECT r) {
  auto& st = GetState();
  FillRect(dc, &r, st.brBackground);
  const bool clickable = (GetWindowLongPtrW(w, GWL_STYLE) & HDS_BUTTONS) != 0;
  const int n = Header_GetItemCount(w);
  for (int i = 0; i < n; ++i) {
    RECT item; if (!Header_GetItemRect(w, i, &item)) continue;
    FillRect(dc, &item, clickable && IsWindowEnabled(w) && IsHot(w, item) ? st.brHotBg : st.brBackground);
    WCHAR text[512]; HDITEMW entry{}; entry.mask = HDI_TEXT | HDI_FORMAT; entry.pszText = text; entry.cchTextMax = 512;
    if (!Header_GetItem(w, i, &entry)) continue;
    const UINT fmt = entry.fmt & HDF_CENTER ? DT_CENTER : entry.fmt & HDF_RIGHT ? DT_RIGHT : DT_LEFT;
    RECT content = item;
    HTHEME ht = GetWindowTheme(w);
    if (ht) GetThemeBackgroundContentRect(ht, dc, HP_HEADERITEM, HIS_NORMAL, &item, &content);
    const int margin = (int)SendMessageW(w, HDM_GETBITMAPMARGIN, 0, 0);
    InflateRect(&content, -margin, 0); item = content;
    if (entry.fmt & (HDF_SORTUP | HDF_SORTDOWN)) {
      RECT arrow = item; arrow.left = arrow.right - 16;
      DrawArrow(dc, arrow, (entry.fmt & HDF_SORTUP) != 0, g_Text);
      item.right = arrow.left;
    }
    DrawTextW(dc, text, -1, &item, fmt | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
  }
  for (int i = 0; i < n; ++i) {
    RECT item; if (!Header_GetItemRect(w, i, &item)) continue;
    RECT div = {item.right - 1, item.top, item.right, item.bottom};
    HBRUSH sep = CreateSolidBrush(RGB(48, 48, 48));
    FillRect(dc, &div, sep); DeleteObject(sep);
  }
}

constexpr wchar_t kComboAnchorSelProp[] = L"MonkeySounds.ComboAnchorSel";

void DrawComboBox(HWND w, HDC dc, RECT r) {
  auto& st = GetState();
  FillRect(dc, &r, GetSurfaceBrush(w));
  COMBOBOXINFO info = {sizeof(COMBOBOXINFO)};
  if (!GetComboBoxInfo(w, &info)) return;
  const DWORD style = (DWORD)GetWindowLongPtrW(w, GWL_STYLE);
  const bool list    = (style & CBS_DROPDOWNLIST) == CBS_DROPDOWNLIST;
  const bool enabled = IsWindowEnabled(w) != FALSE;
  const bool hot     = enabled && IsHot(w, info.rcButton);
  const int status   = !enabled ? CBRO_DISABLED : hot ? CBRO_HOT : CBRO_NORMAL;
  DrawControlFrame(w, dc, r, L"Combobox", CP_READONLY, status,
      enabled ? st.brControlBg : st.brBackground);
  if (list) {
    RECT label = info.rcItem;
    const bool dropped = SendMessageW(w, CB_GETDROPPEDSTATE, 0, 0) != 0;
    HANDLE anchor = dropped ? GetPropW(w, kComboAnchorSelProp) : nullptr;
    if (anchor) {
      const int sel = static_cast<int>(reinterpret_cast<INT_PTR>(anchor)) - 1;
      WCHAR text[4096]{};
      if (sel >= 0 &&
          SendMessageW(w, CB_GETLBTEXT, sel, reinterpret_cast<LPARAM>(text)) != CB_ERR) {
        DrawTextW(dc, text, -1, &label,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
      } else {
        DrawLabel(w, dc, label,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
      }
    } else {
      DrawLabel(w, dc, label,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
    }
    if (GetFocus() == w && !(SendMessageW(w, WM_QUERYUISTATE, 0, 0) & UISF_HIDEFOCUS))
      DrawFocusRect(dc, &info.rcItem);
  }
  if (hot) { RECT btn = info.rcButton; InflateRect(&btn, -1, -1); FillRect(dc, &btn, st.brHotBg); }
  DrawArrow(dc, info.rcButton, false, enabled ? g_Text : g_DisabledText);
}

void DrawListBox(HWND w, HDC dc, RECT r) {
  FillRect(dc, &r, GetState().brBackground);
  const int count = (int)SendMessageW(w, LB_GETCOUNT, 0, 0);
  const int sel   = (int)SendMessageW(w, LB_GETCURSEL, 0, 0);
  const int top   = (int)SendMessageW(w, LB_GETTOPINDEX, 0, 0);
  for (int i = std::max(0, top); i < count; ++i) {
    RECT item; if (SendMessageW(w, LB_GETITEMRECT, i, (LPARAM)&item) == LB_ERR) continue;
    if (item.top >= r.bottom) break;
    const bool hi = (i == sel);
    if (hi) FillRect(dc, &item, GetSysColorBrush(COLOR_HIGHLIGHT));
    SetTextColor(dc, !IsWindowEnabled(w) ? g_DisabledText : hi ? GetSysColor(COLOR_HIGHLIGHTTEXT) : g_Text);
    const int len = (int)SendMessageW(w, LB_GETTEXTLEN, i, 0);
    if (len == LB_ERR || len >= 8192) continue;
    WCHAR text[8192]; SendMessageW(w, LB_GETTEXT, i, (LPARAM)text);
    item.left += 2;
    DrawTextW(dc, text, -1, &item, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
  }
}

void DrawSpinner(HWND w, HDC dc, RECT r) {
  auto& st = GetState();
  const bool horiz   = (GetWindowLongPtrW(w, GWL_STYLE) & UDS_HORZ) != 0;
  const bool enabled = IsWindowEnabled(w) != FALSE;
  for (int i = 0; i < 2; ++i) {
    RECT item = r;
    if (horiz) { if (i == 0) item.right = (r.left+r.right)/2; else item.left = (r.left+r.right)/2; }
    else        { if (i == 0) item.bottom= (r.top+r.bottom)/2; else item.top  = (r.top+r.bottom)/2; }
    FillRect(dc, &item, enabled && IsHot(w, item) ? st.brHotBg : st.brControlBg);
    FrameRect(dc, &item, st.brEdge);
    DrawArrow(dc, item, i == 0, enabled ? g_Text : g_DisabledText);
  }
}

void DrawSeparator(HWND w, HDC dc, RECT r) {
  FillRect(dc, &r, GetSurfaceBrush(w));
  if ((GetWindowLongPtrW(w, GWL_STYLE) & SS_TYPEMASK) == SS_ETCHEDHORZ) r.bottom = r.top + 1;
  else r.right = r.left + 1;
  FillRect(dc, &r, GetState().brEdge);
}

void DrawStatusBar(HWND w, HDC dc, RECT r) {
  auto& st = GetState();
  FillRect(dc, &r, st.brBackground);

  // Top border line
  RECT topEdge = { r.left, r.top, r.right, r.top + 1 };
  FillRect(dc, &topEdge, st.brEdge);

  const int partsCount = (int)SendMessageW(w, SB_GETPARTS, 0, 0);
  if (partsCount <= 0) return;

  for (int i = 0; i < partsCount; ++i) {
    RECT partRc{};
    if (!SendMessageW(w, SB_GETRECT, i, (LPARAM)&partRc)) continue;

    if (i > 0) {
      RECT div = { partRc.left, r.top + 2, partRc.left + 1, r.bottom - 2 };
      FillRect(dc, &div, st.brEdge);
    }

    int len = LOWORD((DWORD)SendMessageW(w, SB_GETTEXTLENGTHW, i, 0));
    if (len > 0) {
      std::vector<WCHAR> buf((size_t)len + 1, L'\0');
      SendMessageW(w, SB_GETTEXTW, i, (LPARAM)buf.data());
      RECT textRc = partRc;
      textRc.left += 4;
      textRc.right -= 4;
      DrawTextW(dc, buf.data(), -1, &textRc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
    }
  }
}

// ---------------------------------------------------------------------------
// Buffered paint dispatcher
// ---------------------------------------------------------------------------
void PaintControl(HWND w, HDC dc, ControlType type) {
  RECT r; GetClientRect(w, &r);
  if (r.right <= 0 || r.bottom <= 0) return;
  const int saved = SaveDC(dc);
  auto font = (HFONT)SendMessageW(w, WM_GETFONT, 0, 0);
  if (font) SelectObject(dc, font);
  SetTextColor(dc, IsWindowEnabled(w) ? g_Text : g_DisabledText);
  SetBkMode(dc, TRANSPARENT);
  switch (type) {
  case ControlType::Button:    DrawButton(w, dc, r);    break;
  case ControlType::Tab:       DrawTabs(w, dc, r);      break;
  case ControlType::ComboBox:  DrawComboBox(w, dc, r);  break;
  case ControlType::Header:    DrawHeader(w, dc, r);    break;
  case ControlType::Spinner:   DrawSpinner(w, dc, r);   break;
  case ControlType::ListBox:   DrawListBox(w, dc, r);   break;
  case ControlType::Separator: DrawSeparator(w, dc, r); break;
  case ControlType::StatusBar: DrawStatusBar(w, dc, r); break;
  default: break;
  }
  RestoreDC(dc, saved);
}

void PaintControlMessage(HWND w, UINT msg, WPARAM wParam, ControlType type) {
  PAINTSTRUCT ps{};
  HDC dc = (msg == WM_PAINT) ? BeginPaint(w, &ps) : (HDC)wParam;
  const int saved = SaveDC(dc);
  if (type == ControlType::Button && (GetWindowLongPtrW(w, GWL_STYLE) & BS_TYPEMASK) == BS_GROUPBOX) {
    ClipSiblings(w, dc);
    PaintControl(w, dc, type);
    RestoreDC(dc, saved);
    if (msg == WM_PAINT) EndPaint(w, &ps);
    return;
  }
  if (type == ControlType::Tab) {
    ClipSiblings(w, dc);
  }
  RECT r; GetClientRect(w, &r);
  HDC buf = CreateCompatibleDC(dc);
  HBITMAP bmp = CreateCompatibleBitmap(dc, std::max(1L, (LONG)r.right), std::max(1L, (LONG)r.bottom));
  if (buf && bmp) {
    HGDIOBJ ob = SelectObject(buf, bmp);
    PaintControl(w, buf, type);
    BitBlt(dc, 0, 0, r.right, r.bottom, buf, 0, 0, SRCCOPY);
    SelectObject(buf, ob);
  } else {
    PaintControl(w, dc, type);
  }
  if (bmp) DeleteObject(bmp);
  if (buf) DeleteDC(buf);
  RestoreDC(dc, saved);
  if (msg == WM_PAINT) EndPaint(w, &ps);
}

// ---------------------------------------------------------------------------
// Control WM_PAINT border repair for Edit boxes
// ---------------------------------------------------------------------------
void PaintEditBorder(HWND w) {
  RECT r; GetWindowRect(w, &r); OffsetRect(&r, -r.left, -r.top);
  HDC dc = GetWindowDC(w); if (!dc) return;
  FrameRect(dc, &r, GetState().brEdge);
  if (GetWindowLongPtrW(w, GWL_EXSTYLE) & WS_EX_CLIENTEDGE) {
    InflateRect(&r, -1, -1);
    FrameRect(dc, &r, GetFocus() == w ? GetState().brEdge : GetState().brBackground);
  }
  ReleaseDC(w, dc);
}

// ---------------------------------------------------------------------------
// Control subclass procedure
// ---------------------------------------------------------------------------
static bool ShouldDeferPaint(UINT msg, ControlType type) {
  if (type == ControlType::Edit) return false;
  switch (msg) {
  case WM_ENABLE:
  case WM_SETTEXT:
  case WM_SETFONT:
  case BM_SETCHECK:
  case BM_SETSTATE:
  case CB_SETCURSEL:
    return true;
  default:
    return false;
  }
}

void SetControlSubclass(HWND w, ControlType type);

static void ThemeComboDropList(HWND combo) {
  if (!combo || !IsWindow(combo)) return;
  auto &state = GetState();
  const bool dark = state.isDarkMode.load();
  COMBOBOXINFO info = {sizeof(COMBOBOXINFO)};
  if (!GetComboBoxInfo(combo, &info) || !info.hwndList) return;
  if (state.pfnAllowDarkModeForWindow)
    state.pfnAllowDarkModeForWindow(info.hwndList, dark);
  SetWindowTheme(info.hwndList, dark ? L"DarkMode_Explorer" : L"Explorer", nullptr);
  if (dark) SetControlSubclass(info.hwndList, ControlType::ListBox);
}

LRESULT CALLBACK ControlProc(HWND w, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR id, DWORD_PTR data) {
  const auto type = (ControlType)data;
  if (msg == WM_NCDESTROY) { RemoveWindowSubclass(w, ControlProc, id); return DefSubclassProc(w, msg, wp, lp); }

  if (!WindowsTheme::IsUsingDarkMode()) return DefSubclassProc(w, msg, wp, lp);

  if (type == ControlType::ComboBox &&
      ((msg == CB_SHOWDROPDOWN && wp) || msg == WM_LBUTTONDOWN)) {
    ThemeComboDropList(w);
  }
  if (type == ControlType::ComboBox && msg == CB_SHOWDROPDOWN) {
    if (wp) {
      const int sel = static_cast<int>(SendMessageW(w, CB_GETCURSEL, 0, 0));
      SetPropW(w, kComboAnchorSelProp,
               reinterpret_cast<HANDLE>(static_cast<INT_PTR>(sel + 1)));
    } else {
      RemovePropW(w, kComboAnchorSelProp);
      InvalidateRect(w, nullptr, FALSE);
    }
  }

  if (type == ControlType::Edit) {
    const LRESULT r = DefSubclassProc(w, msg, wp, lp);
    if (msg == WM_NCPAINT || msg == WM_SETFOCUS || msg == WM_KILLFOCUS) PaintEditBorder(w);
    return r;
  }

  if (msg == WM_ERASEBKGND) return TRUE;

  if (msg == WM_PAINT || msg == WM_PRINTCLIENT) {
    PaintControlMessage(w, msg, wp, type);
    return 0;
  }

  if (msg == WM_MOUSEMOVE) {
    TRACKMOUSEEVENT tme = {sizeof(tme), TME_LEAVE, w, 0};
    TrackMouseEvent(&tme);
  }

  LRESULT res;
  if (ShouldDeferPaint(msg, type)) {
    DefSubclassProc(w, WM_SETREDRAW, FALSE, 0);
    res = DefSubclassProc(w, msg, wp, lp);
    DefSubclassProc(w, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(w, nullptr, FALSE);
    return res;
  }

  res = DefSubclassProc(w, msg, wp, lp);

  switch (msg) {
  case WM_MOUSEMOVE:
  case WM_MOUSELEAVE:
    if (type == ControlType::Tab) {
      RECT tabStrip;
      GetClientRect(w, &tabStrip);
      RECT page = tabStrip;
      TabCtrl_AdjustRect(w, FALSE, &page);
      tabStrip.bottom = page.top;
      InvalidateRect(w, &tabStrip, FALSE);
    } else {
      InvalidateRect(w, nullptr, FALSE);
    }
    break;
  case WM_LBUTTONDOWN: case WM_LBUTTONUP:
  case WM_ENABLE: case WM_SETFOCUS: case WM_KILLFOCUS: case WM_UPDATEUISTATE:
  case WM_SETTEXT: case WM_SETFONT: case BM_SETCHECK: case BM_SETSTATE:
  case TCM_SETCURSEL: case CB_SETCURSEL: case LB_SETCURSEL:
  case LB_SETTOPINDEX: case WM_KEYDOWN: case WM_MOUSEWHEEL: case WM_VSCROLL:
    InvalidateRect(w, nullptr, FALSE);
    break;
  case SB_SETTEXTW:
  case SB_SETTEXTA:
  case SB_SETPARTS:
    InvalidateRect(w, nullptr, FALSE);
    break;
  }
  return res;
}

void SetControlSubclass(HWND w, ControlType type) {
  SetWindowSubclass(w, ControlProc, 0, (DWORD_PTR)type);
}

// ---------------------------------------------------------------------------
// Parent-window subclass (WM_ERASEBKGND, WM_CTLCOLOR*, WM_SETTINGCHANGE)
// ---------------------------------------------------------------------------
LRESULT ControlColor(UINT msg, WPARAM wp, LPARAM lp) {
  HDC dc      = (HDC)wp;
  HWND ctrl   = (HWND)lp;
  auto& st    = GetState();
  SetTextColor(dc, IsWindowEnabled(ctrl) ? g_Text : g_DisabledText);
  const DWORD style = (DWORD)GetWindowLongPtrW(ctrl, GWL_STYLE);
  const bool edit   = HasClass(ctrl, WC_EDIT);
  const bool borderlessRO = edit && (style & ES_READONLY) && !(style & WS_BORDER) && !(GetWindowLongPtrW(ctrl, GWL_EXSTYLE) & WS_EX_CLIENTEDGE);
  const bool input  = !borderlessRO && (msg == WM_CTLCOLOREDIT || msg == WM_CTLCOLORLISTBOX || edit);
  HBRUSH surfaceBr = GetSurfaceBrush(ctrl);
  COLORREF bgCol = (surfaceBr == st.brPageBackground) ? g_PageBackground : g_Background;
  SetBkColor(dc, input ? g_Background : bgCol);
  return (LRESULT)(input ? st.brBackground : surfaceBr);
}

LRESULT CALLBACK ParentWindowProc(HWND w, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR id, DWORD_PTR data) {
  if (msg == WM_NCDESTROY) { RemoveWindowSubclass(w, ParentWindowProc, id); return DefSubclassProc(w, msg, wp, lp); }
  if (msg == WM_SETTINGCHANGE) { WindowsTheme::Refresh(); }
  if (!WindowsTheme::IsUsingDarkMode()) return DefSubclassProc(w, msg, wp, lp);
  if (msg == WM_ERASEBKGND) {
    HDC dc = (HDC)wp;
    RECT r;
    if (!GetClipBox(dc, &r)) GetClientRect(w, &r);
    FillRect(dc, &r, GetSurfaceBrush(w));
    return TRUE;
  }
  if (msg == WM_PAINT) {
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(w, &ps);
    ClipChildren(w, dc);
    FillRect(dc, &ps.rcPaint, GetSurfaceBrush(w));
    EndPaint(w, &ps);
    return 0;
  }
  if (msg == WM_PRINTCLIENT) {
    HDC dc = (HDC)wp;
    RECT r;
    GetClientRect(w, &r);
    ClipChildren(w, dc);
    FillRect(dc, &r, GetSurfaceBrush(w));
    return 0;
  }
  if (msg >= WM_CTLCOLORMSGBOX && msg <= WM_CTLCOLORSTATIC) return ControlColor(msg, wp, lp);
  return DefSubclassProc(w, msg, wp, lp);
}

BOOL CALLBACK ApplyControlEnum(HWND w, LPARAM) {
  WindowsTheme::ApplyToControl(w);
  return TRUE;
}

// ---------------------------------------------------------------------------
// Settings persistence helpers
// ---------------------------------------------------------------------------
std::string ReadThemePreference() {
  auto &st = GetState();
  std::lock_guard<std::mutex> lock(st.handlerMutex);
  if (st.settingsReader) {
    const std::string value = st.settingsReader();
    if (value == "system" || value == "dark" || value == "light")
      return value;
  }
  return "system";
}

void WriteThemePreference(const std::string &value) {
  auto &st = GetState();
  std::lock_guard<std::mutex> lock(st.handlerMutex);
  if (st.settingsWriter) {
    st.settingsWriter(value);
  }
}

Mode ModeFromString(const std::string &value) {
  if (value == "dark") return Mode::Dark;
  if (value == "light") return Mode::Light;
  return Mode::System;
}

std::string ModeToString(Mode mode) {
  if (mode == Mode::Dark) return "dark";
  if (mode == Mode::Light) return "light";
  return "system";
}

constexpr int kPreferredAppModeAllowDark = 1;
constexpr int kPreferredAppModeForceDark = 2;
constexpr int kPreferredAppModeForceLight = 3;

LRESULT CALLBACK CBTDialogHookProc(int code, WPARAM wParam, LPARAM lParam) {
  if (code == HCBT_ACTIVATE || code == HCBT_CREATEWND) {
    HWND w = (HWND)wParam;
    if (w && IsWindow(w)) {
      WCHAR cls[64]{}; GetClassNameW(w, cls, 64);
      if (wcscmp(cls, L"#32770") == 0) {
        WindowsTheme::ApplyToDialog(w);
        EnumChildWindows(w, ApplyControlEnum, 0);
      }
    }
  }
  return CallNextHookEx(nullptr, code, wParam, lParam);
}

} // anonymous namespace

// ===========================================================================
// Public API
// ===========================================================================

bool IsSystemDarkMode() {
  HKEY hKey = nullptr;
  DWORD value = 1, size = sizeof(value), type = 0;
  const WCHAR* key = L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize";
  if (RegOpenKeyExW(HKEY_CURRENT_USER, key, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
    if (RegQueryValueExW(hKey, L"AppsUseLightTheme", nullptr, &type, (LPBYTE)&value, &size) == ERROR_SUCCESS && type == REG_DWORD) { RegCloseKey(hKey); return value == 0; }
    if (RegQueryValueExW(hKey, L"SystemUsesLightTheme", nullptr, &type, (LPBYTE)&value, &size) == ERROR_SUCCESS && type == REG_DWORD) { RegCloseKey(hKey); return value == 0; }
    RegCloseKey(hKey);
  }
  return false;
}

void SetSettingsHandlers(SettingsReader reader, SettingsWriter writer) {
  auto& st = GetState(); std::lock_guard<std::mutex> lock(st.handlerMutex);
  st.settingsReader = reader; st.settingsWriter = writer;
}

void Initialize() {
  auto& state = GetState();
  if (state.initialized.exchange(true)) { Refresh(); return; }

  HMODULE hUx = GetModuleHandleW(L"uxtheme.dll");
  if (!hUx) hUx = LoadLibraryExW(L"uxtheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
  if (hUx) {
    state.pfnSetPreferredAppMode = (fnSetPreferredAppMode)GetProcAddress(hUx, MAKEINTRESOURCEA(135));
    state.pfnAllowDarkModeForWindow = (fnAllowDarkModeForWindow)GetProcAddress(hUx, MAKEINTRESOURCEA(133));
    state.pfnAllowDarkModeForApp = (fnAllowDarkModeForApp)GetProcAddress(hUx, MAKEINTRESOURCEA(132));
    state.pfnRefreshImmersiveColorPolicyState = (fnRefreshImmersiveColorPolicyState)GetProcAddress(hUx, MAKEINTRESOURCEA(104));
    state.pfnFlushMenuThemes = (fnFlushMenuThemes)GetProcAddress(hUx, MAKEINTRESOURCEA(136));
  }
  Refresh();
}

void Refresh() {
  auto& state = GetState();
  state.mode = ModeFromString(ReadThemePreference());
  if (state.mode == Mode::Dark)
    state.isDarkMode = true;
  else if (state.mode == Mode::Light)
    state.isDarkMode = false;
  else
    state.isDarkMode = IsSystemDarkMode();

  int appMode = kPreferredAppModeAllowDark;
  if (state.mode == Mode::Dark)
    appMode = kPreferredAppModeForceDark;
  else if (state.mode == Mode::Light)
    appMode = kPreferredAppModeForceLight;

  if (state.pfnSetPreferredAppMode)
    state.pfnSetPreferredAppMode(appMode);
  else if (state.pfnAllowDarkModeForApp)
    state.pfnAllowDarkModeForApp(state.isDarkMode.load());
  if (state.pfnRefreshImmersiveColorPolicyState)
    state.pfnRefreshImmersiveColorPolicyState();
  if (state.pfnFlushMenuThemes)
    state.pfnFlushMenuThemes();
}

bool IsUsingDarkMode() { return GetState().isDarkMode.load(); }
Mode GetMode()         { return GetState().mode; }

void SetMode(Mode mode) {
  WriteThemePreference(ModeToString(mode));
  Refresh();
}
void SetMode(const std::string& modeStr) {
  if (modeStr != "system" && modeStr != "dark" && modeStr != "light")
    return;
  WriteThemePreference(modeStr);
  Refresh();
}

void ApplyToControl(HWND w) {
  if (!w || !IsWindow(w)) return;
  auto& state = GetState();
  const bool dark = state.isDarkMode.load();
  if (state.pfnAllowDarkModeForWindow) state.pfnAllowDarkModeForWindow(w, dark);

  WCHAR cls[64]{}; GetClassNameW(w, cls, 64);
  const DWORD style = (DWORD)GetWindowLongPtrW(w, GWL_STYLE);
  const bool isGroupBox = (_wcsicmp(cls, L"BUTTON") == 0) &&
                          ((style & BS_TYPEMASK) == BS_GROUPBOX);
  if (dark && !isGroupBox) {
    SetWindowLongPtrW(w, GWL_STYLE, style | WS_CLIPSIBLINGS);
  }

  if (_wcsicmp(cls, WC_LISTVIEWW) == 0) {
    SetWindowTheme(w, dark ? L"DarkMode_Explorer" : L"Explorer", nullptr);
    if (dark) {
      ListView_SetBkColor(w, g_Background);
      ListView_SetTextBkColor(w, g_Background);
      ListView_SetTextColor(w, g_Text);
    } else {
      ListView_SetBkColor(w, GetSysColor(COLOR_WINDOW));
      ListView_SetTextBkColor(w, GetSysColor(COLOR_WINDOW));
      ListView_SetTextColor(w, GetSysColor(COLOR_WINDOWTEXT));
    }
    HWND hdr = ListView_GetHeader(w);
    if (hdr) {
      if (state.pfnAllowDarkModeForWindow) state.pfnAllowDarkModeForWindow(hdr, dark);
      SetWindowTheme(hdr, dark ? L"DarkMode_ItemsView" : L"Explorer", nullptr);
      if (dark) SetControlSubclass(hdr, ControlType::Header);
    }
  } else if (_wcsicmp(cls, WC_TABCONTROLW) == 0) {
    SetWindowTheme(w, dark ? L"DarkMode_Explorer" : L"Explorer", nullptr);
    if (dark) SetControlSubclass(w, ControlType::Tab);
  } else if (_wcsicmp(cls, STATUSCLASSNAMEW) == 0) {
    SetWindowTheme(w, dark ? L"DarkMode_Explorer" : L"Explorer", nullptr);
    if (dark) SetControlSubclass(w, ControlType::StatusBar);
  } else if (_wcsicmp(cls, L"BUTTON") == 0) {
    const DWORD style = (DWORD)GetWindowLongPtrW(w, GWL_STYLE);
    const DWORD type  = style & BS_TYPEMASK;
    SetWindowTheme(w, dark ? L"DarkMode_Explorer" : L"Explorer", nullptr);
    if (dark && (type != BS_OWNERDRAW)) SetControlSubclass(w, ControlType::Button);
  } else if (_wcsicmp(cls, L"COMBOBOX") == 0) {
    SetWindowTheme(w, dark ? L"DarkMode_CFD" : L"Explorer", nullptr);
    const DWORD style = (DWORD)GetWindowLongPtrW(w, GWL_STYLE);
    COMBOBOXINFO info = {sizeof(COMBOBOXINFO)};
    if (GetComboBoxInfo(w, &info) && info.hwndList) {
      if (state.pfnAllowDarkModeForWindow)
        state.pfnAllowDarkModeForWindow(info.hwndList, dark);
      SetWindowTheme(info.hwndList, dark ? L"DarkMode_Explorer" : L"Explorer", nullptr);
    }
    if (dark && !(style & (CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE)) &&
        (style & CBS_DROPDOWNLIST) != CBS_SIMPLE) {
      SetControlSubclass(w, ControlType::ComboBox);
      if (info.hwndList)
        SetControlSubclass(info.hwndList, ControlType::ListBox);
    }
  } else if (_wcsicmp(cls, L"EDIT") == 0) {
    SetWindowTheme(w, dark ? L"DarkMode_Explorer" : L"Explorer", nullptr);
    if (dark) {
      const DWORD style = (DWORD)GetWindowLongPtrW(w, GWL_STYLE);
      if ((style & WS_BORDER) || (GetWindowLongPtrW(w, GWL_EXSTYLE) & WS_EX_CLIENTEDGE))
        SetControlSubclass(w, ControlType::Edit);
    }
  } else if (_wcsicmp(cls, WC_LISTBOX) == 0) {
    SetWindowTheme(w, dark ? L"DarkMode_Explorer" : L"Explorer", nullptr);
    if (dark) SetControlSubclass(w, ControlType::ListBox);
  } else if (_wcsicmp(cls, UPDOWN_CLASS) == 0) {
    if (dark) SetControlSubclass(w, ControlType::Spinner);
  } else if (_wcsicmp(cls, WC_STATIC) == 0) {
    SetWindowTheme(w, dark ? L"DarkMode_Explorer" : L"Explorer", nullptr);
    const DWORD style = (DWORD)GetWindowLongPtrW(w, GWL_STYLE) & SS_TYPEMASK;
    if (dark && (style == SS_ETCHEDHORZ || style == SS_ETCHEDVERT))
      SetControlSubclass(w, ControlType::Separator);
  } else if (_wcsicmp(cls, WC_HEADERW) == 0) {
    SetWindowTheme(w, dark ? L"DarkMode_ItemsView" : L"Explorer", nullptr);
    if (dark) SetControlSubclass(w, ControlType::Header);
  } else {
    SetWindowTheme(w, dark ? L"DarkMode_Explorer" : L"Explorer", nullptr);
  }
}

void ApplyToWindow(HWND window) {
  if (!window || !IsWindow(window)) return;
  auto& state = GetState();
  const BOOL dark = state.isDarkMode.load() ? TRUE : FALSE;

  DwmSetWindowAttribute(window, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));
  DwmSetWindowAttribute(window, DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1, &dark, sizeof(dark));

  if (state.pfnAllowDarkModeForWindow) state.pfnAllowDarkModeForWindow(window, dark != FALSE);
  SetWindowTheme(window, dark ? L"DarkMode_Explorer" : L"Explorer", nullptr);

  SetWindowSubclass(window, ParentWindowProc, 0, 0);
  EnableDialogTexture(window, (GetWindowLongPtrW(window, GWL_STYLE) & WS_CHILD) != 0);
}

void ApplyToDialog(HWND dialog) {
  if (!dialog || !IsWindow(dialog)) return;
  ApplyToWindow(dialog);
  EnumChildWindows(dialog, ApplyControlEnum, 0);
}

void ApplyToControlTree(HWND window, HWND progressBar) {
  if (!window || !IsWindow(window)) return;
  const BOOL dark = IsUsingDarkMode() ? TRUE : FALSE;

  DwmSetWindowAttribute(window, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));
  DwmSetWindowAttribute(window, DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1, &dark, sizeof(dark));

  ApplyToControl(window);
  EnumChildWindows(window, ApplyControlEnum, 0);

  if (progressBar && IsWindow(progressBar)) {
    if (dark) {
      SetWindowTheme(progressBar, L"", L"");
      SendMessageW(progressBar, PBM_SETBARCOLOR, 0, (LPARAM)RGB(0, 120, 215));
      SendMessageW(progressBar, PBM_SETBKCOLOR,  0, (LPARAM)RGB(48, 48, 48));
    } else {
      SetWindowTheme(progressBar, L"Explorer", L"");
      SendMessageW(progressBar, PBM_SETBARCOLOR, 0, (LPARAM)CLR_DEFAULT);
      SendMessageW(progressBar, PBM_SETBKCOLOR,  0, (LPARAM)CLR_DEFAULT);
    }
  }
}

void ApplyToTooltip(HWND w) {
  if (!w || !IsWindow(w)) return;
  auto& state = GetState();
  const bool dark = state.isDarkMode.load();
  if (state.pfnAllowDarkModeForWindow) state.pfnAllowDarkModeForWindow(w, dark);
  if (dark) {
    SetWindowTheme(w, L"DarkMode_Explorer", L"ToolTip");
    SendMessageW(w, TTM_SETTIPBKCOLOR,   (WPARAM)g_Background, 0);
    SendMessageW(w, TTM_SETTIPTEXTCOLOR, (WPARAM)g_Text, 0);
  } else {
    SetWindowTheme(w, L"Explorer", L"ToolTip");
    SendMessageW(w, TTM_SETTIPBKCOLOR,   (WPARAM)RGB(255,255,255), 0);
    SendMessageW(w, TTM_SETTIPTEXTCOLOR, (WPARAM)RGB(0,0,0), 0);
  }
}

void PrepareMenu(HWND owner) {
  Refresh();
  auto& state = GetState();
  if (owner && IsWindow(owner) && state.pfnAllowDarkModeForWindow)
    state.pfnAllowDarkModeForWindow(owner, state.isDarkMode.load());
  if (state.pfnFlushMenuThemes) state.pfnFlushMenuThemes();
}

COLORREF GetTextColor() {
  return IsUsingDarkMode() ? g_Text : GetSysColor(COLOR_WINDOWTEXT);
}

COLORREF GetDisabledTextColor() {
  return IsUsingDarkMode() ? g_DisabledText : GetSysColor(COLOR_GRAYTEXT);
}

COLORREF GetDarkBackgroundColor() {
  return g_Background;
}

COLORREF GetPageBackgroundColor() {
  return g_PageBackground;
}

HBRUSH GetDarkBackgroundBrush() {
  return GetState().brBackground;
}

HBRUSH GetPageBackgroundBrush() {
  return GetState().brPageBackground;
}

void EnableDialogTexture(HWND window, bool enable) {
  EnableThemeDialogTexture(window, enable && !IsUsingDarkMode() ? ETDT_ENABLETAB : ETDT_DISABLE);
}

DialogHookScope::DialogHookScope() {
  WindowsTheme::Initialize();
  m_Hook = SetWindowsHookExW(WH_CBT, CBTDialogHookProc, nullptr, GetCurrentThreadId());
}
DialogHookScope::~DialogHookScope() {
  if (m_Hook) { UnhookWindowsHookEx(m_Hook); m_Hook = nullptr; }
}

} // namespace WindowsTheme
