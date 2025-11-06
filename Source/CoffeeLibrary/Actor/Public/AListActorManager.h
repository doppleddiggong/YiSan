// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file AListActorManager.h
 * @brief AListActorManager 클래스를 선언합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AListActorManager.generated.h"

/**
 * @brief 에디터에서 배열을 구성하기 위한 도우미 액터.
 *
 * 컴포넌트 버전과 동일한 기능을 액터 단위로 제공한다.
 */
UCLASS()
class COFFEELIBRARY_API AListActorManager : public AActor
{
        GENERATED_BODY()
public:
        /** @brief 수집된 액터 목록. */
        UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="List")
        TArray<AActor*> ArrayActors;

        /** @brief 허용할 액터 클래스 필터. */
        UPROPERTY(EditAnywhere, Category="List")
        TSubclassOf<AActor> ClassFilter;

        /** @brief 현재 선택된 액터로 목록을 덮어쓴다. */
        UFUNCTION(CallInEditor, Category="List", meta=(DevelopmentOnly, DisplayName="Assign Item (Replace)"))
        void AssignItemReplace();

        /** @brief 현재 선택된 액터를 목록에 추가한다. */
        UFUNCTION(CallInEditor, Category="List", meta=(DevelopmentOnly, DisplayName="Assign Item (Append)"))
        void AssignItemAppend();

        /** @brief 액터 라벨 기준 오름차순 정렬. */
        UFUNCTION(CallInEditor, Category="List", meta=(DevelopmentOnly, DisplayName="Sort by Name (Asc)"))
        void SortByNameAsc();

        /** @brief 액터 라벨 기준 내림차순 정렬. */
        UFUNCTION(CallInEditor, Category="List", meta=(DevelopmentOnly, DisplayName="Sort by Name (Desc)"))
        void SortByNameDesc();

#if WITH_EDITOR
protected:
        /** @brief 에디터 선택에서 조건을 만족하는 액터를 모은다. */
        int32 GatherSelectedItem(TArray<AActor*>& Out) const;
        /** @brief 내부 헬퍼: Append/Replace 모드 처리. */
        void AssignInternal(bool bAppend);
#endif
};
