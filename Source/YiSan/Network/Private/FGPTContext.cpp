// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file NetworkData.cpp
 * @brief NetworkData 구조 구현에 대한 Doxygen 주석을 제공합니다.
 */

#include "FGPTContext.h"

// --- GPT 공간 컨텍스트 ---

void FGPTContextLocation::Reset()
{
    name.Reset();
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    bValid = false;
}

void FGPTContextLocation::Set(const FString& InName, const FVector& InPosition)
{
    name = InName;
    x = InPosition.X;
    y = InPosition.Y;
    z = InPosition.Z;
    bValid = !InName.IsEmpty();
}

bool FGPTContextLocation::IsValid() const
{
    return bValid;
}

TSharedPtr<FJsonObject> FGPTContextLocation::ToJsonObject() const
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

void FGPTContextNearBuilding::Reset()
{
    name.Reset();
    distance = 0.0f;
    bValid = false;
}

void FGPTContextNearBuilding::Set(const FString& InName, float InDistanceMeters)
{
    name = InName;
    distance = InDistanceMeters;
    bValid = !InName.IsEmpty();
}

bool FGPTContextNearBuilding::IsValid() const
{
    return bValid;
}

TSharedPtr<FJsonObject> FGPTContextNearBuilding::ToJsonObject() const
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

void FGPTContext::Reset()
{
    current_location.Reset();
    focused_object.Reset();
    nearby_buildings.Reset();
}

bool FGPTContext::HasAnyData() const
{
    if (current_location.IsValid() || focused_object.IsValid())
    {
        return true;
    }

    for (const FGPTContextNearBuilding& Building : nearby_buildings)
    {
        if (Building.IsValid())
        {
            return true;
        }
    }

    return false;
}

void FGPTContext::AddNearbyBuilding(const FGPTContextNearBuilding& InBuilding)
{
    if (!InBuilding.IsValid())
    {
        return;
    }

    nearby_buildings.Add(InBuilding);
}

TSharedPtr<FJsonObject> FGPTContext::ToJsonObject() const
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
    for (const FGPTContextNearBuilding& Building : nearby_buildings)
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
