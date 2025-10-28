// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EDasanState.h"
#include "UTourStateSystem.generated.h"

UCLASS(ClassGroup=(NPC), meta=(BlueprintSpawnableComponent))
class YISAN_API UTourStateSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UTourStateSystem();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	FORCEINLINE ETourState GetCurState() const { return CurState; }
	void SetTourState(const ETourState InState);

	void InitSystem(class ADasanActor* InOwner);
	void UpdateTick(float DeltaTime );

	// RepNotify 함수
	UFUNCTION()
	void OnRep_CurState();

private:
	// 상태별 Enter 함수
	void Enter_TourMove();
	void Enter_TourWait();
	void Enter_TourExplain();
	void Enter_TourEnd();

	// 상태별 Tick 함수
	void Tick_TourMove(float DeltaTime);
	void Tick_TourWait(float DeltaTime);
	void Tick_TourExplain(float DeltaTime);

	// 내부 유틸리티
	bool IsAllPlayersNearby() const;
	bool IsUpdateEnble() const;
	void DrawPlayerRadiusDebug() const;

private:
	UPROPERTY()
	TObjectPtr<class ADasanActor> OwnerDasan;

	UPROPERTY()
	TObjectPtr<class UBroadcastManager> BroadcastManager;

	UPROPERTY(ReplicatedUsing=OnRep_CurState)
	ETourState CurState = ETourState::None;

	ETourState PrevState = ETourState::None;

	// 이동 관련
	UPROPERTY(EditAnywhere, Category="Tour")
	float MoveSpeed = 300.0f;
	
	UPROPERTY(EditAnywhere, Category="Tour")
	float PlayerDetectionRadius = 500.0f;

	UPROPERTY(EditAnywhere, Category="Tour")
	float WaitTimeBeforeTour = 2.0f; // 건물 도착 후 관광 시작 전 대기 시간

	UPROPERTY(EditAnywhere, Category="Debug")
	bool bEnablePlayerRadiusDebugDraw = false;
	
	// 대기 타이머
	float WaitTimer = 0.0f;

	// 관광 시간 관련
	UPROPERTY(EditAnywhere, Category="Tour")
	float TourViewDuration = 10.0f; // 관광 시간 (초)
	float TourViewTimer = 0.0f; // 관광 시간 타이머
	int32 LastReportedTime = -1; // 마지막으로 보고한 남은 시간


	UPROPERTY()
	TObjectPtr<class UAudioComponent> PlayingSound;

	FTimerHandle DebugTimerHandle;
};
