// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file USequenceManager.h
 * @brief USequenceManager 클래스를 선언합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "USequenceManager.generated.h"

/**
 * @brief USequenceManager에서 처리할 개별 시퀀스 명령을 정의하는 구조체입니다.
 * @details 타겟 액터, 활성화/비활성화 여부, 그리고 연출 지속 시간을 포함합니다.
 */
USTRUCT()
struct LATTELIBRARY_API FSequenceCommand
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AActor> Target;

	UPROPERTY()
	bool bActivate = true;

	UPROPERTY()
	float Duration = 0.f;
};

/**
 * @brief 여러 액터를 지정된 간격으로 순차적으로 활성화 또는 비활성화하는 시퀀스 관리 서브시스템입니다.
 * @details ISequenceActivatable 인터페이스를 구현한 액터들을 대상으로, 정해진 순서에 따라 연출을 트리거하는 데 사용됩니다.
 */
UCLASS()
class LATTELIBRARY_API USequenceManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	/** 0.1초 간격으로 Targets를 순차 Activate/Deactivate
	 *  InitialDelay: 첫 실행 지연
	 *  Interval    : 대상 간 간격(예: 0.1f)
	 *  Duration    : Activate 시 각 대상에 전달되는 연출 길이
	 */
	UFUNCTION(BlueprintCallable, Category="Sequence")
	void RequestSequential(const TArray<AActor*>& Targets,
						   bool bActivate,
						   float InitialDelay = 0.f,
						   float Interval = 0.1f,
						   float Duration = 0.5f);

	UFUNCTION(BlueprintCallable, Category="Sequence")
	void CancelAll();

private:
	void TickStep();

private:
	FTimerHandle MainHandle;
	TArray<FSequenceCommand> Queue;
	int32 Index = 0;
	float StepInterval = 0.1f;
};