// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "AYiSanPlayerState.h"
#include "EBuildingType.h"
#include "GameFramework/GameStateBase.h"
#include "AYisanGameState.generated.h"

/**
 * @file AYisanGameState.h
 * @brief AYisanGameState 클래스를 선언합니다.
 */

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerListUpdated, const TArray<FString>& /* PlayerNames */);

/**
 * @brief @c AGameStateBase를 확장하여 퀘스트 조율과 복제 UI 데이터를 제공합니다.
 */
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
        /** @brief 모든 클라이언트에 토스트 알림을 송출합니다. */
        UFUNCTION(NetMulticast, Reliable)
        void MulticastRPC_ToastMessage(const FString& Message);

        /** @brief 활성 퀘스트 대상을 클라이언트 간에 동기화합니다. */
        UFUNCTION(NetMulticast, Reliable)
        void MulticastRPC_UpdateQuestTarget(const EBuildingType InBuildingType);

        /** @brief 초기 로딩이 완료되었음을 클라이언트에 알립니다. */
        UFUNCTION(NetMulticast, Reliable)
        void MulticastRPC_LoadingComplete();

        /** @brief 공유 사운드 큐 재생을 서버에 요청합니다. */
        UFUNCTION(Server, Reliable)
        void ServerRPC_PlaySound(EGameSoundType SoundType);

        /** @brief 모든 클라이언트에서 사운드 재생 지시를 실행합니다. */
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
        /** @brief 권한 서버에 플레이어 목록 재구성을 요청합니다. */
        UFUNCTION(Server, Reliable)
        void ServerRPC_UpdatePlayerList();

        /** @brief 퀘스트 매니저 참조가 클라이언트에 복제될 때 호출됩니다. */
        UFUNCTION()
        void OnRep_QuestManager();

        /** @brief 복제된 플레이어 목록 배열이 변할 때 호출됩니다. */
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
