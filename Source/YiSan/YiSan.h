// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * @brief YiSan 모듈 전역에서 사용하는 기본 로그 카테고리.
 *
 * 모듈 초기화 시 정의되며 게임 시스템 전반에서 공통적으로 사용한다.
 */
DECLARE_LOG_CATEGORY_EXTERN(LogYiSan, Log, All);


/**
 * @defgroup CoreLibs Core Libraries
 * @brief 게임플레이 프레임워크의 기반이 되는 핵심 라이브러리 모음.
 *
 * 공용 유틸리티, 데이터 처리, 네트워크, 그리고 이동/전투와 같은 핵심 로직을 제공합니다.
 */

/**
 * @defgroup GameFramework Game Framework
 * @brief 게임의 기본 구조와 규칙을 정의하는 클래스 모음.
 *
 * 게임 모드, 플레이어 컨트롤러, 월드 설정 등을 포함합니다.
 */

/**
 * @defgroup Character Character & Combat
 * @brief 플레이어와 NPC를 포함한 모든 캐릭터의 속성, 행동, 전투 시스템.
 */

/**
 * @defgroup AI Artificial Intelligence
 * @brief NPC의 의사결정과 행동을 제어하는 시스템.
 *
 * 행동 트리, EQS, 군중 제어 등을 포함합니다.
 */

/**
 * @defgroup Interaction Interaction Systems
 * @brief 캐릭터와 월드 간의 상호작용을 처리하는 시스템.
 *
 * 승마, 음성 인식 등의 기능을 포함합니다.
 */