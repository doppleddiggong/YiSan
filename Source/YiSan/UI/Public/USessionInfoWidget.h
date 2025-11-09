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
 * @brief 검색된 멀티플레이어 세션 정보를 표시하고 참여 버튼을 제공하는 UI 위젯입니다.
 * @details 로비에서 세션 목록의 각 항목을 나타냅니다.
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
