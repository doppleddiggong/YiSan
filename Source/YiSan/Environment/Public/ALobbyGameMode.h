// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ALobbyGameMode.generated.h"

/// @file ALobbyGameMode.h
/// @brief 로비 맵 전용 게임 모드를 선언합니다.

/// @brief 로비 맵에서 사용되는 게임 모드입니다.
/// @details 플레이어의 입장/퇴장을 처리하고, 세션 준비 상태를 관리합니다.
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerLoggedIn, APlayerController*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerLoggedOut, AController*);

UCLASS()
class YISAN_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALobbyGameMode();
	virtual void BeginPlay() override;

	// ========================================
	// Overrides
	// ========================================

	/// @brief 플레이어가 로그인할 때 호출됩니다.
	/// @param NewPlayer 새로 로그인한 플레이어 컨트롤러
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/// @brief 플레이어가 로그아웃할 때 호출됩니다.
	/// @param Exiting 로그아웃하는 플레이어 컨트롤러
	virtual void Logout(AController* Exiting) override;
};
