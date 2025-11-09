// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UYiSanLoading.h"

/**
 * @file UYiSanLoading.cpp
 * @brief UYiSanLoading의 동작을 구현합니다.
 */

#include "APlayerControl.h"
#include "AYisanGameState.h"
#include "FComponentHelper.h"
#include "GameLogging.h"

#include "TimerManager.h"
#include "ContentStreaming.h"
#include "ULoadingTransitionManager.h"
#include "UObject/UnrealType.h"

#include "LevelInstance/LevelInstanceActor.h"
#include "LevelInstance/LevelInstanceSubsystem.h"
#include "GameFramework/PlayerController.h"

#include "WorldPartition/WorldPartitionSubsystem.h"

#include "Engine/World.h"
#include "Engine/Engine.h"

/**
 * @file UYiSanLoading.cpp
 * @brief UYiSanLoading의 동작을 구현합니다.
 */


/**
 * @brief 로딩 파이프라인을 초기화하고 이동 프로세스를 시작합니다.
 * @param InURL 이동할 맵의 URL입니다.
 * @param bAbsolute 이동이 절대 경로를 사용하는지 여부입니다.
 * @param bUseLoadingScreen 시네마틱 로딩 플로우를 실행할지 여부입니다.
 */
void UYiSanLoading::InitSystem(const FString& InURL, const bool bAbsolute, const bool bUseLoadingScreen)
{
    PRINTLOG(TEXT("InitSystem(%s, %d, UseLoadingScreen: %s)"), *InURL, bAbsolute, bUseLoadingScreen ? TEXT("true") : TEXT("false"));

    if (UWorld* World = GetWorld())
    {
        if (bUseLoadingScreen)
        {
            PrepareForTravel();
            Broadcast_ShowLoading();
            PRINTLOG(TEXT("[LOADING] 타겟 레벨 로드 시작함 (USE 로딩 화면)"));
            World->ServerTravel(InURL, bAbsolute);
        }
        else
        {
            NonLoadingTravelStartTime = FPlatformTime::Seconds();
            FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UYiSanLoading::HandlePostLoadMapSimple);
            PRINTLOG(TEXT("[LOADING] 타겟 레벨 로드 시작함 (Unuse 로딩 화면)"));
            World->ServerTravel(InURL, bAbsolute);
        }
    }
    else
    {
        PRINTLOG(TEXT("[LOADING] World가 유효하지 않아 ServerTravel을 실행할 수 없음"));
    }
}

/** @brief 고급 로딩 화면 없이 로딩이 완료되었을 때 호출되는 콜백입니다. */
void UYiSanLoading::HandlePostLoadMapSimple(UWorld* World)
{
    const double LoadingTime = FPlatformTime::Seconds() - NonLoadingTravelStartTime;
    PRINTLOG(TEXT("===== 레벨 로드 완료 (로딩 화면 미사용) ====="));
    PRINTLOG(TEXT("총 소요 시간: %.2f초"), LoadingTime);

    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
}

/**
 * @brief 서버가 시작한 이동에 대비해 클라이언트를 준비합니다.
 * @param InURL 이동 대상 맵입니다.
 * @param TravelType 엔진이 설정한 이동 모드입니다.
 * @param bSeamlessTravel 심리스 트래블이 요청되었는지 여부입니다.
 */
void UYiSanLoading::PrepareClientTravel(const FString& InURL, const ETravelType TravelType, const bool bSeamlessTravel)
{
    PRINTLOG(TEXT("PrepareClientTravel(%s, %s, Seamless:%s)"),
        *InURL,
        *ENUM_TO_NAME(ETravelType, TravelType),
        bSeamlessTravel ? TEXT("true") : TEXT("false"));

    PrepareForTravel();
    Broadcast_ShowLoading();
}

/** @brief 진행도 추적기를 초기화하고 로딩 화면을 표시합니다. */
void UYiSanLoading::PrepareForTravel()
{
    PRINTLOG(TEXT("[LOADING_FLOW] PrepareForTravel - Resetting all loading states"));
    TotalTime = FPlatformTime::Seconds();
    
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
    
    if (auto TM = ULoadingTransitionManager::Get(this))
    {
        TM->ShowLoadingScreen();
    }
    
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UYiSanLoading::PostLoadMapWithWorld);
}

/**
 * @brief 맵 로드 완료를 처리하여 스트리밍 상태 폴링을 이어갑니다.
 * @param InWorld 로드된 월드 인스턴스입니다.
 */
void UYiSanLoading::PostLoadMapWithWorld(UWorld* InWorld)
{
    if (!InWorld)
    {
        PRINTLOG(TEXT("[WP] LoadedWorld가 null임."));
        return;
    }

    PRINTLOG(TEXT("[LOADING_FLOW] PostLoadMapWithWorld - Map loaded: %s, NetMode: %s"),
        *InWorld->GetName(),
        InWorld->GetNetMode() == NM_Client ? TEXT("Client") : TEXT("Server/Standalone"));
    PRINTLOG(TEXT("[WP] 맵 로드 완료: %s"), *InWorld->GetName());

    // 게스트 클라이언트가 이미 로드된 맵에 join하는 경우 감지
    // PrepareForTravel이 호출되지 않았다면 TotalTime이 0이거나 매우 작음
    const bool bIsFreshJoin = (TotalTime < KINDA_SMALL_NUMBER);

    if (bIsFreshJoin && InWorld->GetNetMode() == NM_Client)
    {
        PRINTLOG(TEXT("[LOADING_FLOW] Guest client joining already loaded map - skipping loading process"));

        // 게스트 클라이언트는 이미 로드된 맵에 join하므로 즉시 완료 처리
        CompleteState[EState::WP] = true;
        CompleteState[EState::TEXTURE] = true;
        CompleteState[EState::LI] = true;
        CompleteState[EState::COMPLETE] = true;

        Progress_Texture = 1.0f;
        Progress_LI = 1.0f;

        CurState = EState::COMPLETE;

        return; // UpdateTick 시작하지 않음
    }

    // 클라이언트가 자동 travel될 때 (PrepareForTravel 없이), 로딩 상태 초기화
    // 예: 서버 측에서 맵 전환이 발생한 경우
    if (CurState == EState::COMPLETE || CompleteState[EState::COMPLETE])
    {
        PRINTLOG(TEXT("[LOADING_FLOW] PostLoadMapWithWorld - Resetting loading state for new map load"));

        TotalTime = FPlatformTime::Seconds();

        CompleteState[EState::WP] = false;
        CompleteState[EState::TEXTURE] = false;
        CompleteState[EState::LI] = false;
        CompleteState[EState::COMPLETE] = false;

        Progress_Texture = 0.0f;
        Progress_LI = 0.0f;

        bRequestTexture = false;
        TextureRequestCount = 0;
        LastTextureProgressTime = 0.0;
        LastTextureProgress = -1.0f;

        CurState = EState::WP;
    }

    bTextureStreamingComplete = false;

    IStreamingManager::Get().AddLevel(InWorld->PersistentLevel);
    IStreamingManager::Get().NotifyLevelChange();

    PRINTLOG(TEXT("[WP] 텍스처 스트리밍 강제 시작 완료"));

    InWorld->GetTimerManager().ClearTimer(TimeHandlePool);
    InWorld->GetTimerManager().SetTimer(
        TimeHandlePool,
        this,
        &UYiSanLoading::UpdateTick,
        0.1f,
        true);

    PRINTLOG(TEXT("[LOADING_FLOW] PostLoadMapWithWorld - UpdateTick timer started, CurState: %d"), static_cast<int32>(CurState));
}

/** @brief 스트리밍 진행도를 폴링하고 로딩 상태를 전환합니다. */
void UYiSanLoading::UpdateTick()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        PRINTLOG(TEXT("[TICK] World가 null임. 폴링 중단."));
        return;
    }

    PRINTLOG(TEXT("[LOADING_FLOW] UpdateTick - Current State: %d, NetMode: %s"),
        static_cast<int32>(CurState),
        World->GetNetMode() == NM_Client ? TEXT("Client") : TEXT("Server/Standalone"));

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
                PRINTLOG(TEXT("[LOADING_FLOW] [WP] WorldPartition streaming COMPLETE"));
                PRINTLOG(TEXT("[TICK] WorldPartition | 완료"));
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
            PRINTLOG(TEXT("[LOADING_FLOW] [TEXTURE] Texture streaming COMPLETE"));
            PRINTLOG(TEXT("[TICK] 텍스처 스트리밍 | 완료"));
            CurState = EState::LI;
        }
        break;
    }
    case EState::LI:
    {
        Loading_LevelInstance(World);
        if (CompleteState[EState::LI])
        {
            PRINTLOG(TEXT("[LOADING_FLOW] [LI] LevelInstance loading COMPLETE"));
            PRINTLOG(TEXT("[TICK] 레벨 인스턴스 | 완료"));
            CurState = EState::COMPLETE;
       }
        break;
    }
        
    case EState::COMPLETE:
    default:
        break;
    }

    if ( CurState == EState::COMPLETE && ! CompleteState[EState::COMPLETE] )
    {
        // 현재 완료 상태이면서, 완료 도달 상태라면
        CompleteState[EState::COMPLETE] = true;

        PRINTLOG(TEXT("[LOADING_FLOW] ===== ALL LOADING STAGES COMPLETE ====="));
        PRINTLOG(TEXT("[TICK] ===== 모든 준비 완료 ====="));
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
            PRINTLOG(TEXT("[LOADING_FLOW] CompleteProcess - Broadcasting hide loading to all clients"));
            PRINTLOG(TEXT("[COMPLETE] 타겟으로 전환 완료함. 플레이어 입력 활성화."));
            if (UWorld* World = GetWorld())
            {
                Broadcast_HideLoading();

                if (auto GS = World->GetGameState<AYisanGameState>())
                {
                    GS->MulticastRPC_LoadingComplete();
                }
                else
                {
                    Broadcast_HideLoading();
                }
            }
        },
        0.1f, // 전환 시간
        false);
}

/**
 * @brief 텍스처 스트리밍 통계를 폴링해 진행도를 갱신하고 필요 시 타임아웃을 적용합니다.
 * @param InWorld 스트리밍 매니저를 보유한 월드 컨텍스트입니다.
 */
void UYiSanLoading::Loading_Textures(const UWorld* InWorld)
{
    IStreamingManager& StreamingManager = IStreamingManager::Get();
    const double CurTime = FPlatformTime::Seconds();
    const double ElapsedTime = CurTime - TotalTime;

    const int32 PendingRequests = StreamingManager.GetNumWantingResources();
    const bool bStreamingInProgress = (PendingRequests > 0);

    PRINTLOG( TEXT("[Loading] %.2f s elapsed – Pending %d – Streaming %s"),
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
            PRINTLOG(TEXT("[TEXTURE] 텍스처 스트리밍 완료 (소요 시간: %.2f초)"), ElapsedTime);
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
            PRINTLOG(TEXT("[TEXTURE] 텍스처 스트리밍 진행 중 (대기: %d개, 진행률: %.1f%%)"), PendingRequests, Progress_Texture * 100.0f);
            LastTextureProgressTime = CurTime;
            LastTextureProgress = Progress_Texture;
        }
    }

    if (!CompleteState[EState::TEXTURE] && ElapsedTime > TextureStreaming_TimeOut)
    {
        PRINTLOG(TEXT("[TEXTURE] 텍스처 스트리밍 타임아웃 (%.0f초 초과). 남은 대기 텍스처: %d개"), TextureStreaming_TimeOut, PendingRequests);
        Progress_Texture = 1.0f;
        CompleteState[EState::TEXTURE] = true;
    }
}

/** @brief 레벨 인스턴스 스트리밍 진행도를 모니터링하고 상태를 전환합니다. */
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

#pragma region BROADCAST
void UYiSanLoading::Broadcast_ShowLoading() const
{
    PRINTLOG(TEXT("[LOADING_FLOW] Broadcast_ShowLoading called"));
    if (UWorld* World = GetWorld())
    {
        if (World->GetNetMode() == NM_Client)
        {
            if (auto LocalPC = Cast<APlayerControl>(World->GetFirstPlayerController()))
            {
                LocalPC->ShowLoadingScreenLocal();
            }
            return;
        }
        
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
    PRINTLOG(TEXT("[LOADING_FLOW] Broadcast_HideLoading called"));
    if (UWorld* World = GetWorld())
    {
        PRINTLOG(TEXT("[LOADING_FLOW] NetMode: %s"),
            World->GetNetMode() == NM_Client ? TEXT("Client") : TEXT("Server/Standalone"));

        if (World->GetNetMode() == NM_Client)
        {
            PRINTLOG(TEXT("[LOADING_FLOW] Client mode - calling HandleLoadingComplete on local PC"));
            if (auto LocalPC = Cast<APlayerControl>(World->GetFirstPlayerController()))
            {
                LocalPC->HandleLoadingComplete();
            }
            return;
        }

        PRINTLOG(TEXT("[LOADING_FLOW] Server mode - sending ClientRPC_HideLoadingTransition to all clients"));
        for (auto it = World->GetPlayerControllerIterator(); it; ++it)
        {
            if (auto pc = Cast<APlayerControl>(it->Get()))
            {
                PRINTLOG(TEXT("[LOADING_FLOW] Sending RPC to %s"), *GetNameSafe(pc));
                pc->ClientRPC_HideLoadingTransition();
            }
        }
    }
}

bool UYiSanLoading::IsLoadingComplete() const
{
    return CompleteState[EState::COMPLETE];
}
#pragma endregion