// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UGPTContextSystem.h"
#include "APlayerActor.h"
#include "ABuilding.h"

#include "UGameDataManager.h"
#include "FBuildingData.h"
#include "FGPTContext.h"

#include "EngineUtils.h"
#include "GameLogging.h"

#include "Camera/CameraComponent.h"
#include "YiSan/YiSan.h"

UGPTContextSystem::UGPTContextSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGPTContextSystem::InitSystem(APlayerActor* InOwner)
{
    this->Owner = InOwner;
}

void UGPTContextSystem::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if ( Owner == nullptr )
        return;
    
    this->CheckBuildingInView();
}

void UGPTContextSystem::CheckBuildingInView()
{
    const FVector Start = Owner->FollowCamera->GetComponentLocation();
    const FVector End = Start + Owner->FollowCamera->GetForwardVector() * 5000.f;

    FHitResult HitResult;
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(Owner);

    EBuildingType TempBuildingType = EBuildingType::None;
    ABuilding* HitBuilding = nullptr;

    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams))
    {
        if (AActor* HitActor = HitResult.GetActor())
        {
            if (HitActor->ActorHasTag(GameTags::Building))
            {
                if (ABuilding* Building = Cast<ABuilding>(HitActor))
                {
                    TempBuildingType = Building->BuildingType;
                    HitBuilding = Building;
                }
            }
        }
    }

    const bool bTypeChanged = !CurLookBuildingType.IsSet() || CurLookBuildingType.GetValue() != TempBuildingType;

    if (TempBuildingType != EBuildingType::None && HitBuilding)
    {
        if (bTypeChanged)
        {
            CurLookBuildingType = TempBuildingType;
            EBuildingType TypeValue = CurLookBuildingType.GetValue();
            FText DisplayName = StaticEnum<EBuildingType>()->GetDisplayNameTextByValue(static_cast<int64>(TypeValue));

            PRINT_STRING(TEXT("%s"), *DisplayName.ToString());
        }
    }
    else
    {
        if (CurLookBuildingType.IsSet())
            CurLookBuildingType.Reset();
    }
}

struct FBuildingSnapshot
{
    TWeakObjectPtr<ABuilding> Actor;
    FString Name;
    FVector Location;
    float DistanceMeters = 0.0f;
};
 
FGPTContext UGPTContextSystem::GetGPTContext() const
{
    FGPTContext Context;

    UWorld* World = GetWorld();
    if (!World)
        return Context;

    const FVector PlayerLocation = Owner->GetActorLocation();
    UGameDataManager* DataManager = UGameDataManager::Get(World);

    TArray<FBuildingSnapshot> BuildingSnapshots;
    BuildingSnapshots.Reserve(32);

    constexpr float CmToMeters = 0.01f;

    for (TActorIterator<ABuilding> It(World); It; ++It)
    {
        ABuilding* Building = *It;
        if (!IsValid(Building))
        {
            continue;
        }

        FBuildingSnapshot Snapshot;
        Snapshot.Actor = Building;
        Snapshot.Location = Building->GetActorLocation();
        Snapshot.DistanceMeters = FVector::Dist(PlayerLocation, Snapshot.Location) * CmToMeters;
        Snapshot.Name = UGameDataManager::Get(GetWorld())->GetBuildingDataName(Building->BuildingType);
        if (Snapshot.Name.IsEmpty())
        {
            Snapshot.Name = Building->GetName();
        }
        BuildingSnapshots.Add(MoveTemp(Snapshot));
    }

    BuildingSnapshots.Sort([](const FBuildingSnapshot& A, const FBuildingSnapshot& B)
    {
        return A.DistanceMeters < B.DistanceMeters;
    });

    const FString CurLocationName = BuildingSnapshots.Num() > 0 ? BuildingSnapshots[0].Name : FString(TEXT("전하 위치"));
    Context.current_location.Set(CurLocationName, PlayerLocation);

    FBuildingData Params;
    UGameDataManager::Get(GetWorld())->GetBuildingData(CurLookBuildingType.GetValue(), Params);
    Context.focused_object.Set(Params.name, FVector(Params.x, Params.y, Params.z));

    constexpr int32 MaxNearbyBuildings = 4;
    int32 AddedCount = 0;
    TSet<FString> UniqueNames;

    for (const FBuildingSnapshot& Snapshot : BuildingSnapshots)
    {
        if (Snapshot.Name.IsEmpty())
            continue;

        if (UniqueNames.Contains(Snapshot.Name))
            continue;

        FGPTContextNearBuilding Nearby;
        const float RoundedDistance = FMath::RoundToFloat(Snapshot.DistanceMeters * 10.0f) / 10.0f;
        Nearby.Set(Snapshot.Name, RoundedDistance);
        Context.AddNearbyBuilding(Nearby);
        UniqueNames.Add(Snapshot.Name);

        if (++AddedCount >= MaxNearbyBuildings)
            break;
    }

    return Context;
}
