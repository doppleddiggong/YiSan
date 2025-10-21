// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UChatPlayerSystem.generated.h"

UCLASS(ClassGroup=(Chat), meta=(BlueprintSpawnableComponent))
class YISAN_API UChatPlayerSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UChatPlayerSystem();

	void InitSystem(class UChatBoxWidget* InChatBox);

	/** 채팅 입력 처리 */
	void OnEnterPressed();
	void OnScrollUp();
	void OnScrollDown();

public:
	/** 서버 RPC: 메시지 전송 */
	UFUNCTION(Server, Reliable)
	void SendChatMessage(const FString& Message);

	/** 클라 RPC: 자기 자신만 갱신 */
	UFUNCTION(Client, Reliable)
	void AddChatMessageOnOwningClient(const FString& Message);

	/** 멀티캐스트 RPC: 전체 클라 갱신 */
	UFUNCTION(NetMulticast, Reliable)
	void AddChatMessageOnAllClients(const FString& Message);

private:
	UPROPERTY()
	TObjectPtr<UChatBoxWidget> ChatBoxRef;
};
