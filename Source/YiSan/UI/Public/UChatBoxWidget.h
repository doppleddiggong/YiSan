// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UChatBoxWidget.generated.h"

UCLASS()
class YISAN_API UChatBoxWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnTextCommittedHandler(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION(BlueprintCallable)
	void FocusChat();

	UFUNCTION(BlueprintCallable)
	void ExitChat();

	UFUNCTION(BlueprintCallable)
	void Scroll(bool bUp);

	UFUNCTION(BlueprintCallable)
	void AddChatMessage(const FString& Message);

private:
	FString GetPlayerDisplayName() const;

public:
	/** --- 위젯 참조 --- */
	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* ChatInput;

	UPROPERTY(meta = (BindWidget))
	class UScrollBox* ScrollBox;

	UPROPERTY(EditDefaultsOnly, Category = "Chat")
	TSubclassOf<class UChatEntryWidget> ChatEntryClass;

	UPROPERTY()
	TObjectPtr<class UChatPlayerSystem> ChatPlayerSystem;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	float ScrollMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	bool bChatFocused = false;
};
