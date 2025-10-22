// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "NetworkData.h"
#include "Blueprint/UserWidget.h"
#include "UChatBoxWidget.generated.h"

UCLASS()
class YISAN_API UChatBoxWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	void InitSystem(class APlayerActor* InOwner);

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

	/// @brief 입력된 문자열을 네트워크로 전송합니다.
	void SendChatMessage(const FString& InMsg);

	/// @brief ASK 응답을 수신해 UI를 갱신합니다.
	/// @param Response [in] 음성/텍스트 처리 결과입니다.
	/// @param bSuccess [in] 요청 성공 여부입니다.
	UFUNCTION()
	void OnResponseAsk(FResponseAsk& Response, bool bSuccess);
	
public:
	/** --- 위젯 참조 --- */
	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* ChatInput;

	UPROPERTY(meta = (BindWidget))
	class UScrollBox* ScrollBox;

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
