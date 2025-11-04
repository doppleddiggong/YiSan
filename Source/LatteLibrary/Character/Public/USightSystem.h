// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file USightSystem.h
 * @brief USightSystem 클래스를 선언합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "USightSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSightDetect, bool, Target);

/**
 * @brief 시야 각도와 거리를 기반으로 목표 감지 여부를 판별하는 시스템.
 *
 * 선형 트레이스를 통해 가시성 여부를 평가하고 결과를 델리게이트로 방송한다.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup=(Dopple), meta=(BlueprintSpawnableComponent) )
class LATTELIBRARY_API USightSystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	/** @brief 기본 감지 거리와 각도를 설정한다. */
	USightSystem();

protected:
	/** @brief 오너 액터를 캐시하고 내부 상태를 초기화한다. */
	virtual void BeginPlay() override;

public:	
	/**
	 * @brief 매 프레임 시야 감지와 타이머를 갱신한다.
	 * @param DeltaTime 틱 간격(초).
	 * @param TickType 틱 호출 유형.
	 * @param ThisTickFunction 내부 틱 포인터.
	 */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * @brief 시야 시스템을 초기화하고 대상과 파라미터를 설정한다.
	 * @param TargetActor 추적할 목표 액터.
	 * @param SightLength 허용 시야 거리(cm).
	 * @param SightAngle 허용 시야 각도(도).
	 */
	UFUNCTION(BlueprintCallable)
	void InitSightSystem(class AActor* TargetActor, const float SightLength, const float SightAngle);
	
private:
	/**
	 * @brief 선형 트레이스를 수행해 시야 감지를 평가한다.
	 * @param InLength 검사할 최대 거리(cm).
	 * @param InAngle 검사할 최대 각도(도).
	 */
	void SightLineTrace(const float InLength, const float InAngle) const;
	/**
	 * @brief 목표와의 거리 조건을 확인한다.
	 * @param InLength 허용 거리.
	 * @return 시야 거리가 조건을 만족하면 true.
	 */
	bool DetectSightLength(const float InLength) const;
	/**
	 * @brief 목표와의 각도 조건을 확인한다.
	 * @param InAngle 허용 각도.
	 * @return 시야 각도가 조건을 만족하면 true.
	 */
	bool DetectSightAngle(const float InAngle) const;

	/** @brief 내부 타이머를 증가시켜 주기적인 검사에 사용한다. */
	void AddElapsedTime();	
	/** @brief 경과 시간 기반 보간 알파 값을 계산한다. */
	float LerpAlpha() const;
	
public:
	/** @brief 감지 결과를 브로드캐스트하는 델리게이트. */
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnSightDetect OnSightDetect;

	/** @brief 디버그 선을 그릴지 여부. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug")
	bool bDrawDebugLine = false;
	
private:
	/** @brief 시스템을 보유한 액터. */
	UPROPERTY()
	class AActor* OwnerActor;
	
	/** @brief 감지 대상으로 지정된 액터. */
	UPROPERTY()
	class AActor* TargetActor;

	/** @brief 최근 감지 성공 여부. */
	bool DetectTarget = false;
	/** @brief 허용 시야 거리(cm). */
	float SightLength = 1000.0f;
	/** @brief 허용 시야 각도(도). */
	float SightAngle = 45.0f;
	/** @brief 감지 판정에 사용할 보간 시간(초). */
	float Duration = 1.0f;
	/** @brief 경과 시간(초). */
	float ElapsedTime = 0.0f;
};
