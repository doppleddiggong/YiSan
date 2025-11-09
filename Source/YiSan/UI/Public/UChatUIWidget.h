// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

/**
 * @file UChatUIWidget.h
 * @brief UChatUIWidget 클래스를 선언합니다.
 */
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UChatUIWidget.generated.h"

/**
 * @brief UChatBoxWidget을 포함하는 컨테이너 위젯입니다.
 * @details 전체 채팅 UI의 레이아웃을 구성하는 역할을 합니다.
 */
UCLASS()
class YISAN_API UChatUIWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UChatBoxWidget* WBP_ChatBox;
};
