// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GoogleSpeechServiceSettings.generated.h"

/**
 * @brief Developer settings for Google Speech services.
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Google Speech Service"))
class LATTELIBRARY_API UGoogleSpeechServiceSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGoogleSpeechServiceSettings();

	/** @brief Google Cloud API key. */
	UPROPERTY(Config, EditAnywhere, Category="Authentication")
	FString ApiKey;

	/** @brief Default STT language (BCP-47). */
	UPROPERTY(Config, EditAnywhere, Category="SpeechToText")
	FString LanguageCode;

	/** @brief Target sample rate for STT payloads. */
	UPROPERTY(Config, EditAnywhere, Category="SpeechToText", meta=(ClampMin="8000", ClampMax="48000"))
	int32 SpeechSampleRate;

	/** @brief Google TTS voice name (see Google Cloud catalog). */
	UPROPERTY(Config, EditAnywhere, Category="TextToSpeech")
	FString VoiceName;

	/** @brief SSML gender for the selected voice. */
	UPROPERTY(Config, EditAnywhere, Category="TextToSpeech")
	FString VoiceSsmlGender;

	/** @brief Sample rate for synthesized audio. */
	UPROPERTY(Config, EditAnywhere, Category="TextToSpeech", meta=(ClampMin="8000", ClampMax="48000"))
	int32 TextToSpeechSampleRate;

	/** @brief Encoding used for synthesized audio. */
	UPROPERTY(Config, EditAnywhere, Category="TextToSpeech")
	FString AudioEncoding;

	static const UGoogleSpeechServiceSettings* Get();
};
