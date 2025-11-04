// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

/**
 * @file EVoiceCommandType.h
 * @brief EVoiceCommandType 클래스를 선언합니다.
 */

UENUM(BlueprintType)
enum class EVoiceCommandType : uint8
{
	None                     UMETA(DisplayName="None"),

	// 접근 (다가가기, NPC에게로, 가까이가기)
	Cmd_Approach             UMETA(DisplayName="Cmd_Approach"),
	// 건물정보확인 (주변정보, 이곳정보, 건물설명)
	Cmd_BuildingInfo         UMETA(DisplayName="Cmd_BuildingInfo"),
	// 목표로이동 (다음목적지, 퀘스트위치, 목표로출발)
	Cmd_Target               UMETA(DisplayName="Cmd_Target"),
	// 친구에게로 (동료위치로, 파티쪽으로, 팀원한테가)
	Cmd_Friend               UMETA(DisplayName="Cmd_Friend"),
	// 이쪽으로와 (근처로와, 여기로와, 따라와, 여기집결)
	Cmd_Call                 UMETA(DisplayName="Cmd_Call"),
};