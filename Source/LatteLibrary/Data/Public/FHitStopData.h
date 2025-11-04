// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file FHitStopData.h
 * @brief FHitStopData 구조체를 선언합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "EDamageType.h"
#include "Engine/DataTable.h"
#include "FHitStopData.generated.h"

USTRUCT(BlueprintType)
struct FHitStopData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDamageType Type = EDamageType::Normal;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeDilation = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.10f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRefreshIfStronger = true;
};