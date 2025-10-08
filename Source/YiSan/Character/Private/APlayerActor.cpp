// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "APlayerActor.h"
#include "UVoiceRecordSystem.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "UMainWidget.h"
#include "FComponentHelper.h"

#define MAINWIDGET_PATH TEXT("/Game/CustomContents/UI/WBP_Main.WBP_Main_C")

APlayerActor::APlayerActor()
{
	PrimaryActorTick.bCanEverTick = true;

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

	VoiceRecordSystem = CreateDefaultSubobject<UVoiceRecordSystem>(TEXT("VoiceRecordSystem"));
}

void APlayerActor::BeginPlay()
{
	Super::BeginPlay();

	MeshComp = this->GetMesh();
	MoveComp = this->GetCharacterMovement();
	AnimInstance = MeshComp->GetAnimInstance();

	if (MainWidgetClass)
	{
		MainWidgetInst = CreateWidget<UMainWidget>(GetWorld(), MainWidgetClass);
		if (MainWidgetInst)
			MainWidgetInst->AddToViewport();
	}
}

void APlayerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
	VoiceRecordSystem->RecordStart();
}

void APlayerActor::Cmd_RecordEnd_Implementation()
{
	VoiceRecordSystem->RecordStop();
}