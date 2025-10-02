// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file USequenceActivatable.h
 * @brief USequenceActivatable 선언에 대한 Doxygen 주석을 제공합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "USequenceActivatable.generated.h"

UINTERFACE(BlueprintType)
class COFFEELIBRARY_API USequenceActivatable : public UInterface
{
	GENERATED_BODY()
};

class COFFEELIBRARY_API ISequenceActivatable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Sequence")
	void Activate(int Index, int TotalIndex, float Duration);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Sequence")
	void Deactivate(int Index, int TotalIndex, float Duration);
};