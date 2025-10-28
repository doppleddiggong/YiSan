// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EBuildingType.h"
#include "GameFramework/GameStateBase.h"
#include "EDasanState.h"
#include "AYisanGameState.generated.h"

class AQuestManagerActor;

UCLASS()
class YISAN_API AYisanGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AYisanGameState();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_ToastMessage(const FString& Message);

	// 투어 시작
	UFUNCTION(BlueprintCallable, Category = "Tour")
	void StartGlobalTour();

	class AQuestManagerActor* GetQuestManager() const { return QuestManager; }
	EBuildingType GetCurrentQuestTarget() const;
	int32 GetCurrentQuestIndex() const;
	bool HasActiveQuest() const;

	void SetQuestManager(class AQuestManagerActor* InManager);

protected:
	UFUNCTION()
	void OnRep_QuestManager();

private:
	// Dasan NPC 참조
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Tour")
	TObjectPtr<class ADasanActor> DasanNPC;

private:
	UPROPERTY(ReplicatedUsing = OnRep_QuestManager, BlueprintReadOnly, Category = "Tour", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class AQuestManagerActor> QuestManager;
};
