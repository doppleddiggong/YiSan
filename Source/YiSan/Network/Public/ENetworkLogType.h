// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/// @file ENetworkLogType.h
/// @brief 네트워크 로그 메시지 구분자 열거형을 선언합니다.
#pragma once

#include "CoreMinimal.h"

/// @brief 네트워크 요청/응답 로그를 구분하는 타입입니다.
enum class ENetworkLogType : uint8
{
    Get,  ///< HTTP GET 요청을 나타냅니다.
    Post, ///< HTTP POST 요청을 나타냅니다.
    WS,   ///< WebSocket 메시지를 나타냅니다.
};

