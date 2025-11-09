// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

/**
 * @file UPlayerListItem.h
 * @brief UPlayerListItem 클래스를 선언합니다.
 */
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UPlayerListItem.generated.h"


/**
 * @brief 플레이어 목록에 표시되는 개별 플레이어 정보 항목 UI 위젯입니다.
 * @details 플레이어의 이름과 아이콘을 표시하며, 로컬 플레이어 여부에 따라 다르게 보일 수 있습니다.
 */
UCLASS()
class YISAN_API UPlayerListItem : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetPlayerStatus(const int32 InPlayerIndex, const int32 LocalPlayerIndex, const FString& InName);
	
protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UImage> PlayerIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> PlayerNameText;

	UPROPERTY(EditAnywhere, Category = "Player Status Icons")
	TArray<class UTexture2D*> PlayerIconTextures;
};
