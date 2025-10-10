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
            FResponseHealth ResponseData;
            ResponseData.SetFromHttpResponse(ResPtr);

            InDelegate.ExecuteIfBound(ResponseData, bWasSuccessful);
        });

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
            FResponseSTT ResponseData;
            if (bWasSuccessful && ResPtr.IsValid())
            {
                NETWORK_LOG(TEXT("[RES] %s"), *ResPtr->GetContentAsString());
                ResponseData.SetFromHttpResponse(ResPtr);
            }
            InDelegate.ExecuteIfBound(ResponseData, bWasSuccessful);
        });

    HttpRequest->ProcessRequest();
}

void UHttpNetworkSystem::RequestTTS(const FString& Text, const FString& reference_index, bool bReturnAudio, FResponseTTSDelegate InDelegate)
{
    auto HttpRequest = FHttpModule::Get().CreateRequest();

    HttpRequest->SetVerb(NETWORK_POST);
    HttpRequest->SetURL(NetworkConfig::GetFullUrl(RequestAPI::TTS));
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    HttpRequest->SetHeader(TEXT("Accept"), TEXT("application/json"));

    FRequestTTS RequestData;
    RequestData.text = Text;
    RequestData.reference_index = reference_index;
    RequestData.return_audio = bReturnAudio;

    FString RequestBody;
    if (!FJsonObjectConverter::UStructToJsonObjectString(RequestData, RequestBody))
    {
        NETWORK_LOG(TEXT("Failed to convert FRequestTTS to JSON"));
        return;
    }

    HttpRequest->SetContentAsString(RequestBody);

    LogNetwork(ENetworkLogType::Post, *HttpRequest->GetURL(), RequestBody);

    HttpRequest->OnProcessRequestComplete().BindLambda(
        [this, InDelegate](FHttpRequestPtr Req, FHttpResponsePtr ResPtr, bool bWasSuccessful)
        {
            FResponseTTS ResponseData;
            if (bWasSuccessful && ResPtr.IsValid())
            {
                NETWORK_LOG(TEXT("[RES] %s"), *ResPtr->GetContentAsString());
                ResponseData.SetFromHttpResponse(ResPtr);
            }
            InDelegate.ExecuteIfBound(ResponseData, bWasSuccessful);
        });

    HttpRequest->ProcessRequest();
}