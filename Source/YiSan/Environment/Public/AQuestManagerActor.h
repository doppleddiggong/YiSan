// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via
// any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EBuildingType.h"
#include "GameFramework/Actor.h"

#include "AQuestManagerActor.generated.h"

class UBroadcastManager;

/**
 * 서버 권한에서 퀘스트 목표를 관리하고 클라이언트에 동기화하는 액터입니다.
 */
UCLASS()
class YISAN_API AQuestManagerActor : public AActor
{
	GENERATED_BODY()

public:
	AQuestManagerActor();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 투어를 시작하거나 현재 퀘스트 상태를 재전파합니다. */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void StartTour();

	/** 클라이언트에서 건물 접촉을 보고하기 위한 RPC 입니다. */
	UFUNCTION(Server, Reliable)
	void ServerRPC_NotifyContact(EBuildingType InBuildingType);

	/** 현재 목표 건물 타입을 조회합니다. */
	UFUNCTION(BlueprintPure, Category = "Quest")
	EBuildingType GetCurrentQuestTarget() const { return CurrentQuestTarget; }

	/** 현재 퀘스트 인덱스를 조회합니다. */
	UFUNCTION(BlueprintPure, Category = "Quest")
	int32 GetCurrentQuestIndex() const { return CurrentQuestIndex; }

	/** 활성 퀘스트가 존재하는지 여부를 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Quest")
	bool HasActiveQuest() const { return CurrentQuestTarget != EBuildingType::None; }

	/** 월드 컨텍스트에서 싱글톤처럼 접근 가능한 헬퍼입니다. */
	static AQuestManagerActor* Get(const UObject* WorldContextObject);

protected:
	UFUNCTION()
	void OnRep_CurrentQuestTarget();

	UFUNCTION()
	void OnRep_CurrentQuestIndex();

private:
	void InitializeQuestSteps();
	void EnsureBroadcastManager();
	void BroadcastCurrentQuest();
	void AdvanceQuest();
	void ApplyQuestState(EBuildingType InTarget, int32 InQuestIndex);
	void HandleContactInternal(EBuildingType InBuildingType);

	UFUNCTION()
	void HandleBroadcastContact(EBuildingType InBuildingType);

private:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentQuestTarget, VisibleInstanceOnly, Category = "Quest")
	EBuildingType CurrentQuestTarget = EBuildingType::None;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentQuestIndex, VisibleInstanceOnly, Category = "Quest")
	int32 CurrentQuestIndex = INDEX_NONE;

	UPROPERTY()
	TObjectPtr<UBroadcastManager> BroadcastManager;

	UPROPERTY()
	TArray<EBuildingType> QuestSteps;

	bool bQuestInitialized = false;
};

