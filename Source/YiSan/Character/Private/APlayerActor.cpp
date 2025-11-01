// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "APlayerActor.h"
#include "APlayerControl.h"

#include "AQuestManagerActor.h"
#include "AYiSanPlayerState.h"

#include "FComponentHelper.h"
#include "ABuilding.h"
#include "UGameDataManager.h"
#include "FGPTContext.h"
#include "GameLogging.h"
#include "UMainWidget.h"
#include "UVoiceConversationSystem.h"
#include "UGPTContextSystem.h"
#include "UChatPlayerSystem.h"
#include "UChatUIWidget.h"
#include "UChatBoxWidget.h"
#include "UHttpNetworkSystem.h"
#include "TimerManager.h"
#include "UBroadcastManager.h"
#include "UPlayerHeadWidget.h"
#include "YiSan/YiSan.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Camera/CameraComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"

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

    NameTagWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameTagWidget"));
    NameTagWidgetComponent->SetupAttachment(RootComponent);

    NameTagWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen); // 카메라 정면 고정
    NameTagWidgetComponent->SetDrawSize(FVector2D(200.f, 50.f));
    NameTagWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 100.f)); // 머리 위 위치
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

    // // 퀘스트 초기화
    // UQuestManager::Get(GetWorld())->InitSystem();

    // 서버야 일어나라.
    UHttpNetworkSystem::Get(GetWorld())->RequestHealth( FResponseHealthDelegate() );

    if ( HasAuthority() )
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
        {
            if (AQuestManagerActor* QuestManager = AQuestManagerActor::Get(this))
                BroadcastManager->SendUpdateQuest(QuestManager->GetCurrentTarget());
        });
    }
    
    // 이름표 위젯 초기화를 위한 타이머 시작
    GetWorldTimerManager().SetTimer(TimerHandle_InitNameTag, this, &APlayerActor::CheckAndInitNameTag, 0.2f, true);
}

void APlayerActor::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
}

void APlayerActor::OnRep_Controller()
{
    Super::OnRep_Controller();
    OnReadyPawn();
}

void APlayerActor::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    if (IsLocallyControlled())
        UE_LOG(LogTemp, Warning, TEXT("Pawn possessed locally, player can move"));

    OnReadyPawn();
}

void APlayerActor::CheckAndInitNameTag()
{
    // 이름이 아직 설정되지 않았을 수 있으므로, 유효한 이름이 될 때까지 대기합니다.
    if (GetPlayerDisplayName() == GameString::Default )
    {
        return; // 아직 이름이 없으므로, 다음 타이머 틱에서 다시 시도합니다.
    }

    // 위젯이 생성되었는지 확인합니다.
    UPlayerHeadWidget* HeadWidget = Cast<UPlayerHeadWidget>(NameTagWidgetComponent->GetUserWidgetObject());
    if (HeadWidget)
    {
        // 위젯이 준비되면, 위젯에게 소유자가 자신임을 알려주고 이 타이머는 역할을 다했으므로 중지합니다.
        HeadWidget->SetOwningActor(this);
        GetWorldTimerManager().ClearTimer(TimerHandle_InitNameTag);

        // 컨트롤러에 이름이 준비되었음을 알립니다.
        if (auto PC = Cast<APlayerControl>(GetController()))
        {
            PC->OnPawnHasName();
        }
    }
}
void APlayerActor::OnReadyPawn()
{
    if (auto PlayerController = Cast<APlayerController>(GetController()))
    {
        if (!PlayerController->IsLocalController())
            return;

        if (auto PC = Cast<APlayerControl>(PlayerController))
            PC->OnPawnReady(*this);
    }
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

void APlayerActor::Cmd_ShowVoiceGuide_Implementation()
{
    if (MainWidgetInst)
        MainWidgetInst->ShowVoiceGuide();
}

void APlayerActor::Cmd_HideVoiceGuide_Implementation()
{
    if (MainWidgetInst)
        MainWidgetInst->HideVoiceGuide();
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
    PRINT_STRING(TEXT("%s from %s"),
        *FString( ENUM_TO_NAME(EVoiceCommandType, InType)),
        Requester ? *Requester->GetName() : TEXT("Unknown"));

    // 요청자가 플레이어인지 확인
    APlayerActor* RequesterPlayer = Cast<APlayerActor>(Requester);
    if (!RequesterPlayer)
        return;

    // Cmd_BuildingInfo는 로컬 클라이언트에서만 실행 (UI 표시용)
    if (InType == EVoiceCommandType::Cmd_BuildingInfo)
    {
        // 요청자 본인만 실행
        if (this != RequesterPlayer || !IsLocallyControlled())
            return;

        // 가장 가까운 건물 찾기
        auto AllBuildings = FComponentHelper::GetAllOfClass<ABuilding>(GetWorld());
        if (AllBuildings.Num() == 0)
        {
            PRINTLOG(TEXT("[VoiceCommand] 주변에 건물이 없습니다"));
            return;
        }

        float MinDistance = TNumericLimits<float>::Max();
        ABuilding* NearestBuilding = nullptr;

        FVector MyLocation = GetActorLocation();
        for (auto Building : AllBuildings)
        {
            float Distance = FVector::Dist(MyLocation, Building->GetActorLocation());
            if (Distance < MinDistance)
            {
                MinDistance = Distance;
                NearestBuilding = Building;
            }
        }

        if (NearestBuilding && MainWidgetInst)
        {
            MainWidgetInst->ShowMegaPopup(NearestBuilding->BuildingType);

            PRINTLOG(TEXT("[VoiceCommand] %s 정보 표시"),
                *UGameDataManager::Get(GetWorld())->GetBuildingDataName(NearestBuilding->BuildingType));
        }
        return;
    }

    // 나머지 명령들은 서버에서만 실행
    if (!HasAuthority())
        return;

    if (InType == EVoiceCommandType::Cmd_Call)
    {
        // 다른 모든 플레이어를 요청자에게 집결
        // 요청자 본인은 제외
        if (this == RequesterPlayer)
            return;

        FVector RequesterLocation = RequesterPlayer->GetActorLocation();
        FRotator RequesterRotation = RequesterPlayer->GetActorRotation();

        // 요청자의 앞쪽으로 200 유닛 떨어진 위치 계산
        FVector TargetLocation = RequesterLocation + (RequesterRotation.Vector() * 200.f);

        // 자신을 요청자 앞으로 텔레포트
        TeleportTo(TargetLocation, GetActorRotation(), false, true);

        PRINTLOG(TEXT("[VoiceCommand] %s가 %s에게 집결"), *GetPlayerDisplayName(), *RequesterPlayer->GetPlayerDisplayName());
    }
    else if (InType == EVoiceCommandType::Cmd_Friend)
    {
        // 요청자가 다른 플레이어(다음 번째 플레이어)에게로 이동
        // 요청자만 실행
        if (this != RequesterPlayer)
            return;

        // 모든 플레이어 가져오기
        auto AllPlayers = FComponentHelper::GetAllOfClass<APlayerActor>(GetWorld());
        if (AllPlayers.Num() <= 1)
        {
            PRINTLOG(TEXT("[VoiceCommand] 이동할 다른 플레이어가 없습니다"));
            return;
        }

        // 자신의 PlayerIndex 가져오기
        int32 MyIndex = GetPlayerIndex();

        // 다음 플레이어 찾기 (순환)
        APlayerActor* TargetPlayer = nullptr;
        for (auto Player : AllPlayers)
        {
            int32 PlayerIdx = Player->GetPlayerIndex();
            if (PlayerIdx > MyIndex)
            {
                TargetPlayer = Player;
                break;
            }
        }

        // 다음 플레이어가 없으면 첫 번째 플레이어로
        if (!TargetPlayer)
        {
            for (auto Player : AllPlayers)
            {
                if (Player != this)
                {
                    TargetPlayer = Player;
                    break;
                }
            }
        }

        if (TargetPlayer)
        {
            FVector TargetLocation = TargetPlayer->GetActorLocation();
            FRotator TargetRotation = TargetPlayer->GetActorRotation();

            // 대상 플레이어의 앞쪽으로 200 유닛 떨어진 위치 계산
            FVector TeleportLocation = TargetLocation + (TargetRotation.Vector() * 200.f);

            // 텔레포트
            TeleportTo(TeleportLocation, GetActorRotation(), false, true);

            PRINTLOG(TEXT("[VoiceCommand] %s가 %s에게로 이동"), *GetPlayerDisplayName(), *TargetPlayer->GetPlayerDisplayName());
        }
    }
    else if (InType == EVoiceCommandType::Cmd_Target)
    {
        // 요청자가 현재 퀘스트 목표 위치로 이동
        // 요청자만 실행
        if (this != RequesterPlayer)
            return;

        // 퀘스트 매니저에서 현재 목표 가져오기
        AQuestManagerActor* QuestManager = AQuestManagerActor::Get(this);
        if (!QuestManager || !QuestManager->HasActiveQuest())
        {
            PRINTLOG(TEXT("[VoiceCommand] 활성화된 퀘스트가 없습니다"));
            return;
        }

        EBuildingType TargetBuildingType = QuestManager->GetCurrentTarget();

        // 목표 건물 찾기
        auto AllBuildings = FComponentHelper::GetAllOfClass<ABuilding>(GetWorld());
        ABuilding* TargetBuilding = nullptr;
        for (auto Building : AllBuildings)
        {
            if (Building->BuildingType == TargetBuildingType)
            {
                TargetBuilding = Building;
                break;
            }
        }

        if (TargetBuilding)
        {
            FVector BuildingLocation = TargetBuilding->GetActorLocation();
            FRotator BuildingRotation = TargetBuilding->GetActorRotation();

            // 건물 앞쪽으로 200 유닛 떨어진 위치 계산
            FVector TeleportLocation = BuildingLocation + (BuildingRotation.Vector() * 200.f);

            // 텔레포트
            TeleportTo(TeleportLocation, GetActorRotation(), false, true);

            PRINTLOG(TEXT("[VoiceCommand] %s가 퀘스트 목표 %s로 이동"),
                *GetPlayerDisplayName(),
                *UGameDataManager::Get(GetWorld())->GetBuildingDataName(TargetBuildingType));
        }
        else
        {
            PRINTLOG(TEXT("[VoiceCommand] 퀘스트 목표 건물을 찾을 수 없습니다"));
        }
    }
}

FString APlayerActor::GetPlayerDisplayName() const
{
    if (auto PS = GetPlayerState<AYiSanPlayerState>())
    {
        return PS->Nickname;
    }
    
    if (auto PC = GetController())
    {
        if (auto PS = Cast<AYiSanPlayerState>( PC->PlayerState))
        {
            return PS->Nickname;
        }
    }

    return GameString::Default;
}

int32 APlayerActor::GetPlayerIndex() const
{
    if (auto PS = GetPlayerState<AYiSanPlayerState>())
    {
        return PS->PlayerIndex;
    }
    
    if (auto PC = GetController())
    {
        if (auto PS = Cast<AYiSanPlayerState>( PC->PlayerState))
        {
            return PS->PlayerIndex;
        }
    }

    return 0;
}

void APlayerActor::PlayTTSAudio(const TArray<uint8>& AudioData)
{
    VoiceConversationSystem->PlayVoiceAudio(AudioData);
}
