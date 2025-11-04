// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

/**
 * @file EndingWidget.h
 * @brief UEndingWidget 클래스를 선언합니다.
 */
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EBuildingType.h"
#include "EndingWidget.generated.h"

/**
 * 
 */
UCLASS()
class YISAN_API UEndingWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	
	virtual void NativeConstruct() override;

	// UPROPERTY(meta = (BindWidget))
	// class UImage* endImage;
	//
	// UFUNCTION()
	// void EndingLocation();
	//
	//
	// // 별주 건물을 찾기 위한 함수
	// void FindByeoljuBuilding();
	//
	// FTimerHandle EndingTimerHandle;
	//
	// // 월드에 존재하는 별주 건물에 대한 포인터
	// UPROPERTY()
	// TObjectPtr<class ABuilding> ByeoljuBuilding;
	//
	// // 플레이어와 별주 건물 사이의 거리를 확인하기 위한 값
	// UPROPERTY(EditAnywhere, Category = "Ending Condition")
	// float CheckDistance = 200.0f;
};