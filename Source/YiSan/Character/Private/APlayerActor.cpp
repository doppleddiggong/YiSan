// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "APlayerActor.h"

#include "UMainWidget.h"
#include "GameLogging.h"
#include "UVoiceConversationSystem.h"
#include "FComponentHelper.h"
#include "UHttpNetworkSystem.h"
#include "UWebSocketSystem.h"
#include "ABuilding.h"
#include "UGameDataManager.h"
#include "FBuildingData.h"
#include "EngineUtils.h"
#include "Algo/Sort.h"
#include "Camera/CameraComponent.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "YiSan/YiSan.h"

#define MAINWIDGET_PATH TEXT("/Game/CustomContents/UI/WBP_Main.WBP_Main_C")

APlayerActor::APlayerActor()
{
    PrimaryActorTick.bCanEverTick = true;

    Tags.Add(GameTags::Player);

    MainWidgetClass = FComponentHelper::LoadClass<UMainWidget>(MAINWIDGET_PATH);
    
    SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArmComp->SetupAttachment(GetCapsuleComponent());
    SpringArmComp->TargetArmLength = 400.f;
    SpringArmComp->bUsePawnControlRotation = true;
    SpringArmComp->bInheritPitch = true;
    SpringArmComp->bInheritRoll = false;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
    
    VoiceConversationSystem = CreateDefaultSubobject<UVoiceConversationSystem>(TEXT("VoiceConversationSystem"));
}

void APlayerActor::BeginPlay()
{
    Super::BeginPlay();

    LastContextLocation = GetActorLocation();
    bHasLastContextLocation = true;
    bSpatialContextDirty = true;

    MeshComp = this->GetMesh();
    MoveComp = this->GetCharacterMovement();
    AnimInstance = MeshComp->GetAnimInstance();
    
    MainWidgetInst = CreateWidget<UMainWidget>(GetWorld(), MainWidgetClass);
    if (MainWidgetInst)
        MainWidgetInst->AddToViewport();

    // \uC77C\uC5B4\uB098\uB77C \uC11C\uBC84
    UHttpNetworkSystem::Get(GetWorld())->RequestHealth( FResponseHealthDelegate() );
}

void APlayerActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    CheckBuildingInView();
    UpdateSpatialContext(DeltaTime);
}

void APlayerActor::CheckBuildingInView()
{
    if (!FollowCamera)
    {
        return;
    }

    const FVector Start = FollowCamera->GetComponentLocation();
    const FVector End = Start + FollowCamera->GetForwardVector() * 5000.f;

    FHitResult HitResult;
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(this);

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
    const bool bActorChanged = FocusedBuildingActor.Get() != HitBuilding;

    if (TempBuildingType != EBuildingType::None && HitBuilding)
    {
        if (bTypeChanged)
        {
            CurLookBuildingType = TempBuildingType;
            EBuildingType TypeValue = CurLookBuildingType.GetValue();
            FText DisplayName = StaticEnum<EBuildingType>()->GetDisplayNameTextByValue(static_cast<int64>(TypeValue));
            PRINT_STRING(TEXT("%s"), *DisplayName.ToString());
        }

        if (bActorChanged)
        {
            FocusedBuildingActor = HitBuilding;
        }

        if (bTypeChanged || bActorChanged)
        {
            bSpatialContextDirty = true;
        }
    }
    else
    {
        if (CurLookBuildingType.IsSet() || FocusedBuildingActor.IsValid())
        {
            CurLookBuildingType.Reset();
            FocusedBuildingActor.Reset();
            bSpatialContextDirty = true;
        }
    }
}

void APlayerActor::UpdateSpatialContext(float DeltaTime)
{
    constexpr float SpatialContextUpdateInterval = 0.5f;
    constexpr float MovementThresholdCm = 150.0f;

    SpatialContextElapsed += DeltaTime;

    const FVector CurrentLocation = GetActorLocation();
    if (!bHasLastContextLocation || FVector::DistSquared(CurrentLocation, LastContextLocation) > FMath::Square(MovementThresholdCm))
    {
        LastContextLocation = CurrentLocation;
        bHasLastContextLocation = true;
        bSpatialContextDirty = true;
    }

    if (!bSpatialContextDirty && SpatialContextElapsed < SpatialContextUpdateInterval)
    {
        return;
    }

    SpatialContextElapsed = 0.0f;

    FGPTSpatialContext Context = BuildSpatialContextSnapshot();
    bSpatialContextDirty = false;

    if (!Context.HasAnyData())
    {
        return;
    }

    if (UWebSocketSystem* WebSocketSystem = UWebSocketSystem::Get(GetWorld()))
    {
        WebSocketSystem->SendSpatialContext(Context);
    }
}

FGPTSpatialContext APlayerActor::BuildSpatialContextSnapshot() const
{
    FGPTSpatialContext Context;

    UWorld* World = GetWorld();
    if (!World)
    {
        return Context;
    }

    const FVector PlayerLocation = GetActorLocation();
    UGameDataManager* DataManager = UGameDataManager::Get(World);

    auto ResolveName = [DataManager](EBuildingType Type) -> FString
    {
        if (DataManager)
        {
            FBuildingData Data;
            if (DataManager->GetBuildingData(Type, Data) && !Data.name.IsEmpty())
            {
                return Data.name;
            }
        }

        if (Type != EBuildingType::None)
        {
            return StaticEnum<EBuildingType>()->GetDisplayNameTextByValue(static_cast<int64>(Type)).ToString();
        }

        return FString();
    };

    struct FBuildingSnapshot
    {
        TWeakObjectPtr<ABuilding> Actor;
        FString Name;
        FVector Location;
        float DistanceMeters = 0.0f;
    };

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
        Snapshot.Name = ResolveName(Building->BuildingType);
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

    const FString CurrentLocationName = BuildingSnapshots.Num() > 0 ? BuildingSnapshots[0].Name : FString(TEXT("\uC804\uD558 \uC704\uCE58"));
    Context.current_location.Set(CurrentLocationName, PlayerLocation);

    if (FocusedBuildingActor.IsValid())
    {
        if (ABuilding* Focused = FocusedBuildingActor.Get())
        {
            FString FocusName = ResolveName(Focused->BuildingType);
            if (FocusName.IsEmpty())
            {
                FocusName = Focused->GetName();
            }
            Context.focused_object.Set(FocusName, Focused->GetActorLocation());
        }
    }

    const int32 MaxNearbyBuildings = 4;
    int32 AddedCount = 0;
    TSet<FString> UniqueNames;

    for (const FBuildingSnapshot& Snapshot : BuildingSnapshots)
    {
        if (Snapshot.Name.IsEmpty())
        {
            continue;
        }

        if (UniqueNames.Contains(Snapshot.Name))
        {
            continue;
        }

        FGPTSpatialContextNearbyBuilding Nearby;
        const float RoundedDistance = FMath::RoundToFloat(Snapshot.DistanceMeters * 10.0f) / 10.0f;
        Nearby.Set(Snapshot.Name, RoundedDistance);
        Context.AddNearbyBuilding(Nearby);
        UniqueNames.Add(Snapshot.Name);

        if (++AddedCount >= MaxNearbyBuildings)
        {
            break;
        }
    }

    return Context;
}


void APlayerActor::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APlayerActor::Cmd_Move_Implementation(const FVector2D& Axis)
{
    if ( !Controller)
        return;

    const FRotator ControlRotation = Controller->GetControlRotation();
    const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    AddMovementInput(ForwardDirection, Axis.Y);
    AddMovementInput(RightDirection, Axis.X);
}

void APlayerActor::Cmd_Look_Implementation(const FVector2D& Axis)
{
    AddControllerYawInput(Axis.X);
    AddControllerPitchInput(Axis.Y);
}

void APlayerActor::Cmd_Jump_Implementation()
{
    this->Jump();
}

void APlayerActor::Cmd_Chat_Implementation()
{
    if (MainWidgetInst)
        MainWidgetInst->ToggleChatBox();
}

void APlayerActor::Cmd_RecordStart_Implementation()
{
    PRINT_STRING(TEXT("Cmd_RecordStart_Implementation"));
    VoiceConversationSystem->StartRecording();
}

void APlayerActor::Cmd_RecordEnd_Implementation()
{
    PRINT_STRING(TEXT("Cmd_RecordEnd_Implementation"));
    VoiceConversationSystem->StopRecording();
}

