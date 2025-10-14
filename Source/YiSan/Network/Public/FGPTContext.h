#pragma once

#include "CoreMinimal.h"
#include "FGPTContext.generated.h"

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

	void Reset();
	void Set(const FString& InName, const FVector& InPosition);
	bool IsValid() const;
	TSharedPtr<FJsonObject> ToJsonObject() const;
};

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

	void Reset();
	void Set(const FString& InName, float InDistanceMeters);
	bool IsValid() const;
	TSharedPtr<FJsonObject> ToJsonObject() const;
};

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

	void Reset();
	bool HasAnyData() const;
	void AddNearbyBuilding(const FGPTContextNearBuilding& InBuilding);
	TSharedPtr<FJsonObject> ToJsonObject() const;
};