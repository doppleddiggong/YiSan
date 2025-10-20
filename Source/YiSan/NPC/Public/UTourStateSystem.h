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

public:
	FORCEINLINE ETourState GetCurState() const { return CurState; }
	FORCEINLINE void SetTourState(const ETourState InState) { CurState = InState; }

	void InitSystem(class ADasanActor* InOwner);
	void UpdateTick(float DeltaTime );
	
private:
	// 상태별 Enter 함수
	void Enter_TourMove();
	void Enter_TourWait();
	void Enter_TourEnd();

	// 상태별 Tick 함수
	void Tick_TourMove(float DeltaTime);
	void Tick_TourWait(float DeltaTime);

	// 내부 유틸리티
	void MoveToCurWaypoint();
	bool IsNearWaypoint() const;
	bool IsAllPlayersNearby() const;

private:
	bool IsUpdateEnble();

private:
	UPROPERTY()
	TObjectPtr<class ADasanActor> OwnerDasan;

	ETourState CurState = ETourState::None;
	ETourState PrevState = ETourState::None;

	// 이동 관련
	UPROPERTY(EditAnywhere, Category="Tour")
	float MoveSpeed = 300.0f;

	UPROPERTY(EditAnywhere, Category="Tour")
	float WaypointReachDistance = 100.0f;

	UPROPERTY(EditAnywhere, Category="Tour")
	float PlayerDetectionRadius = 500.0f;

	UPROPERTY(EditAnywhere, Category="Tour")
	float WaitTimeBeforeExplain = 2.0f;

	// 대기 타이머
	float WaitTimer = 0.0f;
};
