// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "ECharacterType.h"
#include "Components/ActorComponent.h"
#include "UStatSystem.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Dopple), meta=(BlueprintSpawnableComponent) )
class LATTELIBRARY_API UStatSystem : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UStatSystem();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
	FORCEINLINE bool IsPlayer()
	{
		return bIsPlayer;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
	FORCEINLINE bool IsDead()
	{
		return bIsDead;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
	FORCEINLINE float GetMaxHP()
	{
		return MaxHP;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
	FORCEINLINE float GetCurHP()
	{
		return CurHP;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
	FORCEINLINE bool IsBlastShotEnable()
	{
		return true;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
	FORCEINLINE float GetSightLength()
	{
		return SightLength;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
	FORCEINLINE float GetSightAngle()
	{
		return SightAngle;
	}	
	
	UFUNCTION(BlueprintCallable, Category="Stats")
	float GetRandDmg(float Damage);

	UFUNCTION(BlueprintCallable, Category="Stats")
	float GetAttackDamage();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Stats")
	void InitStat(const bool InIsPlayer, const ECharacterType InECharacterType);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Stats")
	void IncreaseHealth(float InHealPoint);	
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Stats")
	bool DecreaseHealth(float InDamagePoint);	
	
protected:
	// Health
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|HP")
	float CurHP = 1000;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|HP")
	float MaxHP = 1000;

	// Attack
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats")
	float AttackDamage = 30;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|Sight")
	float SightLength = 1000;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|Sight")
	float SightAngle = 45;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|State")
	ECharacterType CharacterType;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|State")
	bool bIsPlayer = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|State")
	bool bIsDead = false;
};
