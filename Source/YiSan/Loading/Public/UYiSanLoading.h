// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineBaseTypes.h"
#include "Macro.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UYiSanLoading.generated.h"

/**
 * @file UYiSanLoading.h
 * @brief UYiSanLoading 클래스를 선언합니다.
 */


/**
 * @brief 이동 준비, 로딩 UI, 스트리밍 진행 폴링을 관리하는 서브시스템입니다.
 */
UCLASS()
class YISAN_API UYiSanLoading : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    DEFINE_SUBSYSTEM_GETTER_INLINE(UYiSanLoading);

    enum struct EState : uint8
    {
        WP          UMETA(DisplayName = "월드 파티션"),
        TEXTURE     UMETA(DisplayName = "텍스쳐"),
        LI          UMETA(DisplayName = "레벨 인스턴스"),
        COMPLETE    UMETA(DisplayName = "완료"),
        MAX         UMETA(Hidden)
    };
    
    /** @brief 선택적 로딩 화면 오케스트레이션과 함께 서버 이동을 시작합니다. */
    void InitSystem(const FString& InURL, const bool bAbsolute, const bool bUseLoadingScreen = true);
    /** @brief 서버가 트리거한 클라이언트 이동을 준비합니다. */
    void PrepareClientTravel(const FString& InURL, const ETravelType TravelType, const bool bSeamlessTravel);

    // 클라이언트 측 로딩 완료 여부 확인
    /** @brief 추적 중인 모든 로딩 단계가 완료되었는지 여부를 반환합니다. */
    bool IsLoadingComplete() const;

private:
    void PrepareForTravel();
    void PostLoadMapWithWorld(UWorld* InWorld);
    void CompleteProcess(const UWorld* InWorld);
    void UpdateTick();
    void Loading_Textures(const UWorld* InWorld);
    void Loading_LevelInstance(UWorld* InWorld);

    void HandlePostLoadMapSimple(UWorld* World);

#pragma region BROADCAST
    void Broadcast_ShowLoading() const;
    void Broadcast_HideLoading() const;
#pragma endregion   

private:
    double NonLoadingTravelStartTime = 0.0;
    const double TextureStreaming_TimeOut = 60.0;
    const double TextureProgress_LogInterval = 1.0;
        
private:
    double TotalTime = 0.0;

    TMap<EState, bool> CompleteState
    {
        { EState::WP, false },
        { EState::TEXTURE, false },
        { EState::LI, false },
        { EState::COMPLETE, false }
    };
    EState CurState = EState::WP;

    FTimerHandle TimeHandlePool;
    int32 LastPercent = -10;
       
    bool bTextureStreamingComplete = false;
    
    float Progress_Texture = 0.0f;
    float Progress_LI = 0.0f;

    bool bRequestTexture = false;
    int32 TextureRequestCount = 0;
    double LastTextureProgressTime = 0.0;
    float LastTextureProgress = -1.0f;
};
