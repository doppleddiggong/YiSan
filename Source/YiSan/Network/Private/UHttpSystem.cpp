// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UHttpSystem.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "NetworkLog.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

// --- Subsystem Lifecycle ---

void UHttpSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LogHttp(TEXT("HttpSystem initialized."));
}

void UHttpSystem::Deinitialize()
{
	LogHttp(TEXT("HttpSystem deinitialized."));
	Super::Deinitialize();
}

// --- HTTP Request Methods ---

void UHttpSystem::RequestHealth(FResponseHealthDelegate OnComplete)
{
	const FString Url = NetworkConfig::GetFullUrl(RequestAPI::Health);
	LogHttp(FString::Printf(TEXT("GET %s"), *Url));

	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(Url);
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	HttpRequest->OnProcessRequestComplete().BindUObject(
		this, &UHttpSystem::OnHealthResponseReceived, OnComplete
	);

	HttpRequest->ProcessRequest();
}

void UHttpSystem::RequestHelpChat(const FRequestHelpChat& Request, FResponseHelpChatDelegate OnComplete)
{
	const FString Url = NetworkConfig::GetFullUrl(RequestAPI::HelpChat);
	const FString JsonBody = ConvertStructToJsonString(&Request, FRequestHelpChat::StaticStruct());

	LogHttp(FString::Printf(TEXT("POST %s | Body: %s"), *Url, *JsonBody));

	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(Url);
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	HttpRequest->SetContentAsString(JsonBody);

	HttpRequest->OnProcessRequestComplete().BindUObject(
		this, &UHttpSystem::OnHelpChatResponseReceived, OnComplete
	);

	HttpRequest->ProcessRequest();
}

void UHttpSystem::RequestSTT(const FString& FilePath, FResponseSTTDelegate OnComplete)
{
	if (!FPaths::FileExists(FilePath))
	{
		LogHttp(FString::Printf(TEXT("File not found: %s"), *FilePath));
		FResponseSTT Response;
		OnComplete.ExecuteIfBound(Response, false);
		return;
	}

	const FString Url = NetworkConfig::GetFullUrl(RequestAPI::STT);
	LogHttp(FString::Printf(TEXT("POST %s | File: %s"), *Url, *FilePath));

	// Load file as byte array
	TArray<uint8> FileData = LoadFileToByteArray(FilePath);
	if (FileData.Num() == 0)
	{
		LogHttp(TEXT("Failed to load file data."));
		FResponseSTT Response;
		OnComplete.ExecuteIfBound(Response, false);
		return;
	}

	// Create multipart/form-data boundary
	const FString Boundary = TEXT("----UnrealEngine5Boundary") + FGuid::NewGuid().ToString();
	const FString BeginBoundary = TEXT("\r\n--") + Boundary + TEXT("\r\n");
	const FString EndBoundary = TEXT("\r\n--") + Boundary + TEXT("--\r\n");

	// Build multipart body
	FString ContentDisposition = TEXT("Content-Disposition: form-data; name=\"file\"; filename=\"");
	ContentDisposition += FPaths::GetCleanFilename(FilePath) + TEXT("\"\r\n");
	ContentDisposition += TEXT("Content-Type: audio/wav\r\n\r\n");

	// Combine into byte array
	TArray<uint8> BodyData;
	BodyData.Append((uint8*)TCHAR_TO_UTF8(*BeginBoundary), BeginBoundary.Len());
	BodyData.Append((uint8*)TCHAR_TO_UTF8(*ContentDisposition), ContentDisposition.Len());
	BodyData.Append(FileData);
	BodyData.Append((uint8*)TCHAR_TO_UTF8(*EndBoundary), EndBoundary.Len());

	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(Url);
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("multipart/form-data; boundary=") + Boundary);
	HttpRequest->SetContent(BodyData);

	HttpRequest->OnProcessRequestComplete().BindUObject(
		this, &UHttpSystem::OnSTTResponseReceived, OnComplete
	);

	HttpRequest->ProcessRequest();
}

void UHttpSystem::RequestFishTTSSimple(const FRequestFishTTSSimple& Request, FResponseTTSDelegate OnComplete)
{
	const FString Url = NetworkConfig::GetFullUrl(RequestAPI::FishTTSSimple);
	const FString JsonBody = ConvertStructToJsonString(&Request, FRequestFishTTSSimple::StaticStruct());

	LogHttp(FString::Printf(TEXT("POST %s | Body: %s"), *Url, *JsonBody));

	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(Url);
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	HttpRequest->SetContentAsString(JsonBody);

	HttpRequest->OnProcessRequestComplete().BindUObject(
		this, &UHttpSystem::OnTTSResponseReceived, OnComplete
	);

	HttpRequest->ProcessRequest();
}

void UHttpSystem::RequestTTS(const FRequestTTS& Request, FResponseTTSDelegate OnComplete)
{
	const FString Url = NetworkConfig::GetFullUrl(RequestAPI::TTS);
	const FString JsonBody = ConvertStructToJsonString(&Request, FRequestTTS::StaticStruct());

	LogHttp(FString::Printf(TEXT("POST %s | Body: %s"), *Url, *JsonBody));

	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(Url);
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	HttpRequest->SetContentAsString(JsonBody);

	HttpRequest->OnProcessRequestComplete().BindUObject(
		this, &UHttpSystem::OnTTSResponseReceived, OnComplete
	);

	HttpRequest->ProcessRequest();
}

void UHttpSystem::PostJSON(const FString& Endpoint, const FString& JsonBody, const FString& OnCompleteCallbackName)
{
	const FString Url = NetworkConfig::GetFullUrl(Endpoint);
	LogHttp(FString::Printf(TEXT("POST %s | Body: %s"), *Url, *JsonBody));

	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(Url);
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	HttpRequest->SetContentAsString(JsonBody);

	HttpRequest->OnProcessRequestComplete().BindLambda(
		[this, OnCompleteCallbackName](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
		{
			if (bWasSuccessful && Response.IsValid())
			{
				const FString ResponseBody = Response->GetContentAsString();
				LogHttp(FString::Printf(TEXT("Response [%s]: %s"), *OnCompleteCallbackName, *ResponseBody));
			}
			else
			{
				LogHttp(FString::Printf(TEXT("Request failed [%s]"), *OnCompleteCallbackName));
			}
		}
	);

	HttpRequest->ProcessRequest();
}

// --- HTTP Response Handlers ---

void UHttpSystem::OnHealthResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FResponseHealthDelegate Callback)
{
	FResponseHealth ResponseData;

	if (bWasSuccessful && Response.IsValid())
	{
		ResponseData.SetFromHttpResponse(Response);
		ResponseData.PrintData();
		Callback.ExecuteIfBound(ResponseData, true);
	}
	else
	{
		LogHttp(TEXT("Health request failed."));
		Callback.ExecuteIfBound(ResponseData, false);
	}
}

void UHttpSystem::OnHelpChatResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FResponseHelpChatDelegate Callback)
{
	FResponseHelpChat ResponseData;

	if (bWasSuccessful && Response.IsValid())
	{
		ResponseData.SetFromHttpResponse(Response);
		ResponseData.PrintData();
		Callback.ExecuteIfBound(ResponseData, true);
	}
	else
	{
		LogHttp(TEXT("HelpChat request failed."));
		Callback.ExecuteIfBound(ResponseData, false);
	}
}

void UHttpSystem::OnSTTResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FResponseSTTDelegate Callback)
{
	FResponseSTT ResponseData;

	if (bWasSuccessful && Response.IsValid())
	{
		ResponseData.SetFromHttpResponse(Response);
		ResponseData.PrintData();
		Callback.ExecuteIfBound(ResponseData, true);
	}
	else
	{
		LogHttp(TEXT("STT request failed."));
		Callback.ExecuteIfBound(ResponseData, false);
	}
}

void UHttpSystem::OnTTSResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FResponseTTSDelegate Callback)
{
	FResponseTTS ResponseData;

	if (bWasSuccessful && Response.IsValid())
	{
		ResponseData.SetFromHttpResponse(Response);
		ResponseData.PrintData();
		Callback.ExecuteIfBound(ResponseData, true);
	}
	else
	{
		LogHttp(TEXT("TTS request failed."));
		Callback.ExecuteIfBound(ResponseData, false);
	}
}

// --- Helper Methods ---

void UHttpSystem::LogHttp(const FString& Message)
{
	NETWORK_LOG(TEXT("[HTTP] %s"), *Message);
}

FString UHttpSystem::ConvertStructToJsonString(const void* StructPtr, UScriptStruct* StructType)
{
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);

	if (FJsonObjectConverter::UStructToJsonObjectString(StructType, StructPtr, OutputString, 0, 0))
	{
		return OutputString;
	}

	return TEXT("{}");
}

TArray<uint8> UHttpSystem::LoadFileToByteArray(const FString& FilePath)
{
	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		LogHttp(FString::Printf(TEXT("Failed to load file: %s"), *FilePath));
		FileData.Empty();
	}
	return FileData;
}
