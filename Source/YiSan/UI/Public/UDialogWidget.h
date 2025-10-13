// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UDialogWidget.generated.h"


UCLASS()
class YISAN_API UDialogWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category ="Chat")
	void ShowDialog(FString InString);

public:
	UPROPERTY(meta = (BindWidget))
	class UBorder* DialogBorder;
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DialogText;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category ="Chat")
	FLinearColor ActivateColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category ="Chat")
	FLinearColor DeactivateColor;
};
