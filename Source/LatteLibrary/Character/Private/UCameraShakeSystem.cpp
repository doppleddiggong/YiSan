// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UCameraShakeSystem.cpp
 * @brief UCameraShakeSystem 구현에 대한 Doxygen 주석을 제공합니다.
 */
#include "UCameraShakeSystem.h"
#include "AGameCharacter.h"
#include "FPadFeedbackData.h"
#include "UBroadcastManger.h"

#define SMALL_CAMERASHAKE_PATH      TEXT("/Game/CustomContents/Blueprints/CameraShake/BP_SmallCameraShake.BP_SmallCameraShake_C")
#define NORMAL_CAMERASHAKE_PATH     TEXT("/Game/CustomContents/Blueprints/CameraShake/BP_NormalCameraShake.BP_NormalCameraShake_C")
#define LARGE_CAMERASHAKE_PATH      TEXT("/Game/CustomContents/Blueprints/CameraShake/BP_LargeCameraShake.BP_LargeCameraShake_C")
#define HUGE_CAMERASHAKE_PATH		TEXT("/Game/CustomContents/Blueprints/CameraShake/BP_HugeCameraShake.BP_HugeCameraShake_C")

UCameraShakeSystem::UCameraShakeSystem()
{
	PrimaryComponentTick.bCanEverTick = false;

	{
		static ConstructorHelpers::FClassFinder<UCameraShakeBase> TempCameraShake(SMALL_CAMERASHAKE_PATH);
		if ( TempCameraShake.Succeeded())
			CameraShakeMap.Add( EDamageType::Small, TempCameraShake.Class);
	}

	{
		static ConstructorHelpers::FClassFinder<UCameraShakeBase> TempCameraShake(NORMAL_CAMERASHAKE_PATH);
		if ( TempCameraShake.Succeeded())
			CameraShakeMap.Add( EDamageType::Normal, TempCameraShake.Class);
	}

	{
		static ConstructorHelpers::FClassFinder<UCameraShakeBase> TempCameraShake(LARGE_CAMERASHAKE_PATH);
		if ( TempCameraShake.Succeeded())
			CameraShakeMap.Add( EDamageType::Large, TempCameraShake.Class);
	}

	{
		static ConstructorHelpers::FClassFinder<UCameraShakeBase> TempCameraShake(HUGE_CAMERASHAKE_PATH);
		if ( TempCameraShake.Succeeded())
			CameraShakeMap.Add( EDamageType::Huge, TempCameraShake.Class);
	}
}

void UCameraShakeSystem::InitSystem(AGameCharacter* InOwner)
{
	this->Owner = InOwner;

	if ( auto EventManager = UBroadcastManger::Get(GetWorld()))
	{
		EventManager->OnCameraShake.AddDynamic(this, &UCameraShakeSystem::OnCameraShake);
		EventManager->OnForceCameraShake.AddDynamic(this, &UCameraShakeSystem::OnForceCameraShake);
	}
}

void UCameraShakeSystem::OnCameraShake(AActor* Target, EDamageType Type)
{
	if (Target != Owner)
		return;

	// 카메라 쉐이크 재생
	OnForceCameraShake(Type);
}

void UCameraShakeSystem::OnForceCameraShake(EDamageType Type)
{
	if ( auto PlayerController = GetWorld()->GetFirstPlayerController() )
	{
		auto FeedBackData = GetPadFeedbackData(Type);
		
		PlayerController->PlayerCameraManager->StartCameraShake(CameraShakeMap[Type]);
		PlayerController->PlayDynamicForceFeedback(FeedBackData.Amplitude, FeedBackData.Duration,
			true, true, true, true);
	}
}