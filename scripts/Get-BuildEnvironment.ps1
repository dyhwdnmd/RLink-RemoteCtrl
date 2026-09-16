# SPDX-License-Identifier: GPL-3.0-only
# Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)
<#
.SYNOPSIS
Prints the build-environment summary used to validate a CMake build.

.DESCRIPTION
Collects the OS, Visual Studio/MSVC, Windows SDK, CMake, Qt and libwebrtc
versions and prints them in the report format the upstream author asks for:

    - OS:        Microsoft Windows 10 IoT Enterprise LTSC 21H2 (10.0.19044)
    - VS:        Visual Studio Community 2022 17.14.20 (MSVC toolset 14.44.35207 (cl 19.44.35220), v143)
    - Win SDK:   10.0.26100.0 (target)
    - CMake:     3.31.8  (project minimum: 3.24)
    - Qt:        6.11.2 msvc2022_64
    - WebRTC:    libwebrtc @ 1e2bd46a33bc0a95ff4e032e380f9fcfa2505808 (2026-07-13)

Everything is detected from the machine itself: the registry for the OS,
vswhere plus cl.exe for the toolchain, cmake on PATH, and the repo's
CMakePresets.json for the project minimum. Qt and WebRTC are read from the
configured build (build\CMakeCache.txt) first and from the documented
environment variables (RLINK_QT_DIR, RLINK_WEBRTC_SRC) otherwise, so the script
also works before the first configure as long as those variables are set.

.PARAMETER BuildDir
CMake binary directory to inspect. Defaults to <repo>\build.

.PARAMETER NoIndent
Print the lines without the leading four-space bullet indent, for pasting into
plain text instead of a Markdown list.

.EXAMPLE
.\scripts\Get-BuildEnvironment.ps1

.EXAMPLE
.\scripts\Get-BuildEnvironment.ps1 -NoIndent
#>
[CmdletBinding()]
param(
    [string]$BuildDir,
    [switch]$NoIndent
)

$ErrorActionPreference = 'Stop'

$RepoRoot = Split-Path -Parent $PSScriptRoot
if (-not $BuildDir) {
    $BuildDir = Join-Path $RepoRoot 'build'
}

# --- helpers ----------------------------------------------------------------
function Get-CacheValue([string]$Path, [string]$Name) {
    if (-not (Test-Path -LiteralPath $Path)) { return $null }
    $line = Select-String -LiteralPath $Path -Pattern ("^{0}:[^=]*=(.*)$" -f [regex]::Escape($Name)) |
        Select-Object -First 1
    if ($line) { return $line.Matches[0].Groups[1].Value.Trim() }
    return $null
}

function Get-EnvOrCache([string]$Name, [string]$CachePath) {
    $value = Get-CacheValue $CachePath $Name
    if ($value) { return $value }
    return [Environment]::GetEnvironmentVariable($Name)
}

function Get-CurrentVersionRegistry {
    Get-ItemProperty -LiteralPath 'HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion' -ErrorAction SilentlyContinue
}

# --- OS ---------------------------------------------------------------------
function Get-OSDescription {
    $key = Get-CurrentVersionRegistry
    $caption = (Get-CimInstance Win32_OperatingSystem -ErrorAction SilentlyContinue).Caption
    if (-not $caption) { $caption = $key.ProductName }

    # Windows 11 reports ProductName "Windows 10 ..." from a stale registry
    # string, so the caption from WMI is preferred and only corrected when the
    # build number clearly says 11.
    $build = [int]$key.CurrentBuild
    if ($build -ge 22000 -and $caption -match '^Microsoft Windows 10') {
        $caption = $caption -replace '^Microsoft Windows 10', 'Microsoft Windows 11'
    }

    $release = $key.DisplayVersion
    if (-not $release -and $build -ge 22000) { $release = '22H2' }

    $parts = @($caption)
    if ($release) { $parts += $release }
    $version = '10.0.{0}' -f $build
    return ('{0} ({1})' -f ($parts -join ' '), $version)
}

# --- Visual Studio / MSVC ---------------------------------------------------
function Get-VSInfo {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path -LiteralPath $vswhere)) { return $null }

    $json = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -format json
    if ($LASTEXITCODE -ne 0 -or -not $json) { return $null }
    $instance = @($json | ConvertFrom-Json) | Select-Object -First 1
    if (-not $instance) { return $null }

    $install = $instance.resolvedInstallationPath
    if (-not $install) { $install = $instance.installationPath }

    # Newest installed MSVC toolset under the VS instance.
    $msvcRoot = Join-Path $install 'VC\Tools\MSVC'
    $toolset = $null
    if (Test-Path -LiteralPath $msvcRoot) {
        $toolset = Get-ChildItem -LiteralPath $msvcRoot -Directory -ErrorAction SilentlyContinue |
            Sort-Object { [version]$_.Name } -Descending | Select-Object -First 1
    }

    $cl = $null
    if ($toolset) {
        $clExe = Join-Path $toolset.FullName 'bin\Hostx64\x64\cl.exe'
        if (Test-Path -LiteralPath $clExe) {
            # cl.exe prints its banner on stderr; 2>&1 turns that into an
            # ErrorRecord, which a Stop preference would treat as terminating.
            $previous = $ErrorActionPreference
            $ErrorActionPreference = 'Continue'
            try { $banner = & $clExe 2>&1 | Out-String }
            finally { $ErrorActionPreference = $previous }
            if ($banner -match '(\d+\.\d+\.\d+)') { $cl = $Matches[1] } else { $cl = $null }
        }
    }

    $display = $instance.displayName
    if (-not $display -and $instance.catalog) { $display = 'Visual Studio ' + $instance.catalog.productLineVersion }

    $version = $instance.catalog.productDisplayVersion
    if (-not $version) { $version = $instance.installationVersion }

    return [pscustomobject]@{
        Display  = $display
        Version  = $version
        Toolset  = $(if ($toolset) { $toolset.Name } else { $null })
        Compiler = $cl
    }
}

function Get-ToolsetName([string]$VSVersion, [string]$CachePath) {
    $fromCache = Get-CacheValue $CachePath 'CMAKE_GENERATOR_TOOLSET'
    if ($fromCache) { return $fromCache }
    if ($VSVersion -match '^(\d+)\.') {
        switch ($Matches[1]) {
            '17' { return 'v143' }
            '16' { return 'v142' }
            '15' { return 'v141' }
        }
    }
    return $null
}

# --- Windows SDK ------------------------------------------------------------
function Get-WindowsSdk([string]$CachePath) {
    # The SDK the CMake build actually targets lives in the generated project
    # files; fall back to the newest SDK installed on the machine.
    $vcxproj = Get-ChildItem -LiteralPath $BuildDir -Filter *.vcxproj -Recurse -ErrorAction SilentlyContinue |
        Select-Object -First 1
    if ($vcxproj) {
        $match = Select-String -LiteralPath $vcxproj.FullName -Pattern '<WindowsTargetPlatformVersion>([^<]+)<' |
            Select-Object -First 1
        if ($match) { return $match.Matches[0].Groups[1].Value.Trim() }
    }

    $includeRoot = Join-Path ${env:ProgramFiles(x86)} 'Windows Kits\10\Include'
    if (Test-Path -LiteralPath $includeRoot) {
        $sdk = Get-ChildItem -LiteralPath $includeRoot -Directory -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -match '^10\.\d+\.\d+\.\d+$' } |
            Sort-Object { [version]$_.Name } -Descending | Select-Object -First 1
        if ($sdk) { return $sdk.Name }
    }
    return $null
}

# --- CMake ------------------------------------------------------------------
function Get-CMakeVersion {
    $cmake = Get-Command cmake -ErrorAction SilentlyContinue
    if (-not $cmake) { return $null }
    if ((& cmake --version 2>&1 | Out-String) -match 'cmake version\s+(\S+)') { return $Matches[1] }
    return $null
}

function Get-CMakeMinimum {
    $presets = Join-Path $RepoRoot 'CMakePresets.json'
    if (Test-Path -LiteralPath $presets) {
        $json = Get-Content -LiteralPath $presets -Raw | ConvertFrom-Json
        $min = $json.cmakeMinimumRequired
        if ($min -and $min.major) {
            if ($min.patch) { return ('{0}.{1}.{2}' -f $min.major, $min.minor, $min.patch) }
            return ('{0}.{1}' -f $min.major, $min.minor)
        }
    }
    $list = Join-Path $RepoRoot 'CMakeLists.txt'
    if (Test-Path -LiteralPath $list) {
        $match = Select-String -LiteralPath $list -Pattern 'cmake_minimum_required\s*\(\s*VERSION\s+([0-9.]+)' |
            Select-Object -First 1
        if ($match) { return $match.Matches[0].Groups[1].Value.Trim('.') }
    }
    return $null
}

# --- Qt ---------------------------------------------------------------------
function Get-QtInfo {
    $cache = Join-Path $BuildDir 'CMakeCache.txt'
    $dir = Get-EnvOrCache 'RLINK_QT_DIR' $cache
    if (-not $dir) { return $null }
    $dir = $dir.TrimEnd('\', '/')

    # <prefix>\<version>\<kit>, e.g. D:\dev\Libs\Qt6\6.11.2\msvc2022_64.
    $kit = Split-Path -Leaf $dir
    $versionDir = Split-Path -Leaf (Split-Path -Parent $dir)

    $version = $versionDir
    $qmake = Join-Path $dir 'bin\qmake.exe'
    if (Test-Path -LiteralPath $qmake) {
        $query = (& $qmake -query QT_VERSION 2>$null | Select-Object -First 1)
        if ($query -and $query -match '^\d+\.\d+\.\d+') { $version = $query.Trim() }
    }
    return ('{0} {1}' -f $version, $kit)
}

# --- WebRTC -----------------------------------------------------------------
function Get-WebRtcInfo {
    $cache = Join-Path $BuildDir 'CMakeCache.txt'
    $src = Get-EnvOrCache 'RLINK_WEBRTC_SRC' $cache
    if (-not $src -or -not (Test-Path -LiteralPath (Join-Path $src '.git'))) { return $null }

    $commit = (& git -C $src rev-parse HEAD 2>$null | Select-Object -First 1)
    if (-not $commit) { return $null }
    $commit = $commit.Trim()
    $date = (& git -C $src show -s --format=%ad --date=short HEAD 2>$null | Select-Object -First 1)
    if ($date) { $date = $date.Trim() }

    $suffix = if ($date) { ' ({0})' -f $date } else { '' }
    return ('libwebrtc @ {0}{1}' -f $commit, $suffix)
}

# --- report -----------------------------------------------------------------
function Format-Line([string]$Label, [string]$Value) {
    if (-not $Value) { $Value = 'not detected' }
    $prefix = if ($NoIndent) { '' } else { '    - ' }
    return ('{0}{1}{2}' -f $prefix, $Label.PadRight(10), $Value)
}

$cache = Join-Path $BuildDir 'CMakeCache.txt'
$vs = Get-VSInfo

$vsText = $null
if ($vs) {
    $vsText = ('{0} {1}' -f $vs.Display, $vs.Version)
    $toolset = Get-ToolsetName $vs.Version $cache
    $detail = @()
    if ($vs.Toolset) {
        if ($vs.Compiler) { $detail += ('MSVC toolset {0} (cl {1})' -f $vs.Toolset, $vs.Compiler) }
        else { $detail += ('MSVC toolset {0}' -f $vs.Toolset) }
    }
    if ($toolset) { $detail += $toolset }
    if ($detail.Count -gt 0) { $vsText += (' ({0})' -f ($detail -join ', ')) }
}

$cmakeText = Get-CMakeVersion
$cmakeMin = Get-CMakeMinimum
if ($cmakeText) {
    if ($cmakeMin) { $cmakeText = ('{0}  (project minimum: {1})' -f $cmakeText, $cmakeMin) }
}
elseif ($cmakeMin) {
    $cmakeText = ('not detected (project minimum: {0})' -f $cmakeMin)
}

$sdk = Get-WindowsSdk $cache
if ($sdk) { $sdk = '{0} (target)' -f $sdk }

$lines = @(
    (Format-Line 'OS:' (Get-OSDescription))
    (Format-Line 'VS:' $vsText)
    (Format-Line 'Win SDK:' $sdk)
    (Format-Line 'CMake:' $cmakeText)
    (Format-Line 'Qt:' (Get-QtInfo))
    (Format-Line 'WebRTC:' (Get-WebRtcInfo))
)

$lines | Write-Output
