// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "NetworkData.h"
#include "ENetworkLogType.h"
#include "Macro.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UHttpNetworkSystem.generated.h"

UCLASS()
class COFFEELIBRARY_API UHttpNetworkSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    DEFINE_SUBSYSTEM_GETTER_INLINE(UHttpNetworkSystem);
    
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void RequestHealth( FResponseHealthDelegate InDelegate);
    void RequestHelpChat( const FRequestHelpChat& RequestData, FResponseHelpChatDelegate InDelegate);

private:
    static void LogNetwork(ENetworkLogType InLogType, const FString& URL, const FString& Body = "");
    static const TCHAR* GetLogPrefix(ENetworkLogType InLogType);
};
