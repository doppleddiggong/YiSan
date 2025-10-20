// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EBuildingType.h"
#include "Blueprint/UserWidget.h"
#include "USmallPopup.generated.h"

/// @file USmallPopup.h
/// @brief 근거리 건물 정보를 간단히 표시하는 팝업 위젯을 선언합니다.
/// @brief 주변 건물을 안내하는 소형 팝업 UI입니다.
UCLASS()
class YISAN_API USmallPopup : public UUserWidget
{
    GENERATED_BODY()

public:
    /**
     * @brief 빌딩 타입에 따라 팝업의 정보를 업데이트합니다.
     * @param BuildingType 건물 타입
     */
    void UpdateBuildingInfo(EBuildingType BuildingType);

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UTextBlock> TitleText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UTextBlock> DescText;
    
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UImage> ThumbnailImage;

private:
    EBuildingType BuildingType;
};