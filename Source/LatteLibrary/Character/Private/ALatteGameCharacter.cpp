// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file AGameCharacter.cpp
 * @brief AGameCharacter 구현에 대한 Doxygen 주석을 제공합니다.
 */
#include "ALatteGameCharacter.h"

#include "UStatSystem.h"
#include "UHitStopSystem.h"
#include "UFlySystem.h"
#include "UKnockbackSystem.h"

#include "UCharacterData.h"
#include "EMontageType.h"
#include "GameLogging.h"

#include "FCharacterAssetData.h"
#include "UGameDataManager.h"

#include "Components/ArrowComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceConstant.h"

ALatteGameCharacter::ALatteGameCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	StatSystem			= CreateDefaultSubobject<UStatSystem>(TEXT("StatSystem"));
	HitStopSystem		= CreateDefaultSubobject<UHitStopSystem>(TEXT("HitStopSystem"));
	KnockbackSystem		= CreateDefaultSubobject<UKnockbackSystem>(TEXT("KnockbackSystem"));
	FlySystem			= CreateDefaultSubobject<UFlySystem>(TEXT("FlySystem"));
	
	LeftHandComp = CreateDefaultSubobject<UArrowComponent>(TEXT("LeftHandComp"));
	RightHandComp = CreateDefaultSubobject<UArrowComponent>(TEXT("RightHandComp"));
	LeftFootComp = CreateDefaultSubobject<UArrowComponent>(TEXT("LeftFootComp"));
	RightFootComp = CreateDefaultSubobject<UArrowComponent>(TEXT("RightFootComp"));
}

void ALatteGameCharacter::BeginPlay()
{
	Super::BeginPlay();

	MoveComp = this->GetCharacterMovement();

	this->SetFallingToWalk();
 }

void ALatteGameCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AnimInstance && bDelegatesBound)
		UnbindMontageDelegates(AnimInstance);
	bDelegatesBound = false;
	
	Super::EndPlay(EndPlayReason);
}


void ALatteGameCharacter::SetupCharacterFromType(const ECharacterType Type, const bool bIsAnother)
{
	this->CharacterType = Type;
	
	FCharacterAssetData Params;
	UGameDataManager::Get(GetWorld())->GetCharacterAssetData(Type, Params);

	if (Params.CharacterDataAsset.IsValid())
		this->CharacterData = Params.CharacterDataAsset.Get();
	else
		this->CharacterData = Params.CharacterDataAsset.LoadSynchronous();

	MeshComp = GetMesh();
    if ( IsValid(MeshComp) )
    {
        if (CharacterData->MeshData.IsValid())
			MeshComp->SetSkeletalMesh(CharacterData->MeshData.Get());
        else
            MeshComp->SetSkeletalMesh(CharacterData->MeshData.LoadSynchronous());

    	MeshComp->EmptyOverrideMaterials();
    	for (int32 i = 0; i < CharacterData->MaterialArray.Num(); ++i)
    	{
    		UMaterialInstanceConstant* MaterialInst = nullptr;
    		if (CharacterData->MaterialArray[i].IsValid())
    			MaterialInst = CharacterData->MaterialArray[i].Get();
    		else
    			MaterialInst = CharacterData->MaterialArray[i].LoadSynchronous();

    		if (MaterialInst)
    		{
    			auto DynamicMat = UMaterialInstanceDynamic::Create(MaterialInst, MeshComp);
    			if (DynamicMat)
    			{
    				// 1번이 Cloth
    				if (i == 1)
    					DynamicMat->SetScalarParameterValue(FName("AnotherSide"), bIsAnother ? 1.0f : 0.0f);
    				MeshComp->SetMaterial(i, DynamicMat);
    			}
    		}
    	}
    	
    	MeshComp->SetRelativeLocation( CharacterData->RelativeLocation);
    	MeshComp->SetRelativeRotation(  CharacterData->RelativeRotator );
    	MeshComp->SetRelativeScale3D( CharacterData->RelativeScale );
    	
        if (CharacterData->AnimBluePrint.IsValid())
            MeshComp->SetAnimInstanceClass(CharacterData->AnimBluePrint.Get());
        else
            MeshComp->SetAnimInstanceClass(CharacterData->AnimBluePrint.LoadSynchronous());

    	auto Rule = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
    	
    	LeftHandComp->AttachToComponent(MeshComp, Rule, CharacterData->LeftHandSocketName);
    	RightHandComp->AttachToComponent(MeshComp, Rule, CharacterData->RightHandSocketName);
    	LeftFootComp->AttachToComponent(MeshComp, Rule, CharacterData->LeftFootSocketName);
    	RightFootComp->AttachToComponent(MeshComp, Rule, CharacterData->RightFootSocketName);

    	AnimInstance = MeshComp->GetAnimInstance();
    	BindMontageDelegates(AnimInstance);
    }
}


void ALatteGameCharacter::BindMontageDelegates(UAnimInstance* Anim)
{
	if (!Anim || bDelegatesBound)
		return;

	// 중복 방지용으로 먼저 제거
	Anim->OnPlayMontageNotifyBegin.RemoveDynamic(this, &ALatteGameCharacter::OnMontageNotifyBegin);
	Anim->OnPlayMontageNotifyBegin.AddDynamic(this, &ALatteGameCharacter::OnMontageNotifyBegin);
	bDelegatesBound = true;
}

void ALatteGameCharacter::UnbindMontageDelegates(UAnimInstance* Anim)
{
	if (!Anim || !bDelegatesBound)
		return;

	Anim->OnPlayMontageNotifyBegin.RemoveDynamic(this, &ALatteGameCharacter::OnMontageNotifyBegin);

	bDelegatesBound = false;
}


void ALatteGameCharacter::OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
	UE_LOG(LogTemp, Warning, TEXT("Notify Fired: %s"), *NotifyName.ToString());
}

/** @copydoc IsControlEnable */
bool ALatteGameCharacter::IsControlEnable_Implementation()
{
	if ( this->IsHolding() )
		return false;
	
	if ( IsCombatStart() == false || IsCombatResult())
	{
		// 전투 시작 전
		// 전투 결과 후
		return false;
	}
	
	if ( IsHit )
		return false;

	if ( StatSystem->IsDead() )
		return false;

	return true;
}

bool ALatteGameCharacter::IsMoveEnable_Implementation()
{
	if ( !IsControlEnable() )
		return false;

	return true;
}

bool ALatteGameCharacter::IsAttackEnable_Implementation()
{
	if ( IsHit )
		return false;

	return true;
}

bool ALatteGameCharacter::IsDead_Implementation()
{
	return StatSystem->IsDead();
}

bool ALatteGameCharacter::IsHitting_Implementation()
{
	return IsHit;
}

bool ALatteGameCharacter::IsInSight(const AActor* Other) const
{
	const FVector SelfLoc = GetActorLocation();
	const FVector OtherLoc = Other->GetActorLocation();
	const FVector ToOther = OtherLoc - SelfLoc;

	const float Dist = ToOther.Size();
	if (Dist > SightRange)
		return false;

	const FVector Fwd = GetActorForwardVector();
	const float CosHalfFOV = FMath::Cos(FMath::DegreesToRadians(SightHalfFOVDeg));
	const float CosAngle = FVector::DotProduct(Fwd, ToOther.GetSafeNormal());
	if (CosAngle < CosHalfFOV)
		return false;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(PlayerSightLOS), false, this);

	const bool bBlocked = GetWorld()->LineTraceSingleByChannel(
		Hit,
		SelfLoc + FVector(0,0,50),
		OtherLoc + FVector(0,0,50),
		ECC_Visibility,
		Params
	);
    
	if (bBlocked && Hit.GetActor() != Other)
		return false;
	
	return true;
}



void ALatteGameCharacter::OnLookTarget_Implementation()
{
	if (!TargetActor)
		return;

	const FVector TargetLoc = TargetActor->GetActorLocation();
	const FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetLoc);
	const FRotator NewRot(0.f, LookAt.Yaw, 0.f);

	SetActorRotation(NewRot);
}

void ALatteGameCharacter::OnFlyEnd_Implementation()
{
}

void ALatteGameCharacter::SetFlying()
{
	MoveComp->SetMovementMode(MOVE_Flying);
	PrevMoveMode = EMovementMode::MOVE_Flying;

	this->bUseControllerRotationYaw = true;
	this->bUseControllerRotationPitch = true;
	MoveComp->bOrientRotationToMovement = false;
}

void ALatteGameCharacter::SetFallingToWalk()
{
	MoveComp->SetMovementMode( EMovementMode::MOVE_Falling );
	PrevMoveMode = EMovementMode::MOVE_Falling;

	this->bUseControllerRotationYaw = true;
	this->bUseControllerRotationPitch = false;
	MoveComp->bOrientRotationToMovement = false;
}

void ALatteGameCharacter::RecoveryMovementMode(const EMovementMode InMovementMode)
{
	if ( InMovementMode == MOVE_None)
		return;
	
	auto Movement = this->GetCharacterMovement();

	if ( InMovementMode == MOVE_Flying )
	{
		Movement->SetMovementMode( EMovementMode::MOVE_Flying );
		this->bUseControllerRotationYaw = true;
		this->bUseControllerRotationPitch = true;
		Movement->bOrientRotationToMovement = false;
	}
	else
	{
		Movement->SetMovementMode( InMovementMode );
		this->bUseControllerRotationYaw = false;
		this->bUseControllerRotationPitch = false;
		Movement->bOrientRotationToMovement = true;
	}
}

void ALatteGameCharacter::PlayTypeMontage(const EMontageType Type)
{
	UAnimMontage* AnimMontage = nullptr;

	switch (Type)
	{
	case EMontageType::Death:
		AnimMontage = DeathMontage;
		break;
	}

	this->PlayTargetMontage(AnimMontage);
}

void ALatteGameCharacter::PlayTargetMontage(UAnimMontage* AnimMontage)
{
	if ( IsValid(AnimInstance) && IsValid(AnimMontage) )
	{
		AnimInstance->Montage_Play(
			AnimMontage,
			1.0f,
			EMontagePlayReturnType::MontageLength,
			0.f,
			true);
	}
	else
	{
		PRINTLOG(TEXT("Failed to PlayMontage"));
	}
}

void ALatteGameCharacter::StopTargetMontage(const EMontageType Type, const float BlendInOutTime)
{
	UAnimMontage* AnimMontage = nullptr;

	switch (Type)
	{
	case EMontageType::Death:
		AnimMontage = DeathMontage;
		break;
	}
	
	AnimInstance->Montage_Stop(BlendInOutTime, AnimMontage );
}
