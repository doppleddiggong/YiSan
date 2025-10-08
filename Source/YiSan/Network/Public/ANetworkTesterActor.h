// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file ANetworkTesterActor.h
 * @brief ANetworkTesterActor 선언에 대한 Doxygen 주석을 제공합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetworkData.h"
#include "ANetworkTesterActor.generated.h"

UCLASS()
class YISAN_API ANetworkTesterActor : public AActor
{
    GENERATED_BODY()

public:
    ANetworkTesterActor();

protected:
    
    UFUNCTION(CallInEditor, Category = "TEST|RequestHealth")
    void RequestHealth();
    void OnResponseHealth(FResponseHealth& ResponseData, bool bWasSuccessful);
    
    UFUNCTION(CallInEditor, Category = "TEST|RequestHelpChat")
    FORCEINLINE void BP_RequestHelpChat()
    {
        RequestHelpChat(TEXT("산안기 실기 접수 실패!!!"));
    }
    void RequestHelpChat( const FString InQuestion );
    void OnResponseHelpChat(FResponseHelpChat& ResponseData, bool bWasSuccessful);
};
