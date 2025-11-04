// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UCameraShakeSystem.h
 * @brief UCameraShakeSystem 클래스를 선언합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UCameraShakeSystem.generated.h"

/**
 * @brief 데미지 타입에 따른 카메라 흔들림을 중계하는 캐릭터 전용 시스템.
 *
 * 소유 캐릭터가 발생시키는 피격 이벤트를 구독해 즉시 흔들림 클래스를 재생한다.
 */
UCLASS( Blueprintable, ClassGroup=(Dopple), meta=(BlueprintSpawnableComponent) )
class LATTELIBRARY_API UCameraShakeSystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	/** @brief 시스템 기본값을 구성한다. */
	UCameraShakeSystem();

public:	
	/**
	 * @brief 소유 캐릭터와 이벤트 바인딩을 초기화한다.
	 * @param InOwner 카메라 흔들림을 전달받을 게임 캐릭터.
	 */
	void InitSystem(class ALatteGameCharacter* InOwner);

	/** @brief 데미지 타입별 카메라 흔들림 클래스를 매핑한다. */
	UPROPERTY(EditDefaultsOnly, Category="CameraMotion")
	TMap<EDamageType, TSubclassOf<UCameraShakeBase>> CameraShakeMap;
	
private:
	/**
	 * @brief 피격 이벤트를 수신해 카메라 흔들림을 재생한다.
	 * @param Target 흔들림을 적용할 대상 액터.
	 * @param Type 수신된 데미지 타입.
	 */
	UFUNCTION()
	void OnCameraShake(AActor* Target, EDamageType Type);

	/**
	 * @brief 외부에서 강제 트리거된 흔들림을 처리한다.
	 * @param Type 재생할 흔들림에 대응하는 데미지 타입.
	 */
	UFUNCTION()
	void OnForceCameraShake(EDamageType Type);

	/** @brief 초기화 시 캐시한 소유 캐릭터 레퍼런스. */
	UPROPERTY()
	TObjectPtr<class ALatteGameCharacter> Owner;
};
