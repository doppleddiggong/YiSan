// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "AMassActor.h"

#include "ADasanActor.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"

/**
 * @file AMassActor.cpp
 * @brief AMassActor의 동작을 구현합니다.
 */

/** @brief 필요 시 런타임 로직을 수행할 수 있도록 Tick을 활성화합니다. */
AMassActor::AMassActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

/** @brief 플레이 시작 시 다산 액터를 충돌 상호작용에서 제외합니다. */
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

/** @brief 향후 시뮬레이션 업데이트를 위한 Tick 플레이스홀더입니다. */
void AMassActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

/** @brief 군중 액터가 점유되었을 때 플레이어 입력을 바인딩합니다. */
void AMassActor::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}


