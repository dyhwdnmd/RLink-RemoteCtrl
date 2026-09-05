[CmdletBinding()]
param(
    [string]$OutputFile = ''
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$root = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if ([string]::IsNullOrWhiteSpace($OutputFile)) {
    $OutputFile = Join-Path $root `
        'x64\Release\signaling-dev\private\logto-m2m-secret.txt'
}
$OutputFile = [System.IO.Path]::GetFullPath($OutputFile)
$directory = Split-Path -Parent $OutputFile
New-Item -ItemType Directory -Force -Path $directory | Out-Null

$secret = Read-Host '请输入新的 Logto M2M Application Secret' -AsSecureString
$secretPointer = [IntPtr]::Zero
$plainText = $null
try {
    $secretPointer =
        [Runtime.InteropServices.Marshal]::SecureStringToBSTR($secret)
    $plainText =
        [Runtime.InteropServices.Marshal]::PtrToStringBSTR($secretPointer)
    if ([string]::IsNullOrWhiteSpace($plainText) -or
        $plainText.Length -lt 8 -or $plainText.Length -gt 4096) {
        throw 'M2M Application Secret 长度无效。'
    }
    $utf8WithoutBom = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText(
        $OutputFile, $plainText.Trim(), $utf8WithoutBom)
}
finally {
    $plainText = $null
    if ($secretPointer -ne [IntPtr]::Zero) {
        [Runtime.InteropServices.Marshal]::ZeroFreeBSTR($secretPointer)
    }
    $secret.Dispose()
}

$account = [Security.Principal.WindowsIdentity]::GetCurrent().Name
& icacls.exe $OutputFile '/inheritance:r' '/grant:r' "${account}:(F)" |
    Out-Null
if ($LASTEXITCODE -ne 0) {
    throw '密钥已写入，但无法限制文件 ACL。请勿继续启动服务器。'
}

Write-Output 'LOGTO_M2M_SECRET_READY=YES'
Write-Output "LOGTO_M2M_SECRET_FILE=$OutputFile"
Write-Output "LOGTO_M2M_SECRET_OWNER=$account"
