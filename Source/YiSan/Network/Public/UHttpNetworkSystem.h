// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UHttpNetworkSystem.h
 * @brief UHttpNetworkSystem 선언에 대한 Doxygen 주석을 제공합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "NetworkData.h"
#include "ENetworkLogType.h"
#include "Macro.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UHttpNetworkSystem.generated.h"

UCLASS()
class YISAN_API UHttpNetworkSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    DEFINE_SUBSYSTEM_GETTER_INLINE(UHttpNetworkSystem);
    
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void RequestHealth( FResponseHealthDelegate InDelegate);
    void RequestTestSTT(const FString& FilePath, FResponseTestSTTDelegate InDelegate);

    void RequestTestTTS(const FString& Text, const float SpeakingRate, const float Pitch, FResponseTestTTSDelegate InDelegate);
    void RequestTestGPT(const FString& Text, FResponseTestGPTDelegate InDelegate);

private:
    static void LogNetwork(ENetworkLogType InLogType, const FString& URL, const FString& Body = "");
    static const TCHAR* GetLogPrefix(ENetworkLogType InLogType);
};
