// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NetworkData.h"
#include "UMainWidget.generated.h"

UCLASS()
class YISAN_API UMainWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void ToggleChatBox();

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnMessageComitted(const FText& Text, ETextCommit::Type CommitMethod);
	
private:
	void SendChatMessage(const FString& InMsg);
	void OnResponseTTS(FResponseTTS& ResponseData, bool bWasSuccessful);

protected:
	UPROPERTY(meta = (BindWidget), BlueprintReadWrite, Category = "Chat")
	class UCanvasPanel* ChatBox;
	
	UPROPERTY(meta = (BindWidget), BlueprintReadWrite, Category = "Chat")
	class UEditableTextBox* InputText;
};
