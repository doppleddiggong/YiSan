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

	/** 메시지를 보낸 주체 타입 */
	UPROPERTY(BlueprintReadWrite)
	EChatMessageType SpeakerType;

	/** 메시지가 전송된 시간 */
	UPROPERTY(BlueprintReadWrite)
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadWrite)
	FString SpeakerName;

	UPROPERTY(BlueprintReadWrite)
	FString Message;

	UPROPERTY(BlueprintReadWrite)
	TArray<uint8> AudioData;

	FChatMessage()
		: SpeakerType(EChatMessageType::System)
		, Timestamp(FDateTime::Now())
		, SpeakerName(TEXT(""))
		, Message(TEXT(""))
		, AudioData()
	{
	}

	FChatMessage(EChatMessageType InType, const FString& InSpeakerName, const FString& InMessage)
		: SpeakerType(InType)
		, Timestamp(FDateTime::Now())
		, SpeakerName(InSpeakerName)
		, Message(InMessage)
		, AudioData()
	{
	}

	FChatMessage(EChatMessageType InType, const FString& InSpeakerName, const FString& InMessage, const TArray<uint8>& InAudioData)
		: SpeakerType(InType)
		, Timestamp(FDateTime::Now())
		, SpeakerName(InSpeakerName)
		, Message(InMessage)
		, AudioData(InAudioData)
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
	void Ask(const FString& InMsg, const FGPTContext& SpatialContext);
	
	/** 서버 RPC: 메시지 전송 */
	UFUNCTION(Server, Reliable)
	void ServerRPC_SendChatMessage(const FChatMessage& ChatMessage);

private:
	/** 멀티캐스트 RPC: 전체 클라 갱신 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_AddChatMessage(const FChatMessage& ChatMessage);
	
	void OnResponseAsk(FResponseAsk& Response, bool bSuccess);
	
private:
	UPROPERTY()
	TObjectPtr<class APlayerActor> Owner;

	UPROPERTY()
	TObjectPtr<class UChatBoxWidget> ChatBoxWidget;
};
