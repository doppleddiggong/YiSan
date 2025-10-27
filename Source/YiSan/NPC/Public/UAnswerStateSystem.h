// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EDasanState.h"
#include "UAnswerStateSystem.generated.h"

UCLASS(ClassGroup=(NPC), meta=(BlueprintSpawnableComponent))
class YISAN_API UAnswerStateSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UAnswerStateSystem();

	protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	EAnswerState GetCurState() const { return CurState; }
	void SetCurState(EAnswerState InState);

	void InitSystem(class ADasanActor* InOwner);
	
	// 블루프린트에서 호출 가능 - 질문 감지 시
	UFUNCTION(BlueprintCallable, Category="Answer")
	void OnAnswerReply();

	// TTS 재생 완료 이벤트 핸들러
	UFUNCTION()
	void OnVoiceTalkFinished();

	// Answer 시작 가능 여부 체크
	bool CanStartAnswer(const FString& PlayerName, FString& OutReason) const;

	// Answer 시작 시도 (내부 구현 - 서버에서만 실행)
	bool TryStartAnswer(const FString& PlayerName);

	// Answer 종료 (내부 구현 - 질문자 초기화)
	void FinishAnswer();

	// 질의응답 세션이 활성 상태인지 확인
	bool IsAnswerSessionActive() const;

	UFUNCTION()
	void OnRep_CurState();

	UFUNCTION()
	void OnRep_QuestionerName();

private:
	UPROPERTY()
	TObjectPtr<class ADasanActor> OwnerDasan;

	UPROPERTY()
	TObjectPtr<class UBroadcastManager> BroadcastManager;

	// 현재 질문 중인 플레이어 이름 (멀티플레이어 동기화용)
	UPROPERTY(ReplicatedUsing=OnRep_QuestionerName)
	FString CurQuestionerName;

	// 현재 Answer 상태 (StateSystem 내부 관리)
	UPROPERTY(ReplicatedUsing=OnRep_CurState)
	EAnswerState CurState = EAnswerState::AnswerListen;
	
	EAnswerState PrevState = EAnswerState::AnswerListen;

	// 음성 질의응답 전 Dasan의 메인 상태 (복귀용)
	EDasanState PreviousMainState;

	// 답변 타임아웃 타이머 핸들
	FTimerHandle AnswerTimeoutTimer;

	// 답변 타임아웃 시간 (초)
	UPROPERTY(EditAnywhere, Category = "Answer")
	float AnswerTimeoutDuration = 30.0f;

	// 타임아웃 처리 함수
	void OnAnswerTimeout();
};
