// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ALobbyGameMode.generated.h"

/// @file ALobbyGameMode.h
/// @brief 로비 맵 전용 게임 모드를 선언합니다.

/// @brief 로비 맵에서 사용되는 게임 모드입니다.
/// @details 플레이어의 입장/퇴장을 처리하고, 세션 준비 상태를 관리합니다.
UCLASS()
class YISAN_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALobbyGameMode();

	// ========================================
	// Overrides
	// ========================================

	/// @brief 플레이어가 로그인할 때 호출됩니다.
	/// @param NewPlayer 새로 로그인한 플레이어 컨트롤러
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/// @brief 플레이어가 로그아웃할 때 호출됩니다.
	/// @param Exiting 로그아웃하는 플레이어 컨트롤러
	virtual void Logout(AController* Exiting) override;

	// ========================================
	// Lobby Settings
	// ========================================

	/// @brief 난입 허용 여부 (항상 true로 설정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Lobby")
	bool bAllowJoinInProgress = true;

	/// @brief 최대 플레이어 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Lobby")
	int32 MaxPlayers = 4;

	// ========================================
	// Lobby State
	// ========================================

	/// @brief 현재 로비에 있는 플레이어 수를 반환합니다.
	/// @return 현재 플레이어 수
	UFUNCTION(BlueprintPure, Category="Lobby")
	int32 GetCurrentPlayerCount() const;

	/// @brief 로비가 가득 찼는지 확인합니다.
	/// @return 로비 풀 여부
	UFUNCTION(BlueprintPure, Category="Lobby")
	bool IsLobbyFull() const;

private:
	/// @brief 현재 로비에 있는 플레이어 수
	int32 CurrentPlayerCount = 0;
};
