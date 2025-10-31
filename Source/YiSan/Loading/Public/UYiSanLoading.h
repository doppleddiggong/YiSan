// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineBaseTypes.h"
#include "Macro.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UYiSanLoading.generated.h"


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
    
    void InitSystem(const FString& InURL, bool bAbsolute);
    void PrepareClientTravel(const FString& InURL, ETravelType TravelType, bool bSeamlessTravel);
   
private:
    void PrepareForTravel(const FString& InURL);
    void PostLoadMapWithWorld(UWorld* InWorld);
    void CompleteProcess(const UWorld* InWorld);
    void UpdateTick();
    void Loading_Textures(const UWorld* InWorld);
    void Loading_LevelInstance(UWorld* InWorld);

#pragma region BROADCAST
    void Broadcast_ShowLoading() const;
    void Broadcast_HideLoading() const;
#pragma endregion   

private:
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
