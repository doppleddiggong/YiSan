// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


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

