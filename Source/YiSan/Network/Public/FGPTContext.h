// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "FGPTContext.generated.h"

/// @file FGPTContext.h
/// @brief GPT 상호작용에 사용되는 공간 컨텍스트 구조체를 정의합니다.

/// @brief 위치 정보를 표현하는 GPT 컨텍스트 요소입니다.
USTRUCT(BlueprintType)
struct FGPTContextLocation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString name;

    UPROPERTY(BlueprintReadWrite)
    float x = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float y = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float z = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bValid = false;

    /// @brief 위치 정보를 초기화합니다.
    void Reset();

    /// @brief 이름과 좌표를 설정하고 유효 상태로 전환합니다.
    /// @param InName [in] 위치를 설명하는 이름입니다.
    /// @param InPosition [in] 월드 좌표입니다.
    void Set(const FString& InName, const FVector& InPosition);

    /// @brief 구조체가 유효한 데이터를 보유하고 있는지 확인합니다.
    bool IsValid() const;

    /// @brief JSON 직렬화를 위한 객체를 생성합니다.
    TSharedPtr<FJsonObject> ToJsonObject() const;
};

/// @brief 주변 건물 정보를 표현하는 GPT 컨텍스트 요소입니다.
USTRUCT(BlueprintType)
struct FGPTContextNearBuilding
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString name;

    UPROPERTY(BlueprintReadWrite)
    float distance = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bValid = false;

    /// @brief 필드를 초기 상태로 되돌립니다.
    void Reset();

    /// @brief 건물 이름과 거리를 설정합니다.
    /// @param InName [in] 건물 고유 이름입니다.
    /// @param InDistanceMeters [in] 플레이어와의 거리(미터)입니다.
    void Set(const FString& InName, float InDistanceMeters);

    /// @brief 데이터가 유효한지 여부를 반환합니다.
    bool IsValid() const;

    /// @brief JSON 직렬화를 위한 객체를 생성합니다.
    TSharedPtr<FJsonObject> ToJsonObject() const;
};

/// @brief 플레이어 주변 맥락을 GPT에 전달하기 위한 전체 컨텍스트입니다.
USTRUCT(BlueprintType)
struct FGPTContext
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FGPTContextLocation current_location;

    UPROPERTY(BlueprintReadWrite)
    FGPTContextLocation focused_object;

    UPROPERTY(BlueprintReadWrite)
    TArray<FGPTContextNearBuilding> nearby_buildings;

    /// @brief 내부 데이터를 초기화합니다.
    void Reset();

    /// @brief 어느 필드라도 유효한 정보를 포함하는지 확인합니다.
    bool HasAnyData() const;

    /// @brief 주변 건물 목록에 항목을 추가합니다.
    /// @param InBuilding [in] 추가할 건물 정보입니다.
    void AddNearbyBuilding(const FGPTContextNearBuilding& InBuilding);

    /// @brief JSON 직렬화를 위한 객체를 생성합니다.
    TSharedPtr<FJsonObject> ToJsonObject() const;
};
