// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FHitStopData.h"
#include "UHitStopSystem.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Dopple),  meta=(BlueprintSpawnableComponent) )
class COFFEELIBRARY_API UHitStopSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UHitStopSystem();

public:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="HitStop")
	void InitSystem(AGameCharacter* InOwner);

	UFUNCTION(BlueprintCallable, Category="HitStop")
	void OnHitStop(AActor* Target, const EDamageType Type);
	
	UFUNCTION(BlueprintCallable, Category="HitStop")
	void ApplyHitStop(const EDamageType Type);
	
private:
	void BeginFreeze(const FHitStopData& Params);
	void EndFreeze();

private:
	UPROPERTY()
	TObjectPtr<class AGameCharacter> Owner;
	UPROPERTY()
	TObjectPtr<class USkeletalMeshComponent> MeshComp;
	UPROPERTY()
	TObjectPtr<class UCharacterMovementComponent> MoveComp;

	FHitStopData LastParams;
	
	bool bActive = false;
	double EndRealTimeSeconds = 0.0;

	float SavedCustomTimeDilation = 1.0f;
};
