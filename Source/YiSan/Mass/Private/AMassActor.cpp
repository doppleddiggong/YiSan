// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "AMassActor.h"
#include "Components/InputComponent.h"

AMassActor::AMassActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMassActor::BeginPlay()
{
	Super::BeginPlay();
}

void AMassActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMassActor::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}


