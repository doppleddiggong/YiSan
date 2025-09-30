// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "UHttpNetworkSystem.h"
#include "Shared/NetworkLog.h"
#include "ENetworkLogType.h"
#include "HttpModule.h"
#include "JsonObjectConverter.h"
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


void UHttpNetworkSystem::RequestHelpChat( const FRequestHelpChat& RequestData,
    FResponseHelpChatDelegate InDelegate)
{
    FString RequestBody;
    if (!FJsonObjectConverter::UStructToJsonObjectString(RequestData, RequestBody))
    {
        NETWORK_LOG(TEXT("Failed to convert FRequestHelpChat to JSON"));
        return;
    }
    
    auto HttpRequest = FHttpModule::Get().CreateRequest();

    HttpRequest->SetVerb(NETWORK_POST);
    HttpRequest->SetURL(NetworkConfig::GetFullUrl(RequestAPI::HelpChat));
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    HttpRequest->SetHeader(TEXT("Accept"), TEXT("application/json"));
    HttpRequest->SetContentAsString(RequestBody);

    LogNetwork(ENetworkLogType::Post, *HttpRequest->GetURL(), RequestBody);

    HttpRequest->OnProcessRequestComplete().BindLambda(
    [this, InDelegate](FHttpRequestPtr Req, FHttpResponsePtr ResPtr, bool bWasSuccessful)
    {
        FResponseHelpChat ResponseData;
        ResponseData.SetFromHttpResponse(ResPtr);

        InDelegate.ExecuteIfBound(ResponseData, bWasSuccessful);
    });

    HttpRequest->ProcessRequest();
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
