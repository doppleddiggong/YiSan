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
	UFUNCTION(Server, Reliable)                                                                                                           
	void ServerRPC_ToastMessage(const FString& Message);

	// // 서버 RPC - 상태 변경
	// UFUNCTION(Server, Reliable)
	// void ServerRPC_SetDasanState(EDasanState InState);
	
	// UFUNCTION(Server, Reliable)
	// void ServerRPC_TryStartAnswer(const FString& PlayerName);
	//
	// UFUNCTION(Server, Reliable)
	// void ServerRPC_AnswerReply();
	//
	// UFUNCTION(Server, Reliable)
	// void ServerRPC_FinishAnswer();


	
	// /// @brief 클라이언트가 음성 녹음을 시작했음을 서버에 알립니다.
	// UFUNCTION(Server, Reliable)
	// void ServerRPC_RecordingStart(APlayerController* Player);
	//
	// /// @brief 클라이언트가 음성 녹음을 종료했음을 서버에 알립니다.
	// UFUNCTION(Server, Reliable)
	// void ServerRPC_RecordingEnd(APlayerController* Player);

public:
	// Dasan NPC 참조
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Tour")
	TObjectPtr<class ADasanActor> DasanNPC;

	// QuestManager 참조
	UPROPERTY(BlueprintReadOnly, Category="Tour")
	TObjectPtr<class UQuestManager> QuestManager;

public:
	// 투어 시작
	UFUNCTION(BlueprintCallable, Category="Tour")
	void StartGlobalTour();
};
