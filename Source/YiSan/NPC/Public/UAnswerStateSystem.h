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

public:
	EAnswerState GetCurState() const { return CurState; }
	void SetAnswerState(const EAnswerState InState) { CurState = InState; }
	
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

	// 현재 Answer 상태 (StateSystem 내부 관리)
	EAnswerState CurState = EAnswerState::AnswerListen;
	EAnswerState PrevState = EAnswerState::AnswerListen;

	
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
