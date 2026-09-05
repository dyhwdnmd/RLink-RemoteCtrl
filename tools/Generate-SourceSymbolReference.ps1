[CmdletBinding()]
param(
    [string]$RepositoryRoot = "",
    [string]$OutputDirectory = "源码导航"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = Split-Path -Parent $PSScriptRoot
}
$RepositoryRoot = [System.IO.Path]::GetFullPath($RepositoryRoot)

$sourceRoot = Join-Path $RepositoryRoot "src"
$outputRoot = Join-Path $RepositoryRoot $OutputDirectory

if (-not (Test-Path -LiteralPath $sourceRoot -PathType Container)) {
    throw "Source directory does not exist: $sourceRoot"
}

New-Item -ItemType Directory -Force -Path $outputRoot | Out-Null

# These files are deliberately included in the middle of one function body.
# Parsing them as standalone C++ would turn ordinary calls and local variables
# into fake declarations. Their owning function is indexed from the fragment
# that contains the real function signature.
$bodyOnlyFragments = [System.Collections.Generic.HashSet[string]]::new(
    [System.StringComparer]::OrdinalIgnoreCase)
@(
    'src/apps/controller/ControllerMainWindow.DiagnosticsPeers.inc',
    'src/apps/controller/ControllerMainWindow.DiagnosticsOutbound.inc',
    'src/apps/controller/ControllerMainWindow.DiagnosticsInbound.inc',
    'src/apps/controller/ControllerMainWindow.DiagnosticsPublish.inc',
    'src/apps/controller/ControllerMainWindow.LocalDeviceV2Patch.inc',
    'src/apps/controller/ControllerMainWindow.SnapshotConnectivity.inc',
    'src/apps/controller/ControllerMainWindow.SnapshotLocal.inc',
    'src/apps/controller/ControllerMainWindow.SnapshotDebug.inc',
    'src/apps/controller/ControllerMainWindow.SnapshotCopy.inc',
    'src/apps/controller/ControllerMainWindow.SnapshotWindows.inc',
    'src/apps/controller/ControllerMainWindow.UiDeviceRecent.inc',
    'src/apps/controller/ControllerMainWindow.UiDiagnostics.inc',
    'src/apps/controller/ControllerMainWindow.UiOwnedDevices.inc',
    'src/apps/controller/ControllerMainWindow.UiSettings.inc',
    'src/apps/controller/ControllerMainWindow.UiConnections.inc',
    'src/apps/remote/InProcessSessionEngine.RoomPairDispatchPreamble.inc',
    'src/apps/remote/InProcessSessionEngine.RoomPairControlDispatch.inc',
    'src/apps/remote/InProcessSessionEngine.RoomPairScreenDispatch.inc',
    'src/apps/remote/InProcessSessionEngine.RoomPairInputDispatch.inc'
) | ForEach-Object { [void]$bodyOnlyFragments.Add($_) }

function Normalize-Text {
    param([string]$Text)

    if ([string]::IsNullOrWhiteSpace($Text)) {
        return ""
    }
    return (($Text -replace "\s+", " ").Trim())
}

function Escape-MarkdownCell {
    param([string]$Text)

    $value = Normalize-Text $Text
    $value = $value.Replace("|", "\|")
    $value = $value.Replace("`r", " ").Replace("`n", " ")
    return $value
}

function Write-Utf8LfFile {
    param(
        [string]$Path,
        [string]$Content
    )

    $normalized = $Content.Replace("`r`n", "`n").Replace("`r", "`n")
    [System.IO.File]::WriteAllText(
        $Path, $normalized,
        [System.Text.UTF8Encoding]::new($false))
}

function Get-LeadingComment {
    param(
        [string[]]$Lines,
        [int]$LineIndex
    )

    $comments = [System.Collections.Generic.List[string]]::new()
    $index = $LineIndex - 1
    while ($index -ge 0 -and $comments.Count -lt 4) {
        $line = $Lines[$index].Trim()
        if ($line -match '^//[/!]?\s*(.*)$') {
            $comments.Insert(0, $Matches[1].Trim())
            $index--
            continue
        }
        if ([string]::IsNullOrWhiteSpace($line)) {
            if ($comments.Count -eq 0) {
                $index--
                continue
            }
        }
        break
    }

    $comment = Normalize-Text ($comments -join " ")
    if ($comment.Length -gt 180) {
        $comment = $comment.Substring(0, 177) + "..."
    }
    return $comment
}

function Convert-IdentifierToWords {
    param([string]$Name)

    $leaf = ($Name -split '::')[-1]
    $leaf = $leaf.TrimStart('~').TrimEnd('_')
    $leaf = $leaf -creplace '^k(?=[A-Z])', ''
    $leaf = $leaf -creplace '([a-z0-9])([A-Z])', '$1 $2'
    $leaf = $leaf -creplace '([A-Z]+)([A-Z][a-z])', '$1 $2'
    $leaf = $leaf -replace '_+', ' '
    return (Normalize-Text $leaf).ToLowerInvariant()
}

function Get-FunctionPurpose {
    param(
        [string]$Name,
        [string]$Comment
    )

    if (-not [string]::IsNullOrWhiteSpace($Comment)) {
        return $Comment
    }

    $leaf = ($Name -split '::')[-1]
    $owner = if ($Name.Contains('::')) {
        ($Name -split '::')[-2]
    } else {
        ""
    }
    $subject = Convert-IdentifierToWords $leaf

    if (-not [string]::IsNullOrWhiteSpace($owner) -and
        $leaf -eq $owner) {
        return "构造并初始化 $owner 实例。"
    }
    if ($leaf.StartsWith('~')) {
        $destroyedType = $leaf.TrimStart('~')
        return "停止相关活动并释放 $destroyedType 实例拥有的资源。"
    }

    $rules = @(
        @{ Pattern = '^(Is|Has|Can|Should|Needs|Supports|StartsWith|EndsWith|Same|Valid|Equals)'; Text = '判断' },
        @{ Pattern = '^(Validate|Verify|Check)'; Text = '校验' },
        @{ Pattern = '^(Encode|Serialize|Pack)'; Text = '编码' },
        @{ Pattern = '^(Decode|Deserialize|Unpack|Parse)'; Text = '解码或解析' },
        @{ Pattern = '^(Load|Read|Restore)'; Text = '读取或恢复' },
        @{ Pattern = '^(Save|Write|Persist|Store)'; Text = '保存或写入' },
        @{ Pattern = '^(Create|Make|Build|Construct|Initialize|Init)'; Text = '创建或初始化' },
        @{ Pattern = '^(Destroy|Shutdown|Close|Dispose)'; Text = '关闭并清理' },
        @{ Pattern = '^(Start|Begin|Launch|Open)'; Text = '启动' },
        @{ Pattern = '^(Stop|End|Finish|Terminate)'; Text = '停止' },
        @{ Pattern = '^(Connect)'; Text = '建立连接' },
        @{ Pattern = '^(Disconnect)'; Text = '断开连接' },
        @{ Pattern = '^(Request|Query)'; Text = '发起请求或查询' },
        @{ Pattern = '^(Respond|Reply|Acknowledge|Accept|Reject)'; Text = '处理并回复' },
        @{ Pattern = '^(Send|Transmit|Relay|Forward|Publish|Emit)'; Text = '发送或发布' },
        @{ Pattern = '^(Receive|On|Handle|Process|Dispatch)'; Text = '接收并处理' },
        @{ Pattern = '^(Apply|Update|Set|Configure|Retune|Rebuild)'; Text = '更新或应用' },
        @{ Pattern = '^(Get|Find|Lookup|Resolve|Select|Snapshot|Diagnostics|Capabilities)'; Text = '查询并返回' },
        @{ Pattern = '^(Capture|Acquire|Grab)'; Text = '采集' },
        @{ Pattern = '^(Encode)'; Text = '编码' },
        @{ Pattern = '^(Decode)'; Text = '解码' },
        @{ Pattern = '^(Render|Draw|Paint|Present|Upload)'; Text = '准备或呈现' },
        @{ Pattern = '^(Convert|Transform|Scale|Crop)'; Text = '转换或缩放' },
        @{ Pattern = '^(Refresh|Reload)'; Text = '刷新' },
        @{ Pattern = '^(Release|Cancel|Abort)'; Text = '释放或取消' },
        @{ Pattern = '^(Reset|Clear|Erase|Remove)'; Text = '重置或移除' },
        @{ Pattern = '^(Run|Loop|Maintain|Poll|Tick|Schedule)'; Text = '执行后台循环或调度' },
        @{ Pattern = '^(Compute|Calculate|Estimate|Measure|Collect)'; Text = '计算或采集' },
        @{ Pattern = '^(Notify|Report)'; Text = '通知或报告' }
    )

    foreach ($rule in $rules) {
        if ($leaf -match $rule.Pattern) {
            return "$($rule.Text) $subject 相关逻辑。"
        }
    }
    return "实现 $subject 对应的业务或工具逻辑。"
}

function Get-VariablePurpose {
    param(
        [string]$Name,
        [string]$Comment
    )

    if (-not [string]::IsNullOrWhiteSpace($Comment)) {
        return $Comment
    }

    $subject = Convert-IdentifierToWords $Name
    $plainName = $Name.TrimEnd('_')
    if ($plainName -cmatch '^k[A-Z]') {
        return "定义 $subject 的编译期常量或产品边界。"
    }

    $rules = @(
        @{ Pattern = '(Id|ID)$'; Text = '保存身份或作用域标识' },
        @{ Pattern = '(State|Status|Phase)$'; Text = '保存状态机当前状态' },
        @{ Pattern = '(Generation|Epoch|Revision)$'; Text = '标记当前世代，用于拒绝过期异步结果' },
        @{ Pattern = '(Sequence|Seq)$'; Text = '保存单调序号，用于排序或去重' },
        @{ Pattern = '(Mutex|Lock)$'; Text = '保护跨线程共享状态' },
        @{ Pattern = '(Thread|Worker)$'; Text = '拥有后台执行线程或工作器' },
        @{ Pattern = '(Timer|Deadline|Timeout)$'; Text = '保存定时、截止或超时状态' },
        @{ Pattern = '(Callback|Handler|Observer|Sink)$'; Text = '保存回调或观察者入口' },
        @{ Pattern = '(Queue|Tasks|Pending)$'; Text = '保存待处理队列或请求' },
        @{ Pattern = '(Buffer|Frame|Image|Texture)$'; Text = '保存媒体帧、图像或缓冲资源' },
        @{ Pattern = '(Error|Failure|Warning)$'; Text = '保存最近错误或失败原因' },
        @{ Pattern = '(Snapshot)$'; Text = '保存可跨层读取的状态快照' },
        @{ Pattern = '(Enabled|Available|Ready|Active|Open|Valid)$'; Text = '保存能力或开关状态' },
        @{ Pattern = '(Count|Bytes|Size|Width|Height|Rate|Fps|FPS|Bps)$'; Text = '保存计数、尺寸或速率指标' },
        @{ Pattern = '(Path|Directory|Url|URL|Name|Label)$'; Text = '保存路径、地址或显示名称' }
    )

    foreach ($rule in $rules) {
        if ($plainName -match $rule.Pattern) {
            return "$($rule.Text)：$subject。"
        }
    }
    return "保存 $subject 相关配置或运行状态。"
}

function Get-FilePurpose {
    param([string]$RelativePath)

    $fileName = [System.IO.Path]::GetFileName($RelativePath)
    $baseName = [System.IO.Path]::GetFileNameWithoutExtension($fileName)
    $extension = [System.IO.Path]::GetExtension($fileName).ToLowerInvariant()
    $topic = Convert-IdentifierToWords ($baseName -replace '\.', ' ')

    if ($bodyOnlyFragments.Contains($RelativePath.Replace('\', '/'))) {
        return '所属主函数的连续实现片段；这里只包含该函数的局部流程，不单独声明成员变量或顶层函数。'
    }

    if ($extension -eq '.inc') {
        $parts = $baseName -split '\.'
        $owner = $parts[0]
        $slice = if ($parts.Count -gt 1) {
            Convert-IdentifierToWords (($parts[1..($parts.Count - 1)]) -join ' ')
        } else {
            $topic
        }
        return "``$owner`` 的实现切片，集中实现 $slice 相关逻辑；成员状态仍定义在所属头文件中。"
    }
    if ($extension -in @('.h', '.hpp')) {
        return "声明 $topic 相关类型、接口、配置和成员状态。"
    }
    return "实现 $topic 相关函数与文件级辅助逻辑。"
}

function Get-AreaDefinition {
    param([string]$RelativePath)

    $normalized = $RelativePath.Replace('\', '/')
    if ($normalized.StartsWith('src/apps/controller/')) {
        return @{ Key = '01_apps_controller'; Title = '应用层：主界面与远程会话窗口'; Description = 'Qt 主窗口、远程会话窗口、画布、文件窗口、主题与交互控件。' }
    }
    if ($normalized.StartsWith('src/apps/remote/')) {
        return @{ Key = '02_apps_remote'; Title = '应用层：会话引擎与业务控制器'; Description = 'direct/room 会话引擎、媒体路由、文件传输、远程粘贴与缓存状态机。' }
    }
    if ($normalized.StartsWith('src/auth/') -or
        $normalized.StartsWith('src/signaling/')) {
        return @{ Key = '03_auth_signaling'; Title = '认证与客户端信令'; Description = 'Logto/DPAPI 身份、启动配置、WSS 客户端、JSON 消息与恢复。' }
    }
    if ($normalized.StartsWith('src/core/') -or
        $normalized.StartsWith('src/protocol/')) {
        return @{ Key = '04_core_protocol'; Title = '核心接口与二进制协议'; Description = '引擎抽象、快照、策略、控制器执行器以及输入、光标、屏幕、文件和剪贴板协议。' }
    }
    if ($normalized.StartsWith('src/platform/win/')) {
        return @{ Key = '05_platform_windows'; Title = 'Windows 平台能力'; Description = 'DXGI/D3D11、Windows 输入、剪贴板、文件、摄像头、MFT/FFmpeg 编解码和硬件探测。' }
    }
    if ($normalized.StartsWith('src/webrtc/')) {
        return @{ Key = '06_webrtc'; Title = 'WebRTC 传输与媒体运行时'; Description = 'PeerConnection、RTP sender、DataChannel、统计、编解码工厂与运行时线程。' }
    }
    if ($normalized.StartsWith('src/server/')) {
        return @{ Key = '07_server'; Title = '信令服务器'; Description = 'WSS 认证、设备注册、direct session、协作房间、持久化、限流和诊断。' }
    }
    return @{ Key = '08_other'; Title = '其他源码'; Description = '未归入现有模块的源码文件。' }
}

function Get-TypeSymbols {
    param([string[]]$Lines)

    $symbols = [System.Collections.Generic.List[object]]::new()
    for ($index = 0; $index -lt $Lines.Count; $index++) {
        $line = $Lines[$index]
        if ($line -match '^\s*(class|struct|enum\s+class|enum)\s+((?:[A-Za-z_]\w*::)*[A-Za-z_]\w*)') {
            $kind = $Matches[1]
            $name = $Matches[2]
            $comment = Get-LeadingComment -Lines $Lines -LineIndex $index
            if ([string]::IsNullOrWhiteSpace($comment)) {
                $comment = "定义 $name 的 $kind 类型和相关状态。"
            }
            $symbols.Add([pscustomobject]@{
                Line = $index + 1
                Name = $name
                Kind = $kind
                Purpose = $comment
            })
        }
    }
    return $symbols
}

function Get-BlockEndLine {
    param(
        [string[]]$Lines,
        [int]$StartLine
    )

    $started = $false
    $depth = 0
    for ($index = $StartLine; $index -lt $Lines.Count; $index++) {
        $code = $Lines[$index] -replace '//.*$', ''
        $code = $code -replace '"(?:\\.|[^"\\])*"', '""'
        $openCount = ([regex]::Matches($code, '\{')).Count
        $closeCount = ([regex]::Matches($code, '\}')).Count
        if ($openCount -gt 0) {
            $started = $true
        }
        if ($started) {
            $depth += $openCount - $closeCount
            if ($depth -le 0) {
                return $index
            }
        }
    }
    if ($started) {
        return $Lines.Count - 1
    }
    return $StartLine
}

function Get-FunctionSymbols {
    param(
        [string[]]$Lines,
        [string]$Extension
    )

    $symbols = [System.Collections.Generic.List[object]]::new()
    $seen = [System.Collections.Generic.HashSet[string]]::new()
    $bodyLines = [System.Collections.Generic.HashSet[int]]::new()
    $functionBodyUntil = -1
    $skipNames = @('if', 'for', 'while', 'switch', 'catch', 'return',
                   'sizeof', 'alignof', 'decltype', 'static_assert',
                   'begin', 'end', 'size', 'empty', 'increment', 'void',
                   'bool', 'char', 'short', 'int', 'long', 'float', 'double',
                   'size_t', 'uint32_t', 'uint64_t', 'const', 'FAILED',
                   'SUCCEEDED', 'IID_PPV_ARGS')

    for ($start = 0; $start -lt $Lines.Count; $start++) {
        if ($start -le $functionBodyUntil) {
            continue
        }
        $first = $Lines[$start].Trim()
        if ([string]::IsNullOrWhiteSpace($first) -or
            $first.StartsWith('//') -or $first.StartsWith('#') -or
            -not $first.Contains('(')) {
            continue
        }

        $parts = [System.Collections.Generic.List[string]]::new()
        $terminator = ''
        $end = [Math]::Min($Lines.Count - 1, $start + 11)
        for ($cursor = $start; $cursor -le $end; $cursor++) {
            $piece = $Lines[$cursor].Trim()
            $parts.Add($piece)
            $braceIndex = $piece.IndexOf('{')
            $semicolonIndex = $piece.IndexOf(';')
            if ($braceIndex -ge 0 -or $semicolonIndex -ge 0) {
                if ($semicolonIndex -ge 0 -and
                    ($braceIndex -lt 0 -or $semicolonIndex -lt $braceIndex)) {
                    $terminator = ';'
                } else {
                    $terminator = '{'
                }
                break
            }
        }
        if ([string]::IsNullOrWhiteSpace($terminator)) {
            continue
        }
        if ($terminator -eq ';' -and $Extension -notin @('.h', '.hpp')) {
            continue
        }

        $candidate = Normalize-Text ($parts -join ' ')
        $candidate = $candidate -replace '//.*$', ''
        if ($candidate -match '^(if|for|while|switch|catch|return)\b' -or
            $candidate -match '^\[.*\]\s*\(' -or
            $candidate -match '=\s*\[.*\]\s*\(' -or
            $candidate -match '^[^()]*=') {
            continue
        }

        $match = [regex]::Match(
            $candidate,
            '(?<name>(?:[A-Za-z_]\w*::)*~?[A-Za-z_]\w*)\s*\((?<params>[^{};]*)\)')
        if (-not $match.Success) {
            continue
        }
        $name = $match.Groups['name'].Value
        $leaf = ($name -split '::')[-1].TrimStart('~')
        if ($skipNames -contains $leaf -or
            $name -match '^(std|QMetaObject|QString|QObject|QTimer|QFile|QDir|QIcon|QPixmap|QImage|QPainter|QApplication|QCoreApplication|QSettings|QMessageBox|QDesktopServices|QThread)::') {
            continue
        }

        $prefix = $candidate.Substring(0, $match.Index).Trim()
        if ($terminator -eq '{' -and
            ($prefix.Contains('.') -or $prefix.Contains('->') -or
             $prefix.Contains('!') -or $prefix.Contains('[') -or
             $prefix -match '\b(new|delete|throw|co_await|co_return)\b')) {
            continue
        }

        $key = "$($start + 1):$name"
        if (-not $seen.Add($key)) {
            continue
        }
        $comment = Get-LeadingComment -Lines $Lines -LineIndex $start
        $signature = $candidate
        if ($signature.Length -gt 220) {
            $signature = $signature.Substring(0, 217) + '...'
        }
        $symbols.Add([pscustomobject]@{
            Line = $start + 1
            Name = $name
            Signature = $signature.TrimEnd('{', ';', ' ')
            Kind = if ($terminator -eq '{') { '定义' } else { '声明' }
            Purpose = Get-FunctionPurpose -Name $name -Comment $comment
        })
        if ($terminator -eq '{') {
            $functionBodyUntil = Get-BlockEndLine -Lines $Lines -StartLine $start
            for ($bodyLine = $start + 1;
                 $bodyLine -le $functionBodyUntil;
                 $bodyLine++) {
                [void]$bodyLines.Add($bodyLine)
            }
        }
    }
    return [pscustomobject]@{
        Symbols = $symbols
        BodyLines = $bodyLines
    }
}

function Get-VariableSymbols {
    param(
        [string[]]$Lines,
        [string]$Extension,
        [System.Collections.Generic.HashSet[int]]$FunctionBodyLines
    )

    $symbols = [System.Collections.Generic.List[object]]::new()
    $seen = [System.Collections.Generic.HashSet[string]]::new()
    $allowAllDeclarations = $Extension -in @('.h', '.hpp')

    for ($index = 0; $index -lt $Lines.Count; $index++) {
        if ($null -ne $FunctionBodyLines -and
            $FunctionBodyLines.Contains($index)) {
            continue
        }
        $trimmed = $Lines[$index].Trim()
        if ([string]::IsNullOrWhiteSpace($trimmed) -or
            $trimmed.StartsWith('//') -or $trimmed.StartsWith('#') -or
            -not $trimmed.EndsWith(';') -or $trimmed.Contains('(') -or
            $trimmed.Contains(')') -or
            $trimmed -match '^(using|typedef|return|break|continue|static_assert|friend|case)\b') {
            continue
        }
        if (-not $allowAllDeclarations -and
            $trimmed -notmatch '\b(static|constexpr|constinit|thread_local)\b') {
            continue
        }

        $declaration = $trimmed -replace '//.*$', ''
        $left = ($declaration -split '=', 2)[0].Trim().TrimEnd(';').Trim()
        $left = $left -replace '\{.*$', ''
        $left = $left -replace '\[[^\]]*\]\s*$', ''
        $match = [regex]::Match($left, '([A-Za-z_]\w*)\s*$')
        if (-not $match.Success) {
            continue
        }
        $name = $match.Groups[1].Value
        if ($name -in @('public', 'private', 'protected')) {
            continue
        }
        $key = "$($index + 1):$name"
        if (-not $seen.Add($key)) {
            continue
        }

        $comment = Get-LeadingComment -Lines $Lines -LineIndex $index
        $symbols.Add([pscustomobject]@{
            Line = $index + 1
            Name = $name
            Declaration = $declaration
            Purpose = Get-VariablePurpose -Name $name -Comment $comment
        })
    }
    return $symbols
}

$sourceFiles = Get-ChildItem -LiteralPath $sourceRoot -Recurse -File |
    Where-Object { $_.Extension.ToLowerInvariant() -in @('.h', '.hpp', '.cpp', '.inc') } |
    Sort-Object FullName

$areas = [ordered]@{}
$totalFunctions = 0
$totalVariables = 0
$totalTypes = 0
$fileRecords = [System.Collections.Generic.List[object]]::new()

foreach ($file in $sourceFiles) {
    $repositoryPrefix = $RepositoryRoot.TrimEnd('\', '/') + '\'
    if (-not $file.FullName.StartsWith(
            $repositoryPrefix,
            [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Source file is outside repository root: $($file.FullName)"
    }
    $relativePath = $file.FullName.Substring(
        $repositoryPrefix.Length).Replace('\', '/')
    $lines = [System.IO.File]::ReadAllLines($file.FullName)
    if ($bodyOnlyFragments.Contains($relativePath)) {
        $types = @()
        $functions = @()
        $variables = @()
    } else {
        $types = @(Get-TypeSymbols -Lines $lines)
        $functionScan = Get-FunctionSymbols -Lines $lines -Extension $file.Extension.ToLowerInvariant()
        $functions = @($functionScan.Symbols)
        $variables = @(Get-VariableSymbols -Lines $lines -Extension $file.Extension.ToLowerInvariant() -FunctionBodyLines $functionScan.BodyLines)
    }
    $area = Get-AreaDefinition -RelativePath $relativePath

    $record = [pscustomobject]@{
        RelativePath = $relativePath
        Purpose = Get-FilePurpose -RelativePath $relativePath
        Area = $area
        Types = $types
        Functions = $functions
        Variables = $variables
    }
    $fileRecords.Add($record)
    if (-not $areas.Contains($area.Key)) {
        $areas[$area.Key] = [pscustomobject]@{
            Definition = $area
            Files = [System.Collections.Generic.List[object]]::new()
        }
    }
    $areas[$area.Key].Files.Add($record)
    $totalTypes += $types.Count
    $totalFunctions += $functions.Count
    $totalVariables += $variables.Count
}

$sourceBaseline = 'unversioned'
try {
    $sourceBaseline =
        (& git -C $RepositoryRoot rev-parse --short=12 HEAD:src 2>$null).Trim()
    & git -C $RepositoryRoot diff --quiet -- src 2>$null
    if ($LASTEXITCODE -ne 0) {
        $sourceBaseline += '-dirty'
    }
} catch {
    $sourceBaseline = 'unversioned'
}
$generatedAt = Get-Date -Format 'yyyy-MM-dd'

foreach ($entry in ($areas.GetEnumerator() | Sort-Object Name)) {
    $area = $entry.Value
    $definition = $area.Definition
    $builder = [System.Text.StringBuilder]::new()
    [void]$builder.AppendLine("# $($definition.Title)")
    [void]$builder.AppendLine()
    [void]$builder.AppendLine("> 自动生成于 $generatedAt，源码树 ``$sourceBaseline``。请运行 ``tools/Generate-SourceSymbolReference.ps1`` 刷新。")
    [void]$builder.AppendLine()
    [void]$builder.AppendLine($definition.Description)
    [void]$builder.AppendLine()
    [void]$builder.AppendLine("本册共收录 $($area.Files.Count) 个源码文件。函数与变量的中文作用优先采用源码紧邻注释；无注释时根据符号命名生成阅读提示，最终语义仍以源码为准。")

    foreach ($record in $area.Files) {
        $sourceLink = '../' + $record.RelativePath
        [void]$builder.AppendLine()
        [void]$builder.AppendLine("## ``$($record.RelativePath)``")
        [void]$builder.AppendLine()
        [void]$builder.AppendLine("[打开源码]($sourceLink) · **文件作用：** $($record.Purpose)")

        if ($record.Types.Count -gt 0) {
            [void]$builder.AppendLine()
            [void]$builder.AppendLine("### 类型")
            [void]$builder.AppendLine()
            [void]$builder.AppendLine('| 行 | 类型 | 种类 | 作用 |')
            [void]$builder.AppendLine('|---:|---|---|---|')
            foreach ($symbol in $record.Types) {
                $lineLink = "$sourceLink#L$($symbol.Line)"
                $purpose = Escape-MarkdownCell $symbol.Purpose
                [void]$builder.AppendLine("| [L$($symbol.Line)]($lineLink) | ``$($symbol.Name)`` | $($symbol.Kind) | $purpose |")
            }
        }

        if ($record.Variables.Count -gt 0) {
            [void]$builder.AppendLine()
            [void]$builder.AppendLine("### 成员与文件级变量")
            [void]$builder.AppendLine()
            [void]$builder.AppendLine('| 行 | 变量 | 声明 | 作用 |')
            [void]$builder.AppendLine('|---:|---|---|---|')
            foreach ($symbol in $record.Variables) {
                $lineLink = "$sourceLink#L$($symbol.Line)"
                $declaration = Escape-MarkdownCell $symbol.Declaration
                $purpose = Escape-MarkdownCell $symbol.Purpose
                [void]$builder.AppendLine("| [L$($symbol.Line)]($lineLink) | ``$($symbol.Name)`` | ``$declaration`` | $purpose |")
            }
        }

        if ($record.Functions.Count -gt 0) {
            [void]$builder.AppendLine()
            [void]$builder.AppendLine("### 函数")
            [void]$builder.AppendLine()
            [void]$builder.AppendLine('| 行 | 函数 | 类型 | 签名 | 作用 |')
            [void]$builder.AppendLine('|---:|---|---|---|---|')
            foreach ($symbol in $record.Functions) {
                $lineLink = "$sourceLink#L$($symbol.Line)"
                $signature = Escape-MarkdownCell $symbol.Signature
                $purpose = Escape-MarkdownCell $symbol.Purpose
                [void]$builder.AppendLine("| [L$($symbol.Line)]($lineLink) | ``$($symbol.Name)`` | $($symbol.Kind) | ``$signature`` | $purpose |")
            }
        }

        if ($record.Types.Count -eq 0 -and
            $record.Variables.Count -eq 0 -and
            $record.Functions.Count -eq 0) {
            [void]$builder.AppendLine()
            [void]$builder.AppendLine('当前规则未提取到公开类型、成员/文件级变量或函数；该文件可能只包含宏、模板片段、资源数据或条件编译内容，请直接打开源码查看。')
        }
    }

    $outputPath = Join-Path $outputRoot ($definition.Key + '.md')
    Write-Utf8LfFile -Path $outputPath -Content $builder.ToString()
}

$readme = [System.Text.StringBuilder]::new()
[void]$readme.AppendLine('# RLink 源码导航与符号参考')
[void]$readme.AppendLine()
[void]$readme.AppendLine("> 自动生成于 $generatedAt，源码树 ``$sourceBaseline``。")
[void]$readme.AppendLine()
[void]$readme.AppendLine('这套文档按源码文件回答四个问题：这个文件负责什么、定义哪些类型、保存哪些长期状态、提供哪些函数。每个符号都链接到当前源码行。')
[void]$readme.AppendLine()
[void]$readme.AppendLine('建议先读人工整理的 [核心源码符号说明](00_核心符号说明.md)，建立三条会话路径、对象所有权和线程关系；再用下面的自动分册查全量符号。')
[void]$readme.AppendLine()
[void]$readme.AppendLine('## 阅读边界')
[void]$readme.AppendLine()
[void]$readme.AppendLine('- 收录 `src` 下全部 `.h/.hpp/.cpp/.inc` 文件。')
[void]$readme.AppendLine('- “变量”只收录成员字段和文件级常量/状态，不罗列函数内部临时变量、循环下标和 lambda 捕获。')
[void]$readme.AppendLine('- “函数”同时收录头文件声明和实现文件定义，因此同一函数可能在两处出现，方便从接口或实现双向查找。')
[void]$readme.AppendLine('- 紧邻源码注释优先作为作用说明；没有注释时按命名生成中文提示。自动提示是导航，不替代对函数体和调用方的阅读。')
[void]$readme.AppendLine('- `.inc` 属于其主类的实现切片，成员变量通常只会在主 `.h` 中列出。')
[void]$readme.AppendLine()
[void]$readme.AppendLine('## 规模')
[void]$readme.AppendLine()
[void]$readme.AppendLine("- 源码文件：$($sourceFiles.Count)")
[void]$readme.AppendLine("- 类型：$totalTypes")
[void]$readme.AppendLine("- 函数声明/定义：$totalFunctions")
[void]$readme.AppendLine("- 成员与文件级变量：$totalVariables")
[void]$readme.AppendLine()
[void]$readme.AppendLine('## 分册')
[void]$readme.AppendLine()
[void]$readme.AppendLine('| 分册 | 文件数 | 内容 |')
[void]$readme.AppendLine('|---|---:|---|')
foreach ($entry in ($areas.GetEnumerator() | Sort-Object Name)) {
    $area = $entry.Value
    $definition = $area.Definition
    [void]$readme.AppendLine("| [$($definition.Title)]($($definition.Key).md) | $($area.Files.Count) | $($definition.Description) |")
}
[void]$readme.AppendLine()
[void]$readme.AppendLine('## 怎样查一个功能')
[void]$readme.AppendLine()
[void]$readme.AppendLine('1. 先在本页选择所属模块。')
[void]$readme.AppendLine('2. 在分册中搜索类名、函数名、变量名或文件名。')
[void]$readme.AppendLine('3. 点击行号进入源码，检查调用者、锁、generation/sequence 和失败路径。')
[void]$readme.AppendLine('4. 结合人工整理的 [核心源码符号说明](00_核心符号说明.md) 理解跨模块调用和对象所有权。')
[void]$readme.AppendLine()
[void]$readme.AppendLine('常见入口：')
[void]$readme.AppendLine()
[void]$readme.AppendLine('- 我的设备/验证码：搜索 `DirectSessionRuntimeState`、`ConnectDirectDevice`、`OnIncomingSessionRequest`。')
[void]$readme.AppendLine('- 协作房间：搜索 `RoomSnapshot`、`RoomPairRuntime`、`CreateRoom`、`RequestRoomControl`。')
[void]$readme.AppendLine('- 屏幕收发：搜索 `WindowsDesktopCaptureSource`、`SetScreenStreamPreference`、`OnFrame`、`Present`。')
[void]$readme.AppendLine('- 输入/光标：搜索 `RemoteInputEvent`、`SendRemoteInput`、`WindowsInputExecutor`、`WindowsCursorMonitor`。')
[void]$readme.AppendLine('- 文件/剪贴板：搜索 `FileTransferController`、`ClipboardController`、`bufferedAmount`。')
[void]$readme.AppendLine()
[void]$readme.AppendLine('## 更新方式')
[void]$readme.AppendLine()
[void]$readme.AppendLine('在仓库根目录执行：')
[void]$readme.AppendLine()
[void]$readme.AppendLine('```powershell')
[void]$readme.AppendLine('.\tools\Generate-SourceSymbolReference.ps1')
[void]$readme.AppendLine('```')
[void]$readme.AppendLine()
[void]$readme.AppendLine('生成后检查 `git diff -- 源码导航`。若函数改名或职责变化，应同时补充源码注释；这样下一次生成的作用说明也会随源码更新。')

Write-Utf8LfFile -Path (Join-Path $outputRoot 'README.md') -Content $readme.ToString()

Write-Host "Generated source symbol reference:"
Write-Host "  files:     $($sourceFiles.Count)"
Write-Host "  types:     $totalTypes"
Write-Host "  functions: $totalFunctions"
Write-Host "  variables: $totalVariables"
Write-Host "  output:    $outputRoot"
