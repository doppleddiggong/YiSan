// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EBuildingType.h"
#include "FGPTContext.h"
#include "Components/ActorComponent.h"
#include "UGPTContextSystem.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class YISAN_API UGPTContextSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UGPTContextSystem();

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	void InitSystem(class APlayerActor* InOwner);
	FGPTContext GetGPTContext() const;
	
private:
	void CheckBuildingInView();

private:
	UPROPERTY()
	TObjectPtr<class APlayerActor> Owner;
	UPROPERTY()
	TObjectPtr<class UBroadcastManager> BroadcastManager;
	
	UPROPERTY()
	TOptional<EBuildingType> FocusBuildingType;
};
