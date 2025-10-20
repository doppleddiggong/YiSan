// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UMultiplayerSystem.generated.h"

/// @file UMultiplayerSystem.h
/// @brief 멀티플레이 세션 관리 컴포넌트를 선언합니다.

/// @brief 멀티플레이 세션 생성/참가/종료를 담당하는 ActorComponent입니다.
/// @details PlayerController 또는 다른 Actor에 추가하여 세션 관리 기능을 제공합니다.
UCLASS(ClassGroup=(Multiplayer), meta=(BlueprintSpawnableComponent))
class YISAN_API UMultiplayerSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UMultiplayerSystem();

	// ========================================
	// Session Management
	// ========================================

	/// @brief 호스트로 게임 세션을 생성합니다.
	/// @param MapName 호스팅할 맵 이름 (예: "MainLevel_WP")
	/// @param MaxPlayers 최대 플레이어 수 (기본값: 4)
	/// @return 세션 생성 성공 여부
	UFUNCTION(BlueprintCallable, Category="Multiplayer|Session")
	bool HostSession(const FString& MapName = TEXT("MainLevel_WP"), int32 MaxPlayers = 4);

	/// @brief 기존 게임 세션에 클라이언트로 참가합니다.
	/// @param Address 서버 IP 주소 또는 MagicDNS 도메인 (예: "192.168.0.10", "host-pc")
	/// @param Port 서버 포트 (기본값: 7777)
	/// @return 세션 참가 시도 성공 여부
	UFUNCTION(BlueprintCallable, Category="Multiplayer|Session")
	bool JoinSession(const FString& Address, int32 Port = 7777);

	/// @brief 현재 세션에서 연결을 끊고 로비로 돌아갑니다.
	UFUNCTION(BlueprintCallable, Category="Multiplayer|Session")
	void DisconnectSession();

	/// @brief 현재 호스트 상태인지 확인합니다.
	/// @return 호스트 여부
	UFUNCTION(BlueprintPure, Category="Multiplayer|Session")
	bool IsHost() const;

	/// @brief 현재 멀티플레이 세션에 연결되어 있는지 확인합니다.
	/// @return 세션 연결 여부
	UFUNCTION(BlueprintPure, Category="Multiplayer|Session")
	bool IsInSession() const;

protected:
	virtual void BeginPlay() override;

private:
	/// @brief GameInstance를 가져옵니다.
	class UYiSanGameInstance* GetYiSanGameInstance() const;

	/// @brief 세션 상태 캐싱 (로컬)
	bool bIsHost = false;
	bool bIsInSession = false;
};
