// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "FChatMessageData.h"
#include "GameFramework/Actor.h"
#include "AYisanChatManager.generated.h"

UCLASS()
class YISAN_API AYisanChatManager : public AActor
{
	GENERATED_BODY()

public:
    AYisanChatManager();

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    // ──────────────────────────────────────────────────────
    // Singleton Access
    // ──────────────────────────────────────────────────────
    UFUNCTION(BlueprintPure, Category = "Chat", meta = (WorldContext = "WorldContextObject"))
    static AYisanChatManager* GetInstance(const UObject* WorldContextObject);

    // ──────────────────────────────────────────────────────
    // Replicated Data
    // ──────────────────────────────────────────────────────

    /**
     * 복제되는 채팅 메시지 배열
     * 서버가 데이터 추가 → 자동으로 모든 클라이언트에 OnRep 트리거
     */
    UPROPERTY(ReplicatedUsing = OnRep_ChatMessages, BlueprintReadOnly, Category = "Chat|Data")
    TArray<FReplicatedChatMessage> ChatMessages;

    /**
     * 최대 메시지 히스토리 (서버 메모리 관리용)
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat|Settings")
    int32 MaxMessageHistory = 100;

    // ──────────────────────────────────────────────────────
    // Configuration
    // ──────────────────────────────────────────────────────

    /** 로컬 테스트 모드 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat|Debug")
    bool bLocalTestMode = true;

    /** Mock GPT 응답 지연 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat|Debug")
    float MockResponseDelay = 2.0f;

    /** Mock 응답 템플릿 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat|Debug")
    TArray<FString> MockResponses;

    /** Dasan NPC 레퍼런스 */
    UPROPERTY(BlueprintReadWrite, Category = "Chat|Network")
    TObjectPtr<class ADasanActor> DasanNPCRef;

    // ──────────────────────────────────────────────────────
    // Public Interface
    // ──────────────────────────────────────────────────────

    /**
     * /ask 명령 전송
     */
    UFUNCTION(BlueprintCallable, Category = "Chat")
    void SendAskMessage(const FString& Question, APlayerController* Sender);

    /**
     * 일반 채팅 메시지 전송
     */
    UFUNCTION(BlueprintCallable, Category = "Chat")
    void SendChatMessage(const FString& Message, APlayerController* Sender);

    /**
     * ChatWidget 등록
     */
    UFUNCTION(BlueprintCallable, Category = "Chat")
    void RegisterChatWidget(class UChatWidget* Widget);

    /**
     * 특정 메시지 ID 이후의 메시지만 가져오기 (초기 동기화용)
     */
    UFUNCTION(BlueprintCallable, Category = "Chat")
    TArray<FReplicatedChatMessage> GetMessagesAfter(int32 LastMessageID) const;

protected:
    // ──────────────────────────────────────────────────────
    // Replication Callbacks
    // ──────────────────────────────────────────────────────

    /**
     * OnRep: 채팅 메시지 배열이 갱신될 때 자동 호출
     */
    UFUNCTION()
    void OnRep_ChatMessages();

    // ──────────────────────────────────────────────────────
    // Server RPCs (데이터 추가만 담당)
    // ──────────────────────────────────────────────────────

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerRPC_AskMessage(const FString& SenderName, const FString& Question);

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerRPC_SendChatMessage(const FString& SenderName, const FString& Message);

    // ──────────────────────────────────────────────────────
    // Server-Side Methods
    // ──────────────────────────────────────────────────────

    /**
     * 서버: 메시지를 Replicated Array에 추가
     */
    void AddMessageToArray_Server(const FReplicatedChatMessage& Message);

    /**
     * 서버: GPT 요청 처리
     */
    void ProcessGPTRequest_Server(const FString& Question);

    /**
     * 서버: TTS 생성
     */
    void ProcessTTS_Server(const FString& ResponseText);

    // ──────────────────────────────────────────────────────
    // Client-Side Methods
    // ──────────────────────────────────────────────────────

    /**
     * 클라이언트: 새 메시지를 ChatWidget에 표시
     */
    void DisplayNewMessages_Client();

    /**
     * 로컬 테스트: Mock 응답 시뮬레이션
     */
    void SimulateMockGPTResponse_Local(const FString& Question);

    /**
     * Mock 응답 생성
     */
    FString GetRandomMockResponse() const;

    /**
     * 고유 메시지 ID 생성
     */
    int32 GenerateMessageID();

private:
    /** 등록된 ChatWidget 목록 */
    UPROPERTY()
    TArray<class UChatWidget*> RegisteredWidgets;

    /** 마지막으로 처리한 메시지 ID (클라이언트별) */
    int32 LastProcessedMessageID;

    /** 메시지 ID 카운터 (서버 전용) */
    int32 MessageIDCounter;

    /** Mock 응답 타이머 */
    FTimerHandle MockResponseTimerHandle;

    /** 싱글톤 캐시 */
    static AYisanChatManager* InstanceCache;
};
