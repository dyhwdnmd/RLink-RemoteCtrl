param(
    [ValidateRange(1, 65535)]
    [int]$Port = 443,

    [ValidateRange(2, 5)]
    [int]$MaximumRoomMembers = 5,

    [ValidateRange(0, 3600)]
    [int]$DiagnosticsIntervalSeconds = 30,

    [string[]]$IceUrl = @()
)

$ErrorActionPreference = 'Stop'
$serverRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$binDirectory = Join-Path $serverRoot 'bin'
$configDirectory = Join-Path $serverRoot 'config'
$serverExe = Join-Path $binDirectory 'RemoteCSignalServer.exe'
$certificate = Join-Path $configDirectory 'server-cert.pem'
$privateKey = Join-Path $configDirectory 'server-key.pem'
$settingsFile = Join-Path $configDirectory 'server-settings.json'

foreach ($path in @($serverExe, $certificate, $privateKey, $settingsFile)) {
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Required server file is missing: $path"
    }
}
$settings = Get-Content -LiteralPath $settingsFile -Raw | ConvertFrom-Json

$listener = Get-NetTCPConnection -LocalPort $Port -State Listen `
    -ErrorAction SilentlyContinue | Select-Object -First 1
if ($null -ne $listener) {
    $processName = '<unknown>'
    try {
        $processName = (Get-Process -Id $listener.OwningProcess `
            -ErrorAction Stop).ProcessName
    }
    catch {
    }
    throw "TCP port $Port is already listening: PID=$($listener.OwningProcess), process=$processName"
}

$arguments = @(
    '--listen', '0.0.0.0',
    '--port', "$Port",
    '--cert', $certificate,
    '--key', $privateKey,
    '--max-room-members', "$MaximumRoomMembers",
    '--diagnostics-interval-seconds', "$DiagnosticsIntervalSeconds"
)
if ($DiagnosticsIntervalSeconds -gt 0) {
    $logDirectory = Join-Path $serverRoot 'logs'
    New-Item -ItemType Directory -Force -Path $logDirectory | Out-Null
    $diagnosticsLog = Join-Path $logDirectory (
        'signal-server-{0}.log' -f (Get-Date -Format 'yyyyMMdd-HHmmss'))
    $arguments += @('--diagnostics-log-file', $diagnosticsLog)
    Write-Output "REMOTEC_SIGNAL_SERVER_LOG=$diagnosticsLog"
}
if ($settings.schemaVersion -eq 2 -and
    $settings.authMode -eq 'logto_userinfo') {
    $issuer = [string]$settings.logtoIssuer
    $identityDatabase = [string]$settings.identityDatabase
    if ([string]::IsNullOrWhiteSpace($issuer) -or
        [string]::IsNullOrWhiteSpace($identityDatabase)) {
        throw 'Logto server settings are incomplete.'
    }
    if (-not [System.IO.Path]::IsPathRooted($identityDatabase)) {
        $identityDatabase = Join-Path $serverRoot $identityDatabase
    }
    $arguments += @(
        '--logto-issuer', $issuer,
        '--identity-db', $identityDatabase
    )
    $managementClientIdProperty =
        $settings.PSObject.Properties['logtoManagementClientId']
    $managementSecretProperty =
        $settings.PSObject.Properties['logtoManagementClientSecretFile']
    $managementClientId = if ($null -ne $managementClientIdProperty) {
        [string]$managementClientIdProperty.Value
    } else { '' }
    $managementSecretName = if ($null -ne $managementSecretProperty) {
        [string]$managementSecretProperty.Value
    } else { '' }
    if (-not [string]::IsNullOrWhiteSpace($managementClientId)) {
        if ([string]::IsNullOrWhiteSpace($managementSecretName)) {
            throw 'Logto M2M secret filename is missing from server settings.'
        }
        $managementSecretFile = Join-Path $configDirectory `
            $managementSecretName
        if (-not (Test-Path -LiteralPath $managementSecretFile -PathType Leaf)) {
            throw "Logto M2M secret file is missing: $managementSecretFile"
        }
        $arguments += @(
            '--logto-management-client-id',
            $managementClientId,
            '--logto-management-client-secret-file', $managementSecretFile
        )
    }
    $webhookPortProperty =
        $settings.PSObject.Properties['logtoWebhookPort']
    $webhookSecretProperty =
        $settings.PSObject.Properties['logtoWebhookSigningKeyFile']
    $configuredWebhookPort = if ($null -ne $webhookPortProperty) {
        [int]$webhookPortProperty.Value
    } else { 0 }
    $webhookSecretName = if ($null -ne $webhookSecretProperty) {
        [string]$webhookSecretProperty.Value
    } else { '' }
    if ($configuredWebhookPort -gt 0) {
        if ([string]::IsNullOrWhiteSpace($webhookSecretName)) {
            throw 'Logto webhook signing key filename is missing from server settings.'
        }
        $webhookSecretFile = Join-Path $configDirectory `
            $webhookSecretName
        if (-not (Test-Path -LiteralPath $webhookSecretFile -PathType Leaf)) {
            throw "Logto webhook signing key file is missing: $webhookSecretFile"
        }
        $arguments += @(
            '--logto-webhook-listen', '127.0.0.1',
            '--logto-webhook-port', ([string]$configuredWebhookPort),
            '--logto-webhook-signing-key-file', $webhookSecretFile
        )
    }
}
elseif ($settings.schemaVersion -eq 1 -and
        $settings.authMode -eq 'legacy_hmac_test_only' -and
        $settings.testOnly -eq $true) {
    $secretFile = Join-Path $configDirectory 'token-secret.txt'
    if (-not (Test-Path -LiteralPath $secretFile -PathType Leaf)) {
        throw "Legacy test-only token secret is missing: $secretFile"
    }
    $arguments += @('--secret-file', $secretFile)
}
else {
    throw 'Server authentication settings are invalid or not explicitly test-only.'
}
foreach ($url in $IceUrl) {
    if (-not [string]::IsNullOrWhiteSpace($url)) {
        $arguments += @('--ice-url', $url)
    }
}

$env:QT_SSL_BACKEND = 'openssl'
Push-Location $binDirectory
try {
    Write-Output "REMOTEC_SIGNAL_SERVER_STARTING=0.0.0.0:$Port"
    & $serverExe @arguments
    exit $LASTEXITCODE
}
finally {
    Pop-Location
}
