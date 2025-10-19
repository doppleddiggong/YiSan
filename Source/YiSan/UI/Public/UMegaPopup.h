// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EBuildingType.h"
#include "UMegaPopup.generated.h"

/// @file UMegaPopup.h
/// @brief 상세 건물 정보를 제공하는 대형 팝업 위젯을 선언합니다.
/// @brief 메가 팝업 패널을 표현하는 UI 클래스입니다.

UCLASS(BlueprintType, Blueprintable)
class YISAN_API UMegaPopup : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnCloseButtonClicked();


public:
	/**
	 * @brief 빌딩 타입에 따라 팝업의 정보를 업데이트합니다.
	 * @param BuildingType 건물 타입
	 */
	void UpdateBuildingInfo(EBuildingType BuildingType);

	void OnClose();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> DetailTitleText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> DetailText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> SubTitle;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> BackgroundImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> CloseButton;

private:
	UPROPERTY()
	TObjectPtr<class UAudioComponent> PlayingSound;

	EBuildingType BuildingType;
};