// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "UYiSanGameInstance.generated.h"

/// @file YiSanGameInstance.h
/// @brief YiSan 프로젝트의 게임 인스턴스 클래스를 선언합니다.

/// @brief 게임 전체 생명주기를 관리하는 인스턴스 클래스입니다.
/// @details 레벨 로딩, 멀티플레이 세션 생성/참가 기능을 제공합니다.
UCLASS()
class YISAN_API UYiSanGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// ========================================
	// Level Management
	// ========================================

	/// @brief 목표 레벨 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level")
	FName TargetLevel = TEXT("MainLevel_WP");

	/// @brief 레벨 로딩 완료 여부
	UPROPERTY(BlueprintReadOnly, Category="Level")
	bool bLevelReady = false;

	// ========================================
	// Multiplayer Session Management
	// ========================================

	/// @brief 호스트로 게임 세션을 생성합니다.
	/// @param MapName 호스팅할 맵 이름 (예: "MainMap_WP")
	/// @param MaxPlayers 최대 플레이어 수 (기본값: 4)
	/// @return 세션 생성 성공 여부
	UFUNCTION(BlueprintCallable, Category="Multiplayer|Session")
	bool HostGame(const FString& MapName = TEXT("MainLevel_WP"), int32 MaxPlayers = 4);

	/// @brief 기존 게임 세션에 클라이언트로 참가합니다.
	/// @param Address 서버 IP 주소 또는 MagicDNS 도메인 (예: "192.168.0.10", "host-pc")
	/// @param Port 서버 포트 (기본값: 7777)
	/// @return 세션 참가 시도 성공 여부
	UFUNCTION(BlueprintCallable, Category="Multiplayer|Session")
	bool JoinGame(const FString& Address, int32 Port = 7777);

	/// @brief 현재 세션에서 연결을 끊습니다.
	UFUNCTION(BlueprintCallable, Category="Multiplayer|Session")
	void DisconnectFromSession();

	/// @brief 현재 호스트 상태인지 확인합니다.
	/// @return 호스트 여부
	UFUNCTION(BlueprintPure, Category="Multiplayer|Session")
	bool IsHost() const;

	/// @brief 현재 멀티플레이 세션에 연결되어 있는지 확인합니다.
	/// @return 세션 연결 여부
	UFUNCTION(BlueprintPure, Category="Multiplayer|Session")
	bool IsInSession() const;

protected:
	/// @brief 서버 트래블을 수행합니다 (리슨 서버 생성).
	/// @param MapName 이동할 맵 이름
	void ServerTravel(const FString& MapName);

	/// @brief 클라이언트 트래블을 수행합니다 (서버 접속).
	/// @param Address 서버 주소
	void ClientTravel(const FString& Address);

private:
	/// @brief 현재 호스트 여부
	bool bIsHost = false;

	/// @brief 현재 세션에 연결되어 있는지 여부
	bool bIsInSession = false;
};
