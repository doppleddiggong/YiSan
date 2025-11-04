// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file FPadFeedbackData.h
 * @brief FPadFeedbackData 구조체를 선언합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "EDamageType.h"
#include "Engine/DataTable.h"
#include "FPadFeedbackData.generated.h"

USTRUCT(BlueprintType)
struct FPadFeedbackData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Amplitude = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.f;
};
 
static FPadFeedbackData GetPadFeedbackData(EDamageType Type)
{
	switch (Type)
	{
	case EDamageType::Small:   return {0.2f, 0.2f};
	case EDamageType::Normal:  return {0.4f, 0.3f};
	case EDamageType::Large:   return {0.7f, 0.45f};
	case EDamageType::Huge:    return {1.0f, 0.6f};
	default:                        return {};
	}
}