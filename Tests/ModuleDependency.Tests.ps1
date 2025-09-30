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

function Assert-Match {
    param(
        [string]$Operation,
        [string]$Content,
        [string]$Pattern,
        [string]$SuccessMessage,
        [string]$FailureMessage
    )
    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    if ($Content -match $Pattern) {
        Write-StructuredLog -Operation $Operation -Attempt 1 -MaxAttempts 1 -Stopwatch $sw -Outcome "Success" -ErrorType "None" -Message $SuccessMessage
    } else {
        Write-StructuredLog -Operation $Operation -Attempt 1 -MaxAttempts 1 -Stopwatch $sw -Outcome "Fail" -ErrorType "Assertion" -Message $FailureMessage
        $script:Failures++
    }
}

function Assert-NotMatch {
    param(
        [string]$Operation,
        [string]$Content,
        [string]$Pattern,
        [string]$SuccessMessage,
        [string]$FailureMessage
    )
    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    if ($Content -match $Pattern) {
        Write-StructuredLog -Operation $Operation -Attempt 1 -MaxAttempts 1 -Stopwatch $sw -Outcome "Fail" -ErrorType "Assertion" -Message $FailureMessage
        $script:Failures++
    } else {
        Write-StructuredLog -Operation $Operation -Attempt 1 -MaxAttempts 1 -Stopwatch $sw -Outcome "Success" -ErrorType "None" -Message $SuccessMessage
    }
}

# YiSan expectations
$yiSanContent = Get-Content -Path "..\Source\YiSan\YiSan.Build.cs" -Raw
Assert-Match -Operation "YiSanPrivateHasCoffee" -Content $yiSanContent -Pattern 'PrivateDependencyModuleNames[^;]*"CoffeeLibrary"' -SuccessMessage "YiSan.Build.cs Private deps에 CoffeeLibrary 포함" -FailureMessage "YiSan.Build.cs Private deps에 CoffeeLibrary가 없음"
Assert-Match -Operation "YiSanPrivateHasLatte" -Content $yiSanContent -Pattern 'PrivateDependencyModuleNames[^;]*"LatteLibrary"' -SuccessMessage "YiSan.Build.cs Private deps에 LatteLibrary 포함" -FailureMessage "YiSan.Build.cs Private deps에 LatteLibrary가 없음"
Assert-NotMatch -Operation "YiSanPublicNoCoffee" -Content $yiSanContent -Pattern 'PublicDependencyModuleNames[^;]*"CoffeeLibrary"' -SuccessMessage "YiSan.Build.cs Public deps에 CoffeeLibrary 없음" -FailureMessage "YiSan.Build.cs Public deps에 CoffeeLibrary가 존재함"
Assert-NotMatch -Operation "YiSanPublicNoLatte" -Content $yiSanContent -Pattern 'PublicDependencyModuleNames[^;]*"LatteLibrary"' -SuccessMessage "YiSan.Build.cs Public deps에 LatteLibrary 없음" -FailureMessage "YiSan.Build.cs Public deps에 LatteLibrary가 존재함"

# Latte expectations
$latteContent = Get-Content -Path "..\Source\LatteLibrary\LatteLibrary.Build.cs" -Raw
Assert-Match -Operation "LattePrivateHasCoffee" -Content $latteContent -Pattern 'PrivateDependencyModuleNames[^;]*"CoffeeLibrary"' -SuccessMessage "LatteLibrary.Build.cs Private deps에 CoffeeLibrary 포함" -FailureMessage "LatteLibrary.Build.cs Private deps에 CoffeeLibrary가 없음"
Assert-NotMatch -Operation "LattePublicNoCoffee" -Content $latteContent -Pattern 'PublicDependencyModuleNames[^;]*"CoffeeLibrary"' -SuccessMessage "LatteLibrary.Build.cs Public deps에 CoffeeLibrary 없음" -FailureMessage "LatteLibrary.Build.cs Public deps에 CoffeeLibrary가 존재함"

if ($script:Failures -gt 0) {
    exit 1
}

