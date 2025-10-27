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
#include "GameLogging.h"
#include "UBroadcastManager.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"

#include "UDialogManager.h"

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
#define IA_SHOWMOUSE_PATH			TEXT("/Game/CustomContents/Input/IA_Game_Mouse.IA_Game_Mouse")

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
	IA_ShowMouse = FComponentHelper::LoadAsset<UInputAction>(IA_SHOWMOUSE_PATH);
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
			PRINTLOG( TEXT("[YiSan_PC] Pawn missing - request restart"));
			if (AGameModeBase* GM = UGameplayStatics::GetGameMode(this))
			{
				GM->RestartPlayer(this); // GameMode 통해 재스폰
			}
		}
		else
		{
			PRINTLOG( TEXT("[YiSan_PC] Pawn OK: %s"), *GetNameSafe(GetPawn()));
			GetPawn()->EnableInput(this);
		}
	});


	if ( auto BroadcastManager = UBroadcastManager::Get(GetWorld()) )
		BroadcastManager->OnPlayerControlState.AddDynamic(this, &APlayerControl::OnPlayerControlState);
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

		EIC->BindAction(IA_ShowMouse, ETriggerEvent::Started, this, &APlayerControl::OnShowMouse);
		EIC->BindAction(IA_ShowMouse, ETriggerEvent::Completed, this, &APlayerControl::OnHideMouse);
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

void APlayerControl::OnShowMouse(const FInputActionValue& Value)
{
	if (IControllable* C = GetControllable())
		C->Cmd_ShowMouse();
}

void APlayerControl::OnHideMouse(const FInputActionValue& Value)
{
	if (IControllable* C = GetControllable())
		C->Cmd_HideMouse();
}
//------------------------------로딩 관련-------------------------------------

void APlayerControl::ServerStartMapTravel(const FString& MapPath)
{
	// 서버 권한 체크
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("[Travel] 클라이언트가 ServerStartMapTravel 호출 시도 - 무시됨"));
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("[Travel] 서버가 맵 전환 시작: %s"), *MapPath);

	// 1. 모든 클라이언트에게 로딩 UI 표시 알림
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerControl* PC = Cast<APlayerControl>(It->Get()))
		{
			PC->Client_ShowLoadingScreen();
		}
	}

	// 2. 서버도 로딩 UI 표시
	if (IsLocalController())
	{
		Client_ShowLoadingScreen_Implementation();
	}

	// 3. 약간의 딜레이 후 ServerTravel 호출 (UI가 먼저 뜨도록)
	FTimerHandle TravelTimerHandle;
	GetWorldTimerManager().SetTimer(TravelTimerHandle, [this, MapPath]()
	{
		FString TravelURL = MapPath + TEXT("?listen");
        
		UE_LOG(LogTemp, Display, TEXT("[Travel] ServerTravel 실행: %s"), *TravelURL);
		GetWorld()->ServerTravel(TravelURL, true); // true = SeamlessTravel
	}, 0.1f, false);
}

void APlayerControl::Server_RequestMapTravel_Implementation(const FString& MapPath)
{
	UE_LOG(LogTemp, Display, TEXT("[Travel] 클라이언트로부터 맵 전환 요청 받음: %s"), *MapPath);
    
	// 서버에서 실제 맵 전환 실행
	ServerStartMapTravel(MapPath);
}

void APlayerControl::Client_ShowLoadingScreen_Implementation()
{
	
	if (!LoadingWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Travel] LoadingWidgetClass가 설정되지 않음"));
		return;
	}

	// 이미 표시 중이면 스킵
	if (LoadingWidget && LoadingWidget->IsInViewport())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Travel] 로딩 UI 이미 표시 중"));
		return;
	}

	// 로딩 위젯 생성
	LoadingWidget = CreateWidget<UUserWidget>(this, LoadingWidgetClass);
	if (LoadingWidget)
	{
		LoadingWidget->AddToViewport(9999);
        
		// 입력 비활성화
		SetInputMode(FInputModeUIOnly());
		bShowMouseCursor = false;

		UE_LOG(LogTemp, Display, TEXT("[Travel] 로딩 UI 표시 완료 (NetMode: %d)"), (int32)GetNetMode());
	}
}

void APlayerControl::Client_HideLoadingScreen_Implementation()
{
	if (LoadingWidget && LoadingWidget->IsInViewport())
	{
		LoadingWidget->RemoveFromParent();
		LoadingWidget = nullptr;

		// 입력 활성화
		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;

		UE_LOG(LogTemp, Display, TEXT("[Travel] 로딩 UI 제거 완료"));
	}
}

void APlayerControl::ClientRPC_ShowToastMessage_Implementation(const FString& Message)
{
	if (Message.IsEmpty())
	{
		PRINTLOG(TEXT("[PlayerController] Ignored empty toast message"));
		return;
	}
	
	PRINTLOG(TEXT("[PlayerController] Display toast: %s"), *Message);
	UDialogManager::Toast(this, Message);
}