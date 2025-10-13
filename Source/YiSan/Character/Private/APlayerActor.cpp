// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "APlayerActor.h"

#include "UMainWidget.h"
#include "GameLogging.h"
#include "UVoiceConversationSystem.h"
#include "FComponentHelper.h"
#include "UHttpNetworkSystem.h"
#include "ABuilding.h"
#include "UGameDataManager.h"
#include "FBuildingData.h"
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

	MeshComp = this->GetMesh();
	MoveComp = this->GetCharacterMovement();
	AnimInstance = MeshComp->GetAnimInstance();
	
	MainWidgetInst = CreateWidget<UMainWidget>(GetWorld(), MainWidgetClass);
	if (MainWidgetInst)
		MainWidgetInst->AddToViewport();

	// 일어나라 서버
	UHttpNetworkSystem::Get(GetWorld())->RequestHealth( FResponseHealthDelegate() );
}

void APlayerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckBuildingInView();
}

void APlayerActor::CheckBuildingInView()
{
	if (!FollowCamera)
		return;

	const FVector Start = FollowCamera->GetComponentLocation();
	const FVector End = Start + FollowCamera->GetForwardVector() * 5000.f;

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);

	EBuildingType TempBuildingType = EBuildingType::None;

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams))
	{
		if (AActor* HitActor = HitResult.GetActor())
		{
			if (HitActor->ActorHasTag(GameTags::Building))
			{
				if (const ABuilding* Building = Cast<ABuilding>(HitActor))
				{
					TempBuildingType = Building->BuildingType;
				}
			}
		}
	}

	if (!CurLookBuildingType.IsSet() ||
		CurLookBuildingType.GetValue() != TempBuildingType)
	{
		if (TempBuildingType != EBuildingType::None)
		{
			CurLookBuildingType = TempBuildingType;

			EBuildingType TypeValue = CurLookBuildingType.GetValue();
			FText DisplayName = StaticEnum<EBuildingType>()->GetDisplayNameTextByValue((int64)TypeValue);
			PRINT_STRING(TEXT("%s"), *DisplayName.ToString());
		}
		else
		{
			CurLookBuildingType.Reset();
		}
	}
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