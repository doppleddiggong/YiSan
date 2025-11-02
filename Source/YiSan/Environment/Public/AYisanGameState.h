// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "AYiSanPlayerState.h"
#include "EBuildingType.h"
#include "GameFramework/GameStateBase.h"
#include "AYisanGameState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerListUpdated, const TArray<FString>& /* PlayerNames */);

UCLASS()
class YISAN_API AYisanGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AYisanGameState();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

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

	// ========================================
	// Player List Management
	// ========================================

	/** @brief 다음 플레이어 인덱스 (전역적으로 증가) */
	static int32 NextPlayerIndex;

	/** @brief 플레이어 목록을 업데이트하고 브로드캐스트합니다. (서버 전용) */
	UFUNCTION(BlueprintCallable, Category="PlayerList")
	void UpdatePlayerList();

	/** @brief 플레이어 목록 업데이트를 요청합니다. (클라이언트/서버 모두) */
	UFUNCTION(BlueprintCallable, Category="PlayerList")
	void RequestRefreshPlayerList();

	/** @brief 현재 플레이어 목록을 반환합니다. */
	UFUNCTION(BlueprintCallable, Category="PlayerList")
	TArray<FString> GetPlayerList() const { return PlayerList; }

	/** @brief 플레이어 목록이 업데이트될 때 호출되는 델리게이트 */
	FOnPlayerListUpdated OnPlayerListUpdated;

private:
	UFUNCTION(Server, Reliable)
	void ServerRPC_UpdatePlayerList();
	
	UFUNCTION()
	void OnRep_QuestManager();

	UFUNCTION()
	void OnRep_PlayerList();

public:
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Tour")
	TObjectPtr<class ADasanActor> DasanNPC;

private:
	UPROPERTY(ReplicatedUsing=OnRep_QuestManager)
	TObjectPtr<class AQuestManagerActor> QuestManager;

	UPROPERTY(ReplicatedUsing=OnRep_PlayerList)
	TArray<FString> PlayerList;
};
