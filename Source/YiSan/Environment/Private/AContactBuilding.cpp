// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "AContactBuilding.h"

#include "Components/BoxComponent.h"
#include "APlayerActor.h"
#include "UBroadcastManager.h"

AContactBuilding::AContactBuilding()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	RootComponent = TriggerVolume;

	TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
}

void AContactBuilding::BeginPlay()
{
	Super::BeginPlay();

	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AContactBuilding::OnOverlapBegin);
}

void AContactBuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AContactBuilding::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && (OtherActor != this) && Cast<APlayerActor>(OtherActor))
	{
		if (UBroadcastManager* BroadcastManager = UBroadcastManager::Get(GetWorld()))
		{
			BroadcastManager->SendContactBuilding(BuildingType);
		}
	}
}
