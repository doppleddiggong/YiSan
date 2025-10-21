// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "FChatMessageData.generated.h"

/** 채팅 메시지 타입 */
UENUM(BlueprintType)
enum class EChatMessageType : uint8
{
	PlayerMessage   UMETA(DisplayName = "Player Message"),
	DasanResponse   UMETA(DisplayName = "Dasan Response"),
	SystemMessage   UMETA(DisplayName = "System Message")
};

/** 채팅 메시지 구조체 */
USTRUCT(BlueprintType)
struct FChatMessageData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString SenderName;

	UPROPERTY(BlueprintReadOnly)
	FString MessageText;

	UPROPERTY(BlueprintReadOnly)
	EChatMessageType MessageType;

	UPROPERTY(BlueprintReadOnly)
	FLinearColor MessageColor;

	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	FChatMessageData()
		: SenderName(TEXT(""))
		, MessageText(TEXT(""))
		, MessageType(EChatMessageType::SystemMessage)
		, MessageColor(FLinearColor::White)
		, Timestamp(FDateTime::Now())
	{}
};

/** 네트워크 복제용 채팅 메시지 구조체 */
USTRUCT(BlueprintType)
struct FReplicatedChatMessage
{
	GENERATED_BODY()

	UPROPERTY()
	FString SenderName;

	UPROPERTY()
	FString MessageText;

	UPROPERTY()
	EChatMessageType MessageType;

	UPROPERTY()
	int32 MessageID; // 중복 방지용 고유 ID

	UPROPERTY()
	FDateTime Timestamp;

	UPROPERTY()
	FString AudioData; // TTS 데이터 (옵션)

	FReplicatedChatMessage()
		: SenderName(TEXT(""))
		, MessageText(TEXT(""))
		, MessageType(EChatMessageType::SystemMessage)
		, MessageID(0)
		, Timestamp(FDateTime::Now())
		, AudioData(TEXT(""))
	{}

	// 비교 연산자 (중복 체크용)
	bool operator==(const FReplicatedChatMessage& Other) const
	{
		return MessageID == Other.MessageID;
	}
};