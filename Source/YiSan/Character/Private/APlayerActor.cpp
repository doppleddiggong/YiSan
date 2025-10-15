// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "APlayerActor.h"

#include "FComponentHelper.h"
#include "FGPTContext.h"
#include "GameLogging.h"
#include "UMainWidget.h"
#include "UVoiceConversationSystem.h"
#include "UGPTContextSystem.h"
#include "UHttpNetworkSystem.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "ABuilding.h"
#include "UBroadcastManager.h"
#include "UQuestManager.h"

#include "Camera/CameraComponent.h"
#include "YiSan/YiSan.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

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
    GPTContextSystem = CreateDefaultSubobject<UGPTContextSystem>(TEXT("GPTContextSystem"));
}

void APlayerActor::BeginPlay()
{
    Super::BeginPlay();

    MeshComp = this->GetMesh();
    MoveComp = this->GetCharacterMovement();
    AnimInstance = MeshComp->GetAnimInstance();
    
    MainWidgetInst = CreateWidget<UMainWidget>(GetWorld(), MainWidgetClass);
    if (MainWidgetInst)
        MainWidgetInst->AddToViewport();

    VoiceConversationSystem->InitSystem(this);
    GPTContextSystem->InitSystem(this);

    BroadcastManager = UBroadcastManager::Get(GetWorld());
    
    GetWorldTimerManager().SetTimer(FindNearestBuildingTimerHandle, this, &APlayerActor::FindNearestBuilding, 1.0f, true);

    // 퀘스트 초기화
    UQuestManager::Get(GetWorld())->InitSystem();
    
    // 서버야 일어나라.
    UHttpNetworkSystem::Get(GetWorld())->RequestHealth( FResponseHealthDelegate() );
}

void APlayerActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(FindNearestBuildingTimerHandle);
    Super::EndPlay(EndPlayReason);
}

void APlayerActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}


void APlayerActor::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APlayerActor::FindNearestBuilding()
{
    TArray<AActor*> FoundBuildings;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuilding::StaticClass(), FoundBuildings);

    ABuilding* NearestBuilding = nullptr;
    float MinDistance = TNumericLimits<float>::Max();

    for (AActor* Actor : FoundBuildings)
    {
        if (ABuilding* Building = Cast<ABuilding>(Actor))
        {
            float Distance = GetDistanceTo(Building);
            if (Distance < MinDistance)
            {
                MinDistance = Distance;
                NearestBuilding = Building;
            }
        }
    }

    if (NearestBuilding)
    {
        BroadcastManager->SendNearBuilding(NearestBuilding->BuildingType);
    }
}

FGPTContext APlayerActor::GetGPTContext() const
{
    return GPTContextSystem->GetGPTContext();
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

