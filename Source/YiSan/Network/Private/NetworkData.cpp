// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file NetworkData.cpp
 * @brief FResponseHealth의 동작을 구현합니다.
 */
#include "NetworkData.h"

#include "JsonObjectConverter.h"
#include "NetworkLog.h"
#include "UCommonFunctionLibrary.h"
#include "Misc/Base64.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"






void FResponseHealth::SetFromHttpResponse(const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Response)
{
    if (Response.IsValid())
    {
        status = Response->GetResponseCode();
    }
}

void FResponseHealth::PrintData()
{
    FString OutputString;
    FJsonObjectConverter::UStructToJsonObjectString(
        *this,
        OutputString,
        0,
        0
    );
    NETWORK_LOG( TEXT("[RES] %s"), *OutputString);
}

// --- Ask Endpoint Implementation ---
bool FRequestASK::ToJsonString(FString& OutJson) const
{
    TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();

    if (TSharedPtr<FJsonObject> ContextJson = context.ToJsonObject())
        Root->SetObjectField(TEXT("context"), ContextJson);

    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);
    return FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);
}

void FResponseAsk::SetFromHttpResponse(const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Response)
{
    if (!Response.IsValid())
    {
        return;
    }

    FString ResponseBody = Response->GetContentAsString();

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        JsonObject->TryGetStringField(TEXT("transcribed_text"), transcribed_text);
        JsonObject->TryGetStringField(TEXT("gpt_response_text"), gpt_response_text);

        gpt_response_text = UCommonFunctionLibrary::RemoveLineBreaks(gpt_response_text);

        FString audio_content;
        JsonObject->TryGetStringField(TEXT("audio_content"), audio_content);
        FBase64::Decode(audio_content, audio_data);
    }
}

void FResponseAsk::PrintData()
{
    FString OutputString;
    FJsonObjectConverter::UStructToJsonObjectString(
        *this,
        OutputString,
        0,
        0
    );
    NETWORK_LOG( TEXT("[RES] %s"), *OutputString);
}

void FResponseTestSTT::SetFromHttpResponse(const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Response)
{
    if (!Response.IsValid())
    {
        return;
    }

    FString ResponseBody = Response->GetContentAsString();

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        JsonObject->TryGetStringField(TEXT("text"), text);
    }
}

void FResponseTestSTT::PrintData()
{
    FString OutputString;
    FJsonObjectConverter::UStructToJsonObjectString(
        *this,
        OutputString,
        0,
        0
    );
    NETWORK_LOG( TEXT("[RES] %s"), *OutputString);
}

// --- Test Endpoints Implementation ---

void FResponseTTS::SetFromHttpResponse(const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Response)
{
    if (!Response.IsValid())
    {
        return;
    }
    audio_data = Response->GetContent();
}

void FResponseTTS::PrintData()
{
    FString OutputString;
    FJsonObjectConverter::UStructToJsonObjectString(
        *this,
        OutputString,
        0,
        0
    );
    NETWORK_LOG( TEXT("[RES] %s"), *OutputString);
}

bool FRequestGPT::ToJsonString(FString& OutJson) const
{
    TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
    Root->SetStringField(TEXT("user_query"), user_query);

    if (TSharedPtr<FJsonObject> ContextJson = context.ToJsonObject())
        Root->SetObjectField(TEXT("context"), ContextJson);

    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);
    return FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);
}

void FResponseGPT::SetFromHttpResponse(const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Response)
{
    if (!Response.IsValid())
    {
        return;
    }

    FString ResponseBody = Response->GetContentAsString();

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        JsonObject->TryGetStringField(TEXT("response"), response);
    }
}

void FResponseGPT::PrintData()
{
    FString OutputString;
    FJsonObjectConverter::UStructToJsonObjectString(
        *this,
        OutputString,
        0,
        0
    );
    NETWORK_LOG( TEXT("[RES] %s"), *OutputString);
}
