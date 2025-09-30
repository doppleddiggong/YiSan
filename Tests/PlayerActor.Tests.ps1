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

$root = (Resolve-Path "..").Path
$headerPath = Join-Path $root "Source/LatteLibrary/Character/Public/APlayerActor.h"
$cppPath = Join-Path $root "Source/LatteLibrary/Character/Private/APlayerActor.cpp"

$headerContent = Get-Content -Path $headerPath -Raw
$cppContent = Get-Content -Path $cppPath -Raw

# Test: 헤더에 SpringArm 컴포넌트 선언이 존재해야 함
# 통과 기준: UPROPERTY 블록 안에 USpringArmComponent 포인터가 선언되어 있을 것.
Assert-Match -Operation "PlayerActorHeaderSpringArm" -Content $headerContent -Pattern 'UPROPERTY\([^\)]*Camera[^\)]*\)[^;]*TObjectPtr<\s*(?:class\s+)?USpringArmComponent\s*>\s+SpringArmComp;' -SuccessMessage "SpringArm 컴포넌트 선언 확인" -FailureMessage "SpringArm 컴포넌트 선언이 누락됨"

# Test: 헤더에 Camera 컴포넌트 선언이 존재해야 함
# 통과 기준: UPROPERTY 블록 안에 UCameraComponent 포인터가 선언되어 있을 것.
Assert-Match -Operation "PlayerActorHeaderCamera" -Content $headerContent -Pattern 'UPROPERTY\([^\)]*Camera[^\)]*\)[^;]*TObjectPtr<\s*(?:class\s+)?UCameraComponent\s*>\s+FollowCamera;' -SuccessMessage "Camera 컴포넌트 선언 확인" -FailureMessage "Camera 컴포넌트 선언이 누락됨"

# Test: cpp에서 SpringArm과 Camera를 생성해야 함
# 통과 기준: CreateDefaultSubobject 호출 두 개 모두 존재
Assert-Match -Operation "PlayerActorCreateSpringArm" -Content $cppContent -Pattern 'CreateDefaultSubobject<USpringArmComponent>\s*\(.*SpringArm' -SuccessMessage "SpringArm 생성 호출 존재" -FailureMessage "SpringArm 생성 호출 없음"
Assert-Match -Operation "PlayerActorCreateCamera" -Content $cppContent -Pattern 'CreateDefaultSubobject<UCameraComponent>\s*\(.*FollowCamera' -SuccessMessage "Camera 생성 호출 존재" -FailureMessage "Camera 생성 호출 없음"

# Test: 구성요소 부착과 회전 설정이 있어야 함
# 통과 기준: SetupAttachment 호출 및 bUsePawnControlRotation 설정 확인
Assert-Match -Operation "PlayerActorSetupAttachment" -Content $cppContent -Pattern 'SpringArmComp->SetupAttachment' -SuccessMessage "SpringArm 부착 코드 존재" -FailureMessage "SpringArm 부착 코드 없음"
Assert-Match -Operation "PlayerActorCameraAttachment" -Content $cppContent -Pattern 'FollowCamera->SetupAttachment' -SuccessMessage "Camera 부착 코드 존재" -FailureMessage "Camera 부착 코드 없음"
Assert-Match -Operation "PlayerActorControlRotation" -Content $cppContent -Pattern 'SpringArmComp->bUsePawnControlRotation\s*=\s*true' -SuccessMessage "컨트롤 회전 설정 존재" -FailureMessage "컨트롤 회전 설정 없음"
Assert-Match -Operation "PlayerActorDisableCameraControlRotation" -Content $cppContent -Pattern 'FollowCamera->bUsePawnControlRotation\s*=\s*false' -SuccessMessage "카메라 회전 설정 존재" -FailureMessage "카메라 회전 설정 없음"

if ($script:Failures -gt 0) {
    exit 1
}

