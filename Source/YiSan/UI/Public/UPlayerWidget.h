#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/VerticalBox.h"
#include "UPlayerListItem.h"
#include "UPlayerWidget.generated.h"

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
	UPlayerListItem* CreatePlayerListItem(const FString& playerName);

	UFUNCTION()
	void OnPlayerListUpdated(const TArray<FString>& NewPlayerList);

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPlayerListItem> PlayerListItemClass;
};

