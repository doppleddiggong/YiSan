// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "EDasanState.h"
#include "ADasanActor.generated.h"

UCLASS()
class YISAN_API ADasanActor : public ACharacter
{
	GENERATED_BODY()

public:
	ADasanActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// RepNotify 함수
	UFUNCTION()
	void OnRep_DasanState();

	// 서버 RPC - 상태 변경
	UFUNCTION(Server, Reliable)
	void ServerRPC_SetDasanState(EDasanState InState);

	// 현재 메인 상태 (Tour/Explain/Answer)
	UPROPERTY(ReplicatedUsing=OnRep_DasanState, BlueprintReadOnly, Category="State")
	EDasanState DasanState;
	
public:
	// 유틸리티 함수
	void StartTour();
	void NextQuest();

	// 현재 목표 건물 찾기
	class ABuilding* FindCurTargetBuilding() const;
	FORCEINLINE class ABuilding* GetCurTargetBuilding() { return CurTargetBuilding;}
	FORCEINLINE void UpdateTargetBuilding(class ABuilding* InBuilding)	{ CurTargetBuilding = InBuilding; }

	
	// 상태 전환 메서드
	void TransitionToState(EDasanState InMainState);

	float GetTargetBuildingDistnace();

private:
	// 디버그 상태 표시
	void DrawDebugState();

public:
	// 상태 시스템 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State")
	TObjectPtr<class UTourStateSystem> TourStateSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State")
	TObjectPtr<class UExplainStateSystem> ExplainStateSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State")
	TObjectPtr<class UAnswerStateSystem> AnswerStateSystem;


	// Tour 상태를 업데이트하는 헬퍼 함수 (Tick에서 호출)
	void UpdateTourState();

	// 플레이어 폰을 가져오는 헬퍼 함수
	 class APawn* GetPlayerPawn() const;
	
	// 플레이어와의 거리를 계산하는 헬퍼 함수
	float GetPlayerDistance(class APawn* PlayerPawn) const;
	
	// ai control
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<class AAIController> DasanAicontrol;
	 
	UPROPERTY(EditAnywhere)
	float playerMaxDis;
	UPROPERTY(EditAnywhere)
	float wayPointDis;

	//tour wait 상태에서 player 체크용
	float waitChackTimer;

	// 투어 상태 업데이트용 타이머 핸들
	FTimerHandle TourStateTimerHandle;
	
	UPROPERTY()
	TObjectPtr<class UQuestManager> QuestManager;

	UPROPERTY()
	TObjectPtr<class ABuilding> CurTargetBuilding;
};