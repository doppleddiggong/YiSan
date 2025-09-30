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

# Test: ����� SpringArm ������Ʈ ������ �����ؾ� ��
# ��� ����: UPROPERTY ��� �ȿ� USpringArmComponent �����Ͱ� ����Ǿ� ���� ��.
Assert-Match -Operation "PlayerActorHeaderSpringArm" -Content $headerContent -Pattern 'UPROPERTY\([^\)]*Camera[^\)]*\)[^;]*TObjectPtr<\s*(?:class\s+)?USpringArmComponent\s*>\s+SpringArmComp;' -SuccessMessage "SpringArm ������Ʈ ���� Ȯ��" -FailureMessage "SpringArm ������Ʈ ������ ������"

# Test: ����� Camera ������Ʈ ������ �����ؾ� ��
# ��� ����: UPROPERTY ��� �ȿ� UCameraComponent �����Ͱ� ����Ǿ� ���� ��.
Assert-Match -Operation "PlayerActorHeaderCamera" -Content $headerContent -Pattern 'UPROPERTY\([^\)]*Camera[^\)]*\)[^;]*TObjectPtr<\s*(?:class\s+)?UCameraComponent\s*>\s+FollowCamera;' -SuccessMessage "Camera ������Ʈ ���� Ȯ��" -FailureMessage "Camera ������Ʈ ������ ������"

# Test: cpp���� SpringArm�� Camera�� �����ؾ� ��
# ��� ����: CreateDefaultSubobject ȣ�� �� �� ��� ����
Assert-Match -Operation "PlayerActorCreateSpringArm" -Content $cppContent -Pattern 'CreateDefaultSubobject<USpringArmComponent>\s*\(.*SpringArm' -SuccessMessage "SpringArm ���� ȣ�� ����" -FailureMessage "SpringArm ���� ȣ�� ����"
Assert-Match -Operation "PlayerActorCreateCamera" -Content $cppContent -Pattern 'CreateDefaultSubobject<UCameraComponent>\s*\(.*FollowCamera' -SuccessMessage "Camera ���� ȣ�� ����" -FailureMessage "Camera ���� ȣ�� ����"

# Test: ������� ������ ȸ�� ������ �־�� ��
# ��� ����: SetupAttachment ȣ�� �� bUsePawnControlRotation ���� Ȯ��
Assert-Match -Operation "PlayerActorSetupAttachment" -Content $cppContent -Pattern 'SpringArmComp->SetupAttachment' -SuccessMessage "SpringArm ���� �ڵ� ����" -FailureMessage "SpringArm ���� �ڵ� ����"
Assert-Match -Operation "PlayerActorCameraAttachment" -Content $cppContent -Pattern 'FollowCamera->SetupAttachment' -SuccessMessage "Camera ���� �ڵ� ����" -FailureMessage "Camera ���� �ڵ� ����"
Assert-Match -Operation "PlayerActorControlRotation" -Content $cppContent -Pattern 'SpringArmComp->bUsePawnControlRotation\s*=\s*true' -SuccessMessage "��Ʈ�� ȸ�� ���� ����" -FailureMessage "��Ʈ�� ȸ�� ���� ����"
Assert-Match -Operation "PlayerActorDisableCameraControlRotation" -Content $cppContent -Pattern 'FollowCamera->bUsePawnControlRotation\s*=\s*false' -SuccessMessage "ī�޶� ȸ�� ���� ����" -FailureMessage "ī�޶� ȸ�� ���� ����"

if ($script:Failures -gt 0) {
    exit 1
}

