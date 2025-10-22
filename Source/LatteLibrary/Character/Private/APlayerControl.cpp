// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file APlayerControl.cpp
 * @brief APlayerControl 구현에 대한 Doxygen 주석을 제공합니다.
 */
#include "APlayerControl.h"
#include "IControllable.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

#include "InputMappingContext.h"
#include "InputActionValue.h"
#include "InputAction.h"

#include "FComponentHelper.h"
#include "UBroadcastManager.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"

#define IMC_DEFAULT_PATH			TEXT("/Game/CustomContents/Input/IMC_Game_Player.IMC_Game_Player")
#define IA_MOVE_PATH				TEXT("/Game/CustomContents/Input/IA_Game_Movement.IA_Game_Movement")
#define IA_LOOK_PATH				TEXT("/Game/CustomContents/Input/IA_Game_LookAround.IA_Game_LookAround")
#define IA_ALTITUDE_UP_PATH			TEXT("/Game/CustomContents/Input/IA_Game_AltitudeUp.IA_Game_AltitudeUp")
#define IA_ALTITUDE_DOWN_PATH		TEXT("/Game/CustomContents/Input/IA_Game_AltitudeDown.IA_Game_AltitudeDown")
#define IA_JUMP_PATH				TEXT("/Game/CustomContents/Input/IA_Game_Jump.IA_Game_Jump")
#define IA_LANDING_PATH				TEXT("/Game/CustomContents/Input/IA_Game_Landing.IA_Game_Landing")
#define IA_CHAT_PATH				TEXT("/Game/CustomContents/Input/IA_Game_Chat.IA_Game_Chat")
#define IA_CHAT_SCROLL_UP_PATH		TEXT("/Game/CustomContents/Input/IA_Game_ChatScrollUp.IA_Game_ChatScrollUp")
#define IA_CHAT_SCROLL_DOWN_PATH	TEXT("/Game/CustomContents/Input/IA_Game_ChatScrollDown.IA_Game_ChatScrollDown")
#define IA_RECORD_PATH				TEXT("/Game/CustomContents/Input/IA_Game_Record.IA_Game_Record")
#define IA_SHOWDETAIL_PATH			TEXT("/Game/CustomContents/Input/IA_Game_Detail.IA_Game_Detail")

APlayerControl::APlayerControl()
{
	IMC_Default = FComponentHelper::LoadAsset<UInputMappingContext>(IMC_DEFAULT_PATH);

	IA_Move = FComponentHelper::LoadAsset<UInputAction>(IA_MOVE_PATH);
	IA_Look = FComponentHelper::LoadAsset<UInputAction>(IA_LOOK_PATH);
	IA_AltitudeUp = FComponentHelper::LoadAsset<UInputAction>(IA_ALTITUDE_UP_PATH);
	IA_AltitudeDown = FComponentHelper::LoadAsset<UInputAction>(IA_ALTITUDE_DOWN_PATH);
	IA_Jump = FComponentHelper::LoadAsset<UInputAction>(IA_JUMP_PATH);
	IA_Landing = FComponentHelper::LoadAsset<UInputAction>(IA_LANDING_PATH);
	IA_Chat = FComponentHelper::LoadAsset<UInputAction>(IA_CHAT_PATH);
	IA_ChatScrollUp = FComponentHelper::LoadAsset<UInputAction>(IA_CHAT_SCROLL_UP_PATH);
	IA_ChatScrollDown = FComponentHelper::LoadAsset<UInputAction>(IA_CHAT_SCROLL_DOWN_PATH);
	IA_Record = FComponentHelper::LoadAsset<UInputAction>(IA_RECORD_PATH);
	IA_ShowDetail = FComponentHelper::LoadAsset<UInputAction>(IA_SHOWDETAIL_PATH);
}

void APlayerControl::BeginPlay()
{
	Super::BeginPlay();

	if (auto LP = GetLocalPlayer())
	{
		if (auto SubSystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (IMC_Default)
			{
				SubSystem->ClearAllMappings();
				SubSystem->AddMappingContext(IMC_Default, 0);
			}
		}
	}

	// 한 프레임 뒤 보정 체크
	GetWorldTimerManager().SetTimerForNextTick([this]()
	{
		if (!GetPawn())
		{
			UE_LOG(LogTemp, Warning, TEXT("[YiSanPlayerController] Pawn missing - request restart"));
			if (AGameModeBase* GM = UGameplayStatics::GetGameMode(this))
			{
				GM->RestartPlayer(this); // GameMode 통해 재스폰
			}
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("[YiSanPlayerController] Pawn OK: %s"), *GetNameSafe(GetPawn()));
			GetPawn()->EnableInput(this);
		}
	});


	if ( auto BroadcastManager = UBroadcastManager::Get(GetWorld()) )
	{
		BroadcastManager->OnPlayerControlState.AddDynamic(this, &APlayerControl::OnPlayerControlState);
	}
}

void APlayerControl::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->BindAction(IA_Move, ETriggerEvent::Triggered,  this, &APlayerControl::OnMove);
		EIC->BindAction(IA_Look, ETriggerEvent::Triggered,  this, &APlayerControl::OnLook);

		EIC->BindAction(IA_AltitudeUp, ETriggerEvent::Started,  this, &APlayerControl::OnAltitudeUp);
		EIC->BindAction(IA_AltitudeUp, ETriggerEvent::Completed, this, &APlayerControl::OnAltitudeReleased);
		EIC->BindAction(IA_AltitudeUp, ETriggerEvent::Canceled, this, &APlayerControl::OnAltitudeReleased);
		
		EIC->BindAction(IA_AltitudeDown, ETriggerEvent::Started,  this, &APlayerControl::OnAltitudeDown);
		EIC->BindAction(IA_AltitudeDown, ETriggerEvent::Completed, this, &APlayerControl::OnAltitudeReleased);
		EIC->BindAction(IA_AltitudeDown, ETriggerEvent::Canceled, this, &APlayerControl::OnAltitudeReleased);

		EIC->BindAction(IA_Jump, ETriggerEvent::Started,    this, &APlayerControl::OnJump);
		EIC->BindAction(IA_Landing, ETriggerEvent::Started,  this, &APlayerControl::OnLanding);

		EIC->BindAction(IA_Chat, ETriggerEvent::Started, this, &APlayerControl::OnChatEnter);
		EIC->BindAction(IA_ChatScrollUp, ETriggerEvent::Started, this, &APlayerControl::OnChatScrollUp);
		EIC->BindAction(IA_ChatScrollDown, ETriggerEvent::Started, this, &APlayerControl::OnChatScrollDown);

		EIC->BindAction(IA_Record, ETriggerEvent::Started, this, &APlayerControl::OnRecordPressed);
		EIC->BindAction(IA_Record, ETriggerEvent::Completed, this, &APlayerControl::OnRecordReleased);

		EIC->BindAction(IA_ShowDetail, ETriggerEvent::Completed, this, &APlayerControl::OnShowDetail);
	}
}

void APlayerControl::OnPlayerControlState(bool bState, UUserWidget* FocusWidget)
{
	if ( bState )
	{
		// 플레이어 컨트롤 Enable 한다
		FInputModeGameOnly InputMode;
		InputMode.SetConsumeCaptureMouseDown(false);
		this->SetInputMode(InputMode);
		this->SetShowMouseCursor(false);

		GetPawn()->EnableInput(this);
	}
	else
	{
		// 플레이어 컨트롤 disable 하자
		FInputModeUIOnly InputMode;
		if (IsValid(FocusWidget))
			InputMode.SetWidgetToFocus(FocusWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		this->SetInputMode(InputMode);
		this->SetShowMouseCursor(true);

		GetPawn()->DisableInput(this);
	}
}


IControllable* APlayerControl::GetControllable() const
{
	APawn* P = GetPawn();
	if (!P)
		return nullptr;

	// UObject 기반 UInterface 라면 Cast 가능
	if (IControllable* C = Cast<IControllable>(P))
		return C;

	return nullptr;
}

void APlayerControl::OnMove(const FInputActionValue& Value)
{
	if (IControllable* C = GetControllable())
		C->Cmd_Move(Value.Get<FVector2D>());
}

void APlayerControl::OnLook(const FInputActionValue& Value)
{
	if (IControllable* C = GetControllable())
		C->Cmd_Look(Value.Get<FVector2D>());
}

void APlayerControl::OnAltitudeUp(const FInputActionValue& Value)
{
	if (IControllable* C = GetControllable())
		C->Cmd_AltitudeUp();
}

void APlayerControl::OnAltitudeDown(const FInputActionValue& Value)
{
	if (IControllable* C = GetControllable())
		C->Cmd_AltitudeDown();
}

void APlayerControl::OnAltitudeReleased(const FInputActionValue& Value)
{
	if (IControllable* C = GetControllable())
		C->Cmd_AltitudeReleased();
}

void APlayerControl::OnJump(const FInputActionValue&)
{
	if (IControllable* C = GetControllable())
		C->Cmd_Jump();
}

void APlayerControl::OnLanding(const FInputActionValue&)
{
	if (IControllable* C = GetControllable())
		C->Cmd_Landing();
}

void APlayerControl::OnChatEnter(const FInputActionValue&)
{
	if (IControllable* C = GetControllable())
		C->Cmd_ChatEnter();
}

void APlayerControl::OnChatScrollUp(const FInputActionValue&)
{
	if (IControllable* C = GetControllable())
		C->Cmd_ChatScrollUp();
}

void APlayerControl::OnChatScrollDown(const FInputActionValue&)
{
	if (IControllable* C = GetControllable())
		C->Cmd_ChatScrollDown();
}

void APlayerControl::OnRecordPressed(const FInputActionValue& Value)
{
	if (IControllable* C = GetControllable())
		C->Cmd_RecordStart();
}

void APlayerControl::OnRecordReleased(const FInputActionValue& Value)
{
	if (IControllable* C = GetControllable())
		C->Cmd_RecordEnd();
}

void APlayerControl::OnShowDetail(const FInputActionValue& Value)
{
	if (IControllable* C = GetControllable())
		C->Cmd_ShowDetail();
}