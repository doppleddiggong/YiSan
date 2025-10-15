// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

UENUM(BlueprintType)
enum class EVoiceCommandType : uint8
{
	None                     UMETA(DisplayName="None"),

	// 이리오너라
	Cmd_Summon               UMETA(DisplayName="Cmd_Summon"),
	// 문열어라
	Cmd_OpenGate             UMETA(DisplayName="Cmd_OpenGate"),
	// 여기 어디냐
	Cmd_QueryLocation        UMETA(DisplayName="Cmd_QueryLocation"),
	// 목표 알려라
	Cmd_ShowCurrentObjective UMETA(DisplayName="Cmd_ShowCurrentObjective"),
	// 여긴 뭐하는 곳이냐
	Cmd_QueryBuildingInfo    UMETA(DisplayName="Cmd_QueryBuildingInfo"),
};