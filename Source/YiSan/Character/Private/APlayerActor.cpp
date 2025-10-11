// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "APlayerActor.h"
#include "UVoiceRecordSystem.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "UMainWidget.h"
#include "FComponentHelper.h"
#include "GameLogging.h"
#include "UBroadcastManger.h"
#include "UHttpNetworkSystem.h"
#include "UVoiceListenSystem.h"
#include "UWebSocketSystem.h"

#define MAINWIDGET_PATH TEXT("/Game/CustomContents/UI/WBP_Main.WBP_Main_C")

APlayerActor::APlayerActor()
{
	PrimaryActorTick.bCanEverTick = true;

	Tags.Add(FName("Player"));

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
	VoiceListenSystem = CreateDefaultSubobject<UVoiceListenSystem>(TEXT("VoiceListenSystem"));
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

	VoiceRecordSystem->OnRecordingStopped.AddDynamic(this, &APlayerActor::OnRecordingStoppedHandler);
	VoiceListenSystem->InitSystem();

	if (auto WebSocketSystem = UWebSocketSystem::Get(GetWorld()))
		WebSocketSystem->Connect();
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

void APlayerActor::OnRecordingStoppedHandler(const FString& FilePath)
{
	if ( auto ReqNetwork = UHttpNetworkSystem::Get(GetWorld()) )
	{
		ReqNetwork->RequestSTT(FilePath,
			FResponseSTTDelegate::CreateUObject( this, &APlayerActor::OnResponseSTT));
	}
}

void APlayerActor::OnResponseSTT(FResponseSTT& ResponseData, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		ResponseData.PrintData();

		if (auto EventManager = UBroadcastManger::Get(this))
			EventManager->SendToastMessage(ResponseData.text);
	}
	else
	{
		PRINTLOG( TEXT("--- Network Response Received (FAIL) ---"));
	}
}



void APlayerActor::RequestHealth()
{
	if ( auto ReqNetwork = UHttpNetworkSystem::Get(GetWorld()) )
	{
		ReqNetwork->RequestHealth( FResponseHealthDelegate::CreateUObject( this, &APlayerActor::OnResponseHealth));
	}
	else
	{
		PRINTLOG( TEXT("UNetworkSystem not found!"));
	}
}

void APlayerActor::OnResponseHealth(FResponseHealth& ResponseData, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		ResponseData.PrintData();
	}
	else
	{
		PRINTLOG( TEXT("--- Network Response Received (FAIL) ---"));
	}
}