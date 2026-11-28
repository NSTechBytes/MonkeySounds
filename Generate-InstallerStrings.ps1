<#
  Generates installer/InstallerStrings.nsh from installer.* keys in locales/*.json.
  Run this after changing an installer translation:
    .\Generate-InstallerStrings.ps1
#>
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$localeDirectory = Join-Path $root 'locales'
$outputPath = Join-Path $root 'installer\InstallerStrings.nsh'

# Keep this mapping aligned with src/Lang.cpp (kSupportedLocales) and installer/setup.nsi.
$languages = [ordered]@{
  'en'      = 'English';      'ar'      = 'Arabic';       'bg'      = 'Bulgarian'
  'cs'      = 'Czech';        'da'      = 'Danish';       'de'      = 'German'
  'el'      = 'Greek';        'es'      = 'Spanish';      'et'      = 'Estonian'
  'fi'      = 'Finnish';      'fr'      = 'French';       'he'      = 'Hebrew'
  'hr'      = 'Croatian';     'hu'      = 'Hungarian';    'id'      = 'Indonesian'
  'it'      = 'Italian';      'ja'      = 'Japanese';     'ko'      = 'Korean'
  'lt'      = 'Lithuanian';   'lv'      = 'Latvian';      'ms'      = 'Malay'
  'nb'      = 'Norwegian';    'nl'      = 'Dutch';        'pl'      = 'Polish'
  'pt-BR'   = 'PortugueseBR'; 'pt-PT'   = 'Portuguese';   'ro'      = 'Romanian'
  'ru'      = 'Russian';      'sk'      = 'Slovak';       'sl'      = 'Slovenian'
  'sr-Cyrl' = 'Serbian';      'sr-Latn' = 'SerbianLatin'; 'sv'      = 'Swedish'
  'th'      = 'Thai';         'tr'      = 'Turkish';      'uk'      = 'Ukrainian'
  'vi'      = 'Vietnamese';   'zh-Hans' = 'SimpChinese';  'zh-Hant' = 'TradChinese'
}

$strings = [ordered]@{
  'installer.welcomeTitle'         = 'INST_WELCOME_TITLE'
  'installer.welcomeText'          = 'INST_WELCOME_TEXT'
  'installer.run'                  = 'INST_FINISH_LAUNCH'
  'installer.visitGithub'          = 'INST_FINISH_LINK'
  'installer.chooseModeHeader'     = 'INST_MODE_HEADER'
  'installer.chooseModeLabel'      = 'INST_MODE_LABEL'
  'installer.standardRecommended'  = 'INST_MODE_STD'
  'installer.standardDesc'         = 'INST_MODE_STD_DESC'
  'installer.portable'             = 'INST_MODE_PORT'
  'installer.portableDesc'         = 'INST_MODE_PORT_DESC'
  'installer.portableSysDirWarn'   = 'INST_PORT_SYSDIR_WARN'
  'installer.uninstallOptions'     = 'UNINST_OPTIONS_HEADER'
  'installer.removeUserData'       = 'UNINST_REMOVE_DATA'
  'installer.portableNoShortcuts'  = 'DETAIL_PORTABLE_NO_SHORTCUTS'
  'installer.dataRemoved'          = 'DETAIL_DATA_REMOVED'
  'installer.dataPreserved'        = 'DETAIL_DATA_PRESERVED'
  'installer.uacRequired'          = 'INST_UAC_REQUIRED'
  'installer.uacTryAgain'          = 'INST_UAC_TRY_AGAIN'
}

if (-not (Test-Path -LiteralPath $localeDirectory)) {
  throw "Locale directory is missing: $localeDirectory"
}

$lines = [System.Collections.Generic.List[string]]::new()
$lines.Add('; Generated from locales/*.json by Generate-InstallerStrings.ps1. Do not edit by hand.')

foreach ($language in $languages.GetEnumerator()) {
  $catalogPath = Join-Path $localeDirectory "$($language.Key).json"
  if (-not (Test-Path -LiteralPath $catalogPath)) {
    throw "Missing locale catalog: $catalogPath"
  }

  try {
    $catalog = Get-Content -Raw -Encoding utf8 -LiteralPath $catalogPath |
      ConvertFrom-Json
  } catch {
    throw "Invalid JSON in ${catalogPath}: $($_.Exception.Message)"
  }

  foreach ($string in $strings.GetEnumerator()) {
    $property = $catalog.PSObject.Properties[$string.Key]
    if ($null -eq $property -or $property.Value -isnot [string] -or
        [string]::IsNullOrWhiteSpace($property.Value)) {
      throw "$($language.Key).json: missing non-empty '$($string.Key)'"
    }

    # Escape the NSIS characters that have syntax meaning inside a LangString.
    $value = $property.Value.Replace('$', '$$').Replace('"', '$\"').
      Replace("`r", '').Replace("`n", '$\r$\n')
    $lines.Add("LangString $($string.Value) `${LANG_$($language.Value.ToUpperInvariant())} `"$value`"")
  }
}

[System.IO.File]::WriteAllLines(
  $outputPath, $lines, [System.Text.UTF8Encoding]::new($true))

Write-Host "Generated $outputPath with $($lines.Count - 1) language strings." -ForegroundColor Green
