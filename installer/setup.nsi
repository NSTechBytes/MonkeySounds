;---------------------------------------------------------
; MonkeySounds Installer Script
; Supports Standard (recommended) and Portable modes
; Multi-language: asks user at startup (MUI_LANGDLL)
; Translations generated from locales/*.json into InstallerStrings.nsh
;---------------------------------------------------------

!define APP_NAME       "MonkeySounds"
!define APP_EXE        "MonkeySounds.exe"
!define PUBLISHER      "NSTechBytes"
!define VERSION        "1.1.0.0"
!define REGKEY         "Software\MonkeySounds"
!define UNINSTALL_KEY  "Software\Microsoft\Windows\CurrentVersion\Uninstall\MonkeySounds"
!define WEBSITE        "https://github.com/NSTechBytes/MonkeySounds"

Name "${APP_NAME}"
OutFile "dist_output\MonkeySounds_Setup_v${VERSION}.exe"
Unicode True
SetCompressor /SOLID lzma
ReserveFile "plugins\x86-unicode\UAC.dll"

; Default install dir (overridden by portable mode at runtime)
InstallDir "$PROGRAMFILES64\${APP_NAME}"
InstallDirRegKey HKLM "${REGKEY}" "Install_Dir"

; Start as the desktop user. Standard installs elevate only after the user presses
; Install, so Windows shows the UAC shield on that button instead of on the
; installer executable's icon.
RequestExecutionLevel user

;---------------------------------------------------------
; Includes — StrFunc MUST be included and initialized
;            before any function that uses ${StrStr}
;---------------------------------------------------------
!include "StrFunc.nsh"
${StrStr}   ; initialize the StrStr function

!include "x64.nsh"
!include "LogicLib.nsh"
!include "nsDialogs.nsh"
!include "Sections.nsh"
!addplugindir "plugins\x86-unicode"
!include "nsis\UAC.nsh"
!include "MUI2.nsh"

!ifndef BCM_SETSHIELD
!define BCM_SETSHIELD 0x160C
!endif

;---------------------------------------------------------
; Interface
;---------------------------------------------------------
!define MUI_ICON                       "assets\installer.ico"
!define MUI_UNICON                     "assets\uninstaller.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP         "assets\header.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP   "assets\banner.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "assets\banner.bmp"

; Always show the complete Unicode language list. This prevents NSIS from
; hiding a catalog simply because the legacy system ANSI code page differs.
!define MUI_LANGDLL_ALLLANGUAGES

!define MUI_WELCOMEPAGE_TITLE "$(INST_WELCOME_TITLE)"
!define MUI_WELCOMEPAGE_TEXT  "$(INST_WELCOME_TEXT)"

!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_TEXT     "$(INST_FINISH_LAUNCH)"
!define MUI_FINISHPAGE_RUN_FUNCTION FinishRun
!define MUI_FINISHPAGE_LINK         "$(INST_FINISH_LINK)"
!define MUI_FINISHPAGE_LINK_LOCATION "${WEBSITE}"

;---------------------------------------------------------
; Pages
;---------------------------------------------------------
!define MUI_PAGE_CUSTOMFUNCTION_PRE SkipPageIfInnerInstance
!insertmacro MUI_PAGE_WELCOME
!define MUI_PAGE_CUSTOMFUNCTION_PRE SkipPageIfInnerInstance
!insertmacro MUI_PAGE_LICENSE "..\LICENSE.txt"

; Custom page: choose Standard or Portable
Page custom InstallModePageCreate InstallModePageLeave

; Directory page — hidden / overridden in portable mode
!define MUI_PAGE_CUSTOMFUNCTION_PRE SkipPageIfInnerInstance
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE DirectoryPageLeave
!insertmacro MUI_PAGE_DIRECTORY

!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_CONFIRM
UninstPage custom un.RemoveDataPageCreate un.RemoveDataPageLeave
!insertmacro MUI_UNPAGE_INSTFILES

;---------------------------------------------------------
; Languages — Keep aligned with src/Lang.cpp (kSupportedLocales) & Generate-InstallerStrings.ps1
;---------------------------------------------------------
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Arabic"
!insertmacro MUI_LANGUAGE "Bulgarian"
!insertmacro MUI_LANGUAGE "Czech"
!insertmacro MUI_LANGUAGE "Danish"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Greek"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "Estonian"
!insertmacro MUI_LANGUAGE "Finnish"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "Hebrew"
!insertmacro MUI_LANGUAGE "Croatian"
!insertmacro MUI_LANGUAGE "Hungarian"
!insertmacro MUI_LANGUAGE "Indonesian"
!insertmacro MUI_LANGUAGE "Italian"
!insertmacro MUI_LANGUAGE "Japanese"
!insertmacro MUI_LANGUAGE "Korean"
!insertmacro MUI_LANGUAGE "Lithuanian"
!insertmacro MUI_LANGUAGE "Latvian"
!insertmacro MUI_LANGUAGE "Malay"
!insertmacro MUI_LANGUAGE "Norwegian"
!insertmacro MUI_LANGUAGE "Dutch"
!insertmacro MUI_LANGUAGE "Polish"
!insertmacro MUI_LANGUAGE "PortugueseBR"
!insertmacro MUI_LANGUAGE "Portuguese"
!insertmacro MUI_LANGUAGE "Romanian"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "Slovak"
!insertmacro MUI_LANGUAGE "Slovenian"
!insertmacro MUI_LANGUAGE "Serbian"
!insertmacro MUI_LANGUAGE "SerbianLatin"
!insertmacro MUI_LANGUAGE "Swedish"
!insertmacro MUI_LANGUAGE "Thai"
!insertmacro MUI_LANGUAGE "Turkish"
!insertmacro MUI_LANGUAGE "Ukrainian"
!insertmacro MUI_LANGUAGE "Vietnamese"
!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "TradChinese"

!include "InstallerStrings.nsh"

;---------------------------------------------------------
; Variables
;---------------------------------------------------------
Var InstallMode      ; "standard" or "portable"
Var RadioStandard
Var RadioPortable
Var RemoveUserData   ; BST_CHECKED or BST_UNCHECKED
Var UnRemoveCheckbox
Var UserAppDataDir
Var InstallerLocale

;---------------------------------------------------------
; UAC Elevation Macro (same pattern as Novadesk / Rainmeter)
;---------------------------------------------------------
!macro ElevateForStandardInstall
MonkeySoundsUacTryAgain:
  !insertmacro UAC_RunElevated
  ${Switch} $0
    ${Case} 0
      ${IfThen} $1 = 1 ${|} Quit ${|}
      ${IfThen} $3 <> 0 ${|} ${Break} ${|}
      ${If} $1 = 3
        MessageBox MB_YESNO|MB_ICONEXCLAMATION|MB_TOPMOST "$(INST_UAC_TRY_AGAIN)" IDYES MonkeySoundsUacTryAgain
      ${EndIf}
    ${Case} 1223
      Quit
    ${Case} 1062
      MessageBox MB_OK|MB_ICONSTOP "The logon service is not running, so ${APP_NAME} cannot be installed."
      Quit
    ${Default}
      MessageBox MB_OK|MB_ICONSTOP "Unable to obtain administrator access for the ${APP_NAME} installation. Error: $0"
      Quit
  ${EndSwitch}
  SetShellVarContext all
!macroend

;---------------------------------------------------------
; Map NSIS $LANGUAGE ID -> MonkeySounds locale filename
;---------------------------------------------------------
Function SetInstallerLocale
  StrCpy $InstallerLocale "en"
  ${If} $LANGUAGE == ${LANG_ARABIC}
    StrCpy $InstallerLocale "ar"
  ${ElseIf} $LANGUAGE == ${LANG_BULGARIAN}
    StrCpy $InstallerLocale "bg"
  ${ElseIf} $LANGUAGE == ${LANG_SIMPCHINESE}
    StrCpy $InstallerLocale "zh-Hans"
  ${ElseIf} $LANGUAGE == ${LANG_TRADCHINESE}
    StrCpy $InstallerLocale "zh-Hant"
  ${ElseIf} $LANGUAGE == ${LANG_CROATIAN}
    StrCpy $InstallerLocale "hr"
  ${ElseIf} $LANGUAGE == ${LANG_CZECH}
    StrCpy $InstallerLocale "cs"
  ${ElseIf} $LANGUAGE == ${LANG_DANISH}
    StrCpy $InstallerLocale "da"
  ${ElseIf} $LANGUAGE == ${LANG_DUTCH}
    StrCpy $InstallerLocale "nl"
  ${ElseIf} $LANGUAGE == ${LANG_ESTONIAN}
    StrCpy $InstallerLocale "et"
  ${ElseIf} $LANGUAGE == ${LANG_FINNISH}
    StrCpy $InstallerLocale "fi"
  ${ElseIf} $LANGUAGE == ${LANG_FRENCH}
    StrCpy $InstallerLocale "fr"
  ${ElseIf} $LANGUAGE == ${LANG_GERMAN}
    StrCpy $InstallerLocale "de"
  ${ElseIf} $LANGUAGE == ${LANG_GREEK}
    StrCpy $InstallerLocale "el"
  ${ElseIf} $LANGUAGE == ${LANG_HEBREW}
    StrCpy $InstallerLocale "he"
  ${ElseIf} $LANGUAGE == ${LANG_HUNGARIAN}
    StrCpy $InstallerLocale "hu"
  ${ElseIf} $LANGUAGE == ${LANG_INDONESIAN}
    StrCpy $InstallerLocale "id"
  ${ElseIf} $LANGUAGE == ${LANG_ITALIAN}
    StrCpy $InstallerLocale "it"
  ${ElseIf} $LANGUAGE == ${LANG_JAPANESE}
    StrCpy $InstallerLocale "ja"
  ${ElseIf} $LANGUAGE == ${LANG_KOREAN}
    StrCpy $InstallerLocale "ko"
  ${ElseIf} $LANGUAGE == ${LANG_LATVIAN}
    StrCpy $InstallerLocale "lv"
  ${ElseIf} $LANGUAGE == ${LANG_LITHUANIAN}
    StrCpy $InstallerLocale "lt"
  ${ElseIf} $LANGUAGE == ${LANG_MALAY}
    StrCpy $InstallerLocale "ms"
  ${ElseIf} $LANGUAGE == ${LANG_NORWEGIAN}
    StrCpy $InstallerLocale "nb"
  ${ElseIf} $LANGUAGE == ${LANG_POLISH}
    StrCpy $InstallerLocale "pl"
  ${ElseIf} $LANGUAGE == ${LANG_PORTUGUESEBR}
    StrCpy $InstallerLocale "pt-BR"
  ${ElseIf} $LANGUAGE == ${LANG_PORTUGUESE}
    StrCpy $InstallerLocale "pt-PT"
  ${ElseIf} $LANGUAGE == ${LANG_ROMANIAN}
    StrCpy $InstallerLocale "ro"
  ${ElseIf} $LANGUAGE == ${LANG_RUSSIAN}
    StrCpy $InstallerLocale "ru"
  ${ElseIf} $LANGUAGE == ${LANG_SERBIAN}
    StrCpy $InstallerLocale "sr-Cyrl"
  ${ElseIf} $LANGUAGE == ${LANG_SERBIANLATIN}
    StrCpy $InstallerLocale "sr-Latn"
  ${ElseIf} $LANGUAGE == ${LANG_SLOVAK}
    StrCpy $InstallerLocale "sk"
  ${ElseIf} $LANGUAGE == ${LANG_SLOVENIAN}
    StrCpy $InstallerLocale "sl"
  ${ElseIf} $LANGUAGE == ${LANG_SPANISH}
    StrCpy $InstallerLocale "es"
  ${ElseIf} $LANGUAGE == ${LANG_SWEDISH}
    StrCpy $InstallerLocale "sv"
  ${ElseIf} $LANGUAGE == ${LANG_THAI}
    StrCpy $InstallerLocale "th"
  ${ElseIf} $LANGUAGE == ${LANG_TURKISH}
    StrCpy $InstallerLocale "tr"
  ${ElseIf} $LANGUAGE == ${LANG_UKRAINIAN}
    StrCpy $InstallerLocale "uk"
  ${ElseIf} $LANGUAGE == ${LANG_VIETNAMESE}
    StrCpy $InstallerLocale "vi"
  ${EndIf}
FunctionEnd

;---------------------------------------------------------
; UAC Inner/Outer Sync & Page Skip Helpers
;---------------------------------------------------------
Function SyncSettingsToInner
  SetShellVarContext current
  StrCpy $1 $InstallMode
  StrCpy $2 $INSTDIR
  StrCpy $3 "$APPDATA"
  StrCpy $4 $LANGUAGE
  HideWindow
FunctionEnd

Function SkipPageIfInnerInstance
  ${If} ${UAC_IsInnerInstance}
    Abort
  ${EndIf}
FunctionEnd

Function FinishRun
  ; Explorer launches MonkeySounds with the desktop user's token instead of the elevated installer's token.
  ExecShell "" "$WINDIR\explorer.exe" '$\"$INSTDIR\${APP_EXE}$\"'
FunctionEnd

;---------------------------------------------------------
; .onInit — language picker + UAC inner instance sync
;---------------------------------------------------------
Function .onInit
  ${IfNot} ${UAC_IsInnerInstance}
    ${Unless} ${Silent}
      ; Display the exact same language names & order as MonkeySounds Settings (Lang.cpp)
      ; LangDLL expects pairs of: "Display Name" "${LANG_ID}"
      LangDLL::LangDialog "Installer Language" "Please select a language." A \
        "English"                                    "${LANG_ENGLISH}" \
        "العربية (Arabic)"                           "${LANG_ARABIC}" \
        "Български (Bulgarian)"                      "${LANG_BULGARIAN}" \
        "Čeština (Czech)"                            "${LANG_CZECH}" \
        "Dansk (Danish)"                             "${LANG_DANISH}" \
        "Deutsch (German)"                           "${LANG_GERMAN}" \
        "Ελληνικά (Greek)"                           "${LANG_GREEK}" \
        "Español (Spanish)"                          "${LANG_SPANISH}" \
        "Eesti (Estonian)"                           "${LANG_ESTONIAN}" \
        "Suomi (Finnish)"                            "${LANG_FINNISH}" \
        "Français (French)"                          "${LANG_FRENCH}" \
        "עברית (Hebrew)"                             "${LANG_HEBREW}" \
        "Hrvatski (Croatian)"                        "${LANG_CROATIAN}" \
        "Magyar (Hungarian)"                         "${LANG_HUNGARIAN}" \
        "Indonesia (Indonesian)"                     "${LANG_INDONESIAN}" \
        "Italiano (Italian)"                         "${LANG_ITALIAN}" \
        "日本語 (Japanese)"                          "${LANG_JAPANESE}" \
        "한국어 (Korean)"                            "${LANG_KOREAN}" \
        "Lietuvių (Lithuanian)"                      "${LANG_LITHUANIAN}" \
        "Latviešu (Latvian)"                         "${LANG_LATVIAN}" \
        "Melayu (Malay)"                             "${LANG_MALAY}" \
        "Norsk (Norwegian)"                          "${LANG_NORWEGIAN}" \
        "Nederlands (Dutch)"                         "${LANG_DUTCH}" \
        "Polski (Polish)"                            "${LANG_POLISH}" \
        "Português (Brasil) (Portuguese Brazil)"     "${LANG_PORTUGUESEBR}" \
        "Português (Portugal) (Portuguese Portugal)" "${LANG_PORTUGUESE}" \
        "Română (Romanian)"                          "${LANG_ROMANIAN}" \
        "Русский (Russian)"                          "${LANG_RUSSIAN}" \
        "Slovenčina (Slovak)"                        "${LANG_SLOVAK}" \
        "Slovenščina (Slovenian)"                    "${LANG_SLOVENIAN}" \
        "Српски (ћирилица) (Serbian Cyrillic)"       "${LANG_SERBIAN}" \
        "Српски (latinica) (Serbian Latin)"          "${LANG_SERBIANLATIN}" \
        "Svenska (Swedish)"                          "${LANG_SWEDISH}" \
        "ไทย (Thai)"                                 "${LANG_THAI}" \
        "Türkçe (Turkish)"                           "${LANG_TURKISH}" \
        "Українська (Ukrainian)"                     "${LANG_UKRAINIAN}" \
        "Tiếng Việt (Vietnamese)"                    "${LANG_VIETNAMESE}" \
        "简体中文 (Chinese Simplified)"              "${LANG_SIMPCHINESE}" \
        "繁體中文 (Chinese Traditional)"             "${LANG_TRADCHINESE}" \
        ""
      Pop $LANGUAGE
      ${If} $LANGUAGE == "cancel"
        Abort
      ${EndIf}
    ${EndUnless}
  ${EndIf}
  Call SetInstallerLocale
  SetShellVarContext current
  StrCpy $UserAppDataDir "$APPDATA"
  StrCpy $InstallMode "standard"
  !insertmacro SelectSection ${SecMain}

  ${If} ${UAC_IsInnerInstance}
    ${IfNot} ${UAC_IsAdmin}
      MessageBox MB_OK|MB_ICONSTOP "$(INST_UAC_REQUIRED)" /SD IDOK
      Quit
    ${EndIf}
    !insertmacro UAC_AsUser_Call Function SyncSettingsToInner ${UAC_SYNCREGISTERS}
    StrCpy $InstallMode $1
    StrCpy $INSTDIR $2
    StrCpy $UserAppDataDir $3
    StrCpy $LANGUAGE $4
    Call SetInstallerLocale
  ${EndIf}
FunctionEnd

;---------------------------------------------------------
; Custom page: Install Mode
;---------------------------------------------------------
Function InstallModePageCreate
  ${If} ${UAC_IsInnerInstance}
    Abort
  ${EndIf}

  ; Ensure Next button has no UAC shield while on this page
  GetDlgItem $1 $HWNDPARENT 1
  SendMessage $1 ${BCM_SETSHIELD} 0 0

  !insertmacro MUI_HEADER_TEXT "$(INST_MODE_HEADER)" ""
  nsDialogs::Create 1018
  Pop $0
  ${If} $0 == error
    Abort
  ${EndIf}
  nsDialogs::SetRTL $(^RTL)

  ${NSD_CreateLabel} 0 0 100% 20u "$(INST_MODE_LABEL)"
  Pop $0

  ${NSD_CreateRadioButton} 0 26u 100% 14u "$(INST_MODE_STD)"
  Pop $RadioStandard

  ${NSD_CreateLabel} 12u 42u 100% 20u "$(INST_MODE_STD_DESC)"
  Pop $0

  ${NSD_CreateRadioButton} 0 66u 100% 14u "$(INST_MODE_PORT)"
  Pop $RadioPortable

  ${NSD_CreateLabel} 12u 82u 100% 20u "$(INST_MODE_PORT_DESC)"
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

  ; Match Windows' elevation affordance on the Directory page's Install button
  GetDlgItem $1 $HWNDPARENT 1
  ${If} $InstallMode == "standard"
    SendMessage $1 ${BCM_SETSHIELD} 0 1
  ${Else}
    SendMessage $1 ${BCM_SETSHIELD} 0 0
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
      MessageBox MB_ICONEXCLAMATION|MB_OK "$(INST_PORT_SYSDIR_WARN)"
      Abort
    ${EndIf}
  ${EndIf}
FunctionEnd

;---------------------------------------------------------
; Install Section
;---------------------------------------------------------
Section "MonkeySounds" SecMain
  SectionIn RO

  ${If} $InstallMode == "standard"
    ${IfNot} ${UAC_IsAdmin}
      System::Call "shell32::IsUserAnAdmin()i.r0"
      ${If} $0 = 0
        !insertmacro ElevateForStandardInstall
      ${EndIf}
    ${EndIf}
    SetShellVarContext all
  ${EndIf}

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

  ; Locale JSON files (always installed next to EXE)
  SetOutPath "$INSTDIR\locales"
  File /r "..\dist\locales\*.*"
  SetOutPath "$INSTDIR"

  ${If} $InstallMode == "standard"
    ;------------------------------------------------------
    ; STANDARD MODE
    ;------------------------------------------------------
    ${If} $UserAppDataDir == ""
      SetShellVarContext current
      StrCpy $UserAppDataDir "$APPDATA"
      SetShellVarContext all
    ${EndIf}

    ; Sound profiles go to %APPDATA%\MonkeySounds\Sounds
    SetOutPath "$UserAppDataDir\${APP_NAME}\Sounds"
    File /r "..\dist\Sounds\*.*"
    SetOutPath "$INSTDIR"

    ; Write initial settings.json with selected installer language if not present
    ${IfNot} ${FileExists} "$UserAppDataDir\${APP_NAME}\settings.json"
      CreateDirectory "$UserAppDataDir\${APP_NAME}"
      FileOpen  $0 "$UserAppDataDir\${APP_NAME}\settings.json" w
      FileWrite $0 "{$\r$\n  $\"language$\": $\"$InstallerLocale$\"$\r$\n}$\r$\n"
      FileClose $0
    ${EndIf}

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

    ; Write settings.json next to EXE so the app detects portable mode
    ; and starts in the installer language (preserve existing settings on upgrade)
    ${IfNot} ${FileExists} "$INSTDIR\settings.json"
      FileOpen  $0 "$INSTDIR\settings.json" w
      FileWrite $0 "{$\r$\n  $\"language$\": $\"$InstallerLocale$\"$\r$\n}$\r$\n"
      FileClose $0
    ${EndIf}

    DetailPrint "$(DETAIL_PORTABLE_NO_SHORTCUTS)"

  ${EndIf}

SectionEnd

;---------------------------------------------------------
; Uninstaller init — elevate for standard uninstall
;---------------------------------------------------------
Function un.onInit
  !insertmacro ElevateForStandardInstall
FunctionEnd

;---------------------------------------------------------
; Uninstaller custom page — ask about user data removal
;---------------------------------------------------------
Function un.RemoveDataPageCreate
  nsDialogs::Create 1018
  Pop $0
  ${If} $0 == error
    Abort
  ${EndIf}
  nsDialogs::SetRTL $(^RTL)

  ${NSD_CreateLabel} 0 0 100% 24u "$(UNINST_OPTIONS_HEADER)"
  Pop $0

  ${NSD_CreateCheckbox} 0 30u 100% 14u "$(UNINST_REMOVE_DATA)"
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
; Uninstaller (standard mode only — portable has none)
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

  ; Remove locales
  RMDir /r "$INSTDIR\locales"

  ; Remove install dir if now empty
  RMDir "$INSTDIR"

  ; Remove shortcuts
  SetShellVarContext all
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
    SetShellVarContext current
    RMDir /r "$APPDATA\${APP_NAME}"
    SetShellVarContext all
    RMDir /r "$APPDATA\${APP_NAME}"
    DetailPrint "$(DETAIL_DATA_REMOVED)"
  ${Else}
    DetailPrint "$(DETAIL_DATA_PRESERVED)"
  ${EndIf}

SectionEnd
