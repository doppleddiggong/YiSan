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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	EAnswerState GetCurState() const { return CurState; }
	void SetAnswerState(const EAnswerState InState);

	void InitSystem(class ADasanActor* InOwner);
	void UpdateTick(float DeltaTime );

	
	// 블루프린트에서 호출 가능 - 질문 감지 시
	UFUNCTION(BlueprintCallable, Category="Answer")
	void OnQuestionDetected();

	// 블루프린트에서 호출 가능 - 답변 완료 시
	UFUNCTION(BlueprintCallable, Category="Answer")
	void OnAnswerFinished();

	// 질의응답 종료 (다음 웨이포인트로)
	UFUNCTION(BlueprintCallable, Category="Answer")
	void EndAnswer();

	// 음성 녹음 이벤트 핸들러
	UFUNCTION()
	void OnAudioCapture(bool bRecording);

	// TTS 재생 완료 이벤트 핸들러
	UFUNCTION()
	void OnTTSPlaybackFinished();

	// Answer 시작 가능 여부 체크
	bool CanStartAnswer(const FString& PlayerName, FString& OutReason) const;

	// Answer 시작 시도 (성공 시 true, 실패 시 false 반환)
	bool TryStartAnswer(const FString& PlayerName);

	// Answer 종료 (질문자 초기화)
	void FinishAnswer();

	// 질의응답 세션이 활성 상태인지 확인
	bool IsAnswerSessionActive() const;

	UFUNCTION()
	void OnRep_CurState();

private:
	// 상태별 Enter 함수
	void Enter_AnswerListen();
	void Enter_AnswerReply();
	void Enter_AnswerEnd();

	// 상태별 Tick 함수
	void Tick_AnswerListen(float DeltaTime);
	void Tick_AnswerReply(float DeltaTime);

private:
	bool IsUpdateEnble();
	
private:
	UPROPERTY()
	TObjectPtr<class ADasanActor> OwnerDasan;

	UPROPERTY()
	TObjectPtr<class UBroadcastManager> BroadcastManager;

	// 현재 Answer 상태 (StateSystem 내부 관리)
	UPROPERTY(ReplicatedUsing=OnRep_CurState)
	EAnswerState CurState = EAnswerState::AnswerListen;
	EAnswerState PrevState = EAnswerState::AnswerListen;

	// 음성 질의응답 전 Dasan의 메인 상태 (복귀용)
	EDasanState PreviousMainState;

	// 현재 질문 중인 플레이어 이름 (멀티플레이어 동기화용)
	UPROPERTY(Replicated)
	FString CurrentQuestionerName;

	// 답변 관련
	UPROPERTY(EditAnywhere, Category="Answer")
	float ListenTimeout = 30.0f; // 질문 대기 시간

	UPROPERTY(EditAnywhere, Category="Answer")
	float ReplyDuration = 5.0f; // 기본 답변 시간

	UPROPERTY(EditAnywhere, Category="Answer")
	bool bAutoEndAnswer = true; // 자동으로 답변 종료할지 여부

	// 타이머
	float ListenTimer = 0.0f;
	float ReplyTimer = 0.0f;
};
