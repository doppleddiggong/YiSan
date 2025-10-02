// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UGameDamageType.h
 * @brief UGameDamageType 선언에 대한 Doxygen 주석을 제공합니다.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "EDamageType.h"
#include "UGameDamageType.generated.h"

UCLASS()
class LATTELIBRARY_API UGameDamageType : public UDamageType
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage")
    EDamageType DamageType;
};

