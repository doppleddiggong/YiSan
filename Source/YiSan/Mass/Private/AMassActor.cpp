// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "AMassActor.h"

#include "ADasanActor.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"

AMassActor::AMassActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMassActor::BeginPlay()
{
	Super::BeginPlay();
	// 루트 또는 충돌 담당 컴포넌트 얻기
	UPrimitiveComponent* CollisionComp = Cast<UPrimitiveComponent>(GetRootComponent());
	// 모든 DasanActor 인스턴스 찾아 무시 추가
	TArray<AActor*> FoundDasans;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADasanActor::StaticClass(), FoundDasans);
	for (AActor* D : FoundDasans)
	{
		if (D && D != this)
		{
			CollisionComp->IgnoreActorWhenMoving(D, true);
		}
	}
}

void AMassActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMassActor::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}


