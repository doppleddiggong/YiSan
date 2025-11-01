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
    FOnPlayerListUpdated OnPlayerListUpdated;

    UFUNCTION(BlueprintCallable)
    void RequestRefresh(); // Client-side call to request refresh from server
    UFUNCTION(Server, Reliable)
    void ServerRPC_RequestRefresh(); // Server-side implementation

    TArray<FString> GetPlayerList() const { return PlayerList; };

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY(ReplicatedUsing = OnRep_PlayerList)
    TArray<FString> PlayerList;

    UFUNCTION()
    void OnRep_PlayerList();

public:
    void UpdatePlayerListAndBroadcast();
};