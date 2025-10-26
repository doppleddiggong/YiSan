// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UGPTContextSystem.h"
#include "APlayerActor.h"
#include "ABuilding.h"

#include "UGameDataManager.h"
#include "FBuildingData.h"
#include "FGPTContext.h"

#include "EngineUtils.h"
#include "FComponentHelper.h"
#include "UBroadcastManager.h"

#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "YiSan/YiSan.h"

UGPTContextSystem::UGPTContextSystem()
{
	PrimaryComponentTick.bCanEverTick = true;

    TimeSinceLastCheck = 0.0f;       
}

void UGPTContextSystem::InitSystem(APlayerActor* InOwner)
{
    this->Owner = InOwner;

    BroadcastManager = UBroadcastManager::Get(GetWorld());
}

void UGPTContextSystem::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if ( Owner == nullptr )
        return;

    // 로컬 플레이어에 대해서만 GPT 컨텍스트를 업데이트합니다.
    // Host 클라이언트에서 Guest 플레이어의 GPTContextSystem이 동작하지 않도록 방지합니다.
    if (!Owner->IsLocallyControlled())
        return;
        
    // DeltaTime을 계속 더해줍니다.                                                                                                                                                                                                                           
    TimeSinceLastCheck += DeltaTime;                                                                                                                                                                                                                          
                                                                                                                                                                                                                                                             
    // TimeSinceLastCheck가 1초를 넘어섰는지 확인합니다.                                                                                                                                                                                                      
    if (TimeSinceLastCheck >= 1.0f)                                                                                                                                                                                                                           
    {                                                                                                                                                                                                                                                         
       // 타이머를 리셋합니다. (정확한 주기를 위해 1.0f를 빼줍니다)                                                                                                                                                                                          
       TimeSinceLastCheck -= 1.0f;                                                                                                                                                                                                                           
                                                                                                                                                                                                                                                             
       // 1초 마다 실행될 로직                                                                                                                                                                                                                                
       this->CheckBuildingInView();                                                                                                                                                                                                                          
       this->FindNearestBuilding();                                                                                                                                                                                                                          
    }
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
        if (auto HitActor = HitResult.GetActor())
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

    const bool bTypeChanged = !FocusBuildingType.IsSet() || FocusBuildingType.GetValue() != TempBuildingType;

    if (TempBuildingType != EBuildingType::None && HitBuilding)
    {
        if (bTypeChanged)
        {
            FocusBuildingType = TempBuildingType;
            BroadcastManager->SendFocusBuilding(FocusBuildingType.GetValue());
        }
    }
    else
    {
        if (FocusBuildingType.IsSet())
        {
            FocusBuildingType.Reset();
            BroadcastManager->SendFocusBuilding( EBuildingType::None );
        }
    }
}


void UGPTContextSystem::FindNearestBuilding()
{
    auto FoundBuildings = FComponentHelper::GetAllOfClass<ABuilding>(GetWorld());

    float MinDistance = TNumericLimits<float>::Max();

    for (auto Building : FoundBuildings)
    {
        float Distance = Owner->GetDistanceTo(Building);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            NearestBuilding = Building;
        }
    }
    
    if (NearestBuilding.Get())
    {
        // 근접 했다는 정보로 small popup 을 띄울 예정
        BroadcastManager->SendNearBuilding(NearestBuilding->BuildingType);
    }
    else
    {
        // 건물이 없다면 popup 을 닫을예정
        BroadcastManager->SendNearBuilding(EBuildingType::None);
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

    if (FocusBuildingType.IsSet())
    {
        FBuildingData Params;
        if (UGameDataManager::Get(GetWorld())->GetBuildingData(FocusBuildingType.GetValue(), Params))
        {
            Context.focused_object.Set(Params.name, FVector(Params.x, Params.y, Params.z));
        }
    }

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
