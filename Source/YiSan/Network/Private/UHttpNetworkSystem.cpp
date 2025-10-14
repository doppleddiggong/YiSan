// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UHttpNetworkSystem.cpp
 * @brief UHttpNetworkSystem 구현에 대한 Doxygen 주석을 제공합니다.
 */

#include "UHttpNetworkSystem.h"
#include "NetworkLog.h"
#include "ENetworkLogType.h"
#include "HttpModule.h"
#include "FHttpMultipartFormData.h"
#include "JsonObjectConverter.h"
#include "UBroadcastManger.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

#define NETWORK_GET     TEXT("GET")
#define NETWORK_POST    TEXT("POST")

void UHttpNetworkSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UHttpNetworkSystem::Deinitialize()
{
    Super::Deinitialize();
}

void UHttpNetworkSystem::LogNetwork( ENetworkLogType InLogType, const FString& Message, const FString& Body)
{
    if (!Body.IsEmpty())
    {
        NETWORK_LOG(TEXT("%s %s | Body: %s"), GetLogPrefix(InLogType), *Message, *Body);
    }
    else
    {
        NETWORK_LOG(TEXT("%s %s"), GetLogPrefix(InLogType), *Message);
    }
}

const TCHAR* UHttpNetworkSystem::GetLogPrefix(ENetworkLogType InLogType)
{
    switch (InLogType)
    {
    case ENetworkLogType::Get:       return TEXT("[GET]");
    case ENetworkLogType::Post:      return TEXT("[POST]");
    case ENetworkLogType::WS:        return TEXT("[WS]");
    default:                         return TEXT("[UNKNOWN]");
    }
}

void UHttpNetworkSystem::AddNetworkWaitCount(int InValue)
{
    NetworkWaitCount += InValue;
    UBroadcastManger::Get(GetWorld())->SendNetworkWaitCount(NetworkWaitCount);
}

void UHttpNetworkSystem::RequestHealth( FResponseHealthDelegate InDelegate )
{
    auto HttpRequest = FHttpModule::Get().CreateRequest();

    HttpRequest->SetVerb(NETWORK_GET);
    HttpRequest->SetURL(NetworkConfig::GetFullUrl(RequestAPI::Health));
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    LogNetwork(ENetworkLogType::Get, *HttpRequest->GetURL());

    HttpRequest->OnProcessRequestComplete().BindLambda(
        [this, InDelegate](FHttpRequestPtr Req, FHttpResponsePtr ResPtr, bool bWasSuccessful)
        {
            AddNetworkWaitCount(-1);
            FResponseHealth ResponseData;
            ResponseData.SetFromHttpResponse(ResPtr);

            InDelegate.ExecuteIfBound(ResponseData, bWasSuccessful);
        });

    AddNetworkWaitCount(1);
    HttpRequest->ProcessRequest();
}

void UHttpNetworkSystem::RequestASK(const FString& FilePath, FResponseAskDelegate InDelegate)
{
    auto HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetVerb(NETWORK_POST);
    HttpRequest->SetURL(NetworkConfig::GetFullUrl(RequestAPI::ASK));
    HttpRequest->SetHeader(TEXT("Accept"), TEXT("application/json"));

    FHttpMultipartFormData Form;
    if (!Form.AddFile(TEXT("file"), FilePath))
    {
        NETWORK_LOG(TEXT("Ask: file load failed: %s"), *FilePath);
        return;
    }
    Form.SetupHttpRequest(HttpRequest);

    LogNetwork(ENetworkLogType::Post, *HttpRequest->GetURL(), TEXT("Ask (STT->GPT->TTS)"));

    HttpRequest->OnProcessRequestComplete().BindLambda(
        [this, InDelegate](FHttpRequestPtr Req, FHttpResponsePtr ResPtr, bool bWasSuccessful)
        {
            AddNetworkWaitCount(-1);
            FResponseAsk ResponseData;
            if (bWasSuccessful && ResPtr.IsValid())
            {
                NETWORK_LOG(TEXT("[RES] Ask completed: transcribed_text, gpt_response_text, audio_content"));
                ResponseData.SetFromHttpResponse(ResPtr);
            }
            InDelegate.ExecuteIfBound(ResponseData, bWasSuccessful);
        });

    AddNetworkWaitCount(1);
    HttpRequest->ProcessRequest();
}

void UHttpNetworkSystem::RequestSTT(const FString& FilePath, FResponseSTTDelegate InDelegate)
{
    auto HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetVerb(NETWORK_POST);
    HttpRequest->SetURL(NetworkConfig::GetFullUrl(RequestAPI::STT));
    HttpRequest->SetHeader(TEXT("Accept"), TEXT("application/json"));

    FHttpMultipartFormData Form;
    if (!Form.AddFile(TEXT("file"), FilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("STT: file load failed: %s"), *FilePath);
        return;
    }
    Form.SetupHttpRequest(HttpRequest);

    LogNetwork(ENetworkLogType::Post, *HttpRequest->GetURL(), TEXT("STT"));

    HttpRequest->OnProcessRequestComplete().BindLambda(
        [this, InDelegate](FHttpRequestPtr Req, FHttpResponsePtr ResPtr, bool bWasSuccessful)
        {
            AddNetworkWaitCount(-1);
            FResponseTestSTT ResponseData;
            if (bWasSuccessful && ResPtr.IsValid())
            {
                NETWORK_LOG(TEXT("[RES] %s"), *ResPtr->GetContentAsString());
                ResponseData.SetFromHttpResponse(ResPtr);
            }
            InDelegate.ExecuteIfBound(ResponseData, bWasSuccessful);
        });

    AddNetworkWaitCount(1);
    HttpRequest->ProcessRequest();
}

void UHttpNetworkSystem::RequestTTS(
    const FString& Text,
    const float SpeakingRate,
    const float Pitch,
    const FString& VoiceName,
    FResponseTTSDelegate InDelegate)
{
    auto HttpRequest = FHttpModule::Get().CreateRequest();

    HttpRequest->SetVerb(NETWORK_POST);
    HttpRequest->SetURL(NetworkConfig::GetFullUrl(RequestAPI::TTS));
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    FRequestTTS RequestData;
    RequestData.text = Text;
    RequestData.speaking_rate = SpeakingRate;
    RequestData.pitch = Pitch;
    RequestData.voice_name = VoiceName;

    FString RequestBody;
    if (!FJsonObjectConverter::UStructToJsonObjectString(RequestData, RequestBody))
    {
        NETWORK_LOG(TEXT("Failed to convert FRequestTestTTS to JSON"));
        return;
    }

    HttpRequest->SetContentAsString(RequestBody);

    LogNetwork(ENetworkLogType::Post, *HttpRequest->GetURL(), RequestBody);

    HttpRequest->OnProcessRequestComplete().BindLambda(
        [this, InDelegate](FHttpRequestPtr Req, FHttpResponsePtr ResPtr, bool bWasSuccessful)
        {
            AddNetworkWaitCount(-1);
            FResponseTTS ResponseData;
            if (bWasSuccessful && ResPtr.IsValid())
            {
                NETWORK_LOG(TEXT("[RES] %s"), *ResPtr->GetContentAsString());
                ResponseData.SetFromHttpResponse(ResPtr);
            }

            InDelegate.ExecuteIfBound(ResponseData, bWasSuccessful);
        });

    AddNetworkWaitCount(1);
    HttpRequest->ProcessRequest();
}

void UHttpNetworkSystem::RequestGPT(const FString& UserQuery, const FGPTContext& Context, FResponseGPTDelegate InDelegate)
{
    auto HttpRequest = FHttpModule::Get().CreateRequest();

    HttpRequest->SetVerb(NETWORK_POST);
    HttpRequest->SetURL(NetworkConfig::GetFullUrl(RequestAPI::GPT));
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    HttpRequest->SetHeader(TEXT("Accept"), TEXT("application/json"));

    FRequestGPT RequestData;
    RequestData.text = UserQuery;
    RequestData.user_query = UserQuery;
    RequestData.context = Context;

    FString RequestBody;
    if (!RequestData.ToJsonString(RequestBody))
    {
        NETWORK_LOG(TEXT("Failed to serialize FRequestTestGPT to JSON"));
        return;
    }

    HttpRequest->SetContentAsString(RequestBody);

    LogNetwork(ENetworkLogType::Post, *HttpRequest->GetURL(), RequestBody);

    HttpRequest->OnProcessRequestComplete().BindLambda(
        [this, InDelegate](FHttpRequestPtr Req, FHttpResponsePtr ResPtr, bool bWasSuccessful)
        {
            AddNetworkWaitCount(-1);
            FResponseGPT ResponseData;
            if (bWasSuccessful && ResPtr.IsValid())
            {
                NETWORK_LOG(TEXT("[RES] %s"), *ResPtr->GetContentAsString());
                ResponseData.SetFromHttpResponse(ResPtr);
            }
            InDelegate.ExecuteIfBound(ResponseData, bWasSuccessful);
        });

    AddNetworkWaitCount(1);
    HttpRequest->ProcessRequest();
}