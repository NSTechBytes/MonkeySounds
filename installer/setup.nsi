;---------------------------------------------------------
; MonkeySounds Installer Script
; Supports Standard (recommended) and Portable modes
;---------------------------------------------------------

!define APP_NAME       "MonkeySounds"
!define APP_EXE        "MonkeySounds.exe"
!define PUBLISHER      "NSTechBytes"
!define VERSION        "1.0.0.0"
!define REGKEY         "Software\MonkeySounds"
!define UNINSTALL_KEY  "Software\Microsoft\Windows\CurrentVersion\Uninstall\MonkeySounds"
!define WEBSITE        "https://github.com/NSTechBytes/MonkeySounds"

Name "${APP_NAME}"
OutFile "dist_output\MonkeySounds_Setup_v${VERSION}.exe"
SetCompressor /SOLID lzma

; Default install dir (overridden by portable mode at runtime)
InstallDir "$PROGRAMFILES64\${APP_NAME}"
InstallDirRegKey HKLM "${REGKEY}" "Install_Dir"

RequestExecutionLevel admin

;---------------------------------------------------------
; Includes  — StrFunc MUST be included and initialized
;             before any function that uses ${StrStr}
;---------------------------------------------------------
!include "StrFunc.nsh"
${StrStr}   ; initialize the StrStr function

!include "MUI2.nsh"
!include "x64.nsh"
!include "LogicLib.nsh"
!include "nsDialogs.nsh"
!include "Sections.nsh"

;---------------------------------------------------------
; Interface
;---------------------------------------------------------
!define MUI_ICON                       "assets\installer.ico"
!define MUI_UNICON                     "assets\uninstaller.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP         "assets\header.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP   "assets\banner.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "assets\banner.bmp"

!define MUI_WELCOMEPAGE_TITLE "Welcome to ${APP_NAME} ${VERSION} Setup"
!define MUI_WELCOMEPAGE_TEXT  "This will install ${APP_NAME} on your computer.$\r$\n$\r$\nMonkeySounds adds satisfying keyboard and mouse sounds to every click and keystroke.$\r$\n$\r$\nClick Next to continue."

!define MUI_FINISHPAGE_RUN          "$INSTDIR\${APP_EXE}"
!define MUI_FINISHPAGE_RUN_TEXT     "Launch ${APP_NAME}"
!define MUI_FINISHPAGE_LINK         "Visit GitHub"
!define MUI_FINISHPAGE_LINK_LOCATION "${WEBSITE}"

;---------------------------------------------------------
; Pages
;---------------------------------------------------------
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\LICENSE.txt"

; Custom page: choose Standard or Portable
Page custom InstallModePageCreate InstallModePageLeave

; Directory page — hidden / overridden in portable mode
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE DirectoryPageLeave
!insertmacro MUI_PAGE_DIRECTORY

!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_CONFIRM
UninstPage custom un.RemoveDataPageCreate un.RemoveDataPageLeave
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

;---------------------------------------------------------
; Variables
;---------------------------------------------------------
Var InstallMode      ; "standard" or "portable"
Var RadioStandard
Var RadioPortable
Var RemoveUserData       ; BST_CHECKED or BST_UNCHECKED
Var UnRemoveCheckbox

;---------------------------------------------------------
; .onInit  —  default to standard mode
;---------------------------------------------------------
Function .onInit
  StrCpy $InstallMode "standard"
FunctionEnd

;---------------------------------------------------------
; Custom page: Install Mode
;---------------------------------------------------------
Function InstallModePageCreate
  nsDialogs::Create 1018
  Pop $0
  ${If} $0 == error
    Abort
  ${EndIf}

  ${NSD_CreateLabel} 0 0 100% 20u "Choose how to install ${APP_NAME}:"
  Pop $0

  ${NSD_CreateRadioButton} 0 26u 100% 14u \
    "Standard (recommended) — install with uninstaller, registry entries and shortcuts"
  Pop $RadioStandard

  ${NSD_CreateLabel} 12u 42u 100% 20u \
    "Installs to Program Files. Sounds saved in %APPDATA%\MonkeySounds\Sounds."
  Pop $0

  ${NSD_CreateRadioButton} 0 66u 100% 14u \
    "Portable — no registry entries, no uninstaller, runs from any folder"
  Pop $RadioPortable

  ${NSD_CreateLabel} 12u 82u 100% 20u \
    "All files (EXE + Sounds) placed together. Move the folder freely."
  Pop $0

  ; Check the correct radio based on current mode
  ${If} $InstallMode == "portable"
    ${NSD_Check} $RadioPortable
  ${Else}
    ${NSD_Check} $RadioStandard
  ${EndIf}

  nsDialogs::Show
FunctionEnd

Function InstallModePageLeave
  ${NSD_GetState} $RadioPortable $0
  ${If} $0 == ${BST_CHECKED}
    StrCpy $InstallMode "portable"
    ; Default portable dir to the folder the setup EXE is running from
    StrCpy $INSTDIR "$EXEDIR\${APP_NAME}"
  ${Else}
    StrCpy $InstallMode "standard"
    StrCpy $INSTDIR "$PROGRAMFILES64\${APP_NAME}"
  ${EndIf}
FunctionEnd

;---------------------------------------------------------
; Directory page leave — block portable installs inside
; system-protected folders (Program Files / Windows)
;---------------------------------------------------------
Function DirectoryPageLeave
  ${If} $InstallMode == "portable"
    StrCpy $0 "0"

    ${StrStr} $1 "$INSTDIR" "$PROGRAMFILES64"
    ${If} $1 != ""
      StrCpy $0 "1"
    ${EndIf}

    ${StrStr} $1 "$INSTDIR" "$PROGRAMFILES"
    ${If} $1 != ""
      StrCpy $0 "1"
    ${EndIf}

    ${StrStr} $1 "$INSTDIR" "$WINDIR"
    ${If} $1 != ""
      StrCpy $0 "1"
    ${EndIf}

    ${If} $0 == "1"
      MessageBox MB_ICONEXCLAMATION|MB_OK \
        "Portable mode cannot install into system-protected folders \
(Program Files / Windows).$\r$\nGo back and choose Standard mode, \
or pick a different folder."
      Abort
    ${EndIf}
  ${EndIf}
FunctionEnd

;---------------------------------------------------------
; Install Section
;---------------------------------------------------------
Section "MonkeySounds" SecMain
  SectionIn RO

  SetRegView 64
  ${If} ${RunningX64}
    ${DisableX64FSRedirection}
  ${EndIf}

  ; Kill any running instance before overwriting files
  nsExec::ExecToStack 'taskkill /F /IM "${APP_EXE}"'
  Sleep 500

  ; --- Files common to both modes ---
  SetOutPath "$INSTDIR"
  File "..\dist\${APP_EXE}"

  ${If} $InstallMode == "standard"
    ;------------------------------------------------------
    ; STANDARD MODE
    ;------------------------------------------------------

    ; Sound profiles go to %APPDATA%\MonkeySounds\Sounds
    ; so multiple users on the same machine share one copy
    SetOutPath "$APPDATA\${APP_NAME}\Sounds"
    File /r "..\dist\Sounds\*.*"
    SetOutPath "$INSTDIR"

    ; Write uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"

    ; Registry — install dir
    WriteRegStr HKLM "${REGKEY}" "Install_Dir" "$INSTDIR"

    ; Add / Remove Programs
    WriteRegStr   HKLM "${UNINSTALL_KEY}" "DisplayName"     "${APP_NAME} ${VERSION}"
    WriteRegStr   HKLM "${UNINSTALL_KEY}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
    WriteRegStr   HKLM "${UNINSTALL_KEY}" "DisplayIcon"     "$INSTDIR\${APP_EXE}"
    WriteRegStr   HKLM "${UNINSTALL_KEY}" "Publisher"       "${PUBLISHER}"
    WriteRegStr   HKLM "${UNINSTALL_KEY}" "DisplayVersion"  "${VERSION}"
    WriteRegStr   HKLM "${UNINSTALL_KEY}" "URLInfoAbout"    "${WEBSITE}"
    WriteRegStr   HKLM "${UNINSTALL_KEY}" "HelpLink"        "${WEBSITE}"
    WriteRegDWORD HKLM "${UNINSTALL_KEY}" "NoModify"        1
    WriteRegDWORD HKLM "${UNINSTALL_KEY}" "NoRepair"        1

    ; Start Menu shortcuts
    CreateDirectory "$SMPROGRAMS\${APP_NAME}"
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" \
                   "$INSTDIR\${APP_EXE}" "" "$INSTDIR\${APP_EXE}" 0
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk" \
                   "$INSTDIR\Uninstall.exe" "" "$INSTDIR\Uninstall.exe" 0

    ; Desktop shortcut
    CreateShortCut "$DESKTOP\${APP_NAME}.lnk" \
                   "$INSTDIR\${APP_EXE}" "" "$INSTDIR\${APP_EXE}" 0

  ${Else}
    ;------------------------------------------------------
    ; PORTABLE MODE
    ;------------------------------------------------------

    ; Sound profiles sit next to the EXE
    SetOutPath "$INSTDIR\Sounds"
    File /r "..\dist\Sounds\*.*"
    SetOutPath "$INSTDIR"

    ; Write a settings.json stub so the app detects portable mode
    ; (MonkeySounds checks for settings.json next to the EXE)
    FileOpen  $0 "$INSTDIR\settings.json" w
    FileWrite $0 "{}"
    FileClose $0

    DetailPrint "Portable mode: no registry entries, no uninstaller, no shortcuts."

  ${EndIf}

SectionEnd

;---------------------------------------------------------
; Uninstaller custom page — ask about user data removal
;---------------------------------------------------------
Function un.RemoveDataPageCreate
  nsDialogs::Create 1018
  Pop $0
  ${If} $0 == error
    Abort
  ${EndIf}

  ${NSD_CreateLabel} 0 0 100% 24u "Uninstall Options"
  Pop $0

  ${NSD_CreateCheckbox} 0 30u 100% 14u \
    "Also remove all user data (settings, custom sound profiles) from %APPDATA%\MonkeySounds"
  Pop $UnRemoveCheckbox

  ; Default: unchecked — preserve user data by default
  ${NSD_Uncheck} $UnRemoveCheckbox
  StrCpy $RemoveUserData ${BST_UNCHECKED}

  nsDialogs::Show
FunctionEnd

Function un.RemoveDataPageLeave
  ${NSD_GetState} $UnRemoveCheckbox $RemoveUserData
FunctionEnd

;---------------------------------------------------------
; Uninstaller  (standard mode only — portable has none)
;---------------------------------------------------------
Section "Uninstall"

  SetRegView 64
  ReadRegStr $INSTDIR HKLM "${REGKEY}" "Install_Dir"

  ; Kill running instance
  nsExec::ExecToStack 'taskkill /F /IM "${APP_EXE}"'
  Sleep 500

  ; Remove EXE and uninstaller
  Delete "$INSTDIR\${APP_EXE}"
  Delete "$INSTDIR\Uninstall.exe"

  ; Remove install dir if now empty
  RMDir "$INSTDIR"

  ; Remove shortcuts
  Delete "$DESKTOP\${APP_NAME}.lnk"
  Delete "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk"
  Delete "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk"
  RMDir  "$SMPROGRAMS\${APP_NAME}"

  ; Remove registry keys
  DeleteRegKey HKLM "${UNINSTALL_KEY}"
  DeleteRegKey HKLM "${REGKEY}"

  ; Remove auto-start entry if the user had it enabled
  DeleteRegValue HKCU \
    "Software\Microsoft\Windows\CurrentVersion\Run" "${APP_NAME}"

  ; Conditionally remove %APPDATA%\MonkeySounds based on the user's choice
  ${If} $RemoveUserData == ${BST_CHECKED}
    RMDir /r "$APPDATA\${APP_NAME}"
    DetailPrint "Removed user data: $APPDATA\${APP_NAME}"
  ${Else}
    DetailPrint "User data preserved: $APPDATA\${APP_NAME}"
  ${EndIf}

SectionEnd


