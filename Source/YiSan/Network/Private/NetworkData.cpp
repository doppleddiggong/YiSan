// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file NetworkData.cpp
 * @brief NetworkData 구조 구현에 대한 Doxygen 주석을 제공합니다.
 */
#include "NetworkData.h"

#include "JsonObjectConverter.h"
#include "NetworkLog.h"
#include "Misc/Base64.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"


// --- GPT 공간 컨텍스트 ---

void FGPTSpatialContextLocation::Reset()
{
    name.Reset();
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    bValid = false;
}

void FGPTSpatialContextLocation::Set(const FString& InName, const FVector& InPosition)
{
    name = InName;
    x = InPosition.X;
    y = InPosition.Y;
    z = InPosition.Z;
    bValid = !InName.IsEmpty();
}

bool FGPTSpatialContextLocation::IsValid() const
{
    return bValid;
}

TSharedPtr<FJsonObject> FGPTSpatialContextLocation::ToJsonObject() const
{
    if (!IsValid())
    {
        return nullptr;
    }

    TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
    JsonObject->SetStringField(TEXT("name"), name);
    JsonObject->SetNumberField(TEXT("x"), x);
    JsonObject->SetNumberField(TEXT("y"), y);
    JsonObject->SetNumberField(TEXT("z"), z);
    return JsonObject;
}

void FGPTSpatialContextNearbyBuilding::Reset()
{
    name.Reset();
    distance = 0.0f;
    bValid = false;
}

void FGPTSpatialContextNearbyBuilding::Set(const FString& InName, float InDistanceMeters)
{
    name = InName;
    distance = InDistanceMeters;
    bValid = !InName.IsEmpty();
}

bool FGPTSpatialContextNearbyBuilding::IsValid() const
{
    return bValid;
}

TSharedPtr<FJsonObject> FGPTSpatialContextNearbyBuilding::ToJsonObject() const
{
    if (!IsValid())
    {
        return nullptr;
    }

    TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
    JsonObject->SetStringField(TEXT("name"), name);
    JsonObject->SetNumberField(TEXT("distance"), distance);
    return JsonObject;
}

void FGPTSpatialContext::Reset()
{
    current_location.Reset();
    focused_object.Reset();
    nearby_buildings.Reset();
}

bool FGPTSpatialContext::HasAnyData() const
{
    if (current_location.IsValid() || focused_object.IsValid())
    {
        return true;
    }

    for (const FGPTSpatialContextNearbyBuilding& Building : nearby_buildings)
    {
        if (Building.IsValid())
        {
            return true;
        }
    }

    return false;
}

void FGPTSpatialContext::AddNearbyBuilding(const FGPTSpatialContextNearbyBuilding& InBuilding)
{
    if (!InBuilding.IsValid())
    {
        return;
    }

    nearby_buildings.Add(InBuilding);
}

TSharedPtr<FJsonObject> FGPTSpatialContext::ToJsonObject() const
{
    if (!HasAnyData())
    {
        return nullptr;
    }

    TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();

    if (current_location.IsValid())
    {
        if (TSharedPtr<FJsonObject> LocationJson = current_location.ToJsonObject())
        {
            JsonObject->SetObjectField(TEXT("current_location"), LocationJson);
        }
    }

    if (focused_object.IsValid())
    {
        if (TSharedPtr<FJsonObject> FocusJson = focused_object.ToJsonObject())
        {
            JsonObject->SetObjectField(TEXT("focused_object"), FocusJson);
        }
    }

    TArray<TSharedPtr<FJsonValue>> NearbyArray;
    for (const FGPTSpatialContextNearbyBuilding& Building : nearby_buildings)
    {
        if (!Building.IsValid())
        {
            continue;
        }

        if (TSharedPtr<FJsonObject> BuildingJson = Building.ToJsonObject())
        {
            NearbyArray.Add(MakeShared<FJsonValueObject>(BuildingJson));
        }
    }

    if (NearbyArray.Num() > 0)
    {
        JsonObject->SetArrayField(TEXT("nearby_buildings"), NearbyArray);
        JsonObject->SetArrayField(TEXT("nearest_buildings"), NearbyArray);
    }

    return JsonObject;
}

bool FRequestTestGPT::ToJsonString(FString& OutJson) const
{
    TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();

    const bool bHasUserQuery = !user_query.IsEmpty();
    const bool bHasText = !text.IsEmpty();

    if (bHasUserQuery)
    {
        Root->SetStringField(TEXT("user_query"), user_query);
    }

    if (bHasText)
    {
        Root->SetStringField(TEXT("text"), text);
    }
    else if (bHasUserQuery)
    {
        Root->SetStringField(TEXT("text"), user_query);
    }

    if (TSharedPtr<FJsonObject> ContextJson = context.ToJsonObject())
    {
        Root->SetObjectField(TEXT("context"), ContextJson);
    }

    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);
    return FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);
}



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

void FResponseTestTTS::SetFromHttpResponse(const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Response)
{
    if (!Response.IsValid())
    {
        return;
    }
    audio_data = Response->GetContent();
}

void FResponseTestTTS::PrintData()
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


void FResponseTestGPT::SetFromHttpResponse(const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Response)
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

void FResponseTestGPT::PrintData()
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
