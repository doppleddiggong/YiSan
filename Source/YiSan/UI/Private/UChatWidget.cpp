// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "UChatWidget.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"

#include "Kismet/GameplayStatics.h"

void UChatWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 초기 시스템 메시지
    FChatMessageData WelcomeMessage;
    WelcomeMessage.SenderName = TEXT("System");
    WelcomeMessage.MessageText = TEXT("채팅 시스템이 초기화되었습니다. /ask [질문]으로 Dasan에게 질문하세요.");
    WelcomeMessage.MessageType = EChatMessageType::SystemMessage;
    AddMessage(WelcomeMessage);
}

void UChatWidget::AddMessage(const FChatMessageData& MessageData)
{
    // 히스토리에 추가
    MessageHistory.Add(MessageData);

    // 최대 히스토리 제한
    if (MessageHistory.Num() > MaxMessageHistory)
    {
        MessageHistory.RemoveAt(0);
        
        // ScrollBox에서도 첫 번째 항목 제거
        if (ChatScrollBox && ChatScrollBox->GetChildrenCount() > 0)
        {
            ChatScrollBox->RemoveChildAt(0);
        }
    }

    // UI에 메시지 추가
    CreateMessageItem(MessageData);

    // 스크롤 최하단 이동
    ScrollToBottom();
}

void UChatWidget::SendAskCommand(const FString& Question)
{
    if (Question.IsEmpty())
    {
        return;
    }

    // 로컬 표시용 메시지
    FChatMessageData LocalMessage;
    LocalMessage.SenderName = TEXT("You");
    LocalMessage.MessageText = FString::Printf(TEXT("/ask %s"), *Question);
    LocalMessage.MessageType = EChatMessageType::PlayerMessage;
    AddMessage(LocalMessage);

    // GameChatManager를 통해 서버로 전송
    // TODO: GameChatManager 구현 후 활성화
    /*
    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        UGameChatManager* ChatManager = PC->FindComponentByClass<UGameChatManager>();
        if (ChatManager)
        {
            ChatManager->ServerRPC_AskMessage(Question);
        }
    }
    */
}

void UChatWidget::SendChatMessage(const FString& Message)
{
    if (Message.IsEmpty())
    {
        return;
    }

    // 로컬 표시용 메시지
    FChatMessageData LocalMessage;
    LocalMessage.SenderName = TEXT("You");
    LocalMessage.MessageText = Message;
    LocalMessage.MessageType = EChatMessageType::PlayerMessage;
    AddMessage(LocalMessage);

    // TODO: 일반 채팅 서버 전송 구현
    // ChatManager->ServerRPC_SendChatMessage(Message);
}

void UChatWidget::ClearChatHistory()
{
    MessageHistory.Empty();

    if (ChatScrollBox)
    {
        ChatScrollBox->ClearChildren();
    }
}

FLinearColor UChatWidget::GetMessageColor(EChatMessageType MessageType) const
{
    switch (MessageType)
    {
        case EChatMessageType::PlayerMessage:
            return PlayerMessageColor;
        case EChatMessageType::DasanResponse:
            return DasanResponseColor;
        case EChatMessageType::SystemMessage:
            return SystemMessageColor;
        default:
            return FLinearColor::White;
    }
}

void UChatWidget::CreateMessageItem(const FChatMessageData& MessageData)
{
    if (!ChatScrollBox || !ChatMessageItemClass)
    {
        return;
    }

    // 메시지 아이템 위젯 생성
    UUserWidget* MessageItem = CreateWidget<UUserWidget>(GetOwningPlayer(), ChatMessageItemClass);
    
    if (!MessageItem)
    {
        return;
    }

    // 메시지 아이템 내부의 TextBlock 찾기 및 설정
    // 위젯 블루프린트에서 "SenderText", "MessageText"로 바인딩 필요
    UTextBlock* SenderText = Cast<UTextBlock>(MessageItem->GetWidgetFromName(TEXT("SenderText")));
    UTextBlock* MessageText = Cast<UTextBlock>(MessageItem->GetWidgetFromName(TEXT("MessageText")));

    if (SenderText)
    {
        FString SenderDisplay = FString::Printf(TEXT("[%s]"), *MessageData.SenderName);
        SenderText->SetText(FText::FromString(SenderDisplay));
        SenderText->SetColorAndOpacity(GetMessageColor(MessageData.MessageType));
    }

    if (MessageText)
    {
        MessageText->SetText(FText::FromString(MessageData.MessageText));
        MessageText->SetColorAndOpacity(FLinearColor::White);
    }

    // ScrollBox에 추가
    ChatScrollBox->AddChild(MessageItem);
}

void UChatWidget::ScrollToBottom()
{
    if (ChatScrollBox)
    {
        // 다음 프레임에 스크롤 (UI 업데이트 대기)
        GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
        {
            if (ChatScrollBox)
            {
                ChatScrollBox->ScrollToEnd();
            }
        });
    }
}

bool UChatWidget::ProcessCommand(const FString& Input)
{
    FString TrimmedInput = Input.TrimStartAndEnd();

    // /ask 명령 처리
    if (TrimmedInput.StartsWith(TEXT("/ask "), ESearchCase::IgnoreCase))
    {
        FString Question = TrimmedInput.RightChop(5).TrimStartAndEnd(); // "/ask " 제거
        
        if (!Question.IsEmpty())
        {
            SendAskCommand(Question);
            return true;
        }
    }

    // /clear 명령 처리
    if (TrimmedInput.Equals(TEXT("/clear"), ESearchCase::IgnoreCase))
    {
        ClearChatHistory();
        
        FChatMessageData SystemMsg;
        SystemMsg.SenderName = TEXT("System");
        SystemMsg.MessageText = TEXT("채팅 히스토리가 초기화되었습니다.");
        SystemMsg.MessageType = EChatMessageType::SystemMessage;
        AddMessage(SystemMsg);
        
        return true;
    }

    return false;
}