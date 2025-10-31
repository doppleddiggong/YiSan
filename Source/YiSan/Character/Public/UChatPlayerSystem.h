// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "NetworkData.h"
#include "Components/ActorComponent.h"
#include "UChatPlayerSystem.generated.h"

UENUM(BlueprintType)
enum class EChatMessageType : uint8
{
	NPC			UMETA(DisplayName = "NPC"),
	User		UMETA(DisplayName = "User"),
	System		UMETA(DisplayName = "System")
};

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

UCLASS(ClassGroup=(Chat), meta=(BlueprintSpawnableComponent))
class YISAN_API UChatPlayerSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UChatPlayerSystem();

	void InitSystem(class UChatBoxWidget* InChatBox);

	/** 채팅 입력 처리 */
	void OnEnter();
	void OnScrollUp();
	void OnScrollDown();

public:
	/** 서버 RPC: 메시지 전송 */
	UFUNCTION(Server, Reliable)
	void ServerRPC_SendChatMessage(const FChatMessage& ChatMessage);

	/** 멀티캐스트 RPC: 전체 클라 갱신 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_AddChatMessage(const FChatMessage& ChatMessage);

	void AnnouncePlayerJoin();
	
private:
	UPROPERTY()
	TObjectPtr<class APlayerActor> Owner;

	UPROPERTY()
	TObjectPtr<class UChatBoxWidget> ChatBoxWidget;
};
