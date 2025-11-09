// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

/**
 * @file UChatBoxWidget.h
 * @brief UChatBoxWidget 클래스를 선언합니다.
 */
#include "CoreMinimal.h"
#include "NetworkData.h"
#include "Blueprint/UserWidget.h"
#include "UChatPlayerSystem.h"
#include "UChatBoxWidget.generated.h"

/**
 * @brief 채팅 메시지 목록과 입력창을 포함하는 UI 위젯입니다.
 * @details 사용자의 텍스트 입력을 처리하고, UChatPlayerSystem과 연동하여 메시지를 서버로 전송하며, 수신된 메시지를 화면에 표시합니다.
 */
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
