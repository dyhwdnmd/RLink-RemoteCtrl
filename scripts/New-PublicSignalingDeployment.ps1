[CmdletBinding(DefaultParameterSetName = 'Logto')]
param(
    [Parameter(Mandatory = $true)]
    [string]$PublicIp,

    [ValidateRange(1, 65535)]
    [int]$Port = 443,

    [ValidateRange(2, 5)]
    [int]$MaximumRoomMembers = 5,

    [ValidateRange(1, 825)]
    [int]$CertificateDays = 365,

    [ValidateRange(60, 604800)]
    [int]$TestTokenLifetimeSeconds = 604800,

    [Parameter(Mandatory = $true, ParameterSetName = 'Logto')]
    [string]$LogtoIssuer,

    [Parameter(Mandatory = $true, ParameterSetName = 'Logto')]
    [string]$LogtoClientId,

    [Parameter(Mandatory = $true, ParameterSetName = 'LegacyTest')]
    [switch]$TestOnlyLegacyHmac,

    [string]$Configuration = 'Release',
    [string]$QtInstallDirectory = 'E:\Qt6\6.11.1\msvc2022_64',
    [string]$OpenSslRuntimeDirectory = 'C:\Program Files\Git\mingw64\bin',
    [string]$OpenSsl = 'C:\Program Files\Git\usr\bin\openssl.exe',
    [string]$OutputDirectory = ''
)

$ErrorActionPreference = 'Stop'
$parsedAddress = $null
if (-not [System.Net.IPAddress]::TryParse($PublicIp, [ref]$parsedAddress) -or
    $parsedAddress.AddressFamily -ne
        [System.Net.Sockets.AddressFamily]::InterNetwork) {
    throw "PublicIp must be a numeric IPv4 address: $PublicIp"
}
$PublicIp = $parsedAddress.ToString()

$usesLogto = $PSCmdlet.ParameterSetName -eq 'Logto'
$normalizedIssuer = ''
if ($usesLogto) {
    $issuerUri = $null
    if (-not [Uri]::TryCreate($LogtoIssuer, [UriKind]::Absolute,
                             [ref]$issuerUri) -or
        $issuerUri.Scheme -ne 'https' -or
        -not $issuerUri.AbsolutePath.TrimEnd('/').EndsWith('/oidc')) {
        throw 'LogtoIssuer must be an HTTPS issuer ending in /oidc.'
    }
    if ([string]::IsNullOrWhiteSpace($LogtoClientId) -or
        $LogtoClientId.Length -gt 256) {
        throw 'LogtoClientId is missing or too long.'
    }
    $normalizedIssuer = $issuerUri.AbsoluteUri.TrimEnd('/')
}

$root = Split-Path -Parent $PSScriptRoot
$buildDirectory = Join-Path $root "x64\$Configuration"
if ([string]::IsNullOrWhiteSpace($OutputDirectory)) {
    $OutputDirectory = Join-Path $buildDirectory `
        "public-signaling\$PublicIp"
}
$OutputDirectory = [System.IO.Path]::GetFullPath($OutputDirectory)
$serverDirectory = Join-Path $OutputDirectory 'server'
$serverBinDirectory = Join-Path $serverDirectory 'bin'
$serverConfigDirectory = Join-Path $serverDirectory 'config'
$serverTlsDirectory = Join-Path $serverBinDirectory 'tls'
$serverSqlDriverDirectory = Join-Path $serverBinDirectory 'sqldrivers'
$serverDataDirectory = Join-Path $serverDirectory 'data'
$clientDirectory = Join-Path $OutputDirectory 'client'
$privateDirectory = Join-Path $OutputDirectory 'PRIVATE-DO-NOT-UPLOAD'

foreach ($directory in @($serverBinDirectory, $serverConfigDirectory,
                          $serverTlsDirectory, $serverSqlDriverDirectory,
                          $serverDataDirectory,
                          $clientDirectory,
                          $privateDirectory)) {
    New-Item -ItemType Directory -Force -Path $directory | Out-Null
}

if (-not (Test-Path -LiteralPath $OpenSsl -PathType Leaf)) {
    throw "OpenSSL was not found: $OpenSsl"
}

$serverExe = Join-Path $buildDirectory 'RemoteCSignalServer.exe'
$runtimeCopies = @(
    @($serverExe, (Join-Path $serverBinDirectory 'RemoteCSignalServer.exe')),
    @((Join-Path $QtInstallDirectory 'bin\Qt6Core.dll'),
      (Join-Path $serverBinDirectory 'Qt6Core.dll')),
    @((Join-Path $QtInstallDirectory 'bin\Qt6Network.dll'),
      (Join-Path $serverBinDirectory 'Qt6Network.dll')),
    @((Join-Path $QtInstallDirectory 'bin\Qt6HttpServer.dll'),
      (Join-Path $serverBinDirectory 'Qt6HttpServer.dll')),
    @((Join-Path $QtInstallDirectory 'bin\Qt6Sql.dll'),
      (Join-Path $serverBinDirectory 'Qt6Sql.dll')),
    @((Join-Path $QtInstallDirectory 'bin\Qt6WebSockets.dll'),
      (Join-Path $serverBinDirectory 'Qt6WebSockets.dll')),
    @((Join-Path $QtInstallDirectory 'plugins\sqldrivers\qsqlite.dll'),
      (Join-Path $serverSqlDriverDirectory 'qsqlite.dll')),
    @((Join-Path $QtInstallDirectory 'plugins\tls\qopensslbackend.dll'),
      (Join-Path $serverTlsDirectory 'qopensslbackend.dll')),
    @((Join-Path $OpenSslRuntimeDirectory 'libcrypto-3-x64.dll'),
      (Join-Path $serverBinDirectory 'libcrypto-3-x64.dll')),
    @((Join-Path $OpenSslRuntimeDirectory 'libssl-3-x64.dll'),
      (Join-Path $serverBinDirectory 'libssl-3-x64.dll')),
    @((Join-Path $PSScriptRoot 'Start-PublicSignalingServer.ps1'),
      (Join-Path $serverDirectory 'Start-RemoteCSignalServer.ps1')),
    @((Join-Path $PSScriptRoot 'Set-LogtoM2MSecret.ps1'),
      (Join-Path $serverDirectory 'Set-LogtoM2MSecret.ps1'))
)
foreach ($copy in $runtimeCopies) {
    if (-not (Test-Path -LiteralPath $copy[0] -PathType Leaf)) {
        throw "Required deployment source file is missing: $($copy[0])"
    }
    Copy-Item -LiteralPath $copy[0] -Destination $copy[1] -Force
}

$caKey = Join-Path $privateDirectory 'ca-key.pem'
$caCertificate = Join-Path $privateDirectory 'ca-cert.pem'
$serverRequest = Join-Path $privateDirectory 'server.csr'
$serialFile = Join-Path $privateDirectory 'ca-cert.srl'
$extensionFile = Join-Path $privateDirectory 'server-cert.ext'
$serverKey = Join-Path $serverConfigDirectory 'server-key.pem'
$serverCertificate = Join-Path $serverConfigDirectory 'server-cert.pem'
$secretFile = Join-Path $serverConfigDirectory 'token-secret.txt'
$serverSettingsFile = Join-Path $serverConfigDirectory `
    'server-settings.json'
if ($usesLogto -and
    (Test-Path -LiteralPath $secretFile -PathType Leaf)) {
    throw "Refusing to package a legacy token secret in Logto mode. Use a clean OutputDirectory: $OutputDirectory"
}

$certificateFiles = @($caKey, $caCertificate, $serverKey,
                      $serverCertificate)
$existingCertificateFiles = @($certificateFiles | Where-Object {
    Test-Path -LiteralPath $_ -PathType Leaf
})
if ($existingCertificateFiles.Count -ne 0 -and
    $existingCertificateFiles.Count -ne $certificateFiles.Count) {
    throw "Certificate set is incomplete under $OutputDirectory. Remove the incomplete output directory or restore the missing files."
}

if ($existingCertificateFiles.Count -eq 0) {
    & $OpenSsl req -x509 -newkey rsa:3072 -nodes -sha256 `
        -days $CertificateDays `
        -subj '/CN=RemoteC Public Test CA' `
        -addext 'basicConstraints=critical,CA:TRUE' `
        -addext 'keyUsage=critical,keyCertSign,cRLSign' `
        -keyout $caKey -out $caCertificate
    if ($LASTEXITCODE -ne 0) {
        throw 'Failed to create the private test CA.'
    }

    & $OpenSsl req -new -newkey rsa:3072 -nodes -sha256 `
        -subj "/CN=$PublicIp" -keyout $serverKey -out $serverRequest
    if ($LASTEXITCODE -ne 0) {
        throw 'Failed to create the public signaling server CSR.'
    }

    @(
        'basicConstraints=critical,CA:FALSE',
        'keyUsage=critical,digitalSignature,keyEncipherment',
        'extendedKeyUsage=serverAuth',
        "subjectAltName=IP:$PublicIp"
    ) | Set-Content -LiteralPath $extensionFile -Encoding Ascii

    & $OpenSsl x509 -req -in $serverRequest -CA $caCertificate `
        -CAkey $caKey -CAcreateserial -days $CertificateDays -sha256 `
        -extfile $extensionFile -out $serverCertificate
    if ($LASTEXITCODE -ne 0) {
        throw 'Failed to sign the public signaling server certificate.'
    }
}

$certificateText = (& $OpenSsl x509 -in $serverCertificate `
    -noout -subject -issuer -dates -ext subjectAltName 2>&1) -join "`n"
if ($LASTEXITCODE -ne 0 -or
    $certificateText -notmatch [regex]::Escape("IP Address:$PublicIp")) {
    throw "The server certificate does not contain IP SAN $PublicIp."
}

if (-not $usesLogto -and
    -not (Test-Path -LiteralPath $secretFile -PathType Leaf)) {
    & $OpenSsl rand -hex -out $secretFile 32
    if ($LASTEXITCODE -ne 0) {
        throw 'Failed to create the signaling token secret.'
    }
}

$utf8WithoutBom = New-Object System.Text.UTF8Encoding($false)
$serverSettings = if ($usesLogto) {
    [ordered]@{
        schemaVersion = 2
        authMode = 'logto_userinfo'
        logtoIssuer = $normalizedIssuer
        identityDatabase = 'data\remotec-identities.sqlite'
        logtoManagementClientId = ''
        logtoManagementClientSecretFile = ''
        logtoWebhookPort = 0
        logtoWebhookSigningKeyFile = ''
    }
} else {
    [ordered]@{
        schemaVersion = 1
        authMode = 'legacy_hmac_test_only'
        testOnly = $true
    }
}
[System.IO.File]::WriteAllText(
    $serverSettingsFile, ($serverSettings | ConvertTo-Json),
    $utf8WithoutBom)

$clientCaCertificate = Join-Path $clientDirectory 'ca-cert.pem'
Copy-Item -LiteralPath $caCertificate -Destination $clientCaCertificate -Force

$deviceIds = @(
    'signal-test-a', 'signal-test-b', 'signal-test-c',
    'webrtc-test-a', 'webrtc-test-b',
    'dyh', 'lyx'
)
$tokens = @{}
if (-not $usesLogto) {
    foreach ($deviceId in $deviceIds) {
        $issued = (& $serverExe --secret-file $secretFile `
            --issue-token $deviceId `
            --ttl-seconds $TestTokenLifetimeSeconds) -join ''
        $issued = $issued.Trim()
        if ($LASTEXITCODE -ne 0 -or
            [string]::IsNullOrWhiteSpace($issued)) {
            throw "Failed to issue the public test token for $deviceId."
        }
        $tokens[$deviceId] = $issued
    }
}

$clientConfigurationPath = if ($usesLogto) {
    $bootstrap = [ordered]@{
        schemaVersion = 2
        endpoint = "wss://$PublicIp`:$Port/signaling"
        issuer = $normalizedIssuer
        clientId = $LogtoClientId.Trim()
        callbackUrl = 'http://127.0.0.1:18765/callback'
        deviceName = ''
        caCertificate = 'ca-cert.pem'
        qtSslBackend = 'openssl'
    }
    $path = Join-Path $clientDirectory 'RemoteC.bootstrap.json'
    [System.IO.File]::WriteAllText(
        $path, ($bootstrap | ConvertTo-Json), $utf8WithoutBom)
    $path
} else {
    $testConfig = [ordered]@{
        endpoint = "wss://$PublicIp`:$Port/signaling"
        caCertificate = 'ca-cert.pem'
        generatedAtUtc = [DateTime]::UtcNow.ToString('o')
        expiresAtUtc = [DateTime]::UtcNow.AddSeconds(
            $TestTokenLifetimeSeconds).ToString('o')
        signalTokenA = $tokens['signal-test-a']
        signalTokenB = $tokens['signal-test-b']
        signalTokenC = $tokens['signal-test-c']
        webRtcTokenA = $tokens['webrtc-test-a']
        webRtcTokenB = $tokens['webrtc-test-b']
        dyhToken = $tokens['dyh']
        lyxToken = $tokens['lyx']
        testOnly = $true
    }
    $path = Join-Path $clientDirectory 'public-test-config.json'
    [System.IO.File]::WriteAllText(
        $path, ($testConfig | ConvertTo-Json), $utf8WithoutBom)
    $path
}

$serverSecurityLine = if ($usesLogto) {
    '3. Preserve and back up data\\remotec-identities.sqlite across every upgrade; deleting or replacing it reassigns public device IDs. Restrict config\\server-key.pem and data\\ to Administrators and the service account.'
} else {
    '3. Restrict config\\server-key.pem and config\\token-secret.txt to Administrators and the test service account.'
}
$serverReadme = @(
    "RemoteC public signaling test server for $PublicIp`:$Port",
    '',
    '1. Install Microsoft Visual C++ 2015-2022 Redistributable (x64).',
    '2. Upload this entire server directory to the Windows server.',
    $serverSecurityLine,
    '4. Check which process owns TCP 443 before starting.',
    '5. In Logto mode, save the M2M secret as the intended service account:',
    '   .\\Set-LogtoM2MSecret.ps1 -OutputFile .\\config\\logto-m2m-secret.txt',
    '   Set logtoManagementClientId and logtoManagementClientSecretFile in config\\server-settings.json.',
    '6. Start the service:',
    '   powershell -ExecutionPolicy Bypass -File .\\Start-RemoteCSignalServer.ps1',
    '7. Runtime status is printed every 30 seconds and appended under the logs directory.',
    '',
    'Do not upload the sibling PRIVATE-DO-NOT-UPLOAD directory.'
)
$serverReadme | Set-Content -LiteralPath `
    (Join-Path $serverDirectory 'DEPLOYMENT.txt') -Encoding UTF8

$clientReadme = if ($usesLogto) {
    @(
        "Endpoint: wss://$PublicIp`:$Port/signaling",
        'Keep RemoteC.bootstrap.json and ca-cert.pem beside RLinkAPP.exe.',
        'The bootstrap contains no client secret or token.',
        '',
        'Start RLinkAPP.exe and complete login in the system browser.',
        'Registered callback: http://127.0.0.1:18765/callback'
    )
} else {
    @(
        "Endpoint: wss://$PublicIp`:$Port/signaling",
        'The CA certificate must stay beside public-test-config.json.',
        'The JSON file contains short-lived test tokens. Do not publish it.',
        '',
        'Run from the project workspace:',
        "powershell -ExecutionPolicy Bypass -File scripts\\Run-PublicSignalingIntegrationTest.ps1 -ConfigurationFile `"$clientConfigurationPath`""
    )
}
$clientReadme | Set-Content -LiteralPath `
    (Join-Path $clientDirectory 'TEST-INSTRUCTIONS.txt') -Encoding UTF8

$serverArchive = Join-Path $OutputDirectory `
    "RemoteCSignalServer-$PublicIp.zip"
if (Test-Path -LiteralPath $serverArchive -PathType Leaf) {
    Remove-Item -LiteralPath $serverArchive -Force
}
Compress-Archive -Path (Join-Path $serverDirectory '*') `
    -DestinationPath $serverArchive -CompressionLevel Optimal
$archiveHash = (Get-FileHash -LiteralPath $serverArchive `
    -Algorithm SHA256).Hash
$archiveHashFile = "$serverArchive.sha256.txt"
"$archiveHash  $([System.IO.Path]::GetFileName($serverArchive))" |
    Set-Content -LiteralPath $archiveHashFile -Encoding Ascii

Write-Output 'PUBLIC_SIGNALING_DEPLOYMENT_READY=YES'
Write-Output "PUBLIC_SIGNALING_ENDPOINT=wss://$PublicIp`:$Port/signaling"
Write-Output "SERVER_UPLOAD_DIRECTORY=$serverDirectory"
Write-Output "SERVER_UPLOAD_ARCHIVE=$serverArchive"
Write-Output "SERVER_UPLOAD_SHA256=$archiveHash"
Write-Output "CLIENT_CONFIGURATION=$clientConfigurationPath"
Write-Output "AUTH_MODE=$(if ($usesLogto) { 'LOGTO_USERINFO' } else { 'LEGACY_HMAC_TEST_ONLY' })"
Write-Output "PRIVATE_CA_DIRECTORY=$privateDirectory"
Write-Output 'PRIVATE_CA_UPLOAD_ALLOWED=NO'
Write-Output 'SERVER_RUNNING=NO'
