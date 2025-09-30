// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UCameraShakeSystem.generated.h"

UCLASS( Blueprintable, ClassGroup=(Dopple), meta=(BlueprintSpawnableComponent) )
class COFFEELIBRARY_API UCameraShakeSystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCameraShakeSystem();

public:	
	void InitSystem(class AGameCharacter* InOwner);

	UPROPERTY(EditDefaultsOnly, Category="CameraMotion")
	TMap<EDamageType, TSubclassOf<UCameraShakeBase>> CameraShakeMap;
	
private:
	UFUNCTION()
	void OnCameraShake(AActor* Target, EDamageType Type);

	UFUNCTION()
	void OnForceCameraShake(EDamageType Type);

	UPROPERTY()
	TObjectPtr<class AGameCharacter> Owner;
};
