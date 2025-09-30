param()

$script:Failures = 0

function Write-StructuredLog {
    param(
        [string]$Operation,
        [int]$Attempt,
        [int]$MaxAttempts,
        [System.Diagnostics.Stopwatch]$Stopwatch,
        [string]$Outcome,
        [string]$ErrorType,
        [string]$Message
    )
    $log = [ordered]@{
        CorrelationId = [guid]::NewGuid().ToString()
        Operation = $Operation
        Attempt = $Attempt
        MaxAttempts = $MaxAttempts
        DurationMs = [math]::Round($Stopwatch.Elapsed.TotalMilliseconds, 3)
        Outcome = $Outcome
        ErrorType = $ErrorType
        Message = $Message
    }
    $log | ConvertTo-Json -Compress | Write-Output
}

function Assert-True {
    param(
        [string]$Operation,
        [bool]$Condition,
        [string]$SuccessMessage,
        [string]$FailureMessage,
        [string]$ErrorType = "Assertion"
    )
    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    if ($Condition) {
        Write-StructuredLog -Operation $Operation -Attempt 1 -MaxAttempts 1 -Stopwatch $sw -Outcome "Success" -ErrorType "None" -Message $SuccessMessage
    } else {
        Write-StructuredLog -Operation $Operation -Attempt 1 -MaxAttempts 1 -Stopwatch $sw -Outcome "Fail" -ErrorType $ErrorType -Message $FailureMessage
        $script:Failures++
    }
}

function Assert-Equal {
    param(
        [string]$Operation,
        [object]$Actual,
        [object]$Expected,
        [string]$SuccessMessage,
        [string]$FailureMessage
    )
    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    if ($Actual -eq $Expected) {
        Write-StructuredLog -Operation $Operation -Attempt 1 -MaxAttempts 1 -Stopwatch $sw -Outcome "Success" -ErrorType "None" -Message $SuccessMessage
    } else {
        $msg = "$FailureMessage (Actual='$Actual')"
        Write-StructuredLog -Operation $Operation -Attempt 1 -MaxAttempts 1 -Stopwatch $sw -Outcome "Fail" -ErrorType "Assertion" -Message $msg
        $script:Failures++
    }
}

function Assert-CollectionContains {
    param(
        [string]$Operation,
        [System.Collections.IEnumerable]$Collection,
        [object]$Expected,
        [string]$SuccessMessage,
        [string]$FailureMessage
    )
    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    $found = $false
    foreach ($item in $Collection) {
        if ($item -eq $Expected) {
            $found = $true
            break
        }
    }
    if ($found) {
        Write-StructuredLog -Operation $Operation -Attempt 1 -MaxAttempts 1 -Stopwatch $sw -Outcome "Success" -ErrorType "None" -Message $SuccessMessage
    } else {
        Write-StructuredLog -Operation $Operation -Attempt 1 -MaxAttempts 1 -Stopwatch $sw -Outcome "Fail" -ErrorType "Assertion" -Message $FailureMessage
        $script:Failures++
    }
}

$root = (Resolve-Path "..").Path
$targetPath = Join-Path $root "Documents/MasterData/FCharacterInfoData.csv"

$exists = Test-Path $targetPath -PathType Leaf
Assert-True -Operation "FCharacterInfoDataExists" -Condition $exists -SuccessMessage "CSV 파일 존재" -FailureMessage "CSV 파일이 존재하지 않음" -ErrorType "MissingFile"

if ($exists) {
    $headerLine = [string](Get-Content -Path $targetPath -TotalCount 1)
    $header = $headerLine.Trim()
    Assert-Equal -Operation "FCharacterInfoDataHeader" -Actual $header -Expected "Name,Type,HP,ATK,SightLength,SightAngle" -SuccessMessage "CSV 헤더 일치" -FailureMessage "CSV 헤더 불일치"

    try {
        $rows = Import-Csv -Path $targetPath
        Assert-True -Operation "FCharacterInfoDataRowCount" -Condition ($rows.Count -ge 3) -SuccessMessage "최소 3개 이상의 데이터 보유" -FailureMessage "데이터 행 수가 부족함"

        $rowNames = $rows | ForEach-Object { $_.Name }
        foreach ($name in $rowNames) {
            Assert-True -Operation "FCharacterInfoDataRowName[$name]" -Condition ([string]::IsNullOrWhiteSpace($name) -eq $false) -SuccessMessage "RowName '$name' 유효" -FailureMessage "RowName 이 비어 있음" -ErrorType "Validation"
        }

        $types = $rows | ForEach-Object { $_.Type }
        foreach ($expectedType in @("Type01","Type02","Type03")) {
            Assert-CollectionContains -Operation "FCharacterInfoDataHas$expectedType" -Collection $types -Expected $expectedType -SuccessMessage "${expectedType} 데이터 존재" -FailureMessage "${expectedType} 데이터가 존재하지 않음"
        }
    }
    catch {
        Write-StructuredLog -Operation "FCharacterInfoDataParse" -Attempt 1 -MaxAttempts 1 -Stopwatch ([System.Diagnostics.Stopwatch]::StartNew()) -Outcome "Fail" -ErrorType "ParseError" -Message $_.Exception.Message
        $script:Failures++
    }
}

if ($script:Failures -gt 0) {
    exit 1
}

