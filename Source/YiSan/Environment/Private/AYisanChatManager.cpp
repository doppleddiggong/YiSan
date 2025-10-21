// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "AYisanChatManager.h"

#include "AYisanChatManager.h"
#include "UChatWidget.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AYisanChatManager* AYisanChatManager::InstanceCache = nullptr;

AYisanChatManager::AYisanChatManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    bAlwaysRelevant = true;

    LastProcessedMessageID = 0;
    MessageIDCounter = 0;

    MockResponses = {
        TEXT("안녕하세요! 무엇을 도와드릴까요?"),
        TEXT("그것은 흥미로운 질문이네요."),
        TEXT("이해했습니다. 설명드리겠습니다."),
        TEXT("좋은 지적입니다. 추가로 말씀드리면...")
    };
}

void AYisanChatManager::BeginPlay()
{
    Super::BeginPlay();
    InstanceCache = this;

    if (bLocalTestMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("[GameChatManager] 로컬 테스트 모드"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[GameChatManager] 네트워크 모드"));
    }
}

void AYisanChatManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    // ChatMessages 배열을 복제
    DOREPLIFETIME(AYisanChatManager, ChatMessages);
}

// ──────────────────────────────────────────────────────
// Singleton
// ──────────────────────────────────────────────────────

AYisanChatManager* AYisanChatManager::GetInstance(const UObject* WorldContextObject)
{
    if (InstanceCache && IsValid(InstanceCache))
    {
        return InstanceCache;
    }

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return nullptr;

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AYisanChatManager::StaticClass(), FoundActors);

    if (FoundActors.Num() > 0)
    {
        InstanceCache = Cast<AYisanChatManager>(FoundActors[0]);
        return InstanceCache;
    }

    return nullptr;
}

// ──────────────────────────────────────────────────────
// Replication Callback (핵심!)
// ──────────────────────────────────────────────────────

void AYisanChatManager::OnRep_ChatMessages()
{
    UE_LOG(LogTemp, Log, TEXT("[OnRep_ChatMessages] 메시지 배열 갱신됨 (총 %d개)"), ChatMessages.Num());

    // 새로운 메시지만 표시
    DisplayNewMessages_Client();
}

// ──────────────────────────────────────────────────────
// Public Interface
// ──────────────────────────────────────────────────────

void AYisanChatManager::SendAskMessage(const FString& Question, APlayerController* Sender)
{
    if (Question.IsEmpty() || !Sender) return;

    FString PlayerName = Sender->PlayerState ? Sender->PlayerState->GetPlayerName() : TEXT("Player");

    // 로컬 테스트 모드
    if (bLocalTestMode)
    {
        FReplicatedChatMessage PlayerMessage;
        PlayerMessage.SenderName = PlayerName;
        PlayerMessage.MessageText = FString::Printf(TEXT("/ask %s"), *Question);
        PlayerMessage.MessageType = EChatMessageType::PlayerMessage;
        PlayerMessage.MessageID = GenerateMessageID();
        PlayerMessage.Timestamp = FDateTime::Now();

        // 로컬 배열에 직접 추가 (복제 없음)
        ChatMessages.Add(PlayerMessage);
        DisplayNewMessages_Client();

        // Mock 응답 시뮬레이션
        SimulateMockGPTResponse_Local(Question);
        return;
    }

    // 네트워크 모드: 서버 RPC 호출
    ServerRPC_AskMessage(PlayerName, Question);
}

void AYisanChatManager::SendChatMessage(const FString& Message, APlayerController* Sender)
{
    if (Message.IsEmpty() || !Sender) return;

    FString PlayerName = Sender->PlayerState ? Sender->PlayerState->GetPlayerName() : TEXT("Player");

    if (bLocalTestMode)
    {
        FReplicatedChatMessage ChatMessage;
        ChatMessage.SenderName = PlayerName;
        ChatMessage.MessageText = Message;
        ChatMessage.MessageType = EChatMessageType::PlayerMessage;
        ChatMessage.MessageID = GenerateMessageID();
        ChatMessage.Timestamp = FDateTime::Now();

        ChatMessages.Add(ChatMessage);
        DisplayNewMessages_Client();
        return;
    }

    ServerRPC_SendChatMessage(PlayerName, Message);
}

void AYisanChatManager::RegisterChatWidget(UChatWidget* Widget)
{
    if (Widget && !RegisteredWidgets.Contains(Widget))
    {
        RegisteredWidgets.Add(Widget);
        UE_LOG(LogTemp, Log, TEXT("[GameChatManager] ChatWidget 등록 (총 %d개)"), RegisteredWidgets.Num());

        // 기존 메시지 동기화
        DisplayNewMessages_Client();
    }
}

TArray<FReplicatedChatMessage> AYisanChatManager::GetMessagesAfter(int32 LastMessageID) const
{
    TArray<FReplicatedChatMessage> NewMessages;
    
    for (const FReplicatedChatMessage& Msg : ChatMessages)
    {
        if (Msg.MessageID > LastMessageID)
        {
            NewMessages.Add(Msg);
        }
    }

    return NewMessages;
}

// ──────────────────────────────────────────────────────
// Server RPCs
// ──────────────────────────────────────────────────────

void AYisanChatManager::ServerRPC_AskMessage_Implementation(const FString& SenderName, const FString& Question)
{
    UE_LOG(LogTemp, Log, TEXT("[SERVER] /ask from %s: %s"), *SenderName, *Question);

    // 1. 질문 메시지 추가
    FReplicatedChatMessage PlayerMessage;
    PlayerMessage.SenderName = SenderName;
    PlayerMessage.MessageText = FString::Printf(TEXT("/ask %s"), *Question);
    PlayerMessage.MessageType = EChatMessageType::PlayerMessage;
    PlayerMessage.MessageID = GenerateMessageID();
    PlayerMessage.Timestamp = FDateTime::Now();

    AddMessageToArray_Server(PlayerMessage);

    // 2. GPT 처리
    ProcessGPTRequest_Server(Question);
}

bool AYisanChatManager::ServerRPC_AskMessage_Validate(const FString& SenderName, const FString& Question)
{
    return !Question.IsEmpty() && Question.Len() < 500;
}

void AYisanChatManager::ServerRPC_SendChatMessage_Implementation(const FString& SenderName, const FString& Message)
{
    FReplicatedChatMessage ChatMessage;
    ChatMessage.SenderName = SenderName;
    ChatMessage.MessageText = Message;
    ChatMessage.MessageType = EChatMessageType::PlayerMessage;
    ChatMessage.MessageID = GenerateMessageID();
    ChatMessage.Timestamp = FDateTime::Now();

    AddMessageToArray_Server(ChatMessage);
}

bool AYisanChatManager::ServerRPC_SendChatMessage_Validate(const FString& SenderName, const FString& Message)
{
    return !Message.IsEmpty() && Message.Len() < 500;
}

// ──────────────────────────────────────────────────────
// Server-Side Methods
// ──────────────────────────────────────────────────────

void AYisanChatManager::AddMessageToArray_Server(const FReplicatedChatMessage& Message)
{
    if (!HasAuthority())
        return;

    // 배열에 추가
    ChatMessages.Add(Message);

    // 최대 히스토리 제한
    if (ChatMessages.Num() > MaxMessageHistory)
    {
        ChatMessages.RemoveAt(0);
    }

    UE_LOG(LogTemp, Log, TEXT("[SERVER] 메시지 추가됨 (ID: %d, 총 %d개)"), Message.MessageID, ChatMessages.Num());

    // OnRep은 자동으로 모든 클라이언트에서 트리거됨!
}

void AYisanChatManager::ProcessGPTRequest_Server(const FString& Question)
{
    if (!HasAuthority())
        return;

    UE_LOG(LogTemp, Warning, TEXT("[SERVER] GPT 요청 처리: %s"), *Question);

    // TODO: Dasan NPC 상태 변경
    // TODO: GPT API 호출 (비동기)

    // Mock 응답 (2초 후)
    FTimerHandle TempHandle;
    GetWorld()->GetTimerManager().SetTimer(
        TempHandle,
        [this, Question]()
        {
            FString ResponseText = GetRandomMockResponse();

            FReplicatedChatMessage DasanMessage;
            DasanMessage.SenderName = TEXT("Dasan");
            DasanMessage.MessageText = ResponseText;
            DasanMessage.MessageType = EChatMessageType::DasanResponse;
            DasanMessage.MessageID = GenerateMessageID();
            DasanMessage.Timestamp = FDateTime::Now();

            // 서버가 배열에 추가 → OnRep 자동 트리거
            AddMessageToArray_Server(DasanMessage);

            // TTS 처리
            ProcessTTS_Server(ResponseText);
        },
        2.0f,
        false
    );
}

void AYisanChatManager::ProcessTTS_Server(const FString& ResponseText)
{
    if (!HasAuthority()) return;

    UE_LOG(LogTemp, Warning, TEXT("[SERVER] TTS 생성: %s"), *ResponseText);
    
    // TODO: TTS API 호출
    // TODO: AudioData를 메시지에 포함
}

// ──────────────────────────────────────────────────────
// Client-Side Methods
// ──────────────────────────────────────────────────────

void AYisanChatManager::DisplayNewMessages_Client()
{
    // 유효하지 않은 위젯 제거
    RegisteredWidgets.RemoveAll([](UChatWidget* W) { return !IsValid(W); });

    if (RegisteredWidgets.Num() == 0)
        return;

    // 마지막 처리 ID 이후의 메시지만 표시
    for (int32 i = 0; i < ChatMessages.Num(); ++i)
    {
        const FReplicatedChatMessage& Msg = ChatMessages[i];

        if (Msg.MessageID > LastProcessedMessageID)
        {
            // 모든 위젯에 메시지 표시
            for (UChatWidget* Widget : RegisteredWidgets)
            {
                if (Widget)
                {
                    FChatMessageData DisplayData;
                    DisplayData.SenderName = Msg.SenderName;
                    DisplayData.MessageText = Msg.MessageText;
                    DisplayData.MessageType = Msg.MessageType;
                    DisplayData.Timestamp = Msg.Timestamp;

                    Widget->AddMessage(DisplayData);
                }
            }

            LastProcessedMessageID = Msg.MessageID;
        }
    }
}

void AYisanChatManager::SimulateMockGPTResponse_Local(const FString& Question)
{
    GetWorld()->GetTimerManager().SetTimer(
        MockResponseTimerHandle,
        [this]()
        {
            FReplicatedChatMessage DasanMessage;
            DasanMessage.SenderName = TEXT("Dasan");
            DasanMessage.MessageText = GetRandomMockResponse();
            DasanMessage.MessageType = EChatMessageType::DasanResponse;
            DasanMessage.MessageID = GenerateMessageID();
            DasanMessage.Timestamp = FDateTime::Now();

            ChatMessages.Add(DasanMessage);
            DisplayNewMessages_Client();
        },
        MockResponseDelay,
        false
    );
}

FString AYisanChatManager::GetRandomMockResponse() const
{
    if (MockResponses.Num() == 0) return TEXT("응답 없음");
    return MockResponses[FMath::RandRange(0, MockResponses.Num() - 1)];
}

int32 AYisanChatManager::GenerateMessageID()
{
    return ++MessageIDCounter;
}