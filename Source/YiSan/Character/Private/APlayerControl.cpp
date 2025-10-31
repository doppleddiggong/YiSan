// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file APlayerControl.cpp
 * @brief APlayerControl 구현에 대한 Doxygen 주석을 제공합니다.
 */
#include "APlayerControl.h"

#include "ADasanActor.h"
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
#include "YiSanPlayerListManager.h"

#include "UDialogManager.h"
#include "APlayerActor.h"
#include "AYisanGameState.h"
#include "UAnswerStateSystem.h"
#include "AYiSanPlayerState.h"
#include "UNetworkGameInstanceSubsystem.h"
#include "UYiSanGameInstance.h"
#include "ULoadingTransitionManager.h"
#include "UYiSanLoading.h"
#include "UObject/UnrealType.h"

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

	if (IsLocalController())
	{
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

		// 클라이언트에서 닉네임을 서버로 전송
		if (UYiSanGameInstance* GI = GetGameInstance<UYiSanGameInstance>())
		{
			if (UNetworkGameInstanceSubsystem* NetworkSubsystem = GI->GetSubsystem<UNetworkGameInstanceSubsystem>())
			{
				FString Nickname = NetworkSubsystem->GetPlayerNickname();
				if (!Nickname.IsEmpty())
				{
					Server_SetPlayerNickname(Nickname);
				}
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

void APlayerControl::OnPossess(APawn* InPawn)
{
	bAwaitFinish = false;

    Super::OnPossess(InPawn);

    if (IsLocalController())
    {
        if (UYiSanGameInstance* GI = GetGameInstance<UYiSanGameInstance>())
        {
            if (UNetworkGameInstanceSubsystem* NetworkSubsystem = GI->GetSubsystem<UNetworkGameInstanceSubsystem>())
            {
                const FString Nickname = NetworkSubsystem->GetPlayerNickname();
                if (!Nickname.IsEmpty())
                {
                    Server_SetPlayerNickname(Nickname);
                }
            }
        }
   	
    	if (bAwaitFinish && GetPawn())
    		CompleteLoading();
    }
}

void APlayerControl::ClientTravelWithLoading(const FString& URL, ETravelType TravelType, bool bSeamlessTravel, FGuid MapPackageGuid)
{
	PRINTLOG(TEXT("[Travel] ClientTravelWithLoading request URL=%s, Type=%s, Seamless=%s"),
		*URL,
		*ENUM_TO_NAME(ETravelType, TravelType),
		bSeamlessTravel ? TEXT("true") : TEXT("false"));

	bPawnReady = false;
	bAwaitFinish = false;

	if (auto LoadingSubsystem = UYiSanLoading::Get(GetWorld()))
		LoadingSubsystem->PrepareClientTravel(URL, TravelType, bSeamlessTravel);

	Super::ClientTravel(URL, TravelType, bSeamlessTravel, MapPackageGuid );
}


void APlayerControl::Server_SetPlayerNickname_Implementation(const FString& Nickname)
{
    if (AYiSanPlayerState* YSPlayerState = GetPlayerState<AYiSanPlayerState>())
    {
        // GameState에서 플레이어 인덱스를 받아와서 함께 설정
        if (AGameStateBase* GS = GetWorld()->GetGameState())
        {
            int32 NewIndex = GS->PlayerArray.Num() - 1;
            YSPlayerState->SetPlayerInfo(Nickname, NewIndex);
            // 닉네임이 설정된 직후, PlayerListManager를 찾아 목록 업데이트를 요청합니다.
            if (AYiSanPlayerListManager* PlayerListManager = Cast<AYiSanPlayerListManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AYiSanPlayerListManager::StaticClass())))
            {
                PlayerListManager->UpdatePlayerListAndBroadcast();
            }
        }
    }
}

void APlayerControl::ServerRPC_ShowToastMessage_Implementation(const FString& Message)
{
	// 권한 검증 (예: 스팸 방지)                                                                                                                                        
	if (!CanSendToast())
		return;

	// 시간 업데이트                                                                                                                                                
	LastToastTime = GetWorld()->GetTimeSeconds();
	
	// GameState의 Multicast 호출                                                                                                                                       
	if (auto GS = GetWorld()->GetGameState<AYisanGameState>())
		GS->MulticastRPC_ToastMessage(Message);
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
		PRINTLOG( TEXT("[Travel] 클라이언트가 ServerStartMapTravel 호출 시도 - 무시됨"));
		return;
	}

	PRINTLOG(  TEXT("[Travel] 서버가 맵 전환 시작: %s"), *MapPath);

	// 약간의 딜레이 후 ServerTravel 호출 (UI가 먼저 뜨도록)
	FTimerHandle TravelTimerHandle;
	GetWorldTimerManager().SetTimer(TravelTimerHandle, [this, MapPath]()
	{
		if (UWorld* World = GetWorld())
		{
			for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
			{
				if (APlayerControl* PlayerControl = Cast<APlayerControl>(It->Get()))
				{
					PlayerControl->ClientRPC_ShowLoadingTransition();
				}
			}
		}

		if (auto TM = ULoadingTransitionManager::Get(this))
		{
			TM->ShowLoadingScreen();
		}

		if (auto GI = UYiSanLoading::Get(GetWorld()))
		{
			FString TravelURL = MapPath + TEXT("?listen");
			GI->InitSystem(TravelURL, true, true);
		}
	}, 0.1f, false);
}

void APlayerControl::ClientRPC_ShowLoadingTransition_Implementation()
{
	PRINTLOG(TEXT("[LOADING_FLOW] ClientRPC_ShowLoadingTransition received - showing loading screen"));
	ShowLoadingScreenLocal();
}

void APlayerControl::ClientRPC_HideLoadingTransition_Implementation()
{
	PRINTLOG(TEXT("[LOADING_FLOW] ClientRPC_HideLoadingTransition received - checking if ready to hide"));
	HandleLoadingComplete();
}

void APlayerControl::HandleLoadingComplete()
{
	PRINTLOG(TEXT("[LOADING_FLOW] HandleLoadingComplete called - IsLocal: %s"), IsLocalController() ? TEXT("true") : TEXT("false"));

	if (!IsLocalController())
		return;

	if (!IsReadyToFinish())
	{
		if (!bAwaitFinish)
		{
			if (auto CtrlPawn = GetPawn())
				PRINTLOG(TEXT("[LOADING_FLOW] Pawn not fully initialized - deferring loading completion for %s"), *GetNameSafe(CtrlPawn));
			else
				PRINTLOG(TEXT("[LOADING_FLOW] Pawn not ready - deferring loading completion for %s"), *GetName());
		}

		bAwaitFinish = true;
		return;
	}

	PRINTLOG(TEXT("[LOADING_FLOW] All conditions met - proceeding to CompleteLoading"));
	CompleteLoading();
}

void APlayerControl::ShowLoadingScreenLocal()
{
    if (auto TM = ULoadingTransitionManager::Get(this))
    {
        TM->ShowLoadingScreen();
    }
}

void APlayerControl::CompleteLoading()
{
	PRINTLOG(TEXT("[LOADING_FLOW] CompleteLoading called - waiting for rendering preparation"));
	bAwaitFinish = false;

	// 렌더링 준비(SkyLight capture, texture preparation)를 위해 추가 대기
	FTimerHandle RenderWaitHandle;
	GetWorld()->GetTimerManager().SetTimer(
		RenderWaitHandle,
		[this]()
		{
			PRINTLOG(TEXT("[LOADING_FLOW] Rendering preparation complete - hiding loading screen now"));

			if (auto TM = ULoadingTransitionManager::Get(this))
			{
				TM->HideLoadingScreen();
			}

			SetInputMode(FInputModeGameOnly());
			bShowMouseCursor = false;

			PRINTLOG(TEXT("[Travel] Loading transition completed for %s"), *GetName());
		},
		3.0f, // 렌더링 준비 대기 시간
		false);
}

void APlayerControl::OnPawnReady(APawn& InPawn)
{
	if (!IsLocalController())
		return;

	if (&InPawn != GetPawn())
		return;

	if (bPawnReady)
		return;

	PRINTLOG(TEXT("[LOADING_FLOW] OnPawnReady called for %s"), *GetNameSafe(&InPawn));

	bPawnReady = true;

	if (bAwaitFinish)
		CompleteLoading();
}

void APlayerControl::OnPawnHasName()
{
	if (!IsLocalController())
		return;

	if (bPawnHasName)
		return;

	PRINTLOG(TEXT("[LOADING_FLOW] OnPawnHasName called"));
	bPawnHasName = true;

	if (bAwaitFinish)
		CompleteLoading();
}

bool APlayerControl::IsReadyToFinish() const
{
	PRINTLOG(TEXT("[LOADING_FLOW] IsReadyToFinish check - Pawn: %s, bPawnReady: %s, bPawnHasName: %s"),
		GetPawn() ? TEXT("true") : TEXT("false"),
		bPawnReady ? TEXT("true") : TEXT("false"),
		bPawnHasName ? TEXT("true") : TEXT("false"));

	// Pawn과 Name 준비 체크
	if (GetPawn() == nullptr)
	{
		PRINTLOG(TEXT("[LOADING_FLOW] NOT READY: Pawn is null"));
		return false;
	}

	if (!bPawnReady)
	{
		PRINTLOG(TEXT("[LOADING_FLOW] NOT READY: bPawnReady is false"));
		return false;
	}

	if (!bPawnHasName)
	{
		PRINTLOG(TEXT("[LOADING_FLOW] NOT READY: bPawnHasName is false"));
		return false;
	}

	// 클라이언트 측 로딩 완료 체크 (WorldPartition, Texture, LevelInstance)
	if (auto LoadingSubsystem = UYiSanLoading::Get(GetWorld()))
	{
		bool bLoadingComplete = LoadingSubsystem->IsLoadingComplete();
		PRINTLOG(TEXT("[LOADING_FLOW] Client-side loading complete: %s"), bLoadingComplete ? TEXT("true") : TEXT("false"));

		if (!bLoadingComplete)
		{
			PRINTLOG(TEXT("[LOADING_FLOW] NOT READY: Client-side loading not complete yet - deferring"));
			return false;
		}
	}
	else
	{
		PRINTLOG(TEXT("[LOADING_FLOW] WARNING: LoadingSubsystem not found!"));
	}

	PRINTLOG(TEXT("[LOADING_FLOW] READY: All conditions satisfied!"));
	return true;
}

void APlayerControl::Server_RequestMapTravel_Implementation(const FString& MapPath)
{
	PRINTLOG( TEXT("[Travel] 클라이언트로부터 맵 전환 요청 받음: %s"), *MapPath);
    
	// 서버에서 실제 맵 전환 실행
	ServerStartMapTravel(MapPath);
}

void APlayerControl::ClientRPC_ShowToastMessage_Implementation(const FString& Message)
{
	if (Message.IsEmpty())
		return;

	if (auto DM = UDialogManager::Get(this))
	{
		DM->ShowToast(Message);
	}
}

void APlayerControl::ClientRPC_UpdateQuestTarget_Implementation(const EBuildingType BuildingType)
{
	if (auto BM = UBroadcastManager::Get(GetWorld()))
	{
		BM->SendUpdateQuest(BuildingType);
	}
}

void APlayerControl::ServerRPC_NotifyRecordingStart_Implementation()
{
	if (!HasAuthority())
		return;

	if (!GetPawn())
	{
		PRINTLOG(TEXT("[PlayerControl] ServerRPC_NotifyRecordingStart - No Pawn!"));
		return;
	}

	// 플레이어 이름 가져오기
	FString PlayerName = TEXT("Unknown");
	if (auto PlayerActor = Cast<APlayerActor>(GetPawn()))
	{
		PlayerName = PlayerActor->GetPlayerDisplayName();
	}

	PRINTLOG(TEXT("[PlayerControl] ServerRPC_NotifyRecordingStart - Player: %s"), *PlayerName);

	// GameState를 통해 Dasan에게 전달
	if (AYisanGameState* GS = GetWorld()->GetGameState<AYisanGameState>())
	{
		if (GS->DasanNPC && GS->DasanNPC->AnswerStateSystem)
		{
			GS->DasanNPC->AnswerStateSystem->TryStartAnswer(PlayerName);
		}

		// BroadcastManager를 통해 UI 업데이트
		if (UBroadcastManager* BM = UBroadcastManager::Get(GetWorld()))
		{
			BM->SendAudioCapture(true);
		}
	}
}

void APlayerControl::ServerRPC_NotifyRecordingEnd_Implementation()
{
	if (!HasAuthority())
		return;

	PRINTLOG(TEXT("[PlayerControl] ServerRPC_NotifyRecordingEnd"));

	// BroadcastManager를 통해 UI 업데이트
	if (UBroadcastManager* BM = UBroadcastManager::Get(GetWorld()))
	{
		BM->SendAudioCapture(false);
	}
}

void APlayerControl::ServerRPC_TryStartAnswer_Implementation(const FString& PlayerName)
{
	if (!HasAuthority())
		return;

	PRINTLOG(TEXT("[PlayerControl] ServerRPC_TryStartAnswer - Player: %s"), *PlayerName);

	// GameState를 통해 Dasan에게 전달
	if (AYisanGameState* GS = GetWorld()->GetGameState<AYisanGameState>())
	{
		if (GS->DasanNPC && GS->DasanNPC->AnswerStateSystem)
		{
			GS->DasanNPC->AnswerStateSystem->TryStartAnswer(PlayerName);
		}
	}
}

void APlayerControl::ServerRPC_AnswerReply_Implementation()
{
	if (!HasAuthority())
		return;

	PRINTLOG(TEXT("[PlayerControl] ServerRPC_AnswerReply"));

	// GameState를 통해 Dasan에게 전달
	if (AYisanGameState* GS = GetWorld()->GetGameState<AYisanGameState>())
	{
		if (GS->DasanNPC && GS->DasanNPC->AnswerStateSystem)
		{
			GS->DasanNPC->AnswerStateSystem->AnswerReply();
		}
	}
}

void APlayerControl::ServerRPC_FinishAnswer_Implementation()
{
	if (!HasAuthority())
		return;

	PRINTLOG(TEXT("[PlayerControl] ServerRPC_FinishAnswer"));

	// GameState를 통해 Dasan에게 전달
	if (AYisanGameState* GS = GetWorld()->GetGameState<AYisanGameState>())
	{
		if (GS->DasanNPC && GS->DasanNPC->AnswerStateSystem)
		{
			GS->DasanNPC->AnswerStateSystem->FinishAnswer();
		}
	}
}
