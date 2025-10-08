// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UGoogleSpeechService.h"

#include "NetworkLog.h"
#include "Misc/Base64.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Dom/JsonObject.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

namespace GoogleSpeechService::Urls
{
	static const TCHAR* SpeechToText = TEXT("https://speech.googleapis.com/v1/speech:recognize");
	static const TCHAR* TextToSpeech = TEXT("https://texttospeech.googleapis.com/v1/text:synthesize");
}

UGoogleSpeechService* UGoogleSpeechService::Get(const UObject* WorldContext)
{
	if (!IsValid(WorldContext))
	{
		return nullptr;
	}

	const UWorld* World = WorldContext->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<UGoogleSpeechService>();
}

void UGoogleSpeechService::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Settings = GetDefault<USpeechSettings>()->Get();

	ApiKey = Settings.ApiKey;
	LanguageCode = Settings.LanguageCode;
	DefaultSpeechSampleRate = Settings.SpeechSampleRate;
	VoiceName = Settings.VoiceName;
	VoiceGender = Settings.VoiceSsmlGender;
	DefaultTtsSampleRate = Settings.TextToSpeechSampleRate;
	AudioEncoding = Settings.AudioEncoding;
}

void UGoogleSpeechService::RequestSpeechToText(const FString& CorrelationId, const TArray<uint8>& AudioPayload, int32 SampleRate, int32 NumChannels, const FGoogleSpeechToTextDelegate& Delegate)
{
	if (ApiKey.IsEmpty())
	{
		NETWORK_LOG(TEXT("[GoogleSTT][FAIL] CorrelationId=%s ApiKey missing."), *CorrelationId);
		Delegate.ExecuteIfBound(false, CorrelationId, FString(), 0.0f);
		return;
	}

	const FString Url = BuildSpeechToTextUrl();

	TSharedPtr<FJsonObject> Config = MakeShared<FJsonObject>();
	Config->SetStringField(TEXT("languageCode"), LanguageCode);
	Config->SetStringField(TEXT("encoding"), TEXT("LINEAR16"));
	Config->SetNumberField(TEXT("sampleRateHertz"), SampleRate > 0 ? SampleRate : DefaultSpeechSampleRate);
	Config->SetNumberField(TEXT("audioChannelCount"), NumChannels > 0 ? NumChannels : 1);

	TSharedPtr<FJsonObject> Audio = MakeShared<FJsonObject>();
	Audio->SetStringField(TEXT("content"), FBase64::Encode(AudioPayload));

	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetObjectField(TEXT("config"), Config);
	Root->SetObjectField(TEXT("audio"), Audio);

	FString JsonPayload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonPayload);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(JsonPayload);
	Request->OnProcessRequestComplete().BindUObject(this, &UGoogleSpeechService::HandleSpeechToTextResponse);

	FPendingSpeechToTextRequest& Pending = PendingSpeechToText.FindOrAdd(&*Request);
	Pending.CorrelationId = CorrelationId;
	Pending.Delegate = Delegate;
	Pending.SampleRate = SampleRate;
	Pending.NumChannels = NumChannels;

	NETWORK_LOG(TEXT("[REQ][GoogleSTT] CorrelationId=%s PayloadBytes=%d"), *CorrelationId, AudioPayload.Num());
	Request->ProcessRequest();
}

void UGoogleSpeechService::RequestTextToSpeech(const FString& CorrelationId, const FString& Text, int32 SampleRate, const FGoogleTextToSpeechDelegate& Delegate)
{
	if (ApiKey.IsEmpty())
	{
		NETWORK_LOG(TEXT("[GoogleTTS][FAIL] CorrelationId=%s ApiKey missing."), *CorrelationId);
		Delegate.ExecuteIfBound(false, CorrelationId, Text, TArray<uint8>(), 0);
		return;
	}

	const FString Url = BuildTextToSpeechUrl();

	TSharedPtr<FJsonObject> Input = MakeShared<FJsonObject>();
	Input->SetStringField(TEXT("text"), Text);

	TSharedPtr<FJsonObject> Voice = MakeShared<FJsonObject>();
	Voice->SetStringField(TEXT("languageCode"), LanguageCode);
	Voice->SetStringField(TEXT("name"), VoiceName);
	if (!VoiceGender.IsEmpty())
	{
		Voice->SetStringField(TEXT("ssmlGender"), VoiceGender);
	}

	TSharedPtr<FJsonObject> AudioConfig = MakeShared<FJsonObject>();
	AudioConfig->SetStringField(TEXT("audioEncoding"), AudioEncoding);
	AudioConfig->SetNumberField(TEXT("sampleRateHertz"), SampleRate > 0 ? SampleRate : DefaultTtsSampleRate);

	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetObjectField(TEXT("input"), Input);
	Root->SetObjectField(TEXT("voice"), Voice);
	Root->SetObjectField(TEXT("audioConfig"), AudioConfig);

	FString JsonPayload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonPayload);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(JsonPayload);
	Request->OnProcessRequestComplete().BindUObject(this, &UGoogleSpeechService::HandleTextToSpeechResponse);

	FPendingTextToSpeechRequest& Pending = PendingTextToSpeech.FindOrAdd(&*Request);
	Pending.CorrelationId = CorrelationId;
	Pending.Delegate = Delegate;
	Pending.SampleRate = SampleRate;
	Pending.Text = Text;

	NETWORK_LOG(TEXT("[REQ][GoogleTTS] CorrelationId=%s TextBytes=%d"), *CorrelationId, Text.Len());
	Request->ProcessRequest();
}

void UGoogleSpeechService::HandleSpeechToTextResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FPendingSpeechToTextRequest Pending;
	if (!PendingSpeechToText.RemoveAndCopyValue(Request.Get(), Pending))
	{
		return;
	}

	const FString CorrelationId = Pending.CorrelationId;

	if (!bWasSuccessful || !Response.IsValid())
	{
		NETWORK_LOG(TEXT("[RES][GoogleSTT][FAIL] CorrelationId=%s Request failed (transport)."), *CorrelationId);
		Pending.Delegate.ExecuteIfBound(false, CorrelationId, FString(), 0.0f);
		return;
	}

	const int32 StatusCode = Response->GetResponseCode();
	NETWORK_LOG(TEXT("[RES][GoogleSTT] CorrelationId=%s Code=%d"), *CorrelationId, StatusCode);

	if (StatusCode != 200)
	{
		Pending.Delegate.ExecuteIfBound(false, CorrelationId, FString(), 0.0f);
		return;
	}

	FString Transcript;
	float Confidence = 0.0f;
	bool bParsed = false;

	const FString Body = Response->GetContentAsString();
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
	if (FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid())
	{
		const TArray<TSharedPtr<FJsonValue>>* Results = nullptr;
		if (Root->TryGetArrayField(TEXT("results"), Results) && Results && Results->Num() > 0)
		{
			const TSharedPtr<FJsonObject> FirstResult = (*Results)[0]->AsObject();
			const TArray<TSharedPtr<FJsonValue>>* Alternatives = nullptr;
			if (FirstResult.IsValid() && FirstResult->TryGetArrayField(TEXT("alternatives"), Alternatives) && Alternatives && Alternatives->Num() > 0)
			{
				const TSharedPtr<FJsonObject> FirstAlt = (*Alternatives)[0]->AsObject();
				if (FirstAlt.IsValid())
				{
					FirstAlt->TryGetStringField(TEXT("transcript"), Transcript);
					double ConfidenceDouble = 0.0;
					if (FirstAlt->TryGetNumberField(TEXT("confidence"), ConfidenceDouble))
					{
						Confidence = static_cast<float>(ConfidenceDouble);
					}
					bParsed = !Transcript.IsEmpty();
				}
			}
		}
	}

	Pending.Delegate.ExecuteIfBound(bParsed, CorrelationId, Transcript, Confidence);
}

void UGoogleSpeechService::HandleTextToSpeechResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FPendingTextToSpeechRequest Pending;
	if (!PendingTextToSpeech.RemoveAndCopyValue(Request.Get(), Pending))
	{
		return;
	}

	const FString CorrelationId = Pending.CorrelationId;

	if (!bWasSuccessful || !Response.IsValid())
	{
		NETWORK_LOG(TEXT("[RES][GoogleTTS][FAIL] CorrelationId=%s Request failed (transport)."), *CorrelationId);
		Pending.Delegate.ExecuteIfBound(false, CorrelationId, Pending.Text, TArray<uint8>(), 0);
		return;
	}

	const int32 StatusCode = Response->GetResponseCode();
	NETWORK_LOG(TEXT("[RES][GoogleTTS] CorrelationId=%s Code=%d"), *CorrelationId, StatusCode);

	if (StatusCode != 200)
	{
		Pending.Delegate.ExecuteIfBound(false, CorrelationId, Pending.Text, TArray<uint8>(), 0);
		return;
	}

	TArray<uint8> AudioData;
	bool bParsed = false;

	const FString Body = Response->GetContentAsString();
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
	if (FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid())
	{
		FString AudioContent;
		if (Root->TryGetStringField(TEXT("audioContent"), AudioContent))
		{
			bParsed = FBase64::Decode(AudioContent, AudioData);
		}
	}

	Pending.Delegate.ExecuteIfBound(bParsed, CorrelationId, Pending.Text, AudioData, Pending.SampleRate > 0 ? Pending.SampleRate : DefaultTtsSampleRate);
}

FString UGoogleSpeechService::BuildSpeechToTextUrl() const
{
	return FString::Printf(TEXT("%s?key=%s"), GoogleSpeechService::Urls::SpeechToText, *ApiKey);
}

FString UGoogleSpeechService::BuildTextToSpeechUrl() const
{
	return FString::Printf(TEXT("%s?key=%s"), GoogleSpeechService::Urls::TextToSpeech, *ApiKey);
}
