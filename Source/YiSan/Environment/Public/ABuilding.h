// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EBuildingType.h"
#include "GameFramework/Actor.h"
#include "ABuilding.generated.h"

UCLASS()
class YISAN_API ABuilding : public AActor
{
	GENERATED_BODY()

public:
	ABuilding();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnUpdateQuest(EBuildingType InBuildingType);
	
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBuildingType BuildingType = EBuildingType::Yeomingak;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* boxcomp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsNextTargetBuilding = false;

protected:
	// 빛기둥 역할을 할 스태틱 메시 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, Category = "Building")
	TObjectPtr<class UStaticMeshComponent> LightPillarMesh;

	UPROPERTY()
	TObjectPtr<class UBroadcastManager> BroadcastManager;
};