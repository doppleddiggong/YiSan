// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "YiSan/Loading/Public/YiSanGameInstance.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "ContentStreaming.h"
#include "Streaming/StreamingWorldSubsystemInterface.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#if WITH_EDITOR
#include "ShaderCompiler.h"
#endif

UYiSanGameInstance::UYiSanGameInstance()
{
    // 로딩 위젯 클래스 설정 (블루프린트에서도 설정 가능)
    // LoadingWidgetClass = ...;
}

void UYiSanGameInstance::Init()
{
    Super::Init();
    UE_LOG(LogTemp, Log, TEXT("[YiSan GameInstance] Initialized"));
}

void UYiSanGameInstance::LoadLevelWithLoadingScreen(FName InTargetLevelName)
{
    if (InTargetLevelName.IsNone())
    {
        UE_LOG(LogTemp, Error, TEXT("[YiSan] Invalid target level name!"));
        return;
    }

    TargetLevelName = InTargetLevelName;
    UE_LOG(LogTemp, Log, TEXT("[YiSan] Starting level transition to: %s"), *TargetLevelName.ToString());

    // 초기화
    bLevelLoaded = false;
    bAssetsLoaded = false;
    bWorldPartitionReady = false;

    // Step 1: 로딩 레벨로 먼저 이동
    Step1_MoveToLoadingLevel();
}

// ==================== Step 1: 로딩 레벨로 이동 ====================

void UYiSanGameInstance::Step1_MoveToLoadingLevel()
{
    UE_LOG(LogTemp, Log, TEXT("[YiSan Step 1] Moving to loading level"));

    // 로딩 스크린 표시
    ShowLoadingScreen();

    // 로딩 레벨로 이동 (동기)
    // 주의: 로딩 레벨은 가벼워야 합니다!
    UGameplayStatics::OpenLevel(this, FName("/Game/CustomContents/Maps/LoadingMap"));

    // 로딩 레벨이 로드되면 Step2 실행
    FTimerHandle DelayTimer;
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimerForNextTick([this]()
        {
            // 다음 프레임에 Step2 실행을 위한 타이머 설정
            if (UWorld* CurrentWorld = GetWorld())
            {
                FTimerHandle SecondDelayTimer;
                CurrentWorld->GetTimerManager().SetTimer(
                    SecondDelayTimer,
                    this,
                    &UYiSanGameInstance::Step2_StartLoadingTargetLevel,
                    0.5f,  // 로딩 레벨 안정화 대기
                    false
                );
            }
        });
    }
}

// ==================== Step 2: 타겟 레벨 로드 시작 ====================

void UYiSanGameInstance::Step2_StartLoadingTargetLevel()
{
    UE_LOG(LogTemp, Log, TEXT("[YiSan Step 2] Starting to load target level: %s"), *TargetLevelName.ToString());

    // 비동기 레벨 로드 (현재 로딩 레벨 위에 스트리밍)
    FLatentActionInfo LatentInfo;
    LatentInfo.CallbackTarget = this;
    LatentInfo.ExecutionFunction = FName("Step3_OnLevelLoaded");
    LatentInfo.Linkage = 0;
    LatentInfo.UUID = FMath::Rand();

    UGameplayStatics::LoadStreamLevel(this, TargetLevelName, true, false, LatentInfo);
}

// ==================== Step 3: 레벨 로드 완료 ====================

void UYiSanGameInstance::Step3_OnLevelLoaded()
{
    UE_LOG(LogTemp, Log, TEXT("[YiSan Step 3] Target level loaded into memory"));
    bLevelLoaded = true;

    // 이제 리소스(텍스처, 셰이더 등)가 완전히 준비될 때까지 대기
    Step4_CheckResources();
}

// ==================== Step 4: 리소스 스트리밍 체크 ====================

void UYiSanGameInstance::Step4_CheckResources()
{
    UE_LOG(LogTemp, Log, TEXT("[YiSan Step 4] Checking resources..."));

    if (UWorld* World = GetWorld())
    {
        // 주기적으로 리소스 상태 확인
        World->GetTimerManager().SetTimer(
            ResourceCheckTimer,
            [this]()
            {
                bool bTexturesReady = CheckTextureStreaming();
                bool bShadersReady = CheckShaderCompilation();
                bool bWPReady = CheckWorldPartition();

                // 진행률 로그
                UE_LOG(LogTemp, Verbose, TEXT("[YiSan] Resources - Textures: %d, Shaders: %d, WP: %d"),
                    bTexturesReady, bShadersReady, bWPReady);

                if (bTexturesReady && bShadersReady && bWPReady)
                {
                    // 모두 준비 완료!
                    if (UWorld* CurrentWorld = GetWorld())
                    {
                        CurrentWorld->GetTimerManager().ClearTimer(ResourceCheckTimer);
                    }
                    Step5_TransitionToTarget();
                }
            },
            0.1f,  // 0.1초마다 체크
            true   // 반복
        );
    }
}

bool UYiSanGameInstance::CheckTextureStreaming()
{
    // IStreamingManager::Get()을 통해 스트리밍 매니저에 접근합니다.
    IStreamingManager* StreamingManager = &IStreamingManager::Get();
    
    if (StreamingManager)
    {
        // 텍스처 스트리밍 업데이트
        StreamingManager->UpdateResourceStreaming(0.0f, false);
        
        // 모든 리소스 스트리밍 시도
        float StreamingPercentage = StreamingManager->StreamAllResources(0.0f);
        
        // 1.0이면 완료
        return StreamingPercentage >= 1.0f;
    }
    
    // 스트리밍 매니저가 없으면 완료로 간주
    return true;
}

bool UYiSanGameInstance::CheckShaderCompilation()
{
#if WITH_EDITOR
    // 에디터에서만 셰이더 컴파일 체크
    if (GShaderCompilingManager)
    {
        int32 RemainingJobs = GShaderCompilingManager->GetNumRemainingJobs();
        
        if (RemainingJobs > 0)
        {
            UE_LOG(LogTemp, Verbose, TEXT("[YiSan] Waiting for %d shader compilation jobs"), RemainingJobs);
            return false;
        }
    }
#endif
    
    // 패키징된 빌드에서는 항상 true (셰이더가 미리 컴파일됨)
    return true;
}

bool UYiSanGameInstance::CheckWorldPartition()
{
    if (UWorld* World = GetWorld())
    {
        if (UWorldPartitionSubsystem* WPS = World->GetSubsystem<UWorldPartitionSubsystem>())
        {
            // World Partition 스트리밍 완료 여부 확인
            bool bCompleted = WPS->IsStreamingCompleted(nullptr);
            
            if (!bCompleted)
            {
                UE_LOG(LogTemp, Verbose, TEXT("[YiSan] World Partition streaming in progress..."));
            }
            
            return bCompleted;
        }
    }
    
    // World Partition이 없으면 완료로 간주
    return true;
}

// ==================== Step 5: 타겟 레벨로 전환 ====================

void UYiSanGameInstance::Step5_TransitionToTarget()
{
    UE_LOG(LogTemp, Log, TEXT("[YiSan Step 5] All resources ready! Transitioning to target level"));

    if (UWorld* World = GetWorld())
    {
        // 약간의 딜레이 후 전환 (안정화)
        FTimerHandle FinalDelayTimer;
        World->GetTimerManager().SetTimer(
            FinalDelayTimer,
            [this]()
            {
                // 로딩 스크린 숨김
                HideLoadingScreen();

                // 타겟 레벨로 완전 전환
                // 이제 OpenLevel을 사용하여 로딩 레벨을 언로드하고 타겟 레벨만 남김
                UGameplayStatics::OpenLevel(this, TargetLevelName);

                UE_LOG(LogTemp, Log, TEXT("[YiSan] Level transition complete!"));
            },
            0.3f,  // 300ms 후 전환
            false
        );
    }
}

// ==================== UI 관리 ====================

void UYiSanGameInstance::ShowLoadingScreen()
{
    if (LoadingWidgetClass && !LoadingWidget)
    {
        if (UWorld* World = GetWorld())
        {
            LoadingWidget = CreateWidget<UUserWidget>(World, LoadingWidgetClass);
            if (LoadingWidget)
            {
                LoadingWidget->AddToViewport(9999);  // 최상위 레이어
                UE_LOG(LogTemp, Log, TEXT("[YiSan] Loading screen shown"));
            }
        }
    }
}

void UYiSanGameInstance::HideLoadingScreen()
{
    if (LoadingWidget)
    {
        LoadingWidget->RemoveFromParent();
        LoadingWidget = nullptr;
        UE_LOG(LogTemp, Log, TEXT("[YiSan] Loading screen hidden"));
    }
}