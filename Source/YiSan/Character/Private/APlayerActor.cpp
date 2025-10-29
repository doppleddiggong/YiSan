// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "APlayerActor.h"

#include "FComponentHelper.h"
#include "FGPTContext.h"
#include "GameLogging.h"
#include "UMainWidget.h"
#include "UVoiceConversationSystem.h"
#include "UGPTContextSystem.h"
#include "UChatPlayerSystem.h"
#include "UChatUIWidget.h"
#include "UChatBoxWidget.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "UBroadcastManager.h"
#include "AQuestManagerActor.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Camera/CameraComponent.h"
#include "YiSan/YiSan.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

#define MAINWIDGET_PATH TEXT("/Game/CustomContents/UI/WBP_Main.WBP_Main_C")
#define CHATUIWIDGET_PATH TEXT("/Game/CustomContents/UI/Chat/WB_ChatUI.WB_ChatUI_C")

APlayerActor::APlayerActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // 멀티플레이어 복제 활성화
    bReplicates = true;
    SetReplicateMovement(true);

    Tags.Add(GameTags::Player);

    MainWidgetClass = FComponentHelper::LoadClass<UMainWidget>(MAINWIDGET_PATH);
    ChatUIWidgetClass = FComponentHelper::LoadClass<UChatUIWidget>(CHATUIWIDGET_PATH);
    
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
    ChatPlayerSystem = CreateDefaultSubobject<UChatPlayerSystem>(TEXT("ChatPlayerSystem"));
}

void APlayerActor::BeginPlay()
{
    Super::BeginPlay();

    MeshComp = this->GetMesh();
    MoveComp = this->GetCharacterMovement();
    AnimInstance = MeshComp->GetAnimInstance();

    // LocalController만 UI 생성
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (PC->IsLocalController())
        {
            MainWidgetInst = CreateWidget<UMainWidget>(GetWorld(), MainWidgetClass);
            if (MainWidgetInst)
            {
                MainWidgetInst->AddToViewport(50);
            }

            if (ChatUIWidgetClass)
            {
                auto ChatUIInst = CreateWidget<UChatUIWidget>(GetWorld(), ChatUIWidgetClass);
                if (ChatUIInst && ChatUIInst->WBP_ChatBox)
                {
                    ChatUIInst->AddToViewport(0);
                    ChatBoxWidget = ChatUIInst->WBP_ChatBox;
                    ChatPlayerSystem->InitSystem(ChatBoxWidget.Get());
                }
            }
        }
    }

    VoiceConversationSystem->InitSystem(this);
    GPTContextSystem->InitSystem(this);

    BroadcastManager = UBroadcastManager::Get(GetWorld());
    BroadcastManager->OnExecVoiceCommand.AddDynamic(this, &APlayerActor::OnExecVoiceCommand);


    if ( HasAuthority() )
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
        {
            if (AQuestManagerActor* QuestManager = AQuestManagerActor::Get(this))
                BroadcastManager->SendUpdateQuest(QuestManager->GetCurrentTarget());

            FChatMessage ChatMessage(EChatMessageType::User, *GetPlayerDisplayName(), TEXT("민지 왔쪄요"));
            ChatPlayerSystem->ServerRPC_SendChatMessage(ChatMessage);
        });
    }
}

void APlayerActor::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    if (IsLocallyControlled())
        UE_LOG(LogTemp, Warning, TEXT("Pawn possessed locally, player can move"));
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

void APlayerActor::Cmd_RecordStart_Implementation()
{
    VoiceConversationSystem->StartRecording();
}

void APlayerActor::Cmd_RecordEnd_Implementation()
{
    VoiceConversationSystem->StopRecording();
}

void APlayerActor::Cmd_ShowDetail_Implementation()
{
    BroadcastManager->SendMegaPopupClosed();
}

void APlayerActor::Cmd_ChatEnter_Implementation()
{
    if (ChatPlayerSystem)
        ChatPlayerSystem->OnEnter();
}

void APlayerActor::Cmd_ChatScrollUp_Implementation()
{
    if (ChatPlayerSystem)
        ChatPlayerSystem->OnScrollUp();
}

void APlayerActor::Cmd_ChatScrollDown_Implementation()
{
    if (ChatPlayerSystem)
        ChatPlayerSystem->OnScrollDown();
}

void APlayerActor::Cmd_ShowMouse_Implementation()
{
    if (auto PC = GetWorld()->GetFirstPlayerController() )
    {
        UWidgetBlueprintLibrary::SetInputMode_GameOnly(PC);
        
        // 1. 입력 모드를 '게임과 UI' 모두 사용으로 변경합니다.                                                                                            
        FInputModeGameAndUI InputModeData;                                                                                                                 
        InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // 마우스가 뷰포트에 갇히지 않도록 설정                                   
        PC->SetInputMode(InputModeData);                                                                                                                   
                                                                                                                                                           
        // 2. 마우스 커서를 표시합니다.                                                                                                                    
        PC->SetShowMouseCursor(true); 
    }
}

void APlayerActor::Cmd_HideMouse_Implementation()
{
    if (auto PC = GetWorld()->GetFirstPlayerController() )
    {
        UWidgetBlueprintLibrary::SetInputMode_GameOnly(PC);
        
        // 1. 입력 모드를 '게임 전용'으로 되돌립니다.                                                                                                      
        FInputModeGameOnly InputModeData;                                                                                                                  
        PC->SetInputMode(InputModeData);                                                                                                                   
                                                                                                                                                           
        // 2. 마우스 커서를 숨깁니다.                                                                                                                      
        PC->SetShowMouseCursor(false);
    }   
}

void APlayerActor::OnExecVoiceCommand(EVoiceCommandType InType, AActor* Requester)
{
    PRINT_STRING(TEXT("%s from %s"), *FString( ENUM_TO_NAME(EVoiceCommandType, InType)),
        Requester ? *Requester->GetName() : TEXT("Unknown"));
}

FString APlayerActor::GetPlayerDisplayName() const
{
    int index = GetLocalPlayerIndex();

    if (auto PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (auto PS = PC->PlayerState)
        {
            return FString::Printf(TEXT("%s (%02d)"), *PS->GetPlayerName(), index );
        }
    }

    return TEXT("Yisan");
}

int APlayerActor::GetLocalPlayerIndex() const
{
    if (const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController())
    {
        return LocalPlayer->GetControllerId();
    }

    return 0;
}

void APlayerActor::PlayTTSAudio(const TArray<uint8>& AudioData)
{
    VoiceConversationSystem->PlayVoiceAudio(AudioData);
}