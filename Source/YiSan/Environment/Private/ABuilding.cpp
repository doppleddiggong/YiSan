// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "ABuilding.h"

#include "APlayerActor.h"
#include "GameLogging.h"
#include "UBroadcastManager.h"
#include "Components/BoxComponent.h"
#include "YiSan/YiSan.h"

ABuilding::ABuilding()
{
	PrimaryActorTick.bCanEverTick = true;

	Tags.Add(GameTags::Building);

	boxcomp = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
	boxcomp->SetupAttachment(GetRootComponent());

	boxcomp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	boxcomp->SetCollisionResponseToAllChannels(ECR_Overlap);
	
	LightPillarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LightPillarMesh"));
	LightPillarMesh->SetupAttachment(boxcomp);

	// 엔진 기본 실린더 메시를 찾아서 할당합니다.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMeshAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMeshAsset.Succeeded())
	{
		LightPillarMesh->SetStaticMesh(CylinderMeshAsset.Object);
		LightPillarMesh->SetRelativeScale3D(FVector(0.01f, 0.01f, 10.0f));
	}
	
	// 빛기둥은 다른 오브젝트와 충돌할 필요가 없습니다.
	LightPillarMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABuilding::BeginPlay()
{
	Super::BeginPlay();

	// // 빛기둥이 처음에는 보이지 않도록 설정합니다.
	// LightPillarMesh->SetVisibility(false);
	
	if (boxcomp)
		boxcomp->OnComponentBeginOverlap.AddDynamic(this, &ABuilding::OnOverlapBegin);

	BroadcastManager = UBroadcastManager::Get(GetWorld());
	BroadcastManager->OnUpdateQuest.AddDynamic(this, &ABuilding::OnUpdateQuest);
}

void ABuilding::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 자기 자신이나, OtherActor가 유효하지 않으면 무시
	if (OtherActor && OtherActor != this  && OtherActor->ActorHasTag(GameTags::Dasan))	
	{
		// 로그 출력으로 트리거 발동 확인
		PRINTLOG(TEXT("%s entered the trigger volume of %s"), *OtherActor->GetName(), *this->GetName());
		BroadcastManager->SendContactBuilding(BuildingType);
	}
}

void ABuilding::OnUpdateQuest(EBuildingType InBuildingType)
{
	bIsNextTargetBuilding = BuildingType == InBuildingType;
	LightPillarMesh->SetVisibility(bIsNextTargetBuilding);
}