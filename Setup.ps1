param(
    [string]$Configuration = "Release",
    [string]$Platform      = "x64"
)

$ErrorActionPreference = "Stop"
$RepoRoot = $PSScriptRoot

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  MonkeySounds Setup Script" -ForegroundColor Cyan
Write-Host "  Configuration: $Configuration | Platform: $Platform" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# ---------------------------------------------------------------------------
# 1. Locate makensis.exe
# ---------------------------------------------------------------------------
function Find-Makensis {
    $cmd = Get-Command makensis.exe -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }

    $candidates = @(
        "C:\Program Files (x86)\NSIS\makensis.exe",
        "C:\Program Files\NSIS\makensis.exe"
    )
    foreach ($p in $candidates) {
        if (Test-Path $p) { return $p }
    }

    throw "makensis.exe not found. Install NSIS from https://nsis.sourceforge.io/"
}

$makensis = Find-Makensis
Write-Host "[1/3] Located makensis: $makensis" -ForegroundColor Green

# ---------------------------------------------------------------------------
# 2. Run Build.ps1 (compiles solution + populates dist\)
# ---------------------------------------------------------------------------
$buildScript = Join-Path $RepoRoot "Build.ps1"
if (-not (Test-Path $buildScript)) {
    throw "Build.ps1 not found at: $buildScript"
}

Write-Host "[2/3] Running Build.ps1 ($Configuration|$Platform)..." -ForegroundColor Yellow

& powershell -ExecutionPolicy Bypass -File $buildScript `
    -Configuration $Configuration `
    -Platform $Platform

if ($LASTEXITCODE -ne 0) {
    throw "Build.ps1 failed with exit code $LASTEXITCODE"
}

# ---------------------------------------------------------------------------
# 3. Verify dist\ has what NSIS needs
# ---------------------------------------------------------------------------
$distDir    = Join-Path $RepoRoot "dist"
$distExe    = Join-Path $distDir  "MonkeySounds.exe"
$distSounds = Join-Path $distDir  "Sounds"

if (-not (Test-Path $distExe)) {
    throw "dist\MonkeySounds.exe not found. Build may have failed."
}

if (-not (Test-Path $distSounds)) {
    Write-Warning "dist\Sounds not found - installer will have no sound profiles."
}

# ---------------------------------------------------------------------------
# 4. Ensure installer output directory exists
# ---------------------------------------------------------------------------
$setupOutDir = Join-Path $RepoRoot "installer\dist_output"
if (-not (Test-Path $setupOutDir)) {
    New-Item -ItemType Directory -Path $setupOutDir -Force | Out-Null
}

# ---------------------------------------------------------------------------
# 5. Run NSIS compiler
# ---------------------------------------------------------------------------
$nsiScript = Join-Path $RepoRoot "installer\setup.nsi"
if (-not (Test-Path $nsiScript)) {
    throw "NSIS script not found at: $nsiScript"
}

Write-Host "[3/3] Running NSIS compiler..." -ForegroundColor Yellow

Push-Location (Join-Path $RepoRoot "installer")
try {
    & $makensis $nsiScript
    if ($LASTEXITCODE -ne 0) {
        throw "makensis failed with exit code $LASTEXITCODE"
    }
} finally {
    Pop-Location
}

# ---------------------------------------------------------------------------
# Done
# ---------------------------------------------------------------------------
$outputExe = Get-ChildItem (Join-Path $RepoRoot "installer\dist_output") -Filter "*.exe" |
             Sort-Object LastWriteTime -Descending |
             Select-Object -First 1

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "  Installer built successfully!" -ForegroundColor Green
if ($outputExe) {
    Write-Host "  Output: $($outputExe.FullName)" -ForegroundColor Green
}
Write-Host "========================================" -ForegroundColor Green
