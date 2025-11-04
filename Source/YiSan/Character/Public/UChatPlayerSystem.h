// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "NetworkData.h"
#include "Components/ActorComponent.h"
#include "UChatPlayerSystem.generated.h"

/**
 * @file UChatPlayerSystem.h
 * @brief EChatMessageType 클래스를 선언합니다.
 */

UENUM(BlueprintType)
enum class EChatMessageType : uint8
{
	NPC			UMETA(DisplayName = "NPC"),
	User		UMETA(DisplayName = "User"),
	System		UMETA(DisplayName = "System")
};

/**
 * @brief 단일 채팅 메시지를 설명하는 직렬화 가능한 페이로드입니다.
 */
USTRUCT(BlueprintType)
struct FChatMessage
{
        GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	EChatMessageType SpeakerType;

	UPROPERTY(BlueprintReadWrite)
	int32 PlayerIndex;

	UPROPERTY(BlueprintReadWrite)
	FString SpeakerName;

	UPROPERTY(BlueprintReadWrite)
	FString Message;

	FChatMessage()
		: SpeakerType(EChatMessageType::System)
		, PlayerIndex(-1)
		, SpeakerName(TEXT(""))
		, Message(TEXT(""))
	{
	}

	FChatMessage(EChatMessageType InType, int32 InPlayerIndex, const FString& InSpeakerName, const FString& InMessage)
		: SpeakerType(InType)
		, PlayerIndex(InPlayerIndex)
		, SpeakerName(InSpeakerName)
		, Message(InMessage)
	{
	}
};

/**
 * @brief 플레이어의 채팅 메시지 구성, UI 연동, 네트워크 복제를 처리합니다.
 */
UCLASS(ClassGroup=(Chat), meta=(BlueprintSpawnableComponent))
class YISAN_API UChatPlayerSystem : public UActorComponent
{
        GENERATED_BODY()

public:
        UChatPlayerSystem();

        /** @brief 런타임 채팅 박스 위젯을 연결하고 로컬 버퍼를 준비합니다. */
        void InitSystem(class UChatBoxWidget* InChatBox);

        /** 채팅 입력 처리 */
        void OnEnter();
        void OnScrollUp();
        void OnScrollDown();

public:
        /** @brief 채팅 메시지를 권한 서버로 전송해 배포를 요청합니다. */
        UFUNCTION(Server, Reliable)
        void ServerRPC_SendChatMessage(const FChatMessage& ChatMessage);

        /** @brief 수신된 채팅 메시지를 연결된 모든 클라이언트로 중계합니다. */
        UFUNCTION(NetMulticast, Reliable)
        void MulticastRPC_AddChatMessage(const FChatMessage& ChatMessage);

        /** @brief 소유 플레이어 입장을 알리는 지역화된 메시지를 추가합니다. */
        void AnnouncePlayerJoin();
	
private:
	UPROPERTY()
	TObjectPtr<class APlayerActor> Owner;

	UPROPERTY()
	TObjectPtr<class UChatBoxWidget> ChatBoxWidget;
};
