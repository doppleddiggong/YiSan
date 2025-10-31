// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UChatPlayerSystem.h"
#include "UChatEntryWidget.generated.h"

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
