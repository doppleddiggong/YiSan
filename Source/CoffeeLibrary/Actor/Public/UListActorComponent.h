// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UListActorComponent.h
 * @brief UListActorComponent 클래스를 선언합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UListActorComponent.generated.h"
/**
 * @brief 에디터에서 선택한 액터를 배열로 수집하는 편의 컴포넌트.
 *
 * 배열 정렬과 추가 방식을 UI 버튼 형태(CallInEditor)로 제공한다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class COFFEELIBRARY_API UListActorComponent : public UActorComponent
{
        GENERATED_BODY()
public:
        /** @brief 수집된 액터 목록. */
        UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="List")
        TArray<AActor*> ArrayActors;

        /** @brief 허용할 액터 클래스 필터. 비어 있으면 전체 허용. */
        UPROPERTY(EditAnywhere, Category="List")
        TSubclassOf<AActor> ClassFilter;

        /** @brief 현재 선택된 액터로 배열을 덮어쓴다. */
        UFUNCTION(CallInEditor, Category="List", meta=(DevelopmentOnly, DisplayName="Assign Item (Replace)"))
        void AssignItemReplace();

        /** @brief 현재 선택된 액터를 배열에 추가한다. */
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
        /** @brief 현재 에디터 선택에서 대상 액터를 수집한다. */
        int32 GatherSelectedItem(TArray<AActor*>& Out) const;
        /** @brief 내부 헬퍼: Append/Replace 모드를 처리한다. */
        void AssignInternal(bool bAppend);
#endif
};
