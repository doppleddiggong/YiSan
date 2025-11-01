// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "AYiSanPlayerState.h"
#include "EBuildingType.h"
#include "GameFramework/GameStateBase.h"
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

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_UpdateQuestTarget(const EBuildingType InBuildingType);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_LoadingComplete();

	UFUNCTION(Server, Reliable)
	void ServerRPC_PlaySound(EGameSoundType SoundType);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_PlaySound(EGameSoundType SoundType);
	
	// 투어 시작
	UFUNCTION(BlueprintCallable, Category="Tour")
	void StartGlobalTour();

	/** @brief 퀘스트 매니저를 설정합니다. 서버 전용입니다. */
	void SetQuestManager(class AQuestManagerActor* InQuestManager);

	/** @brief 현재 퀘스트 매니저를 반환합니다. */
	class AQuestManagerActor* GetQuestManager() const { return QuestManager; }

private:
	UFUNCTION()
	void OnRep_QuestManager();
	
public:
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Tour")
	TObjectPtr<class ADasanActor> DasanNPC;

private:
	UPROPERTY(ReplicatedUsing=OnRep_QuestManager)
	TObjectPtr<class AQuestManagerActor> QuestManager;
};
