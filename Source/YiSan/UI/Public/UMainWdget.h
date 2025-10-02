// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UMainWdget.generated.h"

class UEditableTextBox;
class USoundWave;

/**
 * @brief Simple root widget that can host voice controls and status text.
 */
UCLASS()
class YISAN_API UMainWdget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

public:
	UFUNCTION(BlueprintImplementableEvent, Category="Voice")
	void OnVoiceTranscriptReceived(const FString& CorrelationId, const FString& TranscriptText, float Confidence);

	UFUNCTION(BlueprintImplementableEvent, Category="Voice")
	void OnVoiceSynthesisReady(const FString& CorrelationId, USoundWave* SynthesizedSound);

protected:
	/** @brief Editable text box bound from the widget blueprint. */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> VoiceInputBox;
};
