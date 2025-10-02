// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UVFXDataAsset.h
 * @brief UVFXDataAsset 데이터 자산에 대한 Doxygen 주석을 제공합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "EVFXType.h"
#include "Engine/DataAsset.h"
#include "UVFXDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FVFXData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<class UNiagaraSystem> VFXAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector LocationOffset = FVector::Zero();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRotator RotatorOffset = FRotator(0, 0, 0 );
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Scale = 1.f;
};

UCLASS(BlueprintType)
class LATTELIBRARY_API UVFXDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "VFX")
	TMap<EVFXType, FVFXData> VFXData;
};
