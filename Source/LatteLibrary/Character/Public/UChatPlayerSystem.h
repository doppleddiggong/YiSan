// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "APlayerControl.h"
#include "UChatPlayerSystem.generated.h"

class UChatUIWidget;
class UChatBoxWidget;

UCLASS(ClassGroup=(Chat), meta=(BlueprintSpawnableComponent))
class LATTELIBRARY_API UChatPlayerSystem : public UActorComponent
{				
	GENERATED_BODY()

public:
	UChatPlayerSystem();

	void InitSytstem(APlayerControl* PC);
	
protected:
	/** 로컬 전용: 채팅창 포커스 / 스크롤 처리 */
	void OnEnterPressed();
	void OnMouseWheelUp();
	void OnMouseWheelDown();

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

	
public:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UChatUIWidget> ChatUIClass;

	UPROPERTY()
	TObjectPtr<UChatUIWidget> ChatUIRef;

	UPROPERTY()
	TObjectPtr<UChatBoxWidget> ChatBoxRef;
};
