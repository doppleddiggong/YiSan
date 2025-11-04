/**
 * @file UGameFunctionLibrary.h
 * @brief UGameFunctionLibrary 클래스를 선언합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "UGameDamageType.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UGameFunctionLibrary.generated.h"


UCLASS()
class LATTELIBRARY_API UGameFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Game|Damage")
    static TSubclassOf<UGameDamageType> GetDamageTypeClass(EDamageType InType);
};

