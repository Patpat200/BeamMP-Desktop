param(
    [Parameter(Mandatory=$true)][string]$QtRoot,
    [Parameter(Mandatory=$true)][string]$VcpkgRoot,
    [string]$BuildDirectory = "build-windows-desktop",
    [switch]$SkipTests
)
$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
Set-Location $Root
$QtRoot = (Resolve-Path $QtRoot).Path
$VcpkgRoot = (Resolve-Path $VcpkgRoot).Path
$Toolchain = Join-Path $VcpkgRoot "scripts/buildsystems/vcpkg.cmake"
$Deploy = Join-Path $QtRoot "bin/windeployqt.exe"
if (!(Test-Path $Toolchain)) { throw "vcpkg toolchain missing: $Toolchain" }
if (!(Test-Path $Deploy)) { throw "Use a shared MSVC Qt installation containing windeployqt.exe." }
function Invoke-Checked([string]$Program, [string[]]$Arguments) {
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Program failed with exit code $LASTEXITCODE" }
}
$Tests = if ($SkipTests) { "OFF" } else { "ON" }
Invoke-Checked "cmake" @("-S", ".", "-B", $BuildDirectory, "-A", "x64",
    "-DBEAMMP_BUILD_GUI=ON", "-DBUILD_TESTING=$Tests",
    "-DCMAKE_PREFIX_PATH=$QtRoot", "-DCMAKE_TOOLCHAIN_FILE=$Toolchain",
    "-DVCPKG_TARGET_TRIPLET=x64-windows-static")
Invoke-Checked "cmake" @("--build", $BuildDirectory, "--config", "Release", "--parallel")
$Destination = Join-Path $Root "dist-desktop"
New-Item -ItemType Directory -Force $Destination | Out-Null
Copy-Item (Join-Path $BuildDirectory "Release/BeamMP-Launcher.exe") $Destination -Force
Copy-Item (Join-Path $BuildDirectory "gui/Release/BeamMP-Desktop.exe") $Destination -Force
Invoke-Checked $Deploy @("--release", "--no-translations", (Join-Path $Destination "BeamMP-Desktop.exe"))
if (!$SkipTests) {
    $env:PATH = (Join-Path $QtRoot "bin") + ";" + $env:PATH
    Invoke-Checked "ctest" @("--test-dir", $BuildDirectory, "-C", "Release", "--output-on-failure")
}
Copy-Item "LICENSE" $Destination -Force
Copy-Item "docs/DESKTOP-FR.md" (Join-Path $Destination "LIRE-MOI.md") -Force
Write-Host "Ready: $Destination\BeamMP-Desktop.exe"
