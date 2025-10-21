// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "YiSan/Loading/Public/YiSanGameInstance.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "ContentStreaming.h"
#include "GameLogging.h"
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
    //이미 로딩 중이면 새로운 요청을 무시합니다. (무한 루프 방지)
    if (bIsLoadingLevel)
    {
        UE_LOG(LogTemp, Warning, TEXT("[YiSan] Already in loading process. Ignoring new request to load: %s"), *InTargetLevelName.ToString());
        return;
    }

    if (InTargetLevelName.IsNone())
    {
        UE_LOG(LogTemp, Error, TEXT("[YiSan] Invalid target level name!"));
        return;
    }

    //로딩 시작 플래그 설정
    bIsLoadingLevel = true;
    TargetLevelName = InTargetLevelName;
    PRINTLOG(TEXT("레벨로 이동: %s"), *TargetLevelName.ToString());

    // step1 으로 이동한다
    Step1_MoveToLoadingLevel();
    
}



// ==================== Step 1: 로딩 레벨로 이동 ====================

void UYiSanGameInstance::Step1_MoveToLoadingLevel()
{
    UE_LOG(LogTemp, Warning, TEXT("[YiSan Step 1] Moving to loading level"));

    // 로딩 스크린 표시
    ShowLoadingScreen();
    //step 1 에서 레벨 넘기기만 하기
    UGameplayStatics::OpenLevel(this, FName("/Game/CustomContents/Maps/LoadingMap"));
}

void UYiSanGameInstance::OnLoadingMapReady()
{
    PRINTLOG(TEXT("로딩 안정화"));
    if (UWorld* World = GetWorld())
    {
        // 안정화 용
        FTimerHandle TimerHandle;
        GetTimerManager().SetTimer(TimerHandle,this,&UYiSanGameInstance::Step2_StartLoadingTargetLevel,0.5,false);
    }
}

// ==================== Step 2: 타겟 레벨 로드 시작 ====================

void UYiSanGameInstance::Step2_StartLoadingTargetLevel()
{
    UE_LOG(LogTemp, Warning, TEXT("[YiSan Step 2] Starting to load target level: %s"), *TargetLevelName.ToString());
    
    UGameplayStatics::OpenLevel(this, TargetLevelName);
}


void UYiSanGameInstance::OnTargetLevelReady()
{
    Step3_OnLevelLoaded();
}


// ==================== Step 3: 레벨 로드 완료 ====================

void UYiSanGameInstance::Step3_OnLevelLoaded()
{
    PRINTLOG(TEXT("메모리 로딩"));
    bLevelLoaded = true;
    // 이제 리소스(텍스처, 셰이더 등)가 완전히 준비될 때까지 대기
    Step4_CheckResources();
}

// ==================== Step 4: 리소스 스트리밍 체크 ====================

void UYiSanGameInstance::Step4_CheckResources()
{
    PRINTLOG(TEXT("리소스 체크중"));
    if (UWorld* World = GetWorld())
    {
        GetTimerManager().SetTimer(ResourceCheckTimer,this,&UYiSanGameInstance::PeriodicResourceCheck,0.1f,true);
    }
}

void UYiSanGameInstance::PeriodicResourceCheck()
{
    bool bTextureReady = CheckTextureStreaming();
    bool bShaderReady = CheckShaderCompilation();
    bool worldPartitionReady = CheckWorldPartition();
    if (bTextureReady && bShaderReady && worldPartitionReady)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(ResourceCheckTimer);
        }
        Step5_TransitionToTarget();
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
            PRINTLOG(TEXT(" 쉐이더 %d 다 됐습니다"), RemainingJobs);
            return false;
        }
    }
#endif
    // 패키징된 빌드에서는 항상 true 되어있어야 한다
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
                PRINTLOG(TEXT("월드 파티션쪽도 거의다 끝났슴다"));
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
        World->GetTimerManager().SetTimer(FinalDelayTimer,this,&UYiSanGameInstance::FinalHideLoadingScreen,0.3,false); 
    }
}
void UYiSanGameInstance::FinalHideLoadingScreen()
{
    HideLoadingScreen();
    PRINTLOG(TEXT("레벨 트랜지션 끝났습니다"));
    // 빠져 나옵니다
    bIsLoadingLevel = false;
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
                UE_LOG(LogTemp, Warning, TEXT("[YiSan] Loading screen shown"));
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
        UE_LOG(LogTemp, Warning, TEXT("[YiSan] Loading screen hidden"));
    }
}

