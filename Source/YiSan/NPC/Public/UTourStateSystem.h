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

	void StartExplainVoice();
	
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

	UPROPERTY(EditAnywhere, Category="Debug|Tour")
	bool bEnableDebugDraw = false;
	
	// 대기 타이머
	float WaitTimer = 0.0f;

	// 강제 집결 관련
	UPROPERTY(EditAnywhere, Category="Tour")
	float ForcedGatherTime = 10.0f; // 강제 집결까지 대기 시간 (초)
	float ForcedGatherTimer = 0.0f; // 강제 집결 타이머
	int32 LastReportedGatherTime = -1; // 마지막으로 보고한 강제 집결 남은 시간

	// 관광 시간 관련
	UPROPERTY(EditAnywhere, Category="Tour")
	float TourViewDuration = 10.0f; // 관광 시간 (초)
	float TourViewTimer = 0.0f; // 관광 시간 타이머
	int32 LastReportedTime = -1; // 마지막으로 보고한 남은 시간

	// 설명 출력 관련
	UPROPERTY(EditAnywhere, Category="Tour")
	float ExplainLineInterval = 3.0f; // 설명 라인 간격 (초)

	UPROPERTY(EditAnywhere, Category="Tour")
	float PostExplainSilentDuration = 5.0f; // 설명 완료 후 조용히 대기하는 시간 (초)

	UPROPERTY(EditAnywhere, Category="Tour")
	float PostExplainWaitDuration = 5.0f; // 조용한 대기 후 카운트다운 시간 (초)

	TArray<FString> ExplainLines;			// 설명 라인 배열
	int32 ExplainLineIndex = 0;				// 현재 출력 중인 라인 인덱스
	float ExplainLineTimer = 0.0f;			// 라인별 타이머
	float PostExplainWaitTimer = 0.0f;		// 설명 완료 후 대기 타이머
	bool bExplainCompleted = false;			// 설명 완료 여부

	FTimerHandle DebugTimerHandle;
};
