// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file APlayerActor.cpp
 * @brief APlayerActor 구현에 대한 Doxygen 주석을 제공합니다.
 */


#include "APlayerActor.h"

// CombatCharacter Shared
#include "UStatSystem.h"
#include "UHitStopSystem.h"
#include "UKnockbackSystem.h"
#include "UFlySystem.h"

// PlayerActor Only
#include "UCameraShakeSystem.h"

// Shared
#include "Macro.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"

APlayerActor::APlayerActor()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraShakeSystem = CreateDefaultSubobject<UCameraShakeSystem>(TEXT("CameraShakeSystem"));

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(GetCapsuleComponent());
	SpringArmComp->TargetArmLength = 400.f;
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->bInheritPitch = false;
	SpringArmComp->bInheritRoll = false;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void APlayerActor::BeginPlay()
{
	Super::BeginPlay();

	CameraShakeSystem->InitSystem(this);

	// --- Architecture Demo Start ---
	UE_LOG(LogTemp, Log, TEXT("APlayerActor: Setting up one-way dependency demo."));

	bIsCombatStart = true;

	// ActorComponent initialization
	StatSystem->InitStat(true, CharacterType);
	KnockbackSystem->InitSystem(this);
	FlySystem->InitSystem(this, BIND_DYNAMIC_DELEGATE(FEndCallback, this, APlayerActor, OnFlyEnd));
	HitStopSystem->InitSystem(this);
}

void APlayerActor::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (FlySystem)
		FlySystem->OnLand(Hit);
} 

void APlayerActor::Cmd_Move_Implementation(const FVector2D& Axis)
{
	if (!IsMoveEnable() || !Controller)
	{
		return;
	}

	// Use the controller's rotation to determine movement direction.
	const FRotator ControlRotation = Controller->GetControlRotation();
	const EMovementMode CurrentMovementMode = GetCharacterMovement()->MovementMode;

	if (CurrentMovementMode == MOVE_Walking || CurrentMovementMode == MOVE_Falling)
	{
		// For ground movement, only use the Yaw rotation to prevent pitching into the ground.
		const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

		// Calculate forward and right vectors based on the Yaw rotation.
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, Axis.Y);
		AddMovementInput(RightDirection, Axis.X);
	}
	else if (CurrentMovementMode == MOVE_Flying)
	{
		// For flying, use the full 3D rotation of the controller.
		const FRotator FullRotation(ControlRotation.Pitch, ControlRotation.Yaw, 0.0f);

		const FVector ForwardDirection = FRotationMatrix(FullRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(FullRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, Axis.Y);
		AddMovementInput(RightDirection, Axis.X);
	}
}

void APlayerActor::Cmd_Look_Implementation(const FVector2D& Axis)
{
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void APlayerActor::Cmd_AltitudeUp_Implementation()
{
	if ( !IsMoveEnable() )
		return;

	FlySystem->OnAltitudePress(true);
}

void APlayerActor::Cmd_AltitudeDown_Implementation()
{
	if ( !IsMoveEnable() )
		return;

	FlySystem->OnAltitudePress(false);
}

void APlayerActor::Cmd_AltitudeReleased_Implementation()
{
	if ( !IsMoveEnable() )
		return;

	FlySystem->OnAltitudeRelease();
}

void APlayerActor::Cmd_Jump_Implementation()
{
	if ( !IsMoveEnable() )
		return;

	FlySystem->OnJump();
}

void APlayerActor::Cmd_Landing_Implementation()
{
	if ( !IsControlEnable() )
		return;

	FHitResult HitResult;
	FlySystem->OnLand(HitResult);
}
