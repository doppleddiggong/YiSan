// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UChatMessageItem.generated.h"

/**
 * 개별 채팅 메시지 표시 위젯
 */
UCLASS()
class YISAN_API UChatMessageItem : public UUserWidget
{
	GENERATED_BODY()

protected:
	/** 발화자 이름 표시 */
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* SenderText;

	/** 메시지 내용 표시 */
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* MessageText;

	// /** 타임스탬프 표시 (옵션) */
	// UPROPERTY(meta = (BindWidget))
	// class UTextBlock* TimestampText;
	//
	// /** 아이콘 (옵션) */
	// UPROPERTY(meta = (BindWidget))
	// class UImage* MessageIcon;

public:
	/**
	 * 메시지 데이터 설정
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SetMessageData(const FString& Sender, const FString& Message, const FLinearColor& Color);
};