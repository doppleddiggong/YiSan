// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

namespace GameLayer
{
    static const int32 Loading = 400;
    static const int32 LoadingCircle = 500;
    static const int32 ToastManager = 600;
}


/// @file YiSan.h
/// @brief 게임 전역에서 공유하는 GameplayTag 상수를 정의합니다.
namespace GameTags
{
    /// @brief 플레이어 액터를 식별하기 위한 태그입니다.
    static const FName Player = TEXT("Player");

    /// @brief 문 및 문짝과 관련된 액터에 부여되는 태그입니다.
    static const FName Door = TEXT("Door");

    /// @brief 건물 액터를 구분할 때 사용하는 태그입니다.
    static const FName Building = TEXT("Building");

    static const FName Dasan = TEXT("Dasan");
}


namespace GameLevel
{
    static const FString StartLevel = TEXT("StartLevel");
    static const FString LobbyMap   = TEXT("LobbyMap");
	static const FString LoadingMap = TEXT("LoadingMap");
    static const FString MainMap_WP = TEXT("MainMap_WP");
}


namespace GameString
{
    static const FString NPC = TEXT("정약용");
    static const FString System = TEXT("SYSTEM");
    static const FString Default = TEXT("이산");    
}

