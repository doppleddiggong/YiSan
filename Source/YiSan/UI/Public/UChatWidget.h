// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "FChatMessageData.h"
#include "Blueprint/UserWidget.h"
#include "UChatWidget.generated.h"

/**
 * 멀티플레이 채팅 UI 위젯
 * - 텍스트 메시지 표시
 * - /ask 명령 처리
 * - Dasan NPC와의 대화 인터페이스
 */
UCLASS()
class YISAN_API UChatWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    // ──────────────────────────────────────────────────────
    // UMG Bindings
    // ──────────────────────────────────────────────────────
    
    /** 채팅 메시지 스크롤 영역 */
    UPROPERTY(meta = (BindWidget))
    class UScrollBox* ChatScrollBox;

    // ──────────────────────────────────────────────────────
    // Message Display Settings
    // ──────────────────────────────────────────────────────

    /** 채팅 메시지 아이템 위젯 클래스 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chat|UI")
    TSubclassOf<UUserWidget> ChatMessageItemClass;

    /** 최대 메시지 히스토리 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chat|Settings")
    int32 MaxMessageHistory = 100;

    /** 플레이어 메시지 색상 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chat|Colors")
    FLinearColor PlayerMessageColor = FLinearColor(0.2f, 0.8f, 1.0f, 1.0f);

    /** Dasan 응답 메시지 색상 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chat|Colors")
    FLinearColor DasanResponseColor = FLinearColor(1.0f, 0.8f, 0.2f, 1.0f);

    /** 시스템 메시지 색상 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chat|Colors")
    FLinearColor SystemMessageColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

    // ──────────────────────────────────────────────────────
    // Internal State
    // ──────────────────────────────────────────────────────

    /** 채팅 메시지 히스토리 */
    UPROPERTY()
    TArray<FChatMessageData> MessageHistory;

    /** 채팅창 표시 여부 */
    UPROPERTY(BlueprintReadOnly, Category = "Chat|State")
    bool bIsChatVisible = true;

public:
    virtual void NativeConstruct() override;

    // ──────────────────────────────────────────────────────
    // Public Interface
    // ──────────────────────────────────────────────────────

    /**
     * 새로운 메시지 추가
     * @param MessageData 메시지 데이터
     */
    UFUNCTION(BlueprintCallable, Category = "Chat")
    void AddMessage(const FChatMessageData& MessageData);

    /**
     * /ask 명령 전송
     * @param Question 질문 텍스트
     */
    UFUNCTION(BlueprintCallable, Category = "Chat")
    void SendAskCommand(const FString& Question);

    /**
     * 일반 채팅 메시지 전송
     * @param Message 메시지 텍스트
     */
    UFUNCTION(BlueprintCallable, Category = "Chat")
    void SendChatMessage(const FString& Message);

    /**
     * 채팅 히스토리 초기화
     */
    UFUNCTION(BlueprintCallable, Category = "Chat")
    void ClearChatHistory();

protected:
    /**
     * 메시지 타입에 따른 색상 반환
     */
    FLinearColor GetMessageColor(EChatMessageType MessageType) const;

    /**
     * 메시지 아이템 위젯 생성 및 추가
     */
    void CreateMessageItem(const FChatMessageData& MessageData);

    /**
     * 스크롤을 최하단으로 이동
     */
    void ScrollToBottom();

    /**
     * 명령어 파싱 및 처리
     * @return true if command was handled
     */
    bool ProcessCommand(const FString& Input);
};