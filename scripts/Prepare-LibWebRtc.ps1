# SPDX-License-Identifier: GPL-3.0-only
# Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)
<#
.SYNOPSIS
Prepares the pinned libwebrtc static libraries used by the RLink CMake build.

.DESCRIPTION
Resolves depot_tools (reusing an existing checkout when one is found and
verifying its revision, otherwise cloning the pinned revision) and the WebRTC
revision pinned in BUILDING.md, syncs every DEPS dependency to that revision,
forces the dynamic CRT, writes the GN args and builds the static libraries that
RLinkAPP links against.

The revision is enforced by putting it in the `src` solution URL in .gclient
(`.../src.git@<commit>`), not by a bare `git checkout`: this gclient ignores a
separate `"revision"` key in .gclient, and when the solution URL carries no
revision it tracks `origin/main`, which silently undoes a manual checkout on the
next sync. The sync is performed in a single pass and the resulting HEAD is
verified against the pinned commit.

WebRTC also requires `core.autocrlf=false` (Git for Windows installs with
`core.autocrlf=true` at system scope, which rewrites byte-exact text files) and
long path support. Those are injected for this process tree only through the
GIT_CONFIG_COUNT environment mechanism, so the developer's global git
configuration is left untouched.

The pinned WebRTC revision has no GN argument for the dynamic CRT: its
build/config/win/BUILD.gn default_crt config selects /MT, which cannot link
against RLink (Qt uses /MD). This script therefore patches default_crt to use
:dynamic_crt (=> /MD for Release, /MDd for Debug) before running gn gen. The
patch is idempotent, but it lives in the WebRTC checkout and can be overwritten
by a gclient sync that updates the `build` dependency: re-run this script after
such a sync.

.PARAMETER Root
Directory that contains (or will contain) the WebRTC `src` checkout. Defaults to
the parent of $env:RLINK_WEBRTC_SRC when set.

.PARAMETER DepotTools
depot_tools checkout. Defaults to $env:RLINK_DEPOT_TOOLS, then to the directory
of gclient.bat on PATH. A resolved checkout is reused as-is; its HEAD is compared
against the pinned depot_tools revision and a warning is printed on a mismatch.
When no checkout can be resolved, the pinned revision is cloned instead.

.PARAMETER DepotToolsRoot
Directory into which depot_tools is cloned when no existing checkout can be
resolved. Defaults to the parent of Root plus `depot_tools`, so a single -Root
is enough (-Root E:\webrtc_src clones into E:\depot_tools).

.PARAMETER MsvsPath
Visual Studio installation root used for GYP_MSVS_OVERRIDE_PATH, so WebRTC uses
the same MSVC STL as the CMake/Qt build (see BUILDING.md section 2). It is
validated against the toolset pinned in CMakePresets.json before anything is
fetched or built: an unusable path fails the run instead of producing a
webrtc.lib that cannot link. Defaults to $env:GYP_MSVS_OVERRIDE_PATH, which is
validated the same way and is an error when unusable, then to the installations
of that toolset's Visual Studio release reported by vswhere.

.PARAMETER Configurations
Which GN output trees to build: Release (out\ReleaseMD), Debug (out\DebugMD) or
both. Default: both.

.PARAMETER SkipFetch
Skip the pinned checkout/sync and only re-patch, gn gen and build an existing
checkout. The existing HEAD is reported but not changed. depot_tools is not
cloned either; an existing checkout must already be resolvable.

.PARAMETER SkipBuild
Patch and write args.gn only; do not run gn gen or ninja.

.EXAMPLE
# Reuses depot_tools from PATH, or clones the pinned revision next to -Root.
.\scripts\Prepare-LibWebRtc.ps1 -Root D:\dev\webrtc_src -MsvsPath "C:\Program Files\Microsoft Visual Studio\2022\Community"

.EXAMPLE
.\scripts\Prepare-LibWebRtc.ps1 -SkipFetch -Configurations Release
#>
[CmdletBinding()]
param(
    [string]$Root,
    [string]$DepotTools,
    [string]$DepotToolsRoot,
    [string]$MsvsPath,
    [ValidateSet('Release', 'Debug')]
    [string[]]$Configurations = @('Release', 'Debug'),
    [switch]$SkipFetch,
    [switch]$SkipBuild
)

$ErrorActionPreference = 'Stop'

$WebRtcCommit = '1e2bd46a33bc0a95ff4e032e380f9fcfa2505808'
$WebRtcUrl = 'https://webrtc.googlesource.com/src.git'
$DepotToolsCommit = '3799a497b1e483ab3625b91f9540155e8d311985'
$DepotToolsUrl = 'https://chromium.googlesource.com/chromium/tools/depot_tools.git'

$OutByConfig = @{
    Release = 'out\ReleaseMD'
    Debug   = 'out\DebugMD'
}

# --- toolchain contract -----------------------------------------------------
# WebRTC must be built with the same toolset as the application, which is the one
# CMakePresets.json pins: Get-RequiredMsvcToolset() reads it below instead of the
# script keeping its own copy that could drift. WebRTC's GN build never records
# the toolset in args.gn: build/vs_toolchain.py honours GYP_MSVS_OVERRIDE_PATH,
# build/toolchain/win/setup_toolchain.py runs that installation's vcvarsall.bat,
# and both select the highest VC\Tools\MSVC\14.* of the installation. An install
# whose highest 14.* belongs to another toolset (for example VS 18 / 14.51, which
# is v145) produces objects that reference __std_* helpers the other toolset
# cannot resolve, so it is rejected here instead of when RLinkAPP fails to link
# (BUILDING.md section 2).
#
# Visual Studio release that ships each toolset. Only the toolsets the presets
# may name are listed; it restricts the vswhere fallback, because an unrestricted
# -latest lookup prefers a newer Visual Studio that may not have the toolset at
# all.
$VsReleaseByToolset = @{
    v141 = '15'
    v142 = '16'
    v143 = '17'
    v145 = '18'
}

function Write-Step([string]$Message) {
    Write-Host ("[{0}] {1}" -f (Get-Date -Format 'HH:mm:ss'), $Message)
}

function Invoke-Native([string]$Name, [scriptblock]$Command) {
    Write-Step $Name
    # Native tools (git, gclient, gn, ninja) write progress and warnings to
    # stderr, and with $ErrorActionPreference = 'Stop' PowerShell 5.1 promotes a
    # native command's first stderr line to a terminating error even when the
    # tool succeeded - gclient's "git update is recommended" warning was enough
    # to abort the sync. The exit code checked below is the real verdict.
    $previousPreference = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try {
        & $Command
    }
    finally {
        $ErrorActionPreference = $previousPreference
    }
    if ($LASTEXITCODE -ne 0) {
        throw "$Name failed with exit code $LASTEXITCODE."
    }
}

# VC\Tools\MSVC\14.x directories are just versions: the toolset name the rest of
# the world uses (v140..v145) comes from the tens digit of the minor version.
function Get-MsvcToolsetName([version]$Version) {
    if ($Version.Major -ne 14) {
        return $null
    }
    switch ([int][math]::Floor($Version.Minor / 10)) {
        0 { 'v140' }  # VS 2015
        1 { 'v141' }  # VS 2017
        2 { 'v142' }  # VS 2019
        3 { 'v143' }  # VS 2022, 17.0-17.3
        4 { 'v143' }  # VS 2022, 17.4+
        5 { 'v145' }  # VS 18
        default { $null }
    }
}

# Returns the VC\Tools\MSVC\14.* toolsets of an installation, highest first.
# The ordering mirrors FindVCComponentRoot() in build/vs_toolchain.py, so index 0
# is the toolset WebRTC would actually be compiled with.
function Get-MsvcToolsets([string]$InstallPath) {
    $root = Join-Path $InstallPath 'VC\Tools\MSVC'
    if (-not (Test-Path -LiteralPath $root)) {
        return @()
    }
    $toolsets = @(Get-ChildItem -LiteralPath $root -Directory -ErrorAction SilentlyContinue |
        ForEach-Object {
            if ($_.Name -match '^(\d+)\.(\d+)\.(\d+)') {
                $version = [version]('{0}.{1}.{2}' -f $Matches[1], $Matches[2], $Matches[3])
                [pscustomobject]@{
                    Name    = $_.Name
                    Version = $version
                    Toolset = Get-MsvcToolsetName $version
                    Path    = $_.FullName
                }
            }
        })
    return @($toolsets | Sort-Object -Property Version -Descending)
}

# Returns why an installation cannot build WebRTC for this repository, or $null
# when it can.
function Get-MsvcToolsetProblem([string]$InstallPath, [string]$RequiredToolset) {
    if (-not (Test-Path -LiteralPath $InstallPath)) {
        return "Visual Studio was not found at $InstallPath"
    }
    $toolset = @(Get-MsvcToolsets $InstallPath) | Select-Object -First 1
    if (-not $toolset) {
        return ("$InstallPath has no VC\Tools\MSVC\14.* toolset; install the " +
            "'Desktop development with C++' workload")
    }
    if ($toolset.Toolset -ne $RequiredToolset) {
        $actual = if ($toolset.Toolset) { $toolset.Toolset } else { 'an unrecognized toolset' }
        return ("$InstallPath would build WebRTC with MSVC $($toolset.Name) ($actual), the " +
            "highest toolset it has, but CMakePresets.json pins $RequiredToolset; RLinkAPP " +
            'would fail to link with unresolved __std_* symbols')
    }
    return $null
}

# Installations of the Visual Studio release that ships the required toolset, so
# the caller can validate them. Returns nothing for a toolset with no known
# release, which turns into a clear "pass -MsvsPath" failure instead of silently
# checking unrelated installations.
function Get-VsInstallations([string]$Toolset) {
    $release = $VsReleaseByToolset[$Toolset]
    if (-not $release) {
        return @()
    }
    $programFilesX86 = ${env:ProgramFiles(x86)}
    if (-not $programFilesX86) {
        return @()
    }
    $vswhere = Join-Path $programFilesX86 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path -LiteralPath $vswhere)) {
        return @()
    }
    $range = '[{0}.0,{1}.0)' -f $release, ([int]$release + 1)
    $paths = @(& $vswhere -products * -version $range `
            -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
            -property installationPath)
    return @($paths | Where-Object { $_ -and $_.Trim() } | ForEach-Object { $_.Trim() })
}

# The toolset CMakePresets.json pins, which is what WebRTC has to match. The
# build\CMakeCache.txt of a previous configure is deliberately not consulted:
# WebRTC is prepared before CMake configure runs (BUILDING.md sections 4 and 5),
# so that cache either does not exist yet or describes an older configuration.
function Get-RequiredMsvcToolset([string]$PresetsFile) {
    if (-not (Test-Path -LiteralPath $PresetsFile)) {
        throw "CMakePresets.json not found at $PresetsFile; cannot tell which toolset WebRTC must use."
    }
    $presets = Get-Content -LiteralPath $PresetsFile -Raw | ConvertFrom-Json
    $toolsets = @($presets.configurePresets |
        ForEach-Object { $_.toolset } |
        Where-Object { $_ } |
        ForEach-Object { if ($_ -is [string]) { $_ } else { $_.value } } |
        Where-Object { $_ } |
        Select-Object -Unique)
    if ($toolsets.Count -ne 1) {
        $found = if ($toolsets.Count -eq 0) { 'none' } else { $toolsets -join ', ' }
        throw "Expected exactly one toolset in $PresetsFile, found: $found."
    }
    return $toolsets[0]
}

function Get-WebRtcArgs([bool]$IsDebug) {
    $isDebugValue = if ($IsDebug) { 'true' } else { 'false' }
    # Debug must match Qt's debug DLLs, which use the MSVC default
    # _ITERATOR_DEBUG_LEVEL=2; enabling iterator debugging stops WebRTC from
    # defining _HAS_ITERATOR_DEBUGGING=0 (see build/config/BUILD.gn).
    $iteratorDebugging = $isDebugValue
    return @(
        "is_debug = $isDebugValue",
        "enable_iterator_debugging = $iteratorDebugging",
        'target_cpu = "x64"',
        'rtc_include_tests = false',
        'use_custom_libcxx = false',
        'use_lld = false',
        'proprietary_codecs = true',
        'ffmpeg_branding = "Chrome"'
    )
}

# --- resolve inputs ---------------------------------------------------------
if (-not $Root -and $env:RLINK_WEBRTC_SRC) {
    $Root = Split-Path -Parent $env:RLINK_WEBRTC_SRC
}
if (-not $Root) {
    throw "Specify -Root (the directory that contains the WebRTC 'src' checkout) or set RLINK_WEBRTC_SRC."
}

# depot_tools: an explicit -DepotTools, RLINK_DEPOT_TOOLS or a checkout on PATH
# wins; anything else is cloned into $DepotToolsRoot further down.
if (-not $DepotTools -and $env:RLINK_DEPOT_TOOLS) {
    $DepotTools = $env:RLINK_DEPOT_TOOLS
}
if (-not $DepotTools) {
    $gclient = Get-Command gclient.bat -ErrorAction SilentlyContinue
    if ($gclient) {
        $DepotTools = Split-Path -Parent $gclient.Source
    }
}

$Root = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($Root)
$Src = Join-Path $Root 'src'
$GclientFile = Join-Path $Root '.gclient'

# Where a fresh depot_tools checkout goes when none was resolved. Defaulting to
# the WebRTC root's sibling keeps a single -Root sufficient
# (-Root E:\webrtc_src -> E:\depot_tools).
if (-not $DepotToolsRoot) {
    $DepotToolsRoot = Join-Path (Split-Path -Parent $Root) 'depot_tools'
}
$DepotToolsRoot = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($DepotToolsRoot)
if (-not $DepotTools) {
    $DepotTools = $DepotToolsRoot
}
else {
    $DepotTools = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($DepotTools)
}

# --- git settings WebRTC needs, scoped to this process tree -----------------
# Injected via GIT_CONFIG_COUNT so the user's global git config is not modified.
# (git config --global core.autocrlf false would work too, but it changes every
# repository on the machine.) Applied before any clone below so core.longpaths
# also covers the depot_tools clone.
$gitSettings = @(
    @{ Key = 'core.autocrlf';    Value = 'false' },
    @{ Key = 'core.filemode';    Value = 'false' },
    @{ Key = 'core.fscache';     Value = 'true' },
    @{ Key = 'core.preloadindex'; Value = 'true' },
    @{ Key = 'core.longpaths';   Value = 'true' }
)
$env:GIT_CONFIG_COUNT = [string]$gitSettings.Count
for ($i = 0; $i -lt $gitSettings.Count; $i++) {
    Set-Item -Path ("Env:GIT_CONFIG_KEY_{0}" -f $i) -Value $gitSettings[$i].Key
    Set-Item -Path ("Env:GIT_CONFIG_VALUE_{0}" -f $i) -Value $gitSettings[$i].Value
}

# OS long path support is machine-wide and cannot be changed without elevation.
$longPaths = (Get-ItemProperty -LiteralPath 'HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem' -Name LongPathsEnabled -ErrorAction SilentlyContinue).LongPathsEnabled
if ($longPaths -ne 1) {
    Write-Warning ('Windows long path support (LongPathsEnabled) is off; relying on ' +
        'git core.longpaths=true. Enable it from an elevated shell if a checkout ' +
        'fails with path-too-long errors.')
}

# --- toolchain: pin GYP_MSVS_OVERRIDE_PATH to the pinned toolset -------------
# Resolved before depot_tools so a wrong toolchain fails the run before anything
# is cloned or synced, and before gn gen writes any file. Both -MsvsPath and an
# existing GYP_MSVS_OVERRIDE_PATH are explicit user input and must be usable: a
# stale GYP_MSVS_OVERRIDE_PATH (for example a VS 18 path, whose highest toolset
# is 14.51) is an error, not a warning, because ignoring it would silently
# produce a webrtc.lib that cannot link against the application while the
# variable keeps misleading the manual gn gen/ninja path. vswhere provides
# candidates (the required toolset's Visual Studio release only) when neither is
# set.
$msvcSource = $null
$rejected = @()
$RequiredMsvcToolset = Get-RequiredMsvcToolset (Join-Path $PSScriptRoot '..\CMakePresets.json')

if ($MsvsPath) {
    $msvcSource = '-MsvsPath'
    $problem = Get-MsvcToolsetProblem $MsvsPath $RequiredMsvcToolset
    if ($problem) {
        throw "$problem. Pass -MsvsPath with a $RequiredMsvcToolset installation."
    }
}
elseif ($env:GYP_MSVS_OVERRIDE_PATH) {
    $problem = Get-MsvcToolsetProblem $env:GYP_MSVS_OVERRIDE_PATH $RequiredMsvcToolset
    if ($problem) {
        $usable = @(foreach ($candidate in @(Get-VsInstallations $RequiredMsvcToolset)) {
                if (-not (Get-MsvcToolsetProblem $candidate $RequiredMsvcToolset)) { $candidate }
            })
        $suggestion = if ($usable.Count -gt 0) {
            ' A usable installation was detected at: ' + ($usable -join ', ') + '.'
        }
        else {
            ''
        }
        throw ("GYP_MSVS_OVERRIDE_PATH is set to $($env:GYP_MSVS_OVERRIDE_PATH) but unusable: " +
            "$problem.$suggestion Pass -MsvsPath with a usable installation, or clear " +
            'the variable.')
    }
    $MsvsPath = $env:GYP_MSVS_OVERRIDE_PATH
    $msvcSource = 'GYP_MSVS_OVERRIDE_PATH'
}

if (-not $MsvsPath) {
    foreach ($candidate in @(Get-VsInstallations $RequiredMsvcToolset)) {
        $problem = Get-MsvcToolsetProblem $candidate $RequiredMsvcToolset
        if ($problem) {
            $rejected += $problem
            continue
        }
        $MsvsPath = $candidate
        $msvcSource = 'vswhere'
        break
    }
}

if (-not $MsvsPath) {
    $detail = if ($rejected.Count -gt 0) {
        "Rejected: $($rejected -join '; ')."
    }
    else {
        "No installation of the $RequiredMsvcToolset Visual Studio release was detected."
    }
    throw ("A Visual Studio installation with the $RequiredMsvcToolset toolset, which " +
        "CMakePresets.json pins, is required to build WebRTC (BUILDING.md section 2). " +
        "$detail Pass -MsvsPath or set GYP_MSVS_OVERRIDE_PATH to one and re-run.")
}

$env:GYP_MSVS_OVERRIDE_PATH = $MsvsPath
$msvcToolset = @(Get-MsvcToolsets $MsvsPath) | Select-Object -First 1

# --- depot_tools: reuse and verify, or clone the pinned revision ------------
if (Test-Path -LiteralPath (Join-Path $DepotTools '.git')) {
    $depotHead = (& git -C $DepotTools rev-parse HEAD).Trim()
    if ($depotHead -ne $DepotToolsCommit) {
        Write-Warning ("depot_tools at $DepotTools is at $depotHead, not the pinned " +
            "$DepotToolsCommit; using it as-is. Check it out manually if the build misbehaves.")
    }
    else {
        Write-Step "depot_tools at $DepotTools is the pinned revision."
    }
}
elseif (Test-Path -LiteralPath $DepotTools) {
    Write-Warning "depot_tools at $DepotTools has no .git; the pinned revision cannot be verified."
}
elseif ($SkipFetch) {
    throw "depot_tools not found at $DepotTools and -SkipFetch was given. Pass -DepotTools or set RLINK_DEPOT_TOOLS."
}
else {
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $DepotTools) | Out-Null
    Write-Step "Cloning depot_tools@$DepotToolsCommit into $DepotTools"
    Invoke-Native "clone depot_tools@$DepotToolsCommit" {
        & git clone $DepotToolsUrl $DepotTools
    }
    Invoke-Native "check out depot_tools@$DepotToolsCommit" {
        & git -C $DepotTools checkout --detach $DepotToolsCommit
    }
    $depotHead = (& git -C $DepotTools rev-parse HEAD).Trim()
    if ($depotHead -ne $DepotToolsCommit) {
        throw "depot_tools checkout is at $depotHead, expected $DepotToolsCommit."
    }
    Write-Step "depot_tools src pinned at $depotHead"
}

if (-not (Test-Path -LiteralPath (Join-Path $DepotTools 'gclient.bat'))) {
    throw "depot_tools not found at $DepotTools (no gclient.bat). Pass -DepotTools or remove the directory and re-run."
}

# depot_tools must be first on PATH and must not update itself off the pinned revision.
$env:PATH = "$DepotTools;$env:PATH"
$env:DEPOT_TOOLS_UPDATE = '0'
$env:DEPOT_TOOLS_WIN_TOOLCHAIN = '0'

Write-Step "WebRTC src : $Src"
Write-Step "depot_tools: $DepotTools"
Write-Step "MSVC       : $MsvsPath"
Write-Step "MSVC toolset: $($msvcToolset.Name) ($($msvcToolset.Toolset), via $msvcSource)"
Write-Step "configs    : $($Configurations -join ', ')"

# --- bootstrap the depot_tools wrappers if necessary ------------------------
if (-not (Test-Path -LiteralPath (Join-Path $DepotTools 'git.bat'))) {
    Invoke-Native 'bootstrap depot_tools (win_tools.bat)' {
        & (Join-Path $DepotTools 'bootstrap\win_tools.bat')
    }
}

# --- pin the `src` solution and sync it in one pass -------------------------
if (-not $SkipFetch) {
    New-Item -ItemType Directory -Force -Path $Root | Out-Null

    $pinnedUrl = "$WebRtcUrl@$WebRtcCommit"
    if (-not (Test-Path -LiteralPath $GclientFile)) {
        Write-Step "Writing pinned .gclient ($pinnedUrl)"
        $spec = (@(
                'solutions = [',
                '  {',
                '    "name": "src",',
                ('    "url": "{0}",' -f $pinnedUrl),
                '    "deps_file": "DEPS",',
                '    "custom_deps": {},',
                '  },',
                ']'
            ) -join "`r`n") + "`r`n"
        [System.IO.File]::WriteAllText($GclientFile, $spec, (New-Object System.Text.UTF8Encoding($false)))
    }
    else {
        # Reuse an existing .gclient (for example one written by `fetch`), but
        # force the revision into the src solution URL so the sync cannot drift
        # back to origin/main. Only the URL line is touched; other keys stay.
        $content = [System.IO.File]::ReadAllText($GclientFile)
        $pattern = '"url"\s*:\s*"' + [regex]::Escape($WebRtcUrl) + '(?:@[^"]*)?"'
        if ($content -match $pattern) {
            $updated = [regex]::Replace($content, $pattern, ('"url": "{0}"' -f $pinnedUrl))
            if ($updated -ne $content) {
                [System.IO.File]::WriteAllText($GclientFile, $updated, (New-Object System.Text.UTF8Encoding($false)))
                Write-Step "Pinned the src revision in $GclientFile"
            }
        }
        else {
            Write-Warning "Could not find the WebRTC src URL in $GclientFile; relying on --revision for this sync."
        }
    }

    # gclient refuses to sync a DEPS-managed repository that has uncommitted
    # changes, and the /MD patch this script applies to src\build is exactly
    # that: the patch step below runs after the sync, so a previous run leaves
    # src\build dirty and every later run would fail here. Restore that one file
    # before syncing; the patch is re-applied afterwards.
    $buildRepo = Join-Path $Src 'build'
    $buildGnRelative = 'config/win/BUILD.gn'
    if (Test-Path -LiteralPath (Join-Path $buildRepo '.git')) {
        $dirty = @(& git -C $buildRepo status --porcelain -- $buildGnRelative)
        if ($dirty) {
            Write-Step 'Reverting the local /MD patch in src\build before gclient sync'
            Invoke-Native 'restore src/build/config/win/BUILD.gn' {
                & git -C $buildRepo checkout -- $buildGnRelative
            }
        }
    }

    # One pass: gclient clones/checks out src@<pin> and syncs the DEPS of that
    # exact revision. --revision is kept as a second lock in case an existing
    # .gclient could not be rewritten above.
    Invoke-Native "gclient sync -D (src@$WebRtcCommit)" {
        Push-Location $Root
        try {
            & gclient sync -D --with_branch_heads --revision "src@$WebRtcCommit"
        }
        finally { Pop-Location }
    }

    if (-not (Test-Path -LiteralPath (Join-Path $Src '.git'))) {
        throw "gclient sync finished but $Src\.git does not exist."
    }
    $head = (& git -C $Src rev-parse HEAD).Trim()
    if ($head -ne $WebRtcCommit) {
        throw "WebRTC checkout is at $head, expected $WebRtcCommit. The pinned revision was not applied."
    }
    Write-Step "WebRTC src pinned at $head"
}
else {
    Write-Step 'Skipping checkout/sync (-SkipFetch).'
    if (Test-Path -LiteralPath (Join-Path $Src '.git')) {
        $head = (& git -C $Src rev-parse HEAD).Trim()
        if ($head -ne $WebRtcCommit) {
            Write-Warning "Existing WebRTC checkout is at $head, not the pinned $WebRtcCommit."
        }
        else {
            Write-Step "Existing WebRTC checkout is at the pinned revision."
        }
    }
}

# --- force the dynamic CRT (/MD or /MDd) ------------------------------------
$buildGn = Join-Path $Src 'build\config\win\BUILD.gn'
if (-not (Test-Path -LiteralPath $buildGn)) {
    throw "Not found: $buildGn. Run without -SkipFetch first (see BUILDING.md section 4)."
}

$content = [System.IO.File]::ReadAllText($buildGn)
$staticPattern = '# Desktop Windows: static CRT\.\s*configs = \[ ":static_crt" \]'

if ($content -match '# Desktop Windows: dynamic CRT') {
    Write-Step 'default_crt already uses the dynamic CRT; skipping patch.'
}
elseif ($content -match $staticPattern) {
    $nl = if ($content.Contains("`r`n")) { "`r`n" } else { "`n" }
    $replacement = '# Desktop Windows: dynamic CRT (/MD; /MDd when is_debug = true) to match' + $nl +
                   '      # the Qt/CMake RLink build.' + $nl +
                   '      configs = [ ":dynamic_crt" ]'
    $patched = [regex]::Replace($content, $staticPattern, $replacement, 1)
    [System.IO.File]::WriteAllText($buildGn, $patched)
    Write-Step "Patched default_crt to :dynamic_crt in $buildGn"
}
else {
    throw "Could not find the default_crt desktop branch in $buildGn. Apply the /MD edit manually (BUILDING.md section 4)."
}

# --- write args.gn and build each requested configuration -------------------
$ninjaExe = Join-Path $Src 'third_party\ninja\ninja.exe'

foreach ($config in $Configurations) {
    $isDebug = ($config -eq 'Debug')
    $outRel = $OutByConfig[$config]
    $outDir = Join-Path $Src $outRel
    New-Item -ItemType Directory -Force -Path $outDir | Out-Null

    $argsGnPath = Join-Path $outDir 'args.gn'
    $argsText = ((Get-WebRtcArgs $isDebug) -join "`r`n") + "`r`n"
    [System.IO.File]::WriteAllText($argsGnPath, $argsText)
    Write-Step "Wrote $argsGnPath"

    if ($SkipBuild) {
        continue
    }

    Invoke-Native "gn gen $outRel" {
        Push-Location $Src
        try { & gn gen $outRel } finally { Pop-Location }
    }

    $buildTargets = @(
        'webrtc',
        'builtin_video_decoder_factory',
        'builtin_video_encoder_factory',
        'api/video:adapted_video_track_source'
    )

    Invoke-Native "build $config ($outRel)" {
        Push-Location $Src
        try {
            if (Test-Path -LiteralPath $ninjaExe) {
                & $ninjaExe -C $outRel @buildTargets
            }
            else {
                & autoninja -C $outRel @buildTargets
            }
        }
        finally { Pop-Location }
    }

    $expected = @(
        'obj\webrtc.lib',
        'obj\api\video\adapted_video_track_source.lib',
        'obj\api\video_codecs\builtin_video_decoder_factory.lib',
        'obj\api\video_codecs\builtin_video_encoder_factory.lib',
        'obj\api\video_codecs\rtc_software_fallback_wrappers.lib',
        'obj\media\rtc_internal_video_codecs.lib',
        'obj\media\rtc_simulcast_encoder_adapter.lib',
        'gen'
    )
    $missing = @($expected | Where-Object { -not (Test-Path -LiteralPath (Join-Path $outDir $_)) })
    if ($missing.Count -gt 0) {
        throw "Build finished but these outputs are missing under ${outDir}: $($missing -join ', ')"
    }
    Write-Step "$config build outputs verified."
}

Write-Step 'Done.'
