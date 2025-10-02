// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UBroadcastManger.h
 * @brief UBroadcastManger 선언에 대한 Doxygen 주석을 제공합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "Macro.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UBroadcastManger.generated.h"

UCLASS()
class LATTELIBRARY_API UBroadcastManger : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
    DEFINE_SUBSYSTEM_GETTER_INLINE(UBroadcastManger);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessage, FString, Msg);
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnMessage OnMessage;
	
	UFUNCTION(BlueprintCallable, Category="Events")
	void SendMessage(const FString& InMsg);


	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHitStop, AActor*, Target, EDamageType, Type);
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnHitStop OnHitStop;

	UFUNCTION(BlueprintCallable, Category="Events")
	void SendHitStop(AActor* Target, const EDamageType Type);

	UFUNCTION(BlueprintCallable, Category="Events")
	void SendHitStopPair(AActor* Attacker, const EDamageType AttackerType,
						 AActor* Target,   const EDamageType TargetType);

	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCameraShake, AActor*, Target, EDamageType, Type);
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnCameraShake OnCameraShake;

	UFUNCTION(BlueprintCallable, Category="Events")
	void SendCameraShake(AActor* Target, const EDamageType Type);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnForceCameraShake, EDamageType, Type);
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnForceCameraShake OnForceCameraShake;

	UFUNCTION(BlueprintCallable, Category="Events")
	void SendForceCameraShake(const EDamageType Type);

	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnKnockback,
		AActor*,		   Target,
		AActor*,           Instigator,
		EDamageType,  Type,
		float,             Resistance );
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnKnockback OnKnockback;

	UFUNCTION(BlueprintCallable, Category="Events")
	void SendKnockback(AActor* Target, AActor* Instigator, EDamageType Type, float Resistance);
};
