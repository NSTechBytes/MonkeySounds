param(
    [string]$Configuration = "Debug",
    [string]$Platform = "x64"
)

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  MonkeySounds Build Script" -ForegroundColor Cyan
Write-Host "  Configuration: $Configuration | Platform: $Platform" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# 1. Locate vswhere.exe
$vswherePaths = @(
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe",
    "${env:ProgramFiles}\Microsoft Visual Studio\Installer\vswhere.exe"
)

$vswhere = $null
foreach ($path in $vswherePaths) {
    if (Test-Path $path) {
        $vswhere = $path
        break
    }
}

if (-not $vswhere) {
    $vswhereCmd = Get-Command vswhere -ErrorAction SilentlyContinue
    if ($vswhereCmd) {
        $vswhere = $vswhereCmd.Source
    }
}

if (-not $vswhere) {
    Write-Error "Could not locate vswhere.exe. Please ensure Visual Studio is installed."
    exit 1
}

Write-Host "[1/3] Located vswhere: $vswhere" -ForegroundColor Green

# 2. Find MSBuild using vswhere
$msbuild = & $vswhere -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe | Select-Object -First 1

if (-not $msbuild -or -not (Test-Path $msbuild)) {
    Write-Error "Could not find MSBuild.exe using vswhere."
    exit 1
}

Write-Host "[2/3] Located MSBuild: $msbuild" -ForegroundColor Green

# 3. Build Solution
$slnPath = Join-Path $PSScriptRoot "MonkeySounds.sln"
$outDir  = Join-Path $PSScriptRoot "$Platform\$Configuration"
Write-Host "[3/3] Building solution: $slnPath ($Configuration|$Platform)..." -ForegroundColor Yellow

& $msbuild $slnPath /p:Configuration=$Configuration /p:Platform=$Platform /m /verbosity:minimal

if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}

Write-Host "`nBuild succeeded! Output is located at: $outDir" -ForegroundColor Green

# 4. Populate dist\ folder
$distDir    = Join-Path $PSScriptRoot "dist"
$srcExe     = Join-Path $outDir "MonkeySounds.exe"
$srcSounds  = Join-Path $PSScriptRoot "Sounds"

Write-Host "`n[dist] Preparing dist folder: $distDir" -ForegroundColor Cyan

# Clean and recreate dist
if (Test-Path $distDir) {
    Remove-Item -Recurse -Force $distDir
}
New-Item -ItemType Directory -Path $distDir -Force | Out-Null

# Copy EXE
if (Test-Path $srcExe) {
    Copy-Item -Path $srcExe -Destination $distDir -Force
    Write-Host "[dist] Copied MonkeySounds.exe" -ForegroundColor Green
} else {
    Write-Warning "[dist] EXE not found at: $srcExe"
}

# Copy Sounds folder
if (Test-Path $srcSounds) {
    Copy-Item -Path $srcSounds -Destination (Join-Path $distDir "Sounds") -Recurse -Force
    Write-Host "[dist] Copied Sounds folder" -ForegroundColor Green
} else {
    Write-Warning "[dist] Sounds folder not found at: $srcSounds"
}

Write-Host "`nDist ready at: $distDir" -ForegroundColor Green
