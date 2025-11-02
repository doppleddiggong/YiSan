// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "YiSanPlayerListManager.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerListUpdated, const TArray<FString>& /* PlayerNames */);

UCLASS()
class YISAN_API AYiSanPlayerListManager : public AActor
{
    GENERATED_BODY()

public:
    AYiSanPlayerListManager();
    
protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    void BroadcastPlayerList();

    UFUNCTION(Server, Reliable)
    void ServerRPC_UpdatePlayerList();

    UFUNCTION(BlueprintCallable)
    void RequestRefresh();

    TArray<FString> GetPlayerList() const { return PlayerList; };

public:
    FOnPlayerListUpdated OnPlayerListUpdated;
    
private:
    UPROPERTY(ReplicatedUsing = OnRep_PlayerList)
    TArray<FString> PlayerList;

    UFUNCTION()
    void OnRep_PlayerList();
};