// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "UPlayerWidget.generated.h"

/**
 * 
 */
UCLASS()
class YISAN_API UPlayerWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> PlayerListContainer;

	UFUNCTION()
	void UpdatePlayerList(const TArray<FString>& playerNames);

	UFUNCTION()
	UTextBlock* CreatePlayerText(const FString& playerName);

	UFUNCTION()
	void OnPlayerListUpdated(const TArray<FString>& NewPlayerList);

};
