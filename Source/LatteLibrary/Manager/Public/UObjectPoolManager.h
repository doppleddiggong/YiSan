// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UObjectPoolManager.h
 * @brief UObjectPoolManager 클래스를 선언합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "Macro.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObjectPoolManager.generated.h"

/**
 * @brief 액터를 재활용하여 런타임에 액터 스폰 비용을 최소화하는 오브젝트 풀링(Object Pooling) 관리 서브시스템입니다.
 * @details 비활성화된 액터를 풀에 저장했다가 필요 시 재활성화하여 제공함으로써, 동적 액터 생성 및 소멸로 인한 성능 저하를 방지합니다.
 */
UCLASS()
class LATTELIBRARY_API UObjectPoolManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:	
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    DEFINE_SUBSYSTEM_GETTER_INLINE(UObjectPoolManager);
    
    UFUNCTION(BlueprintCallable, Category="Pool", meta = (WorldContext = "WorldContextObject"))
    AActor* GetPoolItem(const UObject* WorldContextObject, TSubclassOf<AActor> InClass);

    UFUNCTION(BlueprintCallable, Category="Pool", meta = (WorldContext = "WorldContextObject"))
    AActor* GetPoolItemLocationRotator(const UObject* WorldContextObject, const TSubclassOf<AActor> InClass, const FVector Location, const FRotator Rotator );
    
    UFUNCTION(BlueprintCallable, Category="Pool")
    void ReturnActorToPool(AActor* Actor);

private:	
    TMap<UClass*, TArray<AActor*>> PoolMap;
};