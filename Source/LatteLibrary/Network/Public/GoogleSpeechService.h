// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GoogleSpeechServiceSettings.h"
#include "GoogleSpeechService.generated.h"

class IHttpRequest;
class IHttpResponse;

DECLARE_DELEGATE_FourParams(FGoogleSpeechToTextDelegate, bool /*bSuccess*/, const FString& /*CorrelationId*/, const FString& /*Transcript*/, float /*Confidence*/);
DECLARE_DELEGATE_FiveParams(FGoogleTextToSpeechDelegate, bool /*bSuccess*/, const FString& /*CorrelationId*/, const FString& /*Text*/, const TArray<uint8>& /*AudioData*/, int32 /*SampleRate*/);

/**
 * @brief Thin Google Speech (STT/TTS) client.
 */
UCLASS()
class LATTELIBRARY_API UGoogleSpeechService : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UGoogleSpeechService* Get(const UObject* WorldContext);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void RequestSpeechToText(const FString& CorrelationId, const TArray<uint8>& AudioPayload, int32 SampleRate, int32 NumChannels, const FGoogleSpeechToTextDelegate& Delegate);
	void RequestTextToSpeech(const FString& CorrelationId, const FString& Text, int32 SampleRate, const FGoogleTextToSpeechDelegate& Delegate);

private:
	void HandleSpeechToTextResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void HandleTextToSpeechResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	FString BuildSpeechToTextUrl() const;
	FString BuildTextToSpeechUrl() const;

private:
	typedef TMap<IHttpRequest*, struct FPendingSpeechToTextRequest> FSpeechToTextMap;
	typedef TMap<IHttpRequest*, struct FPendingTextToSpeechRequest> FTextToSpeechMap;

	struct FPendingSpeechToTextRequest
	{
		FString CorrelationId;
		FGoogleSpeechToTextDelegate Delegate;
		int32 SampleRate = 16000;
		int32 NumChannels = 1;
	};

	struct FPendingTextToSpeechRequest
	{
		FString CorrelationId;
		FString Text;
		FGoogleTextToSpeechDelegate Delegate;
		int32 SampleRate = 22050;
	};

private:
	const UGoogleSpeechServiceSettings* Settings = nullptr;
	FString ApiKey;
	FString LanguageCode;
	int32 DefaultSpeechSampleRate = 16000;
	FString VoiceName;
	FString VoiceGender;
	int32 DefaultTtsSampleRate = 22050;
	FString AudioEncoding;

	FSpeechToTextMap PendingSpeechToText;
	FTextToSpeechMap PendingTextToSpeech;
};
