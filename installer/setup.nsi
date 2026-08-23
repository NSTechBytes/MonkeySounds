;--------------------------------
; MonkeySounds Installer Script
;--------------------------------

!define APP_NAME       "MonkeySounds"
!define APP_EXE        "MonkeySounds.exe"
!define PUBLISHER      "NSTechBytes"
!define VERSION        "1.0.0.0"
!define REGKEY         "Software\MonkeySounds"
!define UNINSTALL_KEY  "Software\Microsoft\Windows\CurrentVersion\Uninstall\MonkeySounds"
!define WEBSITE        "https://github.com/NSTechBytes/MonkeySounds"

; Output
Name "${APP_NAME}"
OutFile "dist_output\MonkeySounds_Setup_v${VERSION}.exe"
SetCompressor /SOLID lzma

; Default install dir
InstallDir "$PROGRAMFILES64\${APP_NAME}"
InstallDirRegKey HKLM "${REGKEY}" "Install_Dir"

; Require admin
RequestExecutionLevel admin

;--------------------------------
; Includes
;--------------------------------
!include "MUI2.nsh"
!include "x64.nsh"
!include "LogicLib.nsh"

;--------------------------------
; Interface
;--------------------------------
!define MUI_ICON                        "assets\installer.ico"
!define MUI_UNICON                      "assets\uninstaller.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP          "assets\header.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP    "assets\banner.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP  "assets\banner.bmp"

!define MUI_WELCOMEPAGE_TITLE   "Welcome to ${APP_NAME} ${VERSION} Setup"
!define MUI_WELCOMEPAGE_TEXT    "This will install ${APP_NAME} on your computer.$\r$\n$\r$\nMonkeySounds adds satisfying keyboard and mouse sounds to every click and keystroke.$\r$\n$\r$\nClick Next to continue."

!define MUI_FINISHPAGE_RUN              "$INSTDIR\${APP_EXE}"
!define MUI_FINISHPAGE_RUN_TEXT         "Launch ${APP_NAME}"
!define MUI_FINISHPAGE_LINK             "Visit GitHub"
!define MUI_FINISHPAGE_LINK_LOCATION    "${WEBSITE}"

;--------------------------------
; Pages
;--------------------------------
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\LICENSE.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

;--------------------------------
; Install Section
;--------------------------------
Section "MonkeySounds" SecMain
  SectionIn RO

  SetRegView 64
  ${If} ${RunningX64}
    ${DisableX64FSRedirection}
  ${EndIf}

  ; Kill any running instance before overwriting files
  nsExec::ExecToStack 'taskkill /F /IM "${APP_EXE}"'
  Sleep 500

  SetOutPath "$INSTDIR"

  ; Main executable (self-contained — no external assets needed)
  File "..\dist\${APP_EXE}"

  ; Sound profiles
  File /r "..\dist\Sounds"

  ; Store install dir and write uninstaller
  WriteRegStr  HKLM "${REGKEY}" "Install_Dir" "$INSTDIR"
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ; Add/Remove Programs entry
  WriteRegStr   HKLM "${UNINSTALL_KEY}" "DisplayName"     "${APP_NAME} ${VERSION}"
  WriteRegStr   HKLM "${UNINSTALL_KEY}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr   HKLM "${UNINSTALL_KEY}" "DisplayIcon"     "$INSTDIR\${APP_EXE}"
  WriteRegStr   HKLM "${UNINSTALL_KEY}" "Publisher"       "${PUBLISHER}"
  WriteRegStr   HKLM "${UNINSTALL_KEY}" "DisplayVersion"  "${VERSION}"
  WriteRegStr   HKLM "${UNINSTALL_KEY}" "URLInfoAbout"    "${WEBSITE}"
  WriteRegStr   HKLM "${UNINSTALL_KEY}" "HelpLink"        "${WEBSITE}"
  WriteRegDWORD HKLM "${UNINSTALL_KEY}" "NoModify"        1
  WriteRegDWORD HKLM "${UNINSTALL_KEY}" "NoRepair"        1

  ; Start Menu shortcut
  CreateDirectory "$SMPROGRAMS\${APP_NAME}"
  CreateShortCut  "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" \
                  "$INSTDIR\${APP_EXE}" "" "$INSTDIR\${APP_EXE}" 0
  CreateShortCut  "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk" \
                  "$INSTDIR\Uninstall.exe" "" "$INSTDIR\Uninstall.exe" 0

  ; Desktop shortcut
  CreateShortCut "$DESKTOP\${APP_NAME}.lnk" \
                 "$INSTDIR\${APP_EXE}" "" "$INSTDIR\${APP_EXE}" 0

SectionEnd

;--------------------------------
; Auto-start registry entry
; (MonkeySounds writes this itself on first run when the user enables it;
;  the installer does not force auto-start)
;--------------------------------

;--------------------------------
; Uninstall Section
;--------------------------------
Section "Uninstall"

  SetRegView 64
  ReadRegStr $INSTDIR HKLM "${REGKEY}" "Install_Dir"

  ; Kill running instance
  nsExec::ExecToStack 'taskkill /F /IM "${APP_EXE}"'
  Sleep 500

  ; Remove files and Sounds folder
  Delete "$INSTDIR\${APP_EXE}"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir /r "$INSTDIR\Sounds"

  ; Remove install dir if empty
  RMDir "$INSTDIR"

  ; Remove shortcuts
  Delete "$DESKTOP\${APP_NAME}.lnk"
  Delete "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk"
  Delete "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk"
  RMDir  "$SMPROGRAMS\${APP_NAME}"

  ; Remove registry keys
  DeleteRegKey HKLM "${UNINSTALL_KEY}"
  DeleteRegKey HKLM "${REGKEY}"

  ; Remove auto-start entry if it was set
  DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "${APP_NAME}"

  ; Optionally leave %APPDATA%\MonkeySounds so the user keeps their settings
  ; and custom sound profiles. Nothing is force-deleted here.
  DetailPrint "User settings in %APPDATA%\MonkeySounds were preserved."

SectionEnd
