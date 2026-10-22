# Build-VOIDCORE-Private.ps1
# Build a private VOIDCORE VST3 on your own Windows PC, without GitHub Actions minutes.
# Requirements: Visual Studio 2022 with "Desktop development with C++", CMake,
# Python 3.10+ and internet access for the public JUCE archive / pip packages.
# Usage: powershell -ExecutionPolicy Bypass -File .\Build-VOIDCORE-Private.ps1
# Optional: -Edition SAFE_GUI -SkipBanks (only if 200 Banks/*.wav are already present)
[CmdletBinding()]
param(
    [ValidateSet('FULL_GUI', 'SAFE_GUI')]
    [string]$Edition = 'FULL_GUI',
    [switch]$SkipBanks,
    [int]$Parallel = 4
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$repoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location -LiteralPath $repoRoot
$dependencies = Join-Path $repoRoot '.voidcore-deps'
$juceDir = Join-Path $dependencies 'JUCE-8.0.15'
$buildDir = Join-Path $repoRoot ('build/private-' + $Edition)
$releaseDir = Join-Path $repoRoot 'Private-Builds'
$generic = if ($Edition -eq 'SAFE_GUI') { 'ON' } else { 'OFF' }

function Assert-ExternalCommand([string]$name) {
    if (!(Get-Command $name -ErrorAction SilentlyContinue)) {
        throw "Missing '$name'. Install Python, CMake, and Visual Studio 2022 C++ tools before building."
    }
}
function Invoke-Checked([string]$program, [string[]]$arguments) {
    & $program @arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$program failed with exit code $LASTEXITCODE. Arguments: $($arguments -join ' ')"
    }
}

Assert-ExternalCommand 'python'
Assert-ExternalCommand 'cmake'
if (![Environment]::Is64BitOperatingSystem) { throw '64-bit Windows is required.' }
if ($Parallel -lt 1 -or $Parallel -gt 16) { throw 'Use -Parallel between 1 and 16.' }

Write-Host 'VOIDCORE ASTRAL XENO - PRIVATE LOCAL VST3 BUILDER' -ForegroundColor Green
Write-Host "Edition: $Edition"
Write-Host "Source directory: $repoRoot"
Write-Host 'No private files or source code are uploaded by this script.'

Write-Host 'Checking JUCE GUI source wiring...'
Invoke-Checked 'python' @('tests/verify_voidcore_ui.py')
Invoke-Checked 'python' @('tests/verify_astral_skin.py')

if (!$SkipBanks) {
    Write-Host 'Preparing 200 original XENO soundscape WAVs...'
    Invoke-Checked 'python' @('-m', 'pip', 'install', '-r', 'requirements.txt')
    Invoke-Checked 'python' @('Source/generate_xeno_soundscapes.py')
    Invoke-Checked 'python' @('tests/verify_xeno_diversity.py')
}
$bankDir = Join-Path $repoRoot 'Banks'
$wavCount = @(Get-ChildItem -LiteralPath $bankDir -File -Filter '*.wav' -ErrorAction SilentlyContinue).Count
if ($wavCount -ne 200) {
    throw "Expected 200 WAVs in Banks, found $wavCount. Omit -SkipBanks to generate them."
}

if (!(Test-Path -LiteralPath (Join-Path $juceDir 'CMakeLists.txt'))) {
    Write-Host 'Downloading open-source JUCE 8.0.15 into the local dependencies folder...'
    New-Item -ItemType Directory -Path $dependencies -Force | Out-Null
    $archive = Join-Path $dependencies 'juce-8.0.15.zip'
    $url = 'https://codeload.github.com/juce-framework/JUCE/zip/refs/tags/8.0.15'
    $downloaded = $false
    for ($attempt = 1; $attempt -le 5; $attempt++) {
        try {
            Invoke-WebRequest -Uri $url -OutFile $archive -TimeoutSec 180
            Expand-Archive -LiteralPath $archive -DestinationPath $dependencies -Force
            if (!(Test-Path -LiteralPath (Join-Path $juceDir 'CMakeLists.txt'))) {
                throw 'JUCE archive was incomplete'
            }
            $downloaded = $true
            break
        } catch {
            Write-Warning "JUCE download attempt $attempt failed: $_"
            Start-Sleep -Seconds (2 * $attempt)
        }
    }
    if (!$downloaded) { throw 'Could not retrieve JUCE 8.0.15. Verify your network access.' }
}

Write-Host "Configuring Visual Studio 2022 x64 ($Edition)..."
Invoke-Checked 'cmake' @(
    '-S', $repoRoot, '-B', $buildDir,
    '-G', 'Visual Studio 17 2022', '-A', 'x64',
    "-DJUCE_DIR=$juceDir",
    '-DJUCE_BUILD_EXTRAS=OFF',
    "-DNOCTIVORE_GENERIC_EDITOR=$generic"
)

Write-Host 'Compiling the Release VST3 (this may require significant disk space)...'
Invoke-Checked 'cmake' @(
    '--build', $buildDir,
    '--config', 'Release',
    '--target', 'NOCTIVORE_FX_VST3',
    '--parallel', "$Parallel"
)

$bundles = @(Get-ChildItem -LiteralPath $buildDir -Directory -Recurse -Filter '*.vst3' | Where-Object {
    Test-Path -LiteralPath (Join-Path $_.FullName 'Contents/Resources/Banks/00_36.wav')
})
if ($bundles.Count -ne 1) {
    throw "Expected exactly one bundled VST3 containing Banks, found $($bundles.Count)."
}
$bundle = $bundles[0]
$wavBundled = @(Get-ChildItem -LiteralPath (Join-Path $bundle.FullName 'Contents/Resources/Banks') -File -Filter '*.wav').Count
if ($wavBundled -ne 200) { throw "Compiled VST3 contains only $wavBundled WAVs." }
$native = @(Get-ChildItem -LiteralPath (Join-Path $bundle.FullName 'Contents/x86_64-win') -File -Filter '*.vst3' -ErrorAction SilentlyContinue | Where-Object { $_.Length -gt 100000 })
if ($native.Count -ne 1) { throw 'VST3 binary missing from compiled plugin.' }

New-Item -ItemType Directory -Path $releaseDir -Force | Out-Null
$zipPath = Join-Path $releaseDir ("VOIDCORE-ASTRAL-XENO-$Edition-WIN64-VST3.zip")
Compress-Archive -LiteralPath $bundle.FullName -DestinationPath $zipPath -CompressionLevel Fastest -Force
if (!(Test-Path -LiteralPath $zipPath)) { throw 'Archive creation failed.' }
Write-Host 'SUCCESS: Built Windows x64 VOIDCORE VST3 without GitHub Actions.' -ForegroundColor Green
Write-Host "Downloadable ZIP: $zipPath"
Write-Host 'Install the extracted .vst3 folder into C:\Program Files\Common Files\VST3 and rescan the DAW.'
