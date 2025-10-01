<#
    실행 스크립트: Project YiSan Doxygen 생성 워크플로

    사용 방법:
        powershell -ExecutionPolicy Bypass -File Tools/Doxygen/RunDoxygen.ps1
        powershell -ExecutionPolicy Bypass -File Tools/Doxygen/RunDoxygen.ps1 -Clean

    옵션:
        -ConfigPath : 사용할 Doxyfile 경로(기본값은 저장소 루트의 Doxyfile)
        -Clean      : 실행 전 기존 산출물(`Documents/Doxygen`)과 경고 로그를 제거
#>

[CmdletBinding()]
param(
    [string]$ConfigPath,
    [switch]$Clean
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot '..\..')

if ([string]::IsNullOrWhiteSpace($ConfigPath)) {
    $ConfigPath = Join-Path $RepoRoot 'Doxyfile'
}

$ResolvedConfig = Resolve-Path $ConfigPath

$DocsOutput = Join-Path $RepoRoot 'Documents\Doxygen'
$WarningLog = Join-Path $RepoRoot 'doxygen_warnings.log'

if ($Clean) {
    if (Test-Path $DocsOutput) {
        Write-Host "[Doxygen] Removing previous output: $DocsOutput" -ForegroundColor Yellow
        Remove-Item $DocsOutput -Recurse -Force
    }

    if (Test-Path $WarningLog) {
        Write-Host "[Doxygen] Removing previous warning log: $WarningLog" -ForegroundColor Yellow
        Remove-Item $WarningLog -Force
    }
}

Write-Host "[Doxygen] Using config: $ResolvedConfig" -ForegroundColor Cyan

$StartTime = Get-Date

$processInfo = Start-Process -FilePath 'doxygen' -ArgumentList $ResolvedConfig `
    -RedirectStandardError $WarningLog -NoNewWindow -PassThru -Wait

$Duration = (Get-Date) - $StartTime
Write-Host "[Doxygen] Finished in $([Math]::Round($Duration.TotalSeconds, 2))s" -ForegroundColor Green

if ($processInfo.ExitCode -ne 0) {
    Write-Error "[Doxygen] Failed with exit code $($processInfo.ExitCode). See $WarningLog for details."
    exit $processInfo.ExitCode
}

if (Test-Path $WarningLog) {
    $WarningCount = (Get-Content $WarningLog | Measure-Object -Line).Lines
    if ($WarningCount -gt 0) {
        Write-Host "[Doxygen] Warnings detected ($WarningCount lines). Review $WarningLog" -ForegroundColor Yellow
    } else {
        Remove-Item $WarningLog -Force
        Write-Host "[Doxygen] No warnings recorded." -ForegroundColor Green
    }
} else {
    Write-Host "[Doxygen] No warning log produced." -ForegroundColor Green
}
