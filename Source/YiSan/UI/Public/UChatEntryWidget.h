// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UChatEntryWidget.generated.h"

UCLASS()
class YISAN_API UChatEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FString Message;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextBlock;
};
