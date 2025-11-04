// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "ABuilding.h"

#include "APlayerActor.h"
#include "AQuestManagerActor.h"
#include "GameLogging.h"
#include "UBroadcastManager.h"
#include "Components/BoxComponent.h"
#include "YiSan/YiSan.h"

/**
 * @file ABuilding.cpp
 * @brief ABuilding의 동작을 구현합니다.
 */

/** @brief 건물용 충돌 볼륨과 표시 메시를 설정합니다. */
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

/** @brief 겹침 델리게이트를 바인딩하고 퀘스트 알림을 등록합니다. */
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

/**
 * @brief 액터 겹침을 처리하여 퀘스트 및 브로드캐스트 매니저에 알립니다.
 * @param OverlappedComp 겹침을 발생시킨 컴포넌트입니다.
 * @param OtherActor 볼륨에 진입한 액터입니다.
 * @param OtherComp 상대 액터에 속한 컴포넌트입니다.
 * @param OtherBodyIndex 다중 바디 겹침 시 추가 바디 인덱스입니다.
 * @param bFromSweep 겹침이 스윕에서 시작되었는지 여부를 나타냅니다.
 * @param SweepResult 겹침 이벤트 중 포착된 히트 정보입니다.
 */
void ABuilding::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 자기 자신이나, OtherActor가 유효하지 않으면 무시
	if (OtherActor && OtherActor != this  && OtherActor->ActorHasTag(GameTags::Dasan))
	{
		// 로그 출력으로 트리거 발동 확인
		PRINTLOG(TEXT("%s entered the trigger volume of %s"), *OtherActor->GetName(), *this->GetName());
		BroadcastManager->SendContactBuilding(BuildingType);

		if (AQuestManagerActor* QuestManager = AQuestManagerActor::Get(this))
			QuestManager->OnContactBuilding(BuildingType);
	}
}

/** @brief 건물이 현재 퀘스트 목표가 되면 안내 기둥을 토글합니다. */
void ABuilding::OnUpdateQuest(EBuildingType InBuildingType)
{
	bIsNextTargetBuilding = BuildingType == InBuildingType;
	LightPillarMesh->SetVisibility(bIsNextTargetBuilding);
}