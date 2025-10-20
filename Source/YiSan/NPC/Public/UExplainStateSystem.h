// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EDasanState.h"
#include "UExplainStateSystem.generated.h"

UCLASS(ClassGroup=(NPC), meta=(BlueprintSpawnableComponent))
class YISAN_API UExplainStateSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UExplainStateSystem();

public:
	// 상태 접근자
	FORCEINLINE EExplainState GetCurState() const { return CurState; }
	FORCEINLINE void SetExplainState(const EExplainState InState) { CurState = InState; }
	
	void InitSystem(class ADasanActor* InOwner);
	void UpdateTick(float DeltaTime );
	
	// 블루프린트에서 호출 가능 - 설명 완료 시
	UFUNCTION(BlueprintCallable, Category="Explain")
	void OnExplainFinished();

	
private:
	// 상태별 Enter 함수
	void Enter_ExplainWait();
	void Enter_ExplainIng();

	// 상태별 Tick 함수
	void Tick_ExplainWait(float DeltaTime);
	void Tick_ExplainIng(float DeltaTime);

private:
	bool IsUpdateEnble();

private:
	UPROPERTY()
	TObjectPtr<class ADasanActor> OwnerDasan;

	// 현재 Explain 상태 (StateSystem 내부 관리)
	EExplainState CurState = EExplainState::ExplainWait;
	EExplainState PrevState = EExplainState::ExplainWait;

	// 설명 관련
	UPROPERTY(EditAnywhere, Category="Explain")
	float ExplainDuration = 10.0f; // 기본 설명 시간

	UPROPERTY(EditAnywhere, Category="Explain")
	bool bAutoFinishExplain = true; // 자동으로 설명 종료할지 여부

	// 타이머
	float ExplainTimer = 0.0f;
};
