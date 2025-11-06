// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UStatSystem.h
 * @brief UStatSystem 클래스를 선언합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "ECharacterType.h"
#include "Components/ActorComponent.h"
#include "UStatSystem.generated.h"

/**
 * @brief 체력, 공격력, 시야 등 캐릭터 기본 스탯을 관리하는 시스템.
 *
 * 스탯 초기화와 증감 로직을 블루프린트에 노출하여 캐릭터 성장과 피해 처리를 담당한다.
 * @ingroup Character
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup=(Dopple), meta=(BlueprintSpawnableComponent) )
class LATTELIBRARY_API UStatSystem : public UActorComponent
{
	GENERATED_BODY()
	
public:
	/** @brief 스탯 기본값을 초기화한다. */
	UStatSystem();

protected:
	/** @brief 소유자 정보를 확인하고 초기 스탯을 설정한다. */
	virtual void BeginPlay() override;

public:
	/**
	 * @brief 매 프레임 스탯 관련 갱신이 필요한 경우 처리한다.
	 * @param DeltaTime 틱 간격(초).
	 * @param TickType 틱 호출 유형.
	 * @param ThisTickFunction 내부 틱 포인터.
	 */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	/** @brief 플레이어 캐릭터인지 여부를 반환한다. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
	FORCEINLINE bool IsPlayer()
	{
		return bIsPlayer;
	}

	/** @brief 캐릭터가 사망 상태인지 확인한다. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
	FORCEINLINE bool IsDead()
	{
		return bIsDead;
	}

	/** @brief 최대 체력을 반환한다. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
	FORCEINLINE float GetMaxHP()
	{
		return MaxHP;
	}

	/** @brief 현재 체력을 반환한다. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
	FORCEINLINE float GetCurHP()
	{
		return CurHP;
	}

	/** @brief 폭발 사격이 가능한 상태인지 확인한다. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
	FORCEINLINE bool IsBlastShotEnable()
	{
		return true;
	}

	/** @brief 현재 시야 거리(cm)를 반환한다. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
	FORCEINLINE float GetSightLength()
	{
		return SightLength;
	}

	/** @brief 현재 시야 각도(도)를 반환한다. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
	FORCEINLINE float GetSightAngle()
	{
		return SightAngle;
	}	
	
	/**
	 * @brief 랜덤 피해 변동을 계산한다.
	 * @param Damage 기본 피해량.
	 * @return 변동이 적용된 피해량.
	 */
	UFUNCTION(BlueprintCallable, Category="Stats")
	float GetRandDmg(float Damage);

	/** @brief 현재 공격력을 반환한다. */
	UFUNCTION(BlueprintCallable, Category="Stats")
	float GetAttackDamage();

	/**
	 * @brief 스탯을 초기화한다.
	 * @param InIsPlayer 플레이어 여부.
	 * @param InECharacterType 캐릭터 타입.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Stats")
	void InitStat(const bool InIsPlayer, const ECharacterType InECharacterType);
	
	/**
	 * @brief 체력을 증가시킨다.
	 * @param InHealPoint 회복 수치.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Stats")
	void IncreaseHealth(float InHealPoint);	
	
	/**
	 * @brief 체력을 감소시키고 사망 여부를 반환한다.
	 * @param InDamagePoint 감소시킬 피해량.
	 * @return 체력이 0 이하가 되면 true.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Stats")
	bool DecreaseHealth(float InDamagePoint);	
	
protected:
	// Health
	/** @brief 현재 체력 값. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|HP")
	float CurHP = 1000;
	/** @brief 최대 체력 값. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|HP")
	float MaxHP = 1000;

	// Attack
	/** @brief 기본 공격력. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats")
	float AttackDamage = 30;

	/** @brief 감지 가능한 시야 거리(cm). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|Sight")
	float SightLength = 1000;
	/** @brief 감지 가능한 시야 각도(도). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|Sight")
	float SightAngle = 45;

protected:
	/** @brief 캐릭터 유형. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|State")
	ECharacterType CharacterType;
	/** @brief 플레이어 여부. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|State")
	bool bIsPlayer = false;

	/** @brief 사망 상태 여부. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|State")
	bool bIsDead = false;
};
