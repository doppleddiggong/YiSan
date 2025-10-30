// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "NetworkData.h"
#include "Blueprint/UserWidget.h"
#include "UChatPlayerSystem.h"
#include "UChatBoxWidget.generated.h"

UCLASS()
class YISAN_API UChatBoxWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	void InitSystem(class APlayerActor* InOwner);

	UFUNCTION(BlueprintCallable)
	void FocusChat();

	UFUNCTION(BlueprintCallable)
	void ExitChat();

	UFUNCTION(BlueprintCallable)
	void Scroll(bool bUp);

	UFUNCTION(BlueprintCallable)
	void AddChatMessage(const FChatMessage& ChatMessage);

private:
	UFUNCTION()
	void OnTextCommittedHandler(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void Ask(const FString& InMsg, const FGPTContext& SpatialContext);

	UFUNCTION()
	void OnResponseAsk(FResponseAsk& Response, bool bSuccess);
	
public:
	/** --- 위젯 참조 --- */
	UPROPERTY(meta = (BindWidget))
	class UScrollBox* ScrollBox;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* ChatMessagesBox; // UMG에서 이름 정확히 일치시켜야 함!
	
	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* ChatInput;


	UPROPERTY(EditDefaultsOnly, Category = "Chat")
	TSubclassOf<class UChatEntryWidget> ChatEntryClass;

	UPROPERTY()
	TObjectPtr<class UBroadcastManager> BroadcastManager;
	
	UPROPERTY()
	TObjectPtr<class APlayerActor> Owner;

	UPROPERTY()
	TObjectPtr<class UChatPlayerSystem> ChatPlayerSystem;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	float ScrollMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	bool bChatFocused = false;
};
