// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "AGate.h"

#include "UBroadcastManger.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
// #include "FComponentHelper.h"
#include "GameFramework/Pawn.h"
// #include "Materials/MaterialInstanceConstant.h"
#include "YiSan/YiSan.h"

// namespace
// {
//     static const TCHAR* Door_Mesh_Path = TEXT("/Game/HwaseongHaenggung/Meshes/Lower_Parts/SM_D_Gate_1.SM_D_Gate_1");
//     static const TCHAR* Door_Mat0_Path = TEXT("/Game/HwaseongHaenggung/Material/MI_KoreanWood_3.MI_KoreanWood_3");
//     static const TCHAR* Door_Mat1_Path = TEXT("/Game/HwaseongHaenggung/Material/MI_Metal.MI_Metal");
// }

AGate::AGate()
{
    RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = RootComp;
    
    BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
    BoxCollision->SetupAttachment(RootComponent);

    // BoxCollision->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
    // BoxCollision->SetBoxExtent(FVector(200.f, 200.f, 200.f));
    BoxCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    BoxCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    BoxCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    BoxCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &AGate::OnOverlapBegin);
    BoxCollision->OnComponentEndOverlap.AddDynamic(this, &AGate::OnOverlapEnd);

    Door_Left = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door_Left"));
    Door_Left->SetupAttachment(RootComponent);

    Door_Right = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door_Right"));
    Door_Right->SetupAttachment(RootComponent);

    //
    // if( auto DoorMesh = FComponentHelper::LoadAsset<UStaticMesh>(Door_Mesh_Path) )
    // {
    //     Door_Left->SetStaticMesh(DoorMesh);
    //     Door_Right->SetStaticMesh(DoorMesh);
    // }
    //
    // if ( auto WoodMaterial = FComponentHelper::LoadAsset<UMaterialInstanceConstant>(Door_Mat0_Path) )
    // {
    //     Door_Left->SetMaterial(0, WoodMaterial);
    //     Door_Right->SetMaterial(0, WoodMaterial);
    // }
    //
    // if ( auto MetalMaterial = FComponentHelper::LoadAsset<UMaterialInstanceConstant>(Door_Mat1_Path) )
    // {
    //     Door_Left->SetMaterial(1, MetalMaterial);
    //     Door_Right->SetMaterial(1, MetalMaterial);
    // }

    // Door_Left->SetRelativeLocation(FVector(-200.f, 0.f, 0.f));
    //
    // Door_Right->SetRelativeLocation(FVector(190.f, 0.f, 0.f));
    // Door_Right->SetRelativeScale3D(FVector(-1.f, 1.f, 1.f));

    bIsOpen = false;
    GateID = -1;

    PrimaryActorTick.bCanEverTick = false;
}

void AGate::BeginPlay()
{
    Super::BeginPlay();


    if (auto EventManager = UBroadcastManger::Get(this))
    {
        EventManager->OnDoorMessage.AddDynamic(this, &AGate::OnDoorMessage);
    }
}

void AGate::OnDoorMessage(const int32 InGateID, const bool Open)
{
    if ( InGateID != this->GateID)
    {
        // 나한테 명령한거 아님
        return;
    }

    if ( Open == this->bIsOpen)
    {
        // 내가 또 해야하는거 아님
        return;
    }

    if ( Open)
    {
        OpenDoor();
    }
    else
    {
        CloseDoor();
    }
}

void AGate::OpenDoor()
{
    if (bIsOpen == true)
        return;

    bIsOpen = true;
    PlayOpenDoorAnimation();
}

void AGate::CloseDoor()
{
    if (bIsOpen == false)
        return;
    
    bIsOpen = false;
    PlayCloseDoorAnimation();
}

bool AGate::IsPlayerActor(const AActor* OtherActor) const
{
    if ( OtherActor == nullptr)
        return false;

    if ( OtherActor == this)
        return false;

    if ( OtherActor->ActorHasTag(GameTags::Player) == false)
        return false;

    return true;
}

void AGate::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (IsPlayerActor(OtherActor))
    {
        if ( auto Pawn = Cast<APawn>(OtherActor) )
        {
            if ( OverlappingPawns.Contains(Pawn) == false )
                OverlappingPawns.Add(Pawn);
       }
        
        OpenDoor();
    }
}

void AGate::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (IsPlayerActor(OtherActor))
    {
        if ( auto Pawn = Cast<APawn>(OtherActor) )
            OverlappingPawns.Remove(Pawn);

        if (OverlappingPawns.Num() == 0)
            CloseDoor();
    }
}