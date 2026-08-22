#pragma once

#define IDS_APP_TITLE               103
#define IDR_MAINFRAME               128
#define IDI_APP_ICON                107
#define IDI_SMALL                   108
#define IDC_MONKEYSOUNDS            109
#define IDR_TRAY_MENU               110

// Tray menu commands
#define IDM_TRAY_SHOW               201
#define IDM_TRAY_MUTE_KB            202
#define IDM_TRAY_MUTE_MOUSE         203
#define IDM_TRAY_EXIT               204

// Control IDs
#define IDC_TAB_CONTROL             1001
#define IDC_STATUSBAR               1002
#define IDC_VU_METER                1003

// Sounds tab controls
#define IDC_GB_KEYBOARD             1101
#define IDC_CHK_KB_ENABLE           1102
#define IDC_LBL_KB_PRESET           1103
#define IDC_COMBO_KB_PRESET         1104
#define IDC_BTN_KB_CUSTOM           1105
#define IDC_LBL_KB_VOLUME           1106
#define IDC_SLIDER_KB_VOLUME        1107
#define IDC_BTN_KB_TEST             1108
#define IDC_BTN_KB_EXPORT           1109
#define IDC_BTN_KB_FAVORITE         1110
#define IDC_BTN_KB_INFO             1111
#define IDC_BTN_KB_NEW              1112

#define IDC_GB_MOUSE                1201
#define IDC_CHK_MOUSE_ENABLE        1202
#define IDC_LBL_MOUSE_PRESET        1203
#define IDC_COMBO_MOUSE_PRESET      1204
#define IDC_BTN_MOUSE_CUSTOM        1205
#define IDC_LBL_MOUSE_VOLUME        1206
#define IDC_SLIDER_MOUSE_VOLUME     1207
#define IDC_BTN_MOUSE_TEST          1208
#define IDC_BTN_MOUSE_EXPORT        1209
#define IDC_BTN_MOUSE_FAVORITE      1210
#define IDC_BTN_MOUSE_INFO          1211
#define IDC_BTN_MOUSE_NEW           1212

// Settings tab controls
#define IDC_GB_APPLICATION          1301
#define IDC_LBL_VERSION             1302
#define IDC_BTN_CHECK_UPDATES       1303
#define IDC_CHK_AUTOSTART           1304
#define IDC_CHK_STARTUP_NOTIF       1305

// About tab controls
#define IDC_STATIC_ABOUT_LOGO       1310
#define IDC_STATIC_ABOUT_TITLE      1311
#define IDC_STATIC_ABOUT_DESC1      1312
#define IDC_STATIC_ABOUT_DESC2      1313
#define IDC_STATIC_ABOUT_COPY       1314
#define IDC_BTN_GITHUB              1315
#define IDC_BTN_DISCORD             1316
#define IDC_BTN_PATREON             1317

// Custom window messages
#define WM_TRAYICON                 (WM_USER + 1)
#define WM_VU_PULSE                 (WM_USER + 2)
