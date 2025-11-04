// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

/**
 * @file USessionInfoWidget.h
 * @brief USessionInfoWidget 클래스를 선언합니다.
 */
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "USessionInfoWidget.generated.h"

/**
 * 
 */
UCLASS()
class YISAN_API USessionInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* textSessionName;
	UPROPERTY(meta = (BindWidget))
	class UButton* btn_Join;
	int32 sessionIndex;
	void SetSessionInfo(int32 index, FString sessionName);
	UFUNCTION()
	void OnClickJoin();
};
