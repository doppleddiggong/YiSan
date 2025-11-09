// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

/**
 * @file UChatEntryWidget.h
 * @brief UChatEntryWidget 클래스를 선언합니다.
 */
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UChatPlayerSystem.h"
#include "UChatEntryWidget.generated.h"

/**
 * @brief 채팅 목록에 표시되는 개별 채팅 메시지 항목 UI 위젯입니다.
 * @details 발신자 유형(NPC, 유저, 시스템)에 따라 다른 스타일을 적용하여 메시지를 시각적으로 구분합니다.
 */
UCLASS()
class YISAN_API UChatEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FChatMessage ChatMessageData;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Chat")
	void SetupMessageAppearance(EChatMessageType MessageType, int PlayerIndex);

	void ReverseHorizontalBox(bool IsPlayer);
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextBlock;

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* HorizontalBox;

	UPROPERTY(meta = (BindWidget))
	class UBorder* BorderIcon;

	UPROPERTY(meta = (BindWidget))
	class USizeBox* ChatSizeBox;

};
