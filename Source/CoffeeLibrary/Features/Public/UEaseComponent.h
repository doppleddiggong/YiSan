// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UEaseComponent.h
 * @brief UEaseComponent 클래스를 선언합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "FEaseHelper.h"
#include "Components/ActorComponent.h"
#include "UEaseComponent.generated.h"

/**
 * @brief 단일 float 값을 시간에 따라 Ease 처리하는 트랙 데이터입니다.
 */
USTRUCT(BlueprintType)
struct FEaseFloatTrack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEaseType EaseType = EEaseType::Linear;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Current = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Start = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Target = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 1.f;
	float ElapsedTime = 0.f;

    /** @brief 진행도를 0~1 범위로 반환한다. */
    float GetAlpha() const
    {
        return FMath::Clamp(Duration > 0.f ? ElapsedTime / Duration : 1.f, 0.f, 1.f);
}

    /** @brief 경과 시간을 누적하고 Ease 결과를 갱신한다. */
    void Update(float DeltaTime)
    {
        ElapsedTime += DeltaTime;
        float Alpha = FMath::Clamp(Duration > 0.f ? ElapsedTime / Duration : 1.f, 0.f, 1.f);
        Current = FMath::Lerp(Start, Target, FEaseHelper::Ease(Alpha, EaseType));
    }
};

/**
 * @brief FVector 값을 시간에 따라 Ease 처리하는 트랙 데이터입니다.
 */
USTRUCT(BlueprintType)
struct FEaseVectorTrack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEaseType EaseType = EEaseType::Linear;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Current = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Start = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Target = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 1.f;

	float ElapsedTime = 0.f;

    /** @brief 진행도를 0~1 범위로 반환한다. */
    float GetAlpha() const
    {
        return FMath::Clamp(Duration > 0.f ? ElapsedTime / Duration : 1.f, 0.f, 1.f);
    }

    /** @brief 경과 시간을 누적하고 Ease 결과를 갱신한다. */
    void Update(float DeltaTime)
    {
        ElapsedTime += DeltaTime;
        float Alpha = FMath::Clamp(Duration > 0.f ? ElapsedTime / Duration : 1.f, 0.f, 1.f);
        Current = FMath::Lerp(Start, Target, FEaseHelper::Ease(Alpha, EaseType));
    }
};

/**
 * @brief FRotator 값을 시간에 따라 Ease 처리하는 트랙 데이터입니다.
 */
USTRUCT(BlueprintType)
struct FEaseRotatorTrack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEaseType EaseType = EEaseType::Linear;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Current = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Start =  FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Target = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 1.f;

	float ElapsedTime = 0.f;

    /** @brief 진행도를 0~1 범위로 반환한다. */
    float GetAlpha() const
    {
        return FMath::Clamp(Duration > 0.f ? ElapsedTime / Duration : 1.f, 0.f, 1.f);
    }

    /** @brief 경과 시간을 누적하고 Ease 결과를 갱신한다. */
    void Update(float DeltaTime)
    {
        ElapsedTime += DeltaTime;
        float Alpha = FMath::Clamp(Duration > 0.f ? ElapsedTime / Duration : 1.f, 0.f, 1.f);
        Current = FMath::Lerp(Start, Target, FEaseHelper::Ease(Alpha, EaseType));
    }
};


/**
 * @brief 다양한 타입의 Ease 트랙을 업데이트하는 범용 컴포넌트.
 */
UCLASS( ClassGroup=(CoffeeLibrary), meta=(BlueprintSpawnableComponent, DisplayName="EaseComponent") )
class COFFEELIBRARY_API UEaseComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UEaseComponent();

public:
    /** @brief 매 프레임 모든 트랙을 업데이트한다. */
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    /** @brief 외부 호출용 트랙 업데이트 함수. */
    void UpdateTrack(float DeltaTime);

    /** @brief 트랙 진행도를 반환한다. */
    UFUNCTION(BlueprintPure, Category="Track")
    float GetTrackAlpha(FName TrankName) const;

    /** @brief float 트랙의 현재 값을 반환한다. */
    UFUNCTION(BlueprintPure, Category="Track")
    float GetEaseFloatTrack(FName TrackName);
    /** @brief float 트랙을 설정하거나 갱신한다. */
    UFUNCTION(BlueprintCallable, Category="Track")
    void SetEaseFloatTrack(FName TrackName, EEaseType EaseType, float Start, float Target, float Duration = 1.f);

    /** @brief vector 트랙의 현재 값을 반환한다. */
    UFUNCTION(BlueprintPure, Category="Track")
    FVector GetEaseVectorTrack(FName TrackName);
    /** @brief vector 트랙을 설정하거나 갱신한다. */
    UFUNCTION(BlueprintCallable, Category="Track")
    void SetEaseVectorTrack(FName TrackName, EEaseType EaseType, FVector Start, FVector Target, float Duration = 1.f);

    /** @brief rotator 트랙의 현재 값을 반환한다. */
    UFUNCTION(BlueprintPure, Category="Track")
    FRotator GetEaseRotatorTrack(FName TrackName);
    /** @brief rotator 트랙을 설정하거나 갱신한다. */
    UFUNCTION(BlueprintCallable, Category="Track")
    void SetEaseRotatorTrack(FName TrackName, EEaseType EaseType, FRotator Start, FRotator Target, float Duration = 1.f);
	
private:
	TMap<FName, FEaseFloatTrack> FloatTracks;
	TMap<FName, FEaseVectorTrack> VectorTracks;
	TMap<FName, FEaseRotatorTrack> RotatorTracks;
};
