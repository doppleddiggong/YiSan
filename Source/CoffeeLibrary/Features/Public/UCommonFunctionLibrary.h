// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.
/**
 * @file UCommonFunctionLibrary.h
 * @brief UCommonFunctionLibrary 선언에 대한 Doxygen 주석을 제공합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UCommonFunctionLibrary.generated.h"

/**
 * @brief 프로젝트 전반에서 사용하는 공통 블루프린트 함수.
 */
UCLASS()
class COFFEELIBRARY_API UCommonFunctionLibrary : public UBlueprintFunctionLibrary
{
        GENERATED_BODY()

public:
	// static void TestULog();
	// static void TestInBound();

        /** @brief 주어진 인덱스가 배열 범위 안에 있는지 확인한다. */
        UFUNCTION(BlueprintPure, Category = "CoffeeLibrary|Utilities", meta = (DisplayName = "InBounds"))
        static bool InBounds(const int32 Index, const int32 Count);

        /** @brief 정수 배열에서 랜덤 인덱스를 선택한다. */
        UFUNCTION(BlueprintPure, Category="Utils|Array", meta=(CompactNodeTitle="RandElem", ArrayParm="TargetArray", ArrayTypeDependentParams="ReturnValue"))
        static int32 GetRandomIndex(const TArray<int32>& TargetArray, bool& bIsValid);

        /** @brief 몽타주 배열에서 랜덤 항목을 반환한다. */
        UFUNCTION(BlueprintPure, Category="Utils|Array", meta=(CompactNodeTitle="RandElem", ArrayParm="TargetArray", ArrayTypeDependentParams="ReturnValue"))
        static UAnimMontage* GetRandomMontage(const TArray<UAnimMontage*>& Montages);

        /** @brief 현재 시간을 Unix Timestamp(초)로 반환한다. */
        UFUNCTION(BlueprintPure, Category = "CoffeeLibrary|Utilities", meta = (DisplayName = "NowTimestamp"))
        static int64 GetNowTimestamp();

        /** @brief 두 액터 사이의 거리를 계산한다. */
        UFUNCTION(BlueprintPure, Category="CoffeeLibrary|Utilities", meta=(DefaultToSelf="Target", DisplayName="GetDistance"))
        static float GetDistance( AActor* A,  AActor* B);

        /** @brief 머티리얼 인스턴스 다이내믹을 얻거나 새로 생성한다. */
        UFUNCTION(BlueprintCallable, Category="CoffeeLibrary|Material", meta=(DefaultToSelf="Target", DisplayName="Get Or Create MID", AdvancedDisplay="OptionalName"))
        static class UMaterialInstanceDynamic* GetOrCreateMID(
                class UPrimitiveComponent* Target,
                int32 ElementIndex,
                FName OptionalName = NAME_None);

        /** @brief 지정된 위치에서 사운드를 재생한다. */
        UFUNCTION(BlueprintCallable, Category="CoffeeLibrary|Utilities", meta=(DefaultToSelf="Target", DisplayName="PlayCommonSound"))
        static void PlayLocationSound(const AActor* Actor, USoundBase* Sound, const float RetriggerDelay);
};

