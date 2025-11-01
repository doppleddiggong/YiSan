// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UPlayerListItem.generated.h"


UCLASS()
class YISAN_API UPlayerListItem : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetPlayerName(const FString& InName);
	void SetPlayerStatus(const int32 InPlayerIndex);

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UImage> PlayerIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> PlayerNameText;

	UPROPERTY(EditAnywhere, Category = "Player Status Icons")
	TArray<class UTexture2D*> PlayerIconTextures;
};
