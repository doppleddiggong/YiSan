// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UYiSanLoading.h"

#include "APlayerControl.h"
#include "FComponentHelper.h"
#include "GameLogging.h"
#include "UDialogManager.h"

#include "TimerManager.h"
#include "ContentStreaming.h"
#include "ULoadingTransitionManager.h"

#include "LevelInstance/LevelInstanceActor.h"
#include "LevelInstance/LevelInstanceSubsystem.h"
#include "GameFramework/PlayerController.h"

#include "WorldPartition/WorldPartitionSubsystem.h"

#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

void UYiSanLoading::InitSystem(const FString& InURL, bool bAbsolute)
{
    PRINTLOG(TEXT("로딩 레벨 매니저: Step1 로딩 시작 호출함"));

    ResetLoadingState();

    DM = UDialogManager::Get(this);

    BroadcastLoadingScreenShow();

    if (ULoadingTransitionManager* TransitionManager = ULoadingTransitionManager::Get(this))
    {
        TransitionManager->ShowLoadingScreen();
        TransitionManager->UpdateLoadingProgress(0.0f);
    }

    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UYiSanLoading::PostLoadMapWithWorld);

    PRINTLOG(TEXT("[스텝1] 타겟 레벨 로드 시작함"));
    GetWorld()->ServerTravel(InURL, bAbsolute);
}

void UYiSanLoading::PostLoadMapWithWorld(UWorld* World)
{
    if (!World)
    {
        PRINTLOG(TEXT("[스텝2] LoadedWorld가 null임."));
        return;
    }

    PRINTLOG(TEXT("[스텝2] 맵 로드 완료: %s"), *World->GetName());
    DM->ShowToast(FString::Printf(TEXT("[스텝2] 맵 로드 완료: %s"), *World->GetName()));

    ResourceCheckStartTime = World->GetTimeSeconds();
    bInitialTextureStreamingComplete = false;

    MarkStageStart(ELoadingSequenceStage::WorldPartition, ResourceCheckStartTime);

    IStreamingManager::Get().AddLevel(World->PersistentLevel);
    IStreamingManager::Get().NotifyLevelChange();

    PRINTLOG(TEXT("[스텝2] 텍스처 스트리밍 강제 시작 완료"));
    DM->ShowToast(FString::Printf(TEXT("[스텝2] 텍스처 스트리밍 강제 시작 완료")));

    World->GetTimerManager().SetTimer(
        TimeHandlePool,
        this,
        &UYiSanLoading::Poll_StreamingAndInstancesReady,
        0.1f,
        true);
}

void UYiSanLoading::ResetLoadingState()
{
    ResourceCheckStartTime = 0.0;
    TotalProgress = 0.0f;

    bInitialTextureStreamingComplete = false;
    bWorldPartitionReady = false;
    bTextureReady = false;
    bLevelInstancesReady = false;
    bCompletionAnnounced = false;

    StreamingPercentage = 0.0f;
    LevelInstanceProgress = 0.0f;

    bCapturedInitialTextureRequests = false;
    InitialTextureRequestCount = 0;
    LastTextureProgressLogTime = 0.0;
    LastReportedTextureProgress = -1.0f;

    LastReportedPercent = -10;
    CurrentStage = ELoadingSequenceStage::WorldPartition;

    for (FStageTiming& StageTiming : StageTimings)
    {
        StageTiming = FStageTiming();
    }
}

void UYiSanLoading::Poll_StreamingAndInstancesReady()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        PRINTLOG(TEXT("[폴링] World가 null임. 폴링 중단."));
        return;
    }

    switch (CurrentStage)
    {
    case ELoadingSequenceStage::WorldPartition:
    {
        if (!bWorldPartitionReady)
        {
            if (UWorldPartitionSubsystem* WPS = World->GetSubsystem<UWorldPartitionSubsystem>())
            {
                bWorldPartitionReady = WPS->IsStreamingCompleted();
            }
            else
            {
                bWorldPartitionReady = true;
            }

            if (bWorldPartitionReady)
            {
                PRINTLOG(TEXT("[폴링] WorldPartition 스트리밍 완료"));
                AdvanceToStage(ELoadingSequenceStage::Texture);
            }
        }
        break;
    }
    case ELoadingSequenceStage::Texture:
    {
        Loading_Textures(World);
        if (bTextureReady)
        {
            PRINTLOG(TEXT("[폴링] 텍스처 스트리밍 단계 완료"));
            AdvanceToStage(ELoadingSequenceStage::LevelInstances);
        }
        break;
    }
    case ELoadingSequenceStage::LevelInstances:
    {
        Loading_LevelInstance(World);
        if (bLevelInstancesReady)
        {
            AdvanceToStage(ELoadingSequenceStage::Completed);
        }
        break;
    }
    case ELoadingSequenceStage::Completed:
        break;
    }

    TotalProgress = CalculateTotalProgress();
    UpdateLoadingProgress();

    if (CurrentStage == ELoadingSequenceStage::Completed && !bCompletionAnnounced)
    {
        bCompletionAnnounced = true;
        LogStageSummary(World);
        PRINTLOG(TEXT("[폴링] ===== 모든 준비 완료 ====="));
        PRINTLOG(TEXT("WorldPartition: OK"));
        PRINTLOG(TEXT("Texture: %.1f%% (완료)"), StreamingPercentage * 100.0f);
        PRINTLOG(TEXT("LevelInstance: %.0f%% (완료)"), LevelInstanceProgress * 100.0f);
        PRINTLOG(TEXT("총 소요 시간: %.2f초"), World->GetTimeSeconds() - ResourceCheckStartTime);

        CompleteProcess(World);
    }
}

void UYiSanLoading::CompleteProcess(const UWorld* World)
{
    World->GetTimerManager().ClearTimer(TimeHandlePool);

    FTimerHandle DelayHandle;
    World->GetTimerManager().SetTimer(
        DelayHandle,
        [this]()
        {
            PRINTLOG(TEXT("[스텝3] 타겟으로 전환 완료함. 플레이어 입력 활성화."));
            DM->ShowToast(FString::Printf(TEXT("[스텝3] 타겟으로 전환 완료함. 플레이어 입력 활성화.")));

            if (UWorld* World = GetWorld())
            {
                BroadcastLoadingScreenHide();

                if (ULoadingTransitionManager* TransitionManager = ULoadingTransitionManager::Get(this))
                {
                    TransitionManager->HideLoadingScreen();
                }

                if (APlayerController* PC = World->GetFirstPlayerController())
                {
                    PC->SetInputMode(FInputModeGameOnly());
                    PC->bShowMouseCursor = false;

                    DM->ShowToast(FString::Printf(TEXT("GAME START!!!")));
                }
            }
        },
        0.1f,
        false);
}

// 텍스처 스트리밍 상태를 폴링하여 진행률을 추적한다.
// - `GetNumWantingResources` 값이 감소하는 속도를 기반으로 선형 진행률을 계산한다.
// - 엔진의 기본 스트리밍 스레드를 존중하기 위해 `StreamAllResources` 호출은 제거했다.
// - 일정 시간 동안 완료되지 않을 경우, 타임아웃을 적용해 플레이어 진행을 보장한다.
void UYiSanLoading::Loading_Textures(const UWorld* World)
{
    IStreamingManager& StreamingManager = IStreamingManager::Get();
    const double CurrentTime = World->GetTimeSeconds();
    const double ElapsedTime = CurrentTime - ResourceCheckStartTime;

    const int32 PendingRequests = StreamingManager.GetNumWantingResources();
    const bool bStreamingInProgress = (PendingRequests > 0);

    UE_LOG(LogTemp, Log, TEXT("[Loading] %.2f s elapsed – Pending %d – Streaming %s"),
        ElapsedTime, PendingRequests, bStreamingInProgress ? TEXT("InProgress") : TEXT("Done"));


    if (!bCapturedInitialTextureRequests)
    {
        bCapturedInitialTextureRequests = true;
        InitialTextureRequestCount = PendingRequests;
        LastTextureProgressLogTime = CurrentTime;
    }

    if (!bStreamingInProgress && PendingRequests == 0)
    {
        StreamingPercentage = 1.0f;
        bTextureReady = true;

        if (!bInitialTextureStreamingComplete)
        {
            PRINTLOG(TEXT("[폴링] 텍스처 스트리밍 완료 (소요 시간: %.2f초)"), ElapsedTime);
            bInitialTextureStreamingComplete = true;
        }

        LastReportedTextureProgress = StreamingPercentage;
        return;
    }

    bTextureReady = false;

    const int32 EffectiveInitialCount = FMath::Max(InitialTextureRequestCount, 1);
    const float PendingRatio = FMath::Clamp(static_cast<float>(PendingRequests) / static_cast<float>(EffectiveInitialCount), 0.0f, 1.0f);
    StreamingPercentage = 1.0f - PendingRatio;

    if (StreamingPercentage < LastReportedTextureProgress - KINDA_SMALL_NUMBER ||
        StreamingPercentage > LastReportedTextureProgress + KINDA_SMALL_NUMBER)
    {
        const double TimeSinceLastLog = CurrentTime - LastTextureProgressLogTime;
        if (TimeSinceLastLog >= TextureProgressLogIntervalSeconds)
        {
            PRINTLOG(TEXT("[폴링] 텍스처 스트리밍 진행 중 (대기: %d개, 진행률: %.1f%%)"), PendingRequests, StreamingPercentage * 100.0f);
            LastTextureProgressLogTime = CurrentTime;
            LastReportedTextureProgress = StreamingPercentage;
        }
    }

    if (!bTextureReady && ElapsedTime > TextureStreamingTimeoutSeconds)
    {
        PRINTLOG(TEXT("[폴링] 텍스처 스트리밍 타임아웃 (%.0f초 초과). 남은 대기 텍스처: %d개"), TextureStreamingTimeoutSeconds, PendingRequests);
        StreamingPercentage = 1.0f;
        bTextureReady = true;
    }
}

void UYiSanLoading::Loading_LevelInstance(UWorld* World)
{
    if (World->GetSubsystem<ULevelInstanceSubsystem>())
    {
        const TArray<ALevelInstance*> Found = FComponentHelper::GetAllOfClass<ALevelInstance>(World);
        if (Found.Num() == 0)
        {
            LevelInstanceProgress = 1.0f;
            bLevelInstancesReady = true;
            return;
        }

        int32 ReadyCount = 0;
        for (ALevelInstance* LevelInstance : Found)
        {
            if (LevelInstance && LevelInstance->GetLoadedLevel() != nullptr)
            {
                ++ReadyCount;
            }
        }

        LevelInstanceProgress = Found.Num() > 0 ? static_cast<float>(ReadyCount) / static_cast<float>(Found.Num()) : 1.0f;
        bLevelInstancesReady = (ReadyCount == Found.Num());
    }
    else
    {
        bLevelInstancesReady = true;
        LevelInstanceProgress = 1.0f;
    }
}

void UYiSanLoading::UpdateLoadingProgress()
{
    const int32 CurrentPercent = FMath::Clamp(FMath::FloorToInt(TotalProgress * 10.0f) * 10, 0, 100);
    if (CurrentPercent != LastReportedPercent)
    {
        PRINTLOG(TEXT("PROGRESS : %d"), CurrentPercent);
        const float NormalizedProgress = static_cast<float>(CurrentPercent) / 100.0f;

        if (ULoadingTransitionManager* TransitionManager = ULoadingTransitionManager::Get(this))
        {
            TransitionManager->UpdateLoadingProgress(NormalizedProgress);
        }

        BroadcastLoadingProgress(NormalizedProgress);
        LastReportedPercent = CurrentPercent;
    }
}

float UYiSanLoading::CalculateTotalProgress() const
{
    const float WorldPartitionProgress = bWorldPartitionReady ? 1.0f : 0.0f;
    const float TextureProgress = FMath::Clamp(StreamingPercentage, 0.0f, 1.0f);

    return (WorldPartitionProgress * WorldPartitionWeight) +
           (TextureProgress * TextureWeight) +
           (LevelInstanceProgress * LevelInstanceWeight);
}

void UYiSanLoading::AdvanceToStage(const ELoadingSequenceStage NextStage)
{
    if (CurrentStage == NextStage)
    {
        return;
    }

    const UWorld* World = GetWorld();
    const double TimeSeconds = World ? World->GetTimeSeconds() : 0.0;

    MarkStageComplete(CurrentStage, TimeSeconds);

    CurrentStage = NextStage;

    if (CurrentStage == ELoadingSequenceStage::Texture)
    {
        bCapturedInitialTextureRequests = false;
        InitialTextureRequestCount = 0;
        LastTextureProgressLogTime = 0.0;
        LastReportedTextureProgress = -1.0f;
    }

    if (CurrentStage != ELoadingSequenceStage::Completed)
    {
        MarkStageStart(CurrentStage, TimeSeconds);
    }
    else
    {
        MarkStageComplete(CurrentStage, TimeSeconds);
    }
}

void UYiSanLoading::MarkStageStart(const ELoadingSequenceStage Stage, const double TimeSeconds)
{
    const int32 StageIndex = static_cast<int32>(Stage);
    if (!StageTimings[StageIndex].bStarted)
    {
        StageTimings[StageIndex].bStarted = true;
        StageTimings[StageIndex].StartTime = TimeSeconds;

        const double RelativeTime = TimeSeconds - ResourceCheckStartTime;
        PRINTLOG(TEXT("[타이밍] (Legacy 비교) %s 단계 시작 (T+%.2f초)"), GetStageLabel(Stage), RelativeTime);
    }
}

void UYiSanLoading::MarkStageComplete(const ELoadingSequenceStage Stage, const double TimeSeconds)
{
    const int32 StageIndex = static_cast<int32>(Stage);
    FStageTiming& Timing = StageTimings[StageIndex];
    if (Timing.bStarted && !Timing.bCompleted)
    {
        Timing.EndTime = TimeSeconds;
        Timing.bCompleted = true;

        const double StageDuration = Timing.GetDuration();
        const double Accumulated = TimeSeconds - ResourceCheckStartTime;
        PRINTLOG(TEXT("[타이밍] (Legacy 비교) %s 단계 완료 (단계 소요: %.2f초, 누적: %.2f초)"), GetStageLabel(Stage), StageDuration, Accumulated);
    }
}

void UYiSanLoading::LogStageSummary(const UWorld* World) const
{
    if (!World)
    {
        return;
    }

    PRINTLOG(TEXT("[타이밍] (Legacy 비교) 단계별 소요 시간 요약"));
    for (int32 StageIndex = 0; StageIndex < StageCount; ++StageIndex)
    {
        const FStageTiming& Timing = StageTimings[StageIndex];
        if (!Timing.bStarted)
        {
            continue;
        }

        const double Duration = Timing.GetDuration();
        const double StartOffset = Timing.StartTime - ResourceCheckStartTime;
        const double EndOffset = Timing.bCompleted ? Timing.EndTime - ResourceCheckStartTime : StartOffset;

        PRINTLOG(TEXT("[타이밍]  - %s | 시작: T+%.2f초 | 종료: T+%.2f초 | 단계 소요: %.2f초"),
            GetStageLabel(static_cast<ELoadingSequenceStage>(StageIndex)),
            StartOffset,
            EndOffset,
            Duration);
    }
}

const TCHAR* UYiSanLoading::GetStageLabel(const ELoadingSequenceStage Stage) const
{
    switch (Stage)
    {
    case ELoadingSequenceStage::WorldPartition:
        return TEXT("WorldPartition");
    case ELoadingSequenceStage::Texture:
        return TEXT("TextureStreaming");
    case ELoadingSequenceStage::LevelInstances:
        return TEXT("LevelInstance");
    case ELoadingSequenceStage::Completed:
        return TEXT("Completed");
    default:
        return TEXT("Unknown");
    }
}

void UYiSanLoading::BroadcastLoadingScreenShow() const
{
    if (UWorld* World = GetWorld())
    {
        for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
        {
            if (APlayerControl* PlayerControl = Cast<APlayerControl>(It->Get()))
            {
                PlayerControl->ClientRPC_ShowLoadingTransition();
            }
        }
    }
}

void UYiSanLoading::BroadcastLoadingScreenHide() const
{
    if (UWorld* World = GetWorld())
    {
        for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
        {
            if (APlayerControl* PlayerControl = Cast<APlayerControl>(It->Get()))
            {
                PlayerControl->ClientRPC_HideLoadingTransition();
            }
        }
    }
}

void UYiSanLoading::BroadcastLoadingProgress(const float Progress) const
{
    if (UWorld* World = GetWorld())
    {
        for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
        {
            if (APlayerControl* PlayerControl = Cast<APlayerControl>(It->Get()))
            {
                PlayerControl->ClientRPC_UpdateLoadingTransitionProgress(Progress);
            }
        }
    }
}