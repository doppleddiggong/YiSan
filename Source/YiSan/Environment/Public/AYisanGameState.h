// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
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
	UFUNCTION(Server, Reliable)                                                                                                           
	void ServerRPC_BroadcastToastMessage(const FString& Message);    
protected:                                                                                                                                                  
	// Client-side RPC function to display the toast                                                                                                        
	UFUNCTION(Client, Reliable)                                                                                                                             
	void ClientRPC_ShowToastMessage(const FString& Message);   

public:
	// Dasan NPC 참조
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Tour")
	TObjectPtr<class ADasanActor> DasanNPC;

	// QuestManager 참조
	UPROPERTY(BlueprintReadOnly, Category="Tour")
	TObjectPtr<class UQuestManager> QuestManager;

	// 전역 투어 상태
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Tour")
	EDasanState GlobalTourState;

	// 투어 진행 여부
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Tour")
	bool bIsTourActive;

public:
	// 투어 시작
	UFUNCTION(BlueprintCallable, Category="Tour")
	void StartGlobalTour();

	// 투어 상태 업데이트
	void UpdateTourState(EDasanState NewState);

	// 현재 퀘스트 인덱스 가져오기 (QuestManager에서)
	UFUNCTION(BlueprintCallable, Category="Tour")
	int32 GetCurQuestIndex();

	// 현재 목표 건물 타입 가져오기 (QuestManager에서)
	UFUNCTION(BlueprintCallable, Category="Tour")
	EBuildingType GetCurTargetBuilding();
};
