// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UYiSanLoading.h"

#include "APlayerControl.h"
#include "FComponentHelper.h"
#include "GameLogging.h"

#include "TimerManager.h"
#include "ContentStreaming.h"
#include "ULoadingTransitionManager.h"

#include "LevelInstance/LevelInstanceActor.h"
#include "LevelInstance/LevelInstanceSubsystem.h"
#include "GameFramework/PlayerController.h"

#include "WorldPartition/WorldPartitionSubsystem.h"

#include "Engine/World.h"
#include "Engine/Engine.h"

static float WorldWeight = 0.15f;
static float TextureWeight = 0.70f;
static float LevelWeight = 0.15f;


void UYiSanLoading::InitSystem(const FString& InURL, bool bAbsolute)
{
    PRINTLOG(TEXT("UYiSanLoading::InitSystem(%s, %d)"), *InURL, bAbsolute);

    // 기본 데이터 초기화
    TotalTime = FPlatformTime::Seconds();
    TotalProgress = 0.0f;

    bTextureStreamingComplete = false;

    CompleteState[EState::WP] = false;
    CompleteState[EState::TEXTURE] = false;
    CompleteState[EState::LI] =  false;
    CompleteState[EState::COMPLETE] =  false;

    Progress_Texture = 0.0f;
    Progress_LI = 0.0f;

    bRequestTexture = false;
    TextureRequestCount = 0;
    LastTextureProgressTime = 0.0;
    LastTextureProgress = -1.0f;

    LastPercent = -10;
    CurState = EState::WP;

    Broadcast_ShowLoading();
    if (auto TM = ULoadingTransitionManager::Get(this))
    {
        TM->ShowLoadingScreen();
        TM->UpdateLoadingProgress(0.0f);
    }

    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UYiSanLoading::PostLoadMapWithWorld);

    PRINTLOG(TEXT("[스텝1] 타겟 레벨 로드 시작함"));
    GetWorld()->ServerTravel(InURL, bAbsolute);
}

void UYiSanLoading::PostLoadMapWithWorld(UWorld* InWorld)
{
    if (!InWorld)
    {
        PRINTLOG(TEXT("[스텝2] LoadedWorld가 null임."));
        return;
    }

    PRINTLOG(TEXT("[스텝2] 맵 로드 완료: %s"), *InWorld->GetName());


    bTextureStreamingComplete = false;

    IStreamingManager::Get().AddLevel(InWorld->PersistentLevel);
    IStreamingManager::Get().NotifyLevelChange();

    PRINTLOG(TEXT("[스텝2] 텍스처 스트리밍 강제 시작 완료"));

    InWorld->GetTimerManager().SetTimer(
        TimeHandlePool,
        this,
        &UYiSanLoading::UpdateTick,
        0.1f,
        true);
}

void UYiSanLoading::UpdateTick()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        PRINTLOG(TEXT("[폴링] World가 null임. 폴링 중단."));
        return;
    }

    switch (CurState)
    {
    case EState::WP:
    {
        if (!CompleteState[EState::WP] )
        {
            if (auto WPS = World->GetSubsystem<UWorldPartitionSubsystem>())
                CompleteState[EState::WP] = WPS->IsStreamingCompleted();
            else
                CompleteState[EState::WP] = true;

            if (CompleteState[EState::WP])
            {
                PRINTLOG(TEXT("[폴링] WorldPartition | 완료"));
                CurState = EState::TEXTURE;
                
                bRequestTexture = false;
                TextureRequestCount = 0;
                LastTextureProgressTime = 0.0;
                LastTextureProgress = -1.0f;
            }
        }
        break;
    }
    case EState::TEXTURE:
    {
        Loading_Textures(World);
        if (CompleteState[EState::TEXTURE])
        {
            PRINTLOG(TEXT("[폴링] 텍스처 스트리밍 | 완료"));
            CurState = EState::LI;
        }
        break;
    }
    case EState::LI:
    {
        Loading_LevelInstance(World);
        if (CompleteState[EState::LI])
        {
            PRINTLOG(TEXT("[폴링] 레벨 인스턴스 | 완료"));
            CurState = EState::COMPLETE;
       }
        break;
    }
        
    case EState::COMPLETE:
    default:
        break;
    }

    TotalProgress = GetTotalProgress();
    UpdateLoadingProgress();

    if ( CurState == EState::COMPLETE && ! CompleteState[EState::COMPLETE] )
    {
        // 현재 완료 상태이면서, 완료 도달 상태라면
        CompleteState[EState::COMPLETE] = true;

        PRINTLOG(TEXT("[폴링] ===== 모든 준비 완료 ====="));
        PRINTLOG(TEXT("WorldPartition: OK"));
        PRINTLOG(TEXT("Texture: %.1f%% (완료)"), Progress_Texture * 100.0f);
        PRINTLOG(TEXT("LevelInstance: %.0f%% (완료)"), Progress_LI * 100.0f);
        PRINTLOG(TEXT("총 소요 시간: %.2f초"), FPlatformTime::Seconds() - TotalTime);

        CompleteProcess(World);
    }
}

void UYiSanLoading::CompleteProcess(const UWorld* InWorld)
{
    InWorld->GetTimerManager().ClearTimer(TimeHandlePool);

    FTimerHandle DelayHandle;
    
    InWorld->GetTimerManager().SetTimer(
        DelayHandle,
        [this]()
        {
            PRINTLOG(TEXT("[스텝3] 타겟으로 전환 완료함. 플레이어 입력 활성화."));
            if (UWorld* World = GetWorld())
            {
                Broadcast_HideLoading();

                if (auto TM = ULoadingTransitionManager::Get(this))
                {
                    TM->HideLoadingScreen();
                }

                if (auto pc = World->GetFirstPlayerController())
                {
                    pc->SetInputMode(FInputModeGameOnly());
                    pc->bShowMouseCursor = false;

                    PRINTLOG(TEXT("LOADING COMPLTE::GAME START."));
                }
            }
        },
        0.1f, // 전환 시간
        false);
}

// 텍스처 스트리밍 상태를 폴링하여 진행률을 추적한다.
// - `GetNumWantingResources` 값이 감소하는 속도를 기반으로 선형 진행률을 계산한다.
// - 엔진의 기본 스트리밍 스레드를 존중하기 위해 `StreamAllResources` 호출은 제거했다.
// - 일정 시간 동안 완료되지 않을 경우, 타임아웃을 적용해 플레이어 진행을 보장한다.
void UYiSanLoading::Loading_Textures(const UWorld* InWorld)
{
    IStreamingManager& StreamingManager = IStreamingManager::Get();
    const double CurTime = FPlatformTime::Seconds();
    const double ElapsedTime = CurTime - TotalTime;

    const int32 PendingRequests = StreamingManager.GetNumWantingResources();
    const bool bStreamingInProgress = (PendingRequests > 0);

    UE_LOG(LogTemp, Log, TEXT("[Loading] %.2f s elapsed – Pending %d – Streaming %s"),
        ElapsedTime, PendingRequests, bStreamingInProgress ? TEXT("InProgress") : TEXT("Done"));

    if (!bRequestTexture)
    {
        bRequestTexture = true;
        
        TextureRequestCount = PendingRequests;
        LastTextureProgressTime = CurTime;
    }

    if (!bStreamingInProgress && PendingRequests == 0)
    {
        Progress_Texture = 1.0f;
        CompleteState[EState::TEXTURE] = true;

        if (!bTextureStreamingComplete)
        {
            PRINTLOG(TEXT("[폴링] 텍스처 스트리밍 완료 (소요 시간: %.2f초)"), ElapsedTime);
            bTextureStreamingComplete = true;
        }

        LastTextureProgress = Progress_Texture;
        return;
    }

    CompleteState[EState::COMPLETE] = false;

    const int32 RequestCount = FMath::Max(TextureRequestCount, 1);
    const float PendingRatio = FMath::Clamp(static_cast<float>(PendingRequests) / static_cast<float>(RequestCount), 0.0f, 1.0f);
    Progress_Texture = 1.0f - PendingRatio;

    if (Progress_Texture < LastTextureProgress - KINDA_SMALL_NUMBER ||
        Progress_Texture > LastTextureProgress + KINDA_SMALL_NUMBER)
    {
        const double TimeSinceLastLog = CurTime - LastTextureProgressTime;
        if (TimeSinceLastLog >= TextureProgress_LogInterval)
        {
            PRINTLOG(TEXT("[폴링] 텍스처 스트리밍 진행 중 (대기: %d개, 진행률: %.1f%%)"), PendingRequests, Progress_Texture * 100.0f);
            LastTextureProgressTime = CurTime;
            LastTextureProgress = Progress_Texture;
        }
    }

    if (!CompleteState[EState::TEXTURE] && ElapsedTime > TextureStreaming_TimeOut)
    {
        PRINTLOG(TEXT("[폴링] 텍스처 스트리밍 타임아웃 (%.0f초 초과). 남은 대기 텍스처: %d개"), TextureStreaming_TimeOut, PendingRequests);
        Progress_Texture = 1.0f;
        CompleteState[EState::TEXTURE] = true;
    }
}

void UYiSanLoading::Loading_LevelInstance(UWorld* InWorld)
{
    if (auto LIS = InWorld->GetSubsystem<ULevelInstanceSubsystem>())
    {
        auto Found = FComponentHelper::GetAllOfClass<ALevelInstance>(InWorld);

        if (Found.Num() == 0)
        {
            Progress_LI = 1.0f;
            CompleteState[EState::LI] = true;
            return;
        }

        int32 ReadyCount = 0;
        for (auto LevelInstance : Found)
        {
            if (LevelInstance && LevelInstance->GetLoadedLevel() != nullptr)
                ++ReadyCount;
        }

        Progress_LI = Found.Num() > 0 ? static_cast<float>(ReadyCount) / static_cast<float>(Found.Num()) : 1.0f;
        CompleteState[EState::LI] = (ReadyCount == Found.Num());
    }
    else
    {
        CompleteState[EState::LI] = true;
        Progress_LI = 1.0f;
    }
}

float UYiSanLoading::GetTotalProgress() const
{
    const float WorldPartitionProgress = CompleteState[EState::WP] ? 1.0f : 0.0f;
    const float tmpTextureProgress = FMath::Clamp(Progress_Texture, 0.0f, 1.0f);

    return (WorldPartitionProgress * WorldWeight) + (tmpTextureProgress * TextureWeight) + (Progress_LI * LevelWeight);
}

void UYiSanLoading::UpdateLoadingProgress()
{
    const int32 Percent = FMath::Clamp(FMath::FloorToInt(TotalProgress * 10.0f) * 10, 0, 100);
    if (Percent != LastPercent)
    {
        PRINTLOG(TEXT("PROGRESS : %d"), Percent);
        const float NormalizedProgress = static_cast<float>(Percent) / 100.0f;

        if (auto TM = ULoadingTransitionManager::Get(this))
            TM->UpdateLoadingProgress(NormalizedProgress);

        Broadcast_UpdateLoadingProgress(NormalizedProgress);
        LastPercent = Percent;
    }
}

#pragma region BROADCAST
void UYiSanLoading::Broadcast_ShowLoading() const
{
    if (UWorld* World = GetWorld())
    {
        for (auto It = World->GetPlayerControllerIterator(); It; ++It)
        {
            if (auto pc = Cast<APlayerControl>(It->Get()))
            {
                pc->ClientRPC_ShowLoadingTransition();
            }
        }
    }
}

void UYiSanLoading::Broadcast_HideLoading() const
{
    if (UWorld* World = GetWorld())
    {
        for (auto it = World->GetPlayerControllerIterator(); it; ++it)
        {
            if (auto pc = Cast<APlayerControl>(it->Get()))
            {
                pc->ClientRPC_HideLoadingTransition();
            }
        }
    }
}

void UYiSanLoading::Broadcast_UpdateLoadingProgress(const float Progress) const
{
    if (UWorld* World = GetWorld())
    {
        for (auto it = World->GetPlayerControllerIterator(); it; ++it)
        {
            if (auto pc = Cast<APlayerControl>(it->Get()))
            {
                pc->ClientRPC_UpdateLoadingTransitionProgress(Progress);
            }
        }
    }
}
#pragma endregion