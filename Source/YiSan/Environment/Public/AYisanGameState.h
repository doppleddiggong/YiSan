// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EBuildingType.h"
#include "GameFramework/GameStateBase.h"
#include "EDasanState.h"
#include "AYisanGameState.generated.h"

UCLASS()
class YISAN_API AYisanGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AYisanGameState();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_ToastMessage(const FString& Message);

	// 투어 시작
	UFUNCTION(BlueprintCallable, Category="Tour")
	void StartGlobalTour();
public:
	// Dasan NPC 참조
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Tour")
	TObjectPtr<class ADasanActor> DasanNPC;

	// QuestManager 참조
	UPROPERTY(BlueprintReadOnly, Category="Tour")
	TObjectPtr<class UQuestManager> QuestManager;
};
