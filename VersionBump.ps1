param(
    [Parameter(Position = 0)]
    [string]$Version,

    [ValidateSet("major", "minor", "patch", "build")]
    [string]$Bump = "patch",

    [switch]$DryRun
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------
function Parse-Version {
    param([string]$Value)
    if ($Value -notmatch '^\d+\.\d+\.\d+\.\d+$') {
        throw "Invalid version format: '$Value'. Expected MAJOR.MINOR.PATCH.BUILD (e.g. 1.0.1.0)."
    }
    return ($Value -split '\.') | ForEach-Object { [int]$_ }
}

function Join-Version {
    param([int[]]$Parts)
    return ($Parts -join '.')
}

function Increment-Version {
    param([int[]]$Parts, [string]$Level)
    switch ($Level) {
        "major" { $Parts[0]++; $Parts[1] = 0; $Parts[2] = 0; $Parts[3] = 0 }
        "minor" { $Parts[1]++; $Parts[2] = 0; $Parts[3] = 0 }
        "patch" { $Parts[2]++; $Parts[3] = 0 }
        "build" { $Parts[3]++ }
        default { throw "Unsupported bump level: $Level" }
    }
    return $Parts
}

function Update-FileContent {
    param([string]$Path, [scriptblock]$Transform)

    if (-not (Test-Path -LiteralPath $Path)) {
        throw "File not found: $Path"
    }

    $reader = [System.IO.StreamReader]::new($Path, $true)
    try {
        $original = $reader.ReadToEnd()
        $encoding = $reader.CurrentEncoding
    } finally {
        $reader.Dispose()
    }

    $updated = & $Transform $original

    if ($original -ne $updated) {
        if ($DryRun) {
            Write-Host "[DryRun] Would update: $Path" -ForegroundColor Yellow
        } else {
            [System.IO.File]::WriteAllText($Path, $updated, $encoding)
            Write-Host "Updated : $Path" -ForegroundColor Green
        }
    } else {
        Write-Host "No change: $Path"
    }
}

# ---------------------------------------------------------------------------
# Locate repo root and files
# ---------------------------------------------------------------------------
$repoRoot  = Split-Path -Parent $MyInvocation.MyCommand.Path
$rcFile    = Join-Path $repoRoot "src\MonkeySounds.rc"
$nsiFile   = Join-Path $repoRoot "installer\setup.nsi"
$changelog = Join-Path $repoRoot "CHANGELOG.md"

# ---------------------------------------------------------------------------
# Read current version from MonkeySounds.rc
# (looks for: VALUE "FileVersion",  "1.0.0.0")
# ---------------------------------------------------------------------------
if (-not (Test-Path -LiteralPath $rcFile)) {
    throw "Could not locate MonkeySounds.rc at: $rcFile"
}

$rcText = Get-Content -LiteralPath $rcFile -Raw
$match  = [regex]::Match($rcText, 'VALUE\s+"FileVersion",\s+"(?<v>\d+\.\d+\.\d+\.\d+)"')
if (-not $match.Success) {
    throw "Could not parse current version from MonkeySounds.rc"
}

$currentVersion = $match.Groups["v"].Value

# ---------------------------------------------------------------------------
# Determine target version
# ---------------------------------------------------------------------------
$targetVersion = $Version
if ([string]::IsNullOrWhiteSpace($targetVersion)) {
    $parts      = Parse-Version $currentVersion
    $nextParts  = Increment-Version $parts $Bump
    $targetVersion = Join-Version $nextParts
}

[void](Parse-Version $targetVersion)   # validate format

$v          = $targetVersion -split '\.'
$verMajor   = $v[0]
$verMinor   = $v[1]
$verPatch   = $v[2]
$verBuild   = $v[3]
$verComma   = $targetVersion -replace '\.', ','

Write-Host ""
Write-Host "=======================================" -ForegroundColor Cyan
Write-Host "  MonkeySounds Version Bump" -ForegroundColor Cyan
Write-Host "=======================================" -ForegroundColor Cyan
Write-Host "  Current : $currentVersion"
Write-Host "  Target  : $targetVersion"
if ($DryRun) {
    Write-Host "  Mode    : DRY RUN" -ForegroundColor Yellow
}
Write-Host ""

# ---------------------------------------------------------------------------
# 1. MonkeySounds.rc
#    - #define VER_MAJOR / VER_MINOR / VER_PATCH / VER_BUILD
#    - FILEVERSION / PRODUCTVERSION comma form
#    - VALUE "FileVersion" / "ProductVersion" string form
# ---------------------------------------------------------------------------
Update-FileContent -Path $rcFile -Transform {
    param($text)

    # #define macros
    $text = [regex]::Replace($text, '(?m)^(#define\s+VER_MAJOR\s+)\d+',  ('${1}' + $verMajor))
    $text = [regex]::Replace($text, '(?m)^(#define\s+VER_MINOR\s+)\d+',  ('${1}' + $verMinor))
    $text = [regex]::Replace($text, '(?m)^(#define\s+VER_PATCH\s+)\d+',  ('${1}' + $verPatch))
    $text = [regex]::Replace($text, '(?m)^(#define\s+VER_BUILD\s+)\d+',  ('${1}' + $verBuild))

    # FILEVERSION / PRODUCTVERSION
    $text = [regex]::Replace($text, '(?m)^(\s*FILEVERSION\s+)\d+,\d+,\d+,\d+',    ('${1}' + $verComma))
    $text = [regex]::Replace($text, '(?m)^(\s*PRODUCTVERSION\s+)\d+,\d+,\d+,\d+', ('${1}' + $verComma))

    # String values
    $text = [regex]::Replace($text, '(?m)^(\s*VALUE\s+"FileVersion",\s+)"\d+\.\d+\.\d+\.\d+"',    ('${1}"' + $targetVersion + '"'))
    $text = [regex]::Replace($text, '(?m)^(\s*VALUE\s+"ProductVersion",\s+)"\d+\.\d+\.\d+\.\d+"', ('${1}"' + $targetVersion + '"'))

    return $text
}

# ---------------------------------------------------------------------------
# 2. installer\setup.nsi
#    - !define VERSION "x.x.x.x"
# ---------------------------------------------------------------------------
Update-FileContent -Path $nsiFile -Transform {
    param($text)
    [regex]::Replace(
        $text,
        '(?m)^(!define\s+VERSION\s+)"\d+\.\d+\.\d+\.\d+"',
        ('${1}"' + $targetVersion + '"')
    )
}

# ---------------------------------------------------------------------------
# Done
# ---------------------------------------------------------------------------
Write-Host ""
Write-Host "=======================================" -ForegroundColor Green
Write-Host "  Version bump complete: $targetVersion" -ForegroundColor Green
Write-Host "=======================================" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:"
Write-Host "  1. Fill in the CHANGELOG.md entry for $targetVersion"
Write-Host "  2. git add -A"
Write-Host "  3. git commit -m `"chore: bump version to $targetVersion`""
Write-Host "  4. git tag v$targetVersion"
Write-Host "  5. git push origin master --tags"
