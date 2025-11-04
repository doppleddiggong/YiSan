// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

/**
 * @file EDasanState.h
 * @brief EDasanState 클래스를 선언합니다.
 */
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EDasanState.generated.h"

UENUM(BlueprintType)
enum class EDasanState : uint8
{
	Tour			UMETA(DisplayName = "투어"),
	Answer			UMETA(DisplayName = "답변"),
	Max				UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ETourState : uint8
{
	None			UMETA(DisplayName = "없음"),
	TourMove		UMETA(DisplayName = "이동중"),
	TourWait		UMETA(DisplayName = "대기중"),
	TourExplain		UMETA(DisplayName = "자유 관광"),
	TourEnd			UMETA(DisplayName = "투어종료"),
	Max				UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EAnswerState : uint8
{
	AnswerListen	UMETA(DisplayName = "질문듣기"),
	AnswerReply		UMETA(DisplayName = "답변중"),
	AnswerEnd		UMETA(DisplayName = "답변종료"),
	Max				UMETA(Hidden)
};